#ifndef GAMELOOP_H
#define GAMELOOP_H
#include "Buzzy.h"
#include "Render.h"
#include "Hornets.h"
#include "Player.h"
#include "Animate.h"
#include "Sound.h"
#include "SerialController.h"

// Global controller instance - declaration with extern keyword
extern SerialController gSerialController;

void OnStart();
void OnQuit();
void GameLoop(int ms_elapsed);

void Init();
void InitBoard();
void ResetGhostsAndPlayer();
void ResetBoard();
void SetupMenu();

void AnswerTriviaQuestion(int selected_index);
void HandleTriviaMode(int ms_elapsed);

void IncrementGhostHouse();
void CheckPelletCollision();
void CheckGhostCollision();
void UpdateWave(int ms_elapsed);
void UpdateEnergizerTime(int ms_elasped);
void CheckWin();

void CheckHighScore();
void LoadHighScore();
void SaveHighScore();

// game states
void MainLoop(int ms_elasped);
void GameStart(int ms_elasped);
void GameLose(int ms_elasped);
void GameWin(int ms_elasped);
void Menu(int ms_elapsed);

// Serial port initialization
bool InitializeSerialController();

void HandleTriviaCorrectExplanation(int ms_elapsed);
void HandleTriviaIncorrectExplanation(int ms_elapsed);
void UpdateFlowerInteractionCooldown(int ms_elapsed);

void SetupInstructionScreens();
bool CheckButtonPress();
void HandleInstructionScreen1(int ms_elapsed);
void HandleInstructionScreen2(int ms_elapsed);
void HandleInstructionScreen3(int ms_elapsed);
void HandleInstructionScreen4(int ms_elapsed);
void HandleFinalInstructionScreen(int ms_elapsed);

#endif // !GAMELOOP_H