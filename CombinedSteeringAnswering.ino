#include <Adafruit_NeoPixel.h>

// LED strip configuration
#define LED_PIN 6  // LED strip pin
#define LED_COUNT 30
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// Joystick and button pin definitions
const int Lpin = 7;  // Left direction pin
const int Upin = 8;  // Up direction pin
const int Dpin = 4;  // Down direction pin
const int Rpin = 12; // Right direction pin
const int button = 9; // Button pin
const int buttonLED = 3; // LED pin

// Operational mode
bool answeringMode = false; // false = steering mode, true = answering mode

// Button and joystick states
int L, U, R, D;
int buttonState;
int lastButtonState = HIGH;
int lastU = HIGH;
int lastD = HIGH;

// Game state
int direction = 0;
int answer = 1; // default first answer (1-4)
bool gameStarted = false;

// Timing variables
unsigned long lastUpdateTime = 0;
const int updateInterval = 50; // Send updates every 50ms

unsigned long buttonPressStartTime = 0;
const unsigned long BUTTON_RESET_TIME = 5000; // 5 seconds in milliseconds
bool buttonHeldDown = false;

// Debounce timing
unsigned long lastDebounceTime = 0;
unsigned long lastButtonDebounceTime = 0;
const int debounceDelay = 150; // Milliseconds to wait for debounce
const int buttonDebounceDelay = 300; // Longer debounce for button

// Serial command processing
String inputString = "";
bool stringComplete = false;

int count = 0;
bool buttonStateSent = false; // Flag to ensure we only send button state once per press

void setup() {
  // Configure pins for joystick and button
  pinMode(Lpin, INPUT_PULLUP);
  pinMode(Upin, INPUT_PULLUP);
  pinMode(Rpin, INPUT_PULLUP);
  pinMode(Dpin, INPUT_PULLUP);
  pinMode(button, INPUT_PULLUP);
  pinMode(buttonLED, OUTPUT);

  // Initialize LED strip
  strip.begin();
  strip.show();  // Initialize all pixels to 'off'
  
  // Test LED strip startup sequence
  for (int i = 0; i < 3; i++) {
    // Flash different colors to test the strip
    setAllPixels(strip.Color(50, 0, 0)); // Red
    strip.show();
    delay(100);
    setAllPixels(strip.Color(0, 50, 0)); // Green
    strip.show();
    delay(100);
    setAllPixels(strip.Color(0, 0, 50)); // Blue
    strip.show();
    delay(100);
  }
  setAllPixels(strip.Color(0, 0, 0)); // Turn off all LEDs
  strip.show();

  // Start serial communication
  Serial.begin(9600);
  
  // Flash LED to indicate startup
  digitalWrite(buttonLED, HIGH);
  delay(200);
  digitalWrite(buttonLED, LOW);
  delay(200);
  digitalWrite(buttonLED, HIGH);
  delay(200);
  digitalWrite(buttonLED, LOW);
  
  // Initialize empty input string
  inputString.reserve(50);
  
  // Default to steering mode
  answeringMode = false;
  Serial.println("READY:STEERING");
  Serial.println("Press button to start game");
}

