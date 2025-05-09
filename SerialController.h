#ifndef SERIALCONTROLLER_H
#define SERIALCONTROLLER_H

#include <string>
#include <windows.h>

class SerialController {
private:
    HANDLE hSerial;
    bool connected;
    bool joyUp, joyDown, joyLeft, joyRight;
    bool buttonPressed;
    int selectedAnswer;
    void processSerialData(const std::string& data);
    bool resetGameRequested;

public:
    bool setMode(const std::string& mode);
    void setAnsweringMode();
    void setSteeringMode();
    SerialController();
    ~SerialController();
    bool isResetRequested() const;

    bool initialize(const char* portName);
    void disconnect();
    bool isConnected() const { return connected; }

    void update();

    bool isGameStartPressed() const;

    std::string getLastMessage() const;

    bool resetGame();

    bool setFlowerCollected(int flowerNumber);

    bool resetLEDs();

    void resetJoystickFlags();

    // Input state getters
    bool isUpPressed() const { return joyUp; }
    bool isDownPressed() const { return joyDown; }
    bool isLeftPressed() const { return joyLeft; }
    bool isRightPressed() const { return joyRight; }
    bool isButtonPressed() const { return buttonPressed; }
    int getSelectedAnswer() const { return selectedAnswer; }
};

#endif // SERIALCONTROLLER_H