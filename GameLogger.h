#pragma once

#include <string>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>

class GameLogger {
private:
    std::ofstream logFile;
    std::string fileName;
    std::chrono::time_point<std::chrono::system_clock> gameStartTime;
    std::chrono::time_point<std::chrono::system_clock> explanationStartTime;
    bool isExplanationActive;
    int currentGameScore;
    int roundNumber;

public:
    GameLogger();
    ~GameLogger();

    // Initialize the logger with a filename
    bool initialize(const std::string& filename = "game_log.txt");

    // Log game start time
    void logGameStart();

    // Log trivia question answer
    void logTriviaAnswer(const std::string& question, int selectedAnswer, bool wasCorrect);

    void logNewRound();

    // Start tracking explanation screen time
    void startExplanationTimer();

    // End tracking explanation screen time and log it
    void endExplanationTimer();

    // Log final game score
    void logGameScore(int score);

    // Log game over/death time
    void logGameOver();

    // Get current timestamp as string
    std::string getCurrentTimestamp();

    // Helper to convert duration to string
    std::string durationToString(std::chrono::milliseconds duration);

    // Add a separator between game sessions
    void addSessionSeparator();
};

// Global instance
extern GameLogger gGameLogger;