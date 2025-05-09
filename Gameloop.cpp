#include "Gameloop.h"
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <iostream>
#include "SerialController.h"
#include <chrono>
#include <thread>
#include "GameLogger.h"

GameState gState;

SerialController gSerialController;

bool DebugArduinoCommunication(const char* portName, int durationSeconds) {
	std::cout << "\n=== ARDUINO DEBUG MODE ACTIVATED ===\n";
	std::cout << "Listening to Arduino on " << portName << " for " << durationSeconds << " seconds\n";
	std::cout << "Move joystick and press button to see raw output\n";

	// Open serial connection
	std::string fullPortName = std::string("\\\\.\\") + portName;
	HANDLE hSerial = CreateFileA(fullPortName.c_str(),
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (hSerial == INVALID_HANDLE_VALUE) {
		std::cerr << "Failed to open port for debugging\n";
		return false;
	}

	// Set up port parameters
	DCB dcbSerialParams = { 0 };
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
	GetCommState(hSerial, &dcbSerialParams);
	dcbSerialParams.BaudRate = CBR_9600;
	dcbSerialParams.ByteSize = 8;
	dcbSerialParams.StopBits = ONESTOPBIT;
	dcbSerialParams.Parity = NOPARITY;
	SetCommState(hSerial, &dcbSerialParams);

	// Set timeouts
	COMMTIMEOUTS timeouts = { 0 };
	timeouts.ReadIntervalTimeout = MAXDWORD;
	timeouts.ReadTotalTimeoutMultiplier = 0;
	timeouts.ReadTotalTimeoutConstant = 0;
	SetCommTimeouts(hSerial, &timeouts);

	// Clear buffer
	PurgeComm(hSerial, PURGE_RXCLEAR | PURGE_TXCLEAR);

	// Listen for data
	auto startTime = std::chrono::steady_clock::now();
	auto endTime = startTime + std::chrono::seconds(durationSeconds);

	while (std::chrono::steady_clock::now() < endTime) {
		char buffer[256] = { 0 };
		DWORD bytesRead = 0;

		if (ReadFile(hSerial, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
			buffer[bytesRead] = '\0';
			std::string input(buffer);

			if (!input.empty()) {
				std::cout << "RAW DATA: [" << input << "]\n";

				// Print each character code for detailed debugging
				std::cout << "CHAR CODES: ";
				for (size_t i = 0; i < input.length(); i++) {
					std::cout << static_cast<int>(input[i]) << " ";
				}
				std::cout << std::endl;
			}
		}

		// Brief pause to avoid CPU spinning
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	CloseHandle(hSerial);
	std::cout << "=== DEBUG MODE COMPLETED ===\n";
	return true;
}

bool InitializeSerialController() {
	// Debug mode flag - set to true to activate debug mode
	bool debugMode = false;

	if (debugMode) {
		// Listen for 30 seconds in debug mode
		DebugArduinoCommunication("COM3", 30);
	}

	// Try to connect to Arduino
	const char* portName = "COM3";
	if (gSerialController.initialize(portName)) {
		std::cout << "Arduino controller connected successfully!" << std::endl;
		return true;
	}

	std::cout << "Failed to connect to Arduino controller. Using keyboard fallback." << std::endl;
	return false;
}

void OnStart()
{
	// Initialize existing game components
	Init();
	LoadHighScore();

	// Initialize the game logger
	gGameLogger.initialize("game_log.txt");

	// Initialize serial controller for Arduino inputs
	if (InitializeSerialController()) {
		std::cout << "Arduino controller connected successfully!" << std::endl;

		// Reset the Arduino to ensure it's in the correct initial state
		gSerialController.resetGame();
	}
	else {
		std::cout << "Failed to connect to Arduino controller. Using keyboard fallback." << std::endl;
	}
}

void OnQuit()
{
	SaveHighScore();

	// Disconnect from Arduino if connected
	if (gSerialController.isConnected()) {
		gSerialController.disconnect();
	}
}

void LoadHighScore()
{
	std::ifstream infile;
	std::stringstream ss;
	std::string line;
	int hs = 0;
	infile.open("highscore.txt");
	if (!infile) {
		gState.high_score = 0;
		return;
	}
	getline(infile, line);
	ss << line;
	ss >> hs;

	gState.high_score = hs;
}
void SaveHighScore()
{
	std::ofstream outfile("highscore.txt");
	if (!outfile.is_open())
		printf("Cant open file!");

	outfile << gState.high_score;
	outfile.close();
}
void InitBoard()
{
	std::string line;
	std::ifstream infile("Map.txt");
	if (!infile)
		return;
	while (getline(infile, line))
	{
		gState.board.push_back(line);
	}

	infile.close();

}
void Init()
{
	Player* pl = new Player();
	pl->cur_dir = UP;
	pl->pos = { 14,23.5 };
	pl->stopped = true;
	gState.player = pl;

	// Ghosts init
	Ghost* temp = new Ghost();
	temp->type = RED;
	gState.ghosts.push_back(temp);

	temp = new Ghost();
	temp->type = PINK;
	gState.ghosts.push_back(temp);

	temp = new Ghost();
	temp->type = BLUE;
	gState.ghosts.push_back(temp);

	temp = new Ghost();
	temp->type = ORANGE;
	gState.ghosts.push_back(temp);

	InitBoard();
	InitRender();
	ResetGhostsAndPlayer();

	InitSounds();

	// Initialize trivia-related variables
	gState.selected_trivia_answer = 0;
	gState.last_answer_was_correct = false;
	gState.current_explanation = "";

	SetupMenu();
	gState.game_state = MENU;
	gState.pause_time = 2000;

	gState.button_released = true;
	gState.lastButtonPressTime = 0;
	gState.lastButtonMessage = "";
	gState.buttonProcessed = false;
	gState.canAdvanceScreen = false;
	gState.screenChangeTime = 0;

	SetupMenu(); // Will now redirect to instruction screens
	gState.game_state = INSTR_SCREEN1;
	gState.pause_time = 2000;

	gState.can_interact_with_flower = true;
	gState.flower_interaction_cooldown = 0.0f;
}

void AnswerTriviaQuestion(int selected_index)
{
	bool correct = gTriviaManager.CheckAnswer(gState.current_trivia_question, selected_index);
	gState.last_answer_was_correct = correct;

	// Get the explanation based on the selected answer
	gState.current_explanation = gTriviaManager.GetExplanation(gState.current_trivia_question, selected_index);

	// Log the trivia answer
	gGameLogger.logTriviaAnswer(gState.current_trivia_question.question, selected_index, correct);

	// Start timing the explanation screen
	gGameLogger.startExplanationTimer();

	if (correct) {
		PlayCorrectAnswerSound();
		// Add points and activate power-up only if answer is correct
		gState.game_score += gState.current_trivia_question.points_reward;
		gState.energizer_time = fright_time * 1000;
		SetAllGhostState(FRIGHTENED);
		gState.ghosts_eaten_in_powerup = 0; // Reset ghost eaten counter for new power session

		// Increment flowers collected only when correct (cycle between 1-4)
		gState.flowersCollected = (gState.flowersCollected % 4) + 1;

		// Update LED strip on Arduino to show flower was collected
		if (gSerialController.isConnected()) {
			std::cout << "Setting flower collected: " << gState.flowersCollected << std::endl;
			gSerialController.setFlowerCollected(gState.flowersCollected);
		}

		// Go to correct explanation screen
		gState.game_state = TRIVIA_CORRECT_EXPLANATION;
	}
	else {
		PlayWrongAnswerSound();
		// Go to incorrect explanation screen
		gState.game_state = TRIVIA_INCORRECT_EXPLANATION;
	}

	// Switch back to steering mode when leaving trivia mode
	if (gSerialController.isConnected()) {
		gSerialController.setSteeringMode();
	}
}

void HandleTriviaMode(int ms_elapsed)
{
	// Update Arduino controller to get latest inputs
	gSerialController.update();

	// Get current keyboard state
	bool upKeyPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Up);
	bool downKeyPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Down);
	bool enterKeyPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Enter);

	// Static variables to track previous key states
	static bool upKeyWasPressed = false;
	static bool downKeyWasPressed = false;
	static bool enterKeyWasPressed = false;

	// Store the selection value at beginning of frame
	int startingSelection = gState.selected_trivia_answer;

	// Flag to track if selection was changed this frame
	bool selectionChanged = false;

	// Simple debug output
	std::cout << "Trivia Mode - current selection: " << gState.selected_trivia_answer << std::endl;

	// Look for "Selected:" in the last message from Arduino
	if (gSerialController.isConnected()) {
		std::string lastMsg = gSerialController.getLastMessage();
		size_t selectedPos = lastMsg.find("Selected: ");
		if (selectedPos != std::string::npos && selectedPos + 10 < lastMsg.length()) {
			char answerChar = lastMsg[selectedPos + 10];
			if (answerChar >= '1' && answerChar <= '4') {
				int newSelection = answerChar - '0' - 1; // Convert to 0-3 range
				if (newSelection != gState.selected_trivia_answer) {
					gState.selected_trivia_answer = newSelection;
					std::cout << "Arduino selection changed to: " << gState.selected_trivia_answer << std::endl;
					selectionChanged = true;
				}
			}
		}
	}

	// Handle keyboard Up input (only if not already changed by Arduino)
	if (!selectionChanged && upKeyPressed && !upKeyWasPressed) {
		gState.selected_trivia_answer = (gState.selected_trivia_answer > 0) ?
			gState.selected_trivia_answer - 1 : 3;
		selectionChanged = true;
	}

	// Handle keyboard Down input (only if not already changed by Arduino)
	if (!selectionChanged && downKeyPressed && !downKeyWasPressed) {
		gState.selected_trivia_answer = (gState.selected_trivia_answer < 3) ?
			gState.selected_trivia_answer + 1 : 0;
		selectionChanged = true;
	}

	gSerialController.update();

	// Handle button press for submitting answer - use either keyboard or Arduino
	bool buttonPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Enter) ||
		gSerialController.isButtonPressed();

	if (buttonPressed) {
		std::cout << "Button press detected - submitting answer " << gState.selected_trivia_answer << std::endl;
		AnswerTriviaQuestion(gState.selected_trivia_answer);
	}

	// Force a redraw if selection changed
	if (selectionChanged) {
		DrawFrame();
	}

	// Update previous key states for next frame - combine keyboard and Arduino inputs
	upKeyWasPressed = upKeyPressed;
	downKeyWasPressed = downKeyPressed;
	enterKeyWasPressed = buttonPressed;
}


