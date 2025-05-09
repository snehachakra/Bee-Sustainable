#include "GameLogger.h"
#include <iostream>

// Global instance
GameLogger gGameLogger;

GameLogger::GameLogger() :
    isExplanationActive(false),
    currentGameScore(0) {
}

GameLogger::~GameLogger() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

bool GameLogger::initialize(const std::string& filename) {
    fileName = filename;

    // Open the log file in append mode
    logFile.open(fileName, std::ios::app);

    if (!logFile.is_open()) {
        std::cerr << "Failed to open log file: " << fileName << std::endl;
        return false;
    }

    std::cout << "Game logger initialized with file: " << fileName << std::endl;
    return true;
}

void GameLogger::logGameStart() {
    if (!logFile.is_open()) return;

    // Record game start time
    gameStartTime = std::chrono::system_clock::now();

    // Write to log
    logFile << "==== GAME SESSION START ====" << std::endl;
    logFile << "Start Time: " << getCurrentTimestamp() << std::endl;
    logFile << std::endl;
    roundNumber = 1;

    logFile.flush(); // Ensure it's written to disk
    std::cout << "Logged game start time" << std::endl;
}

void GameLogger::logTriviaAnswer(const std::string& question, int selectedAnswer, bool wasCorrect) {
    if (!logFile.is_open()) return;

    logFile << "TRIVIA QUESTION: " << question << std::endl;
    logFile << "Selected Answer: " << (selectedAnswer + 1) << std::endl; // +1 to convert from 0-based to 1-based for readability
    logFile << "Result: " << (wasCorrect ? "CORRECT" : "INCORRECT") << std::endl;
    logFile << std::endl;

    logFile.flush();
    std::cout << "Logged trivia answer" << std::endl;
}

void GameLogger::logNewRound() {
    if (!logFile.is_open()) return;

    roundNumber++;

    logFile << "\nROUND " << roundNumber << " START" << std::endl;
    logFile.flush();
    std::cout << "Logged new round" << std::endl;
}

void GameLogger::startExplanationTimer() {
    explanationStartTime = std::chrono::system_clock::now();
    isExplanationActive = true;
    std::cout << "Started explanation timer" << std::endl;
}

void GameLogger::endExplanationTimer() {
    if (!logFile.is_open() || !isExplanationActive) return;

    auto endTime = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - explanationStartTime);

    logFile << "Time spent on explanation: " << durationToString(duration) << std::endl;
    logFile << std::endl;

    logFile.flush();
    isExplanationActive = false;
    std::cout << "Logged explanation time: " << durationToString(duration) << std::endl;
}

void GameLogger::logGameScore(int score) {
    if (!logFile.is_open()) return;

    currentGameScore = score;

    logFile << "Final Game Score: " << score << " points" << std::endl;
    logFile.flush();
    std::cout << "Logged game score: " << score << std::endl;
}

void GameLogger::logGameOver() {
    if (!logFile.is_open()) return;

    auto endTime = std::chrono::system_clock::now();
    auto gameDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - gameStartTime);

    logFile << "Game Over Time: " << getCurrentTimestamp() << std::endl;
    logFile << "Total Game Duration: " << durationToString(gameDuration) << std::endl;

    // Add the separator to indicate end of game session
    addSessionSeparator();

    logFile.flush();
    std::cout << "Logged game over time" << std::endl;
}

std::string GameLogger::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);

    std::tm timeInfo;

    // Use thread-safe version of localtime
#ifdef _WIN32
    // Windows version
    localtime_s(&timeInfo, &time);
#else
    // POSIX version
    localtime_r(&time, &timeInfo);
#endif

    std::stringstream ss;
    ss << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::string GameLogger::durationToString(std::chrono::milliseconds duration) {
    long long totalSeconds = duration.count() / 1000;
    long long minutes = totalSeconds / 60;
    long long seconds = totalSeconds % 60;
    long long milliseconds = duration.count() % 1000;

    std::stringstream ss;
    ss << minutes << "m " << seconds << "s " << milliseconds << "ms";
    return ss.str();
}

void GameLogger::addSessionSeparator() {
    if (!logFile.is_open()) return;

    logFile << std::endl;
    logFile << "============================" << std::endl;
    logFile << std::endl;

    logFile.flush();
}