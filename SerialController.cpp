#include "SerialController.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <string>

// These globals are preserved
bool gameStartButtonPressed = false;
std::string lastMessage;

SerialController::SerialController() :
    hSerial(INVALID_HANDLE_VALUE),
    connected(false),
    joyUp(false),
    joyDown(false),
    joyLeft(false),
    joyRight(false),
    buttonPressed(false),
    selectedAnswer(1),
    resetGameRequested(false) {
}

SerialController::~SerialController() {
    disconnect();
}

bool SerialController::initialize(const char* portName) {
    std::cout << "Connecting to Arduino on " << portName << "..." << std::endl;

    // Close any existing connection
    if (connected) {
        disconnect();
    }

    // Format COM port name correctly for Windows
    std::string fullPortName = std::string("\\\\.\\") + portName;

    // Open the serial port
    hSerial = CreateFileA(fullPortName.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (hSerial == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        std::cerr << "Error opening serial port! Error code: " << error << std::endl;
        return false;
    }

    // Set serial port parameters
    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    if (!GetCommState(hSerial, &dcbSerialParams)) {
        std::cerr << "Error getting serial port state!" << std::endl;
        CloseHandle(hSerial);
        return false;
    }

    dcbSerialParams.BaudRate = CBR_9600;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;

    if (!SetCommState(hSerial, &dcbSerialParams)) {
        std::cerr << "Error setting serial port state!" << std::endl;
        CloseHandle(hSerial);
        return false;
    }

    // Set timeouts
    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout = MAXDWORD;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.ReadTotalTimeoutConstant = 0;
    timeouts.WriteTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 0;

    if (!SetCommTimeouts(hSerial, &timeouts)) {
        std::cerr << "Error setting timeouts!" << std::endl;
        CloseHandle(hSerial);
        return false;
    }

    // Clear any existing data
    PurgeComm(hSerial, PURGE_RXCLEAR | PURGE_TXCLEAR);

    connected = true;
    std::cout << "Successfully connected to Arduino on " << portName << std::endl;

    // Wait for Arduino to reset
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    return true;
}

void SerialController::disconnect() {
    if (connected) {
        CloseHandle(hSerial);
        hSerial = INVALID_HANDLE_VALUE;
        connected = false;
        std::cout << "Disconnected from Arduino" << std::endl;
    }
}

void SerialController::update() {
    if (!connected) return;

    // Reset transient states
    buttonPressed = false;
    gameStartButtonPressed = false;

    // DO NOT reset joystick flags here - they need to persist
    // joyUp = false;
    // joyDown = false;
    // joyLeft = false;
    // joyRight = false;
    resetGameRequested = false;

    char buffer[256] = { 0 };
    DWORD bytesRead = 0;

    // Read from serial port
    if (ReadFile(hSerial, buffer, sizeof(buffer) - 1, &bytesRead, NULL)) {
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0'; // Null terminate
            std::string input(buffer);

            // Only process if not empty
            if (!input.empty()) {
                lastMessage = input; // Store for debugging

                // Always log raw input for debugging
                std::cout << "ARDUINO RAW: [" << input << "]" << std::endl;

                // Check for reset command
                if (input.find("RESET_GAME") != std::string::npos) {
                    resetGameRequested = true;
                    std::cout << "Game reset requested via button hold" << std::endl;
                }

                // Check for button press in any form
                if (input.find("Button") != std::string::npos ||
                    input.find("NextScreen") != std::string::npos ||
                    input.find("Answering") != std::string::npos) {

                    buttonPressed = true;
                    gameStartButtonPressed = true; // For compatibility
                    std::cout << "BUTTON DETECTED IN MESSAGE" << std::endl;
                }

                // Check for joystick directions - directly check for keywords
                // Use direct string matching to catch any case

                // Looking for "Up" but not when it's part of "Selected: "
                if ((input.find("Up") != std::string::npos) &&
                    (input.find("Selected") == std::string::npos)) {
                    joyUp = true;
                    std::cout << "UP DETECTED IN MESSAGE" << std::endl;
                }

                // Looking for "Down" but not when it's part of "Selected: "
                if ((input.find("Down") != std::string::npos) &&
                    (input.find("Selected") == std::string::npos)) {
                    joyDown = true;
                    std::cout << "DOWN DETECTED IN MESSAGE" << std::endl;
                }

                // Looking for "Left"
                if (input.find("Left") != std::string::npos) {
                    joyLeft = true;
                    std::cout << "LEFT DETECTED IN MESSAGE" << std::endl;
                }

                // Looking for "Right"
                if (input.find("Right") != std::string::npos) {
                    joyRight = true;
                    std::cout << "RIGHT DETECTED IN MESSAGE" << std::endl;
                }

                // Process selected answer for trivia mode
                size_t selectedPos = input.find("Selected: ");
                if (selectedPos != std::string::npos && selectedPos + 10 < input.length()) {
                    char answerChar = input[selectedPos + 10];
                    if (answerChar >= '1' && answerChar <= '4') {
                        selectedAnswer = answerChar - '0';
                        std::cout << "Answer selection changed to: " << selectedAnswer << std::endl;
                    }
                }
            }
        }
    }

    // For debugging - print current joystick state after update
    //std::cout << "CURRENT JOYSTICK STATE: Up=" << joyUp
    //    << " Down=" << joyDown
    //    << " Left=" << joyLeft
    //    << " Right=" << joyRight << std::endl;
}