void ResetGhostsAndPlayer()
{
	Ghost* temp = gState.ghosts[0];
	temp->pos = { 14, 11.5 };
	temp->cur_dir = LEFT;
	temp->target_state = CORNER;
	temp->in_house = false;
	temp->move_speed = ghost_speed;
	//temp->dot_counter = 0;
	temp->enable_draw = true;

	temp = gState.ghosts[1];
	temp->pos = { 14, 14.5 };
	temp->cur_dir = UP;
	temp->target_state = HOMEBASE;
	temp->in_house = true;
	temp->move_speed = inhome_speed;
	//temp->dot_counter = 0;
	temp->enable_draw = true;

	temp = gState.ghosts[2];
	temp->pos = { 12, 14.5 };
	temp->cur_dir = DOWN;
	temp->target_state = HOMEBASE;
	temp->in_house = true;
	temp->move_speed = inhome_speed;
	//temp->dot_counter = 0;
	temp->enable_draw = true;

	temp = gState.ghosts[3];
	temp->pos = { 16, 14.5 };
	temp->cur_dir = DOWN;
	temp->target_state = HOMEBASE;
	temp->in_house = true;
	temp->move_speed = inhome_speed;
	//temp->dot_counter = 0;
	temp->enable_draw = true;

	gState.player->cur_dir = UP;
	gState.player->pos = { 14,23.5 };
	gState.player->stopped = true;
	gState.player->enable_draw = true;

	ResetAnimation();
	gState.energizer_time = 0;
	gState.wave_counter = 0;
	gState.wave_time = 0;

	ResetPPelletFlash();

	if (!gState.first_life)
		gState.using_global_counter = true;

	gState.global_dot_counter = 0;
}
void ResetBoard()
{
	gState.board.clear();
	InitBoard();

	gState.pellets_left = 244;
	gState.flowersCollected = 0; // Reset flower collection count

	gState.first_life = true;
	gState.using_global_counter = false;
	for (int i = 0; i < 4; i++)
		gState.ghosts[i]->dot_counter = 0;

	// Reset all LEDs
	if (gSerialController.isConnected()) {
		gSerialController.resetLEDs();
	}
}