void loop() {
  // Check for commands and data from the game
  checkSerialData();
  
  // Read all inputs at the beginning of each loop
  L = digitalRead(Lpin);
  U = digitalRead(Upin);
  R = digitalRead(Rpin);
  D = digitalRead(Dpin);
  buttonState = digitalRead(button);
  
  // Current time
  unsigned long currentTime = millis();
  
  // Process button state changes with proper debouncing
  if (buttonState == LOW && lastButtonState == HIGH && 
      (currentTime - lastButtonDebounceTime > buttonDebounceDelay)) {
    // Button was just pressed
    lastButtonState = LOW;
    lastButtonDebounceTime = currentTime;
    buttonPressStartTime = currentTime;
    buttonStateSent = false; // Reset this flag on new press
    
    // Visual feedback
    digitalWrite(buttonLED, HIGH);
    
    // Handle immediate button press actions
    if (!gameStarted && count == 4) {
      // Game start button press
      gameStarted = true;
      Serial.println("NextScreen (GameStart)");
      Serial.println("Button"); // Also send "Button" keyword for more reliable detection
    } 
    else if (answeringMode) {
      // Answering mode button press
      Serial.print("Button pressed in answering mode: ");
      Serial.println(answer);
      
      // Send multiple forms for redundancy
      Serial.println("Button");
      Serial.print("Selected: ");
      Serial.println(answer);
      Serial.println("Answering");
      
      buttonStateSent = true;
    }
    else {
      // Regular button press (for navigating instruction screens)
      Serial.println("NextScreen");
      Serial.println("Button"); // Also send "Button" for more reliable detection
      count++;
    }
  }
  // Button is held down - check for reset
  else if (buttonState == LOW && lastButtonState == LOW) {
    // Keep LED on while button is held
    digitalWrite(buttonLED, HIGH);
    
    // Check if button has been held for the reset time (5 seconds)
    if ((currentTime - buttonPressStartTime >= BUTTON_RESET_TIME) && !buttonHeldDown) {
      // Button has been held for 5 seconds - trigger game reset
      buttonHeldDown = true; // Flag to prevent multiple resets while button is held
      
      // Visual feedback - rapid flashing of LED
      for (int i = 0; i < 5; i++) {
        digitalWrite(buttonLED, HIGH);
        delay(50);
        digitalWrite(buttonLED, LOW);
        delay(50);
      }
      
      // Send reset command to the game
      Serial.println("RESET_GAME");
      
      // Reset game state variables
      gameStarted = false;
      answeringMode = false;
      count = 0;
    }
    
    // In answering mode, repeatedly send button state while held
    if (answeringMode && !buttonStateSent && 
        (currentTime - buttonPressStartTime > 150) && 
        (currentTime - buttonPressStartTime < 800)) {
      // Send multiple forms for redundancy
      Serial.println("Button");
      Serial.print("Button pressed in answering mode: ");
      Serial.println(answer);
      buttonStateSent = true; // Set flag to avoid flooding
    }
  } 
  // Button released
  else if (buttonState == HIGH && lastButtonState == LOW) {
    // Button was just released
    lastButtonState = HIGH;
    buttonHeldDown = false; // Reset the held flag when button is released
    digitalWrite(buttonLED, LOW); // Turn off LED
  }
  
  // Handle input based on current mode
  if (answeringMode) {
    // ANSWERING MODE
    
    // Handle Down press to cycle through answers - with better debouncing
    if (D == LOW && lastD == HIGH && (currentTime - lastDebounceTime > debounceDelay)) {
      lastDebounceTime = currentTime; // Reset debounce timer
      if (answer < 4) {
        answer++;
      } else {
        answer = 1;
      }
      Serial.print("Selected: ");
      Serial.println(answer);
      lastD = LOW;
      
      // Flash LED briefly to confirm
      digitalWrite(buttonLED, HIGH);
      delay(50);
      digitalWrite(buttonLED, LOW);
    } 
    else if (D == HIGH && lastD == LOW) {
      lastD = HIGH;
    }
    
    // Handle Up press to cycle answers backwards - with better debouncing
    if (U == LOW && lastU == HIGH && (currentTime - lastDebounceTime > debounceDelay)) {
      lastDebounceTime = currentTime; // Reset debounce timer
      if (answer > 1) {
        answer--;
      } else {
        answer = 4;
      }
      Serial.print("Selected: ");
      Serial.println(answer);
      lastU = LOW;
      
      // Flash LED briefly to confirm
      digitalWrite(buttonLED, HIGH);
      delay(50);
      digitalWrite(buttonLED, LOW);
    } 
    else if (U == HIGH && lastU == LOW) {
      lastU = HIGH;
    }
    
    // Send periodic updates of current selection
    if (currentTime - lastUpdateTime >= updateInterval) {
      // Repeatedly send the current answer selection to ensure it's received
      Serial.print("Selected: ");
      Serial.println(answer);
      lastUpdateTime = currentTime;
    }
    
  } else {
    // STEERING MODE
    
    // Only process movement if game has started
    if (gameStarted) {
      // Determine current direction
      int newDirection = 0;
      if (L == LOW) {
        newDirection = 4; // Left
      }
      else if (U == LOW) {
        newDirection = 1; // Up
      }
      else if (R == LOW) {
        newDirection = 2; // Right
      }
      else if (D == LOW) {
        newDirection = 3; // Down
      }
      
      // Send direction on change immediately
      if (newDirection != direction) {
        direction = newDirection;
        
        // Only send if a direction is active
        if (direction != 0) {
          switch (direction) {
            case 1:
              Serial.println("Up");
              break;
            case 2:
              Serial.println("Right");
              break;
            case 3:
              Serial.println("Down");
              break;
            case 4:
              Serial.println("Left");
              break;
          }
        }
      }
      
      // Send periodic updates while direction is held
      if (direction != 0 && currentTime - lastUpdateTime >= updateInterval) {
        switch (direction) {
          case 1:
            Serial.println("Up");
            break;
          case 2:
            Serial.println("Right");
            break;
          case 3:
            Serial.println("Down");
            break;
          case 4:
            Serial.println("Left");
            break;
        }
        
        lastUpdateTime = currentTime;
      }
    }
  }
  
  // Small delay to avoid serial flooding
  delay(10);
}