bool SerialController::isGameStartPressed() const {
    return gameStartButtonPressed || buttonPressed;  // Either will work
}

bool SerialController::isResetRequested() const {
    return resetGameRequested;
}

std::string SerialController::getLastMessage() const {
    return lastMessage;
}

bool SerialController::setMode(const std::string& mode) {
    if (!connected) return false;

    std::cout << "Setting Arduino mode to: " << mode << std::endl;

    // Create command string (with newline)
    std::string command = "MODE:" + mode + "\n";

    // Send the command to Arduino
    DWORD bytesWritten = 0;
    if (!WriteFile(hSerial, command.c_str(), command.length(), &bytesWritten, NULL) ||
        bytesWritten != command.length()) {
        std::cerr << "Failed to send mode command to Arduino" << std::endl;
        return false;
    }

    // Clear the input buffer
    PurgeComm(hSerial, PURGE_RXCLEAR);

    // Wait a moment for Arduino to process
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    return true;
}

void SerialController::setAnsweringMode() {
    setMode("ANSWERING");

    // Reset flags to avoid input issues
    joyUp = false;
    joyDown = false;
    joyLeft = false;
    joyRight = false;
    buttonPressed = false;
}

void SerialController::setSteeringMode() {
    setMode("STEERING");

    // Reset flags to avoid input issues
    joyUp = false;
    joyDown = false;
    joyLeft = false;
    joyRight = false;
    buttonPressed = false;
}

bool SerialController::resetGame() {
    if (!connected) return false;

    std::cout << "Resetting game on Arduino..." << std::endl;

    // Send reset command
    std::string command = "GAME:RESET\n";
    DWORD bytesWritten = 0;

    if (!WriteFile(hSerial, command.c_str(), command.length(), &bytesWritten, NULL) ||
        bytesWritten != command.length()) {
        std::cerr << "Failed to send reset command to Arduino" << std::endl;
        return false;
    }

    // Wait for acknowledgment
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    return true;
}

bool SerialController::setFlowerCollected(int flowerNumber) {
    if (!connected) return false;

    std::cout << "Sending flower collection: " << flowerNumber << std::endl;

    // Make sure flower number is valid (1-5)
    if (flowerNumber < 1 || flowerNumber > 5) {
        std::cerr << "Invalid flower number: " << flowerNumber << ", valid range is 1-5" << std::endl;
        return false;
    }

    // Convert to string and send the flower number
    std::string command = std::to_string(flowerNumber) + "\n";

    DWORD bytesWritten = 0;
    if (!WriteFile(hSerial, command.c_str(), command.length(), &bytesWritten, NULL) ||
        bytesWritten != command.length()) {
        std::cerr << "Failed to send flower number to Arduino" << std::endl;
        return false;
    }

    // Short delay to ensure Arduino has time to process
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    return true;
}

bool SerialController::resetLEDs() {
    // Sending flower number 5 resets all LEDs to off
    return setFlowerCollected(5);
}

void SerialController::resetJoystickFlags() {
    // Reset all joystick direction flags
    joyUp = false;
    joyDown = false;
    joyLeft = false;
    joyRight = false;
}