void IncrementGhostHouse()
{
	Ghost* first_ghost = nullptr;
	// using global counter, increment it
	if (gState.using_global_counter) {
		gState.global_dot_counter++;
	}
	for (int i = 0; i < 4; i++) {
		if (gState.ghosts[i]->target_state == HOMEBASE) {
			first_ghost = gState.ghosts[i];
			break;
		}
	}
	if (first_ghost == nullptr) {
		// no more ghosts in house, switch back to local counters
		if (gState.using_global_counter) {
			gState.using_global_counter = false;
		}
	}
	// if not using global and ghost is in house, use local counter
	else if (!gState.using_global_counter) {
		first_ghost->dot_counter++;
	}
}
void CheckPelletCollision()
{
	// Skip collision check during cooldown
	if (!gState.can_interact_with_flower) {
		return;
	}

	char tile = GetTile(gState.player->pos.x, gState.player->pos.y);
	bool collided = false;

	if (tile == '.') {
		collided = true;
		gState.game_score += 10;
		PlayMunch();

		RemovePellet(gState.player->pos.x, gState.player->pos.y);
		SetTile(gState.player->pos.x, gState.player->pos.y, ' ');
		IncrementGhostHouse();
		gState.pellet_eaten = true;
		gState.pellets_left--;
	}
	else if (tile == 'o') {
		// This is a flower (power pellet)
		collided = true;

		// Switch to answering mode when entering trivia mode
		if (gSerialController.isConnected()) {
			gSerialController.setAnsweringMode();
		}

		// Get a new random question
		TriviaQuestion currentQuestion = gTriviaManager.GetRandomQuestion();
		gState.current_trivia_question = currentQuestion;
		gState.game_state = TRIVIA_MODE;
		gState.selected_trivia_answer = 0; // Reset selected answer

		gState.energizer_time = 0; // Pause energizer until question is answered

		// Don't remove the flower yet - we'll only remove it if answered correctly
		return;
	}
}
void CheckGhostCollision()
{
	int px = (int)gState.player->pos.x;
	int py = (int)gState.player->pos.y;

	for (int i = 0; i < 4; i++) {
		if ((int)gState.ghosts[i]->pos.x == px && (int)gState.ghosts[i]->pos.y == py) {
			if (gState.ghosts[i]->target_state == FRIGHTENED) {
				SetGhostState(*gState.ghosts[i], GOHOME);
				gState.recent_eaten = gState.ghosts[i];
				gState.ghosts_eaten_in_powerup++;
				gState.game_score += (pow(2, gState.ghosts_eaten_in_powerup) * 100);

				gState.player_eat_ghost = true;
				gState.pause_time = 500;

				gState.ghosts[i]->enable_draw = false;
				gState.player->enable_draw = false;

				PlayEatGhost();
			}
			else if (gState.ghosts[i]->target_state != GOHOME) {
				gState.game_state = GAMELOSE;
				gState.pause_time = 2000;
				gState.player_lives -= 1;
				gState.first_life = false;
				StartPacManDeath();
				StopSounds();
				PlayDeathSound();
				printf("RESET\n");
			}
		}
	}
}
void UpdateWave(int ms_elapsed)
{
	// indefinte chase mode
	if (gState.wave_counter >= 7)
		return;

	gState.wave_time += ms_elapsed;
	if (gState.wave_time / 1000 >= wave_times[gState.wave_counter]) {
		gState.wave_counter++;
		printf("New wave\n");
		if (gState.energizer_time <= 0)
			SetAllGhostState(GetGlobalTarget());
		gState.wave_time = 0;
	}

}
void UpdateEnergizerTime(int ms_elasped)
{
	if (gState.energizer_time <= 0)
		return;

	gState.energizer_time -= ms_elasped;
	if (gState.energizer_time <= 0) {
		SetAllGhostState(GetGlobalTarget());
		gState.ghosts_eaten_in_powerup = 0; // Reset the counter when power mode ends
	}
}
void CheckHighScore()
{
	if (gState.game_score > gState.high_score)
		gState.high_score = gState.game_score;
}
void CheckWin()
{
	if (gState.pellets_left <= 0) {
		gState.game_state = GAMEWIN;
		for (int i = 0; i < 4; i++) {
			gState.ghosts[i]->enable_draw = false;
		}
		gState.player->stopped = true;
		gState.pause_time = 2000;
		StopSounds();
		SetPulseFrequency(200);
	}
}

