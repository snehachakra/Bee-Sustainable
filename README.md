# Bee Sustainable

## Technical Overview
This project implements a bee-themed Pacman game using C++ and SFML with Arduino hardware integration. The game runs at 60fps with sprite-based rendering, custom animation states, and serial communication to a physical controller.

## Software Architecture
The game's architecture consists of several key classes and structures as shown in the class diagram:

### Core Classes
- **GameLogger** - Handles logging and debug information
- **SerialController** - Manages communication with Arduino hardware
- **TriviaManager** - Controls the educational trivia system

### Game State Structures
- **GameState** - Main structure that holds the current game state
- **Animation** - Controls animation states and timing
- **Hornet** - Represents enemy entities and their behaviors
- **Player** - Represents the bee character controlled by the user
- **RenderItems** - Manages visual elements and sprites
- **Sounds** - Handles audio resources
- **SoundState** - Tracks the current audio playback state
- **Textures** - Manages image resources
- **TriviaQuestion** - Represents individual educational questions

### Enumerations
- **BkState** - Background music state (NO_SOUND, SIREN, RETREAT, PPELLET)
- **Dir** - Direction enumeration (UP, DOWN, LEFT, RIGHT, NONE)
- **GhostType** - Hornet types (RED, PINK, BLUE, ORANGE)
- **State** - Game states (MENU, GAMESTART, MAINLOOP, TRIVIA_MODE, etc.)
- **TargetState** - Hornet AI states (CHASE, FRIGHTENED, CORNER, etc.)

The architecture follows a component-based design where each class handles a specific aspect of the game, with the GameState structure serving as the central data repository.

## Build Requirements
- C++17 compiler (Visual Studio 2019/2022 recommended)
- SFML 2.5.1 library
- Windows 10/11 (for COM port access)
- Arduino IDE 1.8.13 or later
- CMake 3.15+ (optional)

## Complete Project Structure
```
project/
├── audio/                              # Sound files
├── CombinedSteeringAnswering/          # Arduino controller code folder
│   └── CombinedSteeringAnswering.ino   # Arduino controller code
├── textures/                           # Texture files
├── Animate.cpp                         # Animation system implementation
├── Animate.h                           # Animation system header
├── Buzzy.cpp                           # Main game implementation
├── Buzzy.h                             # Main game header
├── game_log                            # Game log file
├── GameLogger.cpp                      # Logging system implementation
├── GameLogger.h                        # Logging system header
├── Gameloop.cpp                        # Game loop implementation
├── Gameloop.h                          # Game loop header
├── highscore                           # Highscore file
├── Hornets.cpp                         # Enemy implementation
├── Hornets.h                           # Enemy header
├── main.cpp                            # Main entry point
├── Map                                 # Map data file
├── Player.cpp                          # Player implementation
├── Player.h                            # Player header
├── Render.cpp                          # Rendering system implementation
├── Render.h                            # Rendering system header
├── SerialController.cpp                # Arduino communication
├── SerialController.h                  # Arduino communication header
├── Sound.cpp                           # Sound system implementation
├── Sound.h                             # Sound system header
├── Trivia.cpp                          # Trivia system implementation
└── Trivia.h                            # Trivia system header
```

## Build Configuration
### Visual Studio
1. Create a new C++ project
2. Add all source files to the project
3. Configure project properties:
   - C/C++ → General → Additional Include Directories: Add path to SFML include directory
   - Linker → General → Additional Library Directories: Add path to SFML lib directory
   - Linker → Input → Additional Dependencies: Add the following:
     ```
     sfml-graphics-d.lib
     sfml-window-d.lib
     sfml-system-d.lib
     sfml-audio-d.lib
     ```
     (Use non-d versions for Release build)
4. Place SFML DLLs in project output directory

## Arduino Hardware
### Components
- Arduino Uno Mini
- Joystick module
- Button
- LED strip
- Jumper wires

### Circuit Connections
```
Joystick:
- LEFT pin   → Arduino pin 7
- RIGHT pin  → Arduino pin 12
- UP pin     → Arduino pin 8
- DOWN pin   → Arduino pin 4

Button:
- Input      → Arduino pin 9
- Light      → Arduino pin 3

LED Strip:
- Data pin   → Arduino pin 6
- VCC        → Arduino 5V
- GND        → Arduino GND
```

The Arduino is connected to the PC via USB, which provides both power and serial communication.

## Data Flow Between Components

The SerialController class communicates with the Arduino hardware, processing inputs from the joystick and button and sending commands to control the LED strip. This data is then passed to the GameState structure, which holds the current state of the game.

The Hornets and Player structures manage the game entities, while the Animation structure controls their visual appearance. The RenderItems and Textures structures handle the rendering of these entities to the screen.

The TriviaManager class manages the educational content, selecting questions from the pool and verifying answers. The Sounds and SoundState structures handle audio feedback based on game events.

All of these components communicate through the central GameState structure, which serves as the hub for game information.

## Texture Files
The game uses several texture files for its visual elements:

- **buzzy.png**: The main character sprite (bee)
- **buzzy_and_friends.png**: Character with friends for menu screens
- **dots.png**: Nectar dots that the player collects
- **flower.png**: Flower power-up that triggers trivia questions
- **font.png**: Font sprite sheet for text rendering
- **map.png**: Main game map texture
- **map_white.png**: Alternate map texture for special states
- **poweredbees.png**: Special sprites for powered-up bee state
- **sprites.png**: Collection of game sprites including ghosts

## Audio Files
The game uses a variety of audio files for different game events:

- **answer_correct.wav**: Plays when trivia answer is correct
- **answer_incorrect.wav**: Plays when trivia answer is wrong
- **button_press.wav**: Plays when a button is pressed
- **death_1.wav**, **death_2.wav**: Two-part death sound
- **eat_ghost.wav**: Plays when eating a ghost during powered-up state
- **game_start.wav**: Plays at the beginning of the game
- **munch_1.wav** through **munch_5.wav**: Various nectar eating sounds
- **power_pellet.wav**: Plays during powered-up state
- **retreating.wav**: Plays when ghosts are retreating
- **siren_3.wav**, **siren_4.wav**: Background ambience sounds

## Serial Communication Protocol
The game communicates with Arduino through serial port at 9600 baud. If you need to change the port, modify:
```cpp
// In SerialController.cpp
const char* portName = "COM3"; // Change to match your Arduino's port
```

### Protocol Format
- Commands from PC to Arduino: `COMMAND:VALUE\n`
- Messages from Arduino to PC: Formatted strings with joystick and button states

### Example Commands
```
MODE:STEERING    // Set controller to movement mode
MODE:ANSWERING   // Set controller to trivia mode
GAME:RESET       // Reset Arduino state
5                // Reset all LEDs (flower count = 5)
1-4              // Set flower count/light up LEDs
```

## Key Game Parameters
These constants can be adjusted to modify game behavior:

```cpp
// Global game parameters in GameTypes.h
constexpr float player_speed = 0.1f;     // Bee movement speed
constexpr float ghost_speed = 0.07f;     // Hornet regular speed
constexpr float ghost_fright = 0.04f;    // Hornet frightened speed
constexpr float inhome_speed = 0.03f;    // Hornet spawn area speed
constexpr float gohome_speed = 0.15f;    // Hornet return to spawn speed
constexpr int fright_time = 7;           // Power-up duration in seconds
```

## State Machine Implementation
The game uses a state machine pattern implemented through the State enumeration:
- MENU: Title screen and start menu
- GAMESTART: Initial "Ready!" screen
- MAINLOOP: Active gameplay
- TRIVIA_MODE: Educational question screen
- GAMELOSE: Player death animation and life reduction
- GAMEWIN: Level completion celebration
- GAMEOVER: Game over screen

## Trivia System
To add new questions, add entries to the `bee_questions` vector in `TriviaManager::InitializeQuestions()`:

```cpp
bee_questions = {
    {
        "Question text goes here?",
        {"Answer 1", "Answer 2", "Answer 3", "Answer 4"},
        correct_index,  // Index of correct answer (0-3)
        points_reward   // Points awarded for correct answer
    },
    // Add more questions here
};
```

The game includes 18 trivia questions about bees, pollination, and environmental challenges. When players collect special items (flowers), the game transitions to a trivia mode where:
1. The game presents a randomly selected question
2. Players use the joystick to select from multiple-choice answers
3. Correct answers grant power-up abilities and points
4. LED indicators on the physical controller track flower collection progress

## Animation System
The powered-up bee animation uses a separate sprite sheet with the same layout as the main sprites. The animation is controlled by toggling textures based on the energizer timer:

```cpp
// In DrawFrame function (Render.cpp)
bool isPowered = (gState.energizer_time > 0 && !animate.death_animation);
if (isPowered) {
    RItems.player.setTexture(RTextures.powered_pacman);
    RItems.player.setTextureRect(GetPoweredPacManFrame(gState.player->cur_dir));
} else {
    RItems.player.setTexture(RTextures.sprites);
    RItems.player.setTextureRect(GetPacManFrame(gState.player->cur_dir));
}
```

## Game Controls
- Joystick: Move bee character through the maze
  - UP/DOWN/LEFT/RIGHT inputs control character movement
- Button: Select options in menus and answer trivia questions
- During trivia mode:
  - Joystick UP/DOWN: Navigate answer options
  - Button: Select answer

## Map Format
The game level is defined in `Map.txt` with these characters:
- `.` : Nectar dot
- `o` : Flower (power pellet)
- `|` : Wall
- `-` : Ghost house wall
- ` ` : Empty space

## Troubleshooting
- If Arduino connection fails, check COM port number and update in SerialController.cpp
- If sprites don't appear, verify texture paths and check textures folder
- If game performance is poor, reduce window size or animation complexity
- For Arduino communication issues, ensure baud rate is set to 9600 in both Arduino sketch and SerialController.cpp
- If LED strip doesn't light up, check the data pin connection (pin 6) and power supply
- If SFML-related errors occur, ensure all DLL files are in the correct location
- Game logs are stored in the "game_log" file for debugging purposes