// Process serial input for commands and data from the game
void checkSerialData() {
  while (Serial.available()) {
    // Try to parse a flower number first
    if (Serial.peek() >= '0' && Serial.peek() <= '9') {
      int flowerNum = Serial.parseInt();
      if (flowerNum >= 1 && flowerNum <= 5) {
        setColorBasedOnFlower(flowerNum);
        return;
      }
    }
    
    // If not a number, read as a command
    char inChar = (char)Serial.read();
    
    // Add to the input string if not a newline
    if (inChar != '\n') {
      inputString += inChar;
    }
    // If the incoming character is a newline, set a flag
    else {
      stringComplete = true;
    }
    
    // Process the command if complete
    if (stringComplete) {
      // Check for mode switching commands
      if (inputString.indexOf("MODE:ANSWERING") >= 0) {
        setAnsweringMode();
      }
      else if (inputString.indexOf("MODE:STEERING") >= 0) {
        setSteeringMode();
      }
      else if (inputString.indexOf("GAME:RESET") >= 0) {
        resetGame();
      }
      
      // Clear the string for the next command
      inputString = "";
      stringComplete = false;
    }
  }
}

// Set colors on LED strip based on flower number
void setColorBasedOnFlower(int flowerNum) {
  //green,red, purple, blue
  uint32_t color;
  
  if (flowerNum == 1) {
    color = strip.Color(0, 128, 0);  //green
    for (int i = 1; i < 4; i++) {
      strip.setPixelColor(i, color);
    }
  } else if (flowerNum == 2) {
    color = strip.Color(160, 32, 240);  //purple
    for (int i = 5; i < 8; i++) {
      strip.setPixelColor(i, color);
    }
  } else if (flowerNum == 3) {
    color = strip.Color(255, 0, 0);  //red
    for (int i = 9; i < 12; i++) {
      strip.setPixelColor(i, color);
    }
  } else if (flowerNum == 4) {
    color = strip.Color(0, 0, 255);  //blue
    for (int i = 13; i < 16; i++) {
      strip.setPixelColor(i, color);
    }
  } else { //turns off strip when end game
    for (int i = 0; i < LED_COUNT; i++) {
      strip.setPixelColor(i, strip.Color(0, 0, 0));  
    }
  }
  
  strip.show();
  
  // Acknowledge receipt of flower command
  Serial.print("Set flower: ");
  Serial.println(flowerNum);
}

// Set all pixels to a specific color
void setAllPixels(uint32_t color) {
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, color);
  }
}

// Switch to answering mode
void setAnsweringMode() {
  if (!answeringMode) {
    answeringMode = true;
    
    // Reset answer to 1 when entering answering mode
    answer = 1;
    
    // Visual feedback (flash LED)
    flashLED(3);
    
    // Inform the game
    Serial.println("READY:ANSWERING");
    
    // Ensure initial selection is sent multiple times
    Serial.print("Selected: ");
    Serial.println(answer);
    delay(50);
    Serial.print("Selected: ");
    Serial.println(answer);
    
    // Reset button state
    lastButtonState = HIGH;
    buttonStateSent = false;
  }
}

// Switch to steering mode
void setSteeringMode() {
  if (answeringMode) {
    answeringMode = false;
    
    // Visual feedback (flash LED)
    flashLED(2);
    
    // Inform the game
    Serial.println("READY:STEERING");
    
    // Reset states
    lastButtonState = HIGH;
    buttonStateSent = false;
  }
}

// Reset the game state
void resetGame() {
  gameStarted = false;
  direction = 0;
  answeringMode = false;
  answer = 1;
  
  // Turn off all LEDs on the strip
  setAllPixels(strip.Color(0, 0, 0));
  strip.show();
  
  // Visual feedback
  flashLED(5);
  
  // Inform the game
  Serial.println("READY:STEERING");
  Serial.println("Press button to start game");
  
  // Reset states
  lastButtonState = HIGH;
  buttonStateSent = false;
}

// Flash the LED n times
void flashLED(int n) {
  for (int i = 0; i < n; i++) {
    digitalWrite(buttonLED, HIGH);
    delay(100);
    digitalWrite(buttonLED, LOW);
    delay(100);
  }
}