void MainLoop(int ms_elapsed)
{
	UpdateFlowerInteractionCooldown(ms_elapsed);

	if (gState.player_eat_ghost) {
		gState.pause_time -= ms_elapsed;
		if (gState.pause_time < 0) {
			gState.recent_eaten->enable_draw = true;
			gState.player->enable_draw = true;
			gState.player_eat_ghost = false;
		}
		DrawFrame();

		return;
	}

	// pacman doesnt move for one frame if he eats a pellet
	// from the original game
	if (!gState.pellet_eaten)
		PlayerMovement();
	else gState.pellet_eaten = false;
	// check collision first so less funny stuff
	CheckGhostCollision();
	CheckPelletCollision();
	UpdateGhosts();
	UpdateWave(ms_elapsed);
	UpdateEnergizerTime(ms_elapsed);
	CheckHighScore();
	CheckWin();
	UpdateGameSounds(ms_elapsed);

	AnimateUpdate(ms_elapsed);
	DrawFrame();
}
void GameStart(int ms_elasped)
{
	gState.pause_time -= ms_elasped;
	if (gState.pause_time <= 0) {
		gState.game_state = MAINLOOP;
		SetPulseFrequency(150);

		// Log the game start
		gGameLogger.logGameStart();
	}

	DrawFrame();
}
void GameLose(int ms_elapsed)
{
	gState.pause_time -= ms_elapsed;
	if (gState.pause_time <= 0) {
		if (gState.player_lives == 0) {
			gState.game_state = GAMEOVER;
			gState.pause_time = 5000;
			for (int i = 0; i < 4; i++)
				gState.ghosts[i]->enable_draw = false;
			gState.player->enable_draw = false;

			// Log the final score and game over time
			gGameLogger.logGameScore(gState.game_score);
			gGameLogger.logGameOver();
		}
		else {
			gState.game_state = GAMESTART;
			gState.pause_time = 2000;

			ResetGhostsAndPlayer();
		}
	}
	UpdateGameSounds(ms_elapsed);
	AnimateUpdate(ms_elapsed);
	DrawFrame();
}
void GameWin(int ms_elapsed)
{
	gState.pause_time -= ms_elapsed;
	if (gState.pause_time <= 0) {
		// Log the final score and game over time before resetting
		gGameLogger.logGameScore(gState.game_score);
		gGameLogger.logNewRound();

		ResetPellets();
		ResetBoard();
		ResetGhostsAndPlayer();
		gState.pause_time = 2000;
		gState.game_state = GAMESTART;
	}
	AnimateUpdate(ms_elapsed);
	DrawFrame();
}
void SetupMenu()
{
    // Original menu setup
    for (int i = 0; i < 4; i++) {
        gState.ghosts[i]->enable_draw = true;
        gState.ghosts[i]->pos = { 6, 5.5f + i * 3.f };
        gState.ghosts[i]->cur_dir = RIGHT;
        gState.ghosts[i]->target_state = CHASE;
        gState.ghosts[i]->in_house = false;
    }
    gState.player->enable_draw = true;
    gState.player->pos = { 6, 17.5f };
    gState.player->cur_dir = RIGHT;
    SetPacManMenuFrame();
    SetPulseFrequency(200);
    
    // Start with instruction screens instead of menu
    SetupInstructionScreens();
}
void Menu(int ms_elapsed)
{
	// Update Arduino controller to get latest inputs
	gSerialController.update();

	PulseUpdate(ms_elapsed);

	// Check for Enter key or Arduino button press to start the game
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter) || gSerialController.isGameStartPressed())
	{
		ResetPellets();
		ResetBoard();
		ResetGhostsAndPlayer();
		gState.game_score = 0;
		gState.player_lives = 3;
		PlayGameStart();
		gState.pause_time = 4000;
		gState.game_state = GAMESTART;
	}
}
void GameLoop(int ms_elapsed)
{
	// Check for reset request from controller first
	gSerialController.update();
	if (gSerialController.isResetRequested()) {
		// Perform full game reset
		std::cout << "Resetting game due to button hold request" << std::endl;

		// Reset game variables
		gState.game_score = 0;
		gState.player_lives = 3;

		// Reset board and game elements
		ResetPellets();
		ResetBoard();
		ResetGhostsAndPlayer();

		// Reset Arduino controller
		if (gSerialController.isConnected()) {
			gSerialController.resetLEDs();
			gSerialController.resetGame();
		}

		// Go back to instruction screens
		SetupInstructionScreens();
		gState.game_state = INSTR_SCREEN1;

		// Add a small delay to prevent immediately processing another input
		std::this_thread::sleep_for(std::chrono::milliseconds(500));

		return; // Skip the rest of the game loop for this frame
	}

	switch (gState.game_state)
	{
	case MAINLOOP:
		MainLoop(ms_elapsed);
		break;
	case GAMESTART:
		GameStart(ms_elapsed);
		break;
	case GAMELOSE:
		GameLose(ms_elapsed);
		break;
	case GAMEOVER:
		gState.pause_time -= ms_elapsed;
		if (gState.pause_time < 0) {
			// Turn off all LEDs when game over
			if (gSerialController.isConnected()) {
				gSerialController.resetLEDs();
				gSerialController.resetGame();
			}

			SetupMenu();
			gState.game_state = INSTR_SCREEN1;
		}
		DrawFrame();
		break;
	case GAMEWIN:
		GameWin(ms_elapsed);
		break;
	case TRIVIA_MODE:
		HandleTriviaMode(ms_elapsed);
		break;
	case TRIVIA_CORRECT_EXPLANATION:
		HandleTriviaCorrectExplanation(ms_elapsed);
		break;
	case TRIVIA_INCORRECT_EXPLANATION:
		HandleTriviaIncorrectExplanation(ms_elapsed);
		break;
	case MENU:
		Menu(ms_elapsed);
		break;
	case INSTR_SCREEN1:
		HandleInstructionScreen1(ms_elapsed);
		break;
	case INSTR_SCREEN2:
		HandleInstructionScreen2(ms_elapsed);
		break;
	case INSTR_SCREEN3:
		HandleInstructionScreen3(ms_elapsed);
		break;
	case INSTR_SCREEN4:
		HandleInstructionScreen4(ms_elapsed);
		break;
	case INSTR_FINAL_SCREEN:
		HandleFinalInstructionScreen(ms_elapsed);
		break;
	}
}

bool CheckButtonPress()
{
	// Update Arduino controller to get latest inputs
	gSerialController.update();

	bool keyboardEnter = sf::Keyboard::isKeyPressed(sf::Keyboard::Enter);

	// Get button press from serial controller
	bool arduinoButton = false;

	// Instead of using the built-in functions, check the raw serial data
	static bool buttonReceived = false;

	// If we've just received a "Button" message from the Arduino, set the flag
	if (!buttonReceived && gSerialController.getLastMessage().find("Button") != std::string::npos) {
		buttonReceived = true;
		arduinoButton = true;
		std::cout << "Raw button message detected in CheckButtonPress" << std::endl;
	}
	else if (gSerialController.isButtonPressed() || gSerialController.isGameStartPressed()) {
		arduinoButton = true;
	}

	// Reset the flag if no button message in the last message
	if (gSerialController.getLastMessage().find("Button") == std::string::npos) {
		buttonReceived = false;
	}

	// Return true if either input is detected
	return keyboardEnter || arduinoButton;
}

void HandleInstructionScreen1(int ms_elapsed)
{
	// Get current time
	unsigned long currentTime = GetTickCount();

	// Draw the instruction screen
	DrawInstructionScreen1();

	// Only allow screen advance after a delay from screen load
	if (currentTime - gState.screenChangeTime > 100) {
		gState.canAdvanceScreen = true;
	}

	// Update Arduino controller to get latest inputs
	gSerialController.update();

	// Check for button or key press
	bool buttonPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Enter) ||
		gSerialController.isButtonPressed() ||
		gSerialController.isGameStartPressed();

	// Only advance if we're allowed and a button is pressed
	if (gState.canAdvanceScreen && buttonPressed) {
		PlayButtonSound();

		// Advance to next screen
		gState.game_state = INSTR_SCREEN2;

		// Reset states for next screen
		gState.canAdvanceScreen = false;
		gState.screenChangeTime = currentTime;

		std::cout << "Advanced to screen 2" << std::endl;
	}

	// Update pulse effect
	PulseUpdate(ms_elapsed);
}

void HandleInstructionScreen2(int ms_elapsed)
{
	unsigned long currentTime = GetTickCount();

	DrawInstructionScreen2();

	if (currentTime - gState.screenChangeTime > 100) {
		gState.canAdvanceScreen = true;
	}

	gSerialController.update();

	bool buttonPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Enter) ||
		gSerialController.isButtonPressed() ||
		gSerialController.isGameStartPressed();

	if (gState.canAdvanceScreen && buttonPressed) {
		PlayButtonSound();
		gState.game_state = INSTR_SCREEN3;
		gState.canAdvanceScreen = false;
		gState.screenChangeTime = currentTime;
		std::cout << "Advanced to screen 3" << std::endl;
	}

	PulseUpdate(ms_elapsed);
}

void HandleInstructionScreen3(int ms_elapsed)
{
	unsigned long currentTime = GetTickCount();

	DrawInstructionScreen3();

	if (currentTime - gState.screenChangeTime > 100) {
		gState.canAdvanceScreen = true;
	}

	gSerialController.update();

	bool buttonPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Enter) ||
		gSerialController.isButtonPressed() ||
		gSerialController.isGameStartPressed();

	if (gState.canAdvanceScreen && buttonPressed) {
		PlayButtonSound();
		gState.game_state = INSTR_SCREEN4;
		gState.canAdvanceScreen = false;
		gState.screenChangeTime = currentTime;
		std::cout << "Advanced to screen 4" << std::endl;
	}

	PulseUpdate(ms_elapsed);
}

void HandleInstructionScreen4(int ms_elapsed)
{
	unsigned long currentTime = GetTickCount();

	DrawInstructionScreen4();

	if (currentTime - gState.screenChangeTime > 100) {
		gState.canAdvanceScreen = true;
	}

	gSerialController.update();

	bool buttonPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Enter) ||
		gSerialController.isButtonPressed() ||
		gSerialController.isGameStartPressed();

	if (gState.canAdvanceScreen && buttonPressed) {
		PlayButtonSound();
		gState.game_state = INSTR_FINAL_SCREEN;
		gState.canAdvanceScreen = false;
		gState.screenChangeTime = currentTime;
		std::cout << "Advanced to final screen" << std::endl;
	}

	PulseUpdate(ms_elapsed);
}

void HandleFinalInstructionScreen(int ms_elapsed)
{
	unsigned long currentTime = GetTickCount();

	DrawFinalInstructionScreen();

	if (currentTime - gState.screenChangeTime > 100) {
		gState.canAdvanceScreen = true;
	}

	gSerialController.update();

	bool buttonPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Enter) ||
		gSerialController.isButtonPressed() ||
		gSerialController.isGameStartPressed();

	if (gState.canAdvanceScreen && buttonPressed) {
		PlayButtonSound();
		std::cout << "Starting game from final screen" << std::endl;
		ResetPellets();
		ResetBoard();
		ResetGhostsAndPlayer();
		gState.game_score = 0;
		gState.player_lives = 3;
		PlayGameStart();
		gState.pause_time = 4000;
		gState.game_state = GAMESTART;
	}

	PulseUpdate(ms_elapsed);
}

// Update SetupInstructionScreens to initialize screen transition states
void SetupInstructionScreens()
{
	PlayInstructionAmbient();
	gState.game_state = INSTR_SCREEN1;
	gState.canAdvanceScreen = false;
	gState.screenChangeTime = GetTickCount();

	// Position ghosts off-screen during instructions
	for (int i = 0; i < 4; i++) {
		gState.ghosts[i]->enable_draw = false;
	}

	// Hide player during first instruction (we use custom sprite)
	gState.player->enable_draw = false;

	// Set pulsing effect for button prompt
	SetPulseFrequency(400);
}
void HandleTriviaCorrectExplanation(int ms_elapsed)
{
	// Draw the explanation screen
	DrawTriviaExplanationScreen(true);

	// Update Arduino controller to get latest inputs
	gSerialController.update();

	// Check for button press to continue
	bool buttonPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Enter) ||
		gSerialController.isButtonPressed();

	// If button is pressed, return to main game
	if (buttonPressed) {
		// End explanation timer
		gGameLogger.endExplanationTimer();
		// Make sure LED is updated one more time before leaving screen
		if (gSerialController.isConnected()) {
			std::cout << "Confirmation LED update for flower: " << gState.flowersCollected << std::endl;
			gSerialController.setFlowerCollected(gState.flowersCollected);
		}

		// Since the answer was correct, now we can remove the flower
		RemovePellet(gState.player->pos.x, gState.player->pos.y);
		SetTile(gState.player->pos.x, gState.player->pos.y, ' ');
		IncrementGhostHouse();
		gState.pellet_eaten = true;
		gState.pellets_left--;

		gState.game_state = MAINLOOP;

		// Reset the selected answer for next trivia question
		gState.selected_trivia_answer = 0;
	}

	// Update pulse effect
	PulseUpdate(ms_elapsed);
}

void HandleTriviaIncorrectExplanation(int ms_elapsed)
{
	// Draw the explanation screen
	DrawTriviaExplanationScreen(false);

	// Update Arduino controller to get latest inputs
	gSerialController.update();

	// Check for button press to continue
	bool buttonPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Enter) ||
		gSerialController.isButtonPressed();

	// If button is pressed, return to main game
	if (buttonPressed) {
		// End explanation timer
		gGameLogger.endExplanationTimer();

		gState.game_state = MAINLOOP;

		// Reset the selected answer for next trivia question
		gState.selected_trivia_answer = 0;

		// Disable flower interaction temporarily to let player move away
		gState.can_interact_with_flower = false;
		gState.flower_interaction_cooldown = 1.0f; // Half-second cooldown

		// The flower remains on the board for future attempts
		// When the player returns, they'll get a different question
	}

	// Update pulse effect
	PulseUpdate(ms_elapsed);
}

void UpdateFlowerInteractionCooldown(int ms_elapsed)
{
	if (!gState.can_interact_with_flower) {
		gState.flower_interaction_cooldown -= ms_elapsed / 1000.0f;

		if (gState.flower_interaction_cooldown <= 0.0f) {
			gState.can_interact_with_flower = true;
			gState.flower_interaction_cooldown = 0.0f;
		}
	}
}