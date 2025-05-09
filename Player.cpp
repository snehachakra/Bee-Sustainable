#include "Player.h"
#include "SerialController.h"
#include "Gameloop.h"
#include <iostream>

Dir GetCorrection(Dir pdir, sf::Vector2f ppos)
{
	switch (pdir)
	{
	case UP:
	case DOWN:
		if (ppos.x - (int)ppos.x >= 0.5)
			return LEFT;
		return RIGHT;
		break;
	case LEFT:
	case RIGHT:
		if (ppos.y - (int)ppos.y >= 0.5)
			return UP;
		return DOWN;
		break;
	}
}
void Cornering()
{
	gState.player->pos += dir_addition[gState.player->correction] * player_speed;
	bool done = false;
	switch (gState.player->correction)
	{
	case UP:
		done = (gState.player->pos.y - (int)gState.player->pos.y <= 0.5);
		break;
	case DOWN:
		done = (gState.player->pos.y - (int)gState.player->pos.y >= 0.5);
		break;
	case LEFT:
		done = (gState.player->pos.x - (int)gState.player->pos.x <= 0.5);
		break;
	case RIGHT:
		done = (gState.player->pos.x - (int)gState.player->pos.x >= 0.5);
		break;
	}
	if (done) {
		CenterObject(gState.player->cur_dir, gState.player->pos);
		gState.player->cornering = false;
	}
}
void ResolveCollision()
{
	switch (gState.player->cur_dir) {
	case UP:
		gState.player->pos.y = (int)gState.player->pos.y + 0.5;
		break;
	case DOWN:
		gState.player->pos.y = (int)gState.player->pos.y + 0.5;
		break;
	case LEFT:
		gState.player->pos.x = (int)gState.player->pos.x + 0.5;
		break;
	case RIGHT:
		gState.player->pos.x = (int)gState.player->pos.x + 0.5;
		break;
	}
}
void PlayerMovement()
{
    // Update Arduino controller to get latest inputs FIRST
    gSerialController.update();

    Dir try_dir = NONE;

    // Store current player direction to check if it changes
    Dir old_dir = gState.player->cur_dir;

    // Debug all joystick states
    if (gSerialController.isConnected()) {
        bool upPressed = gSerialController.isUpPressed();
        bool downPressed = gSerialController.isDownPressed();
        bool leftPressed = gSerialController.isLeftPressed();
        bool rightPressed = gSerialController.isRightPressed();
    }

    // Check keyboard input
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
        try_dir = UP;
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
        try_dir = DOWN;
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
        try_dir = RIGHT;
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
        try_dir = LEFT;

    // Check Arduino joystick input (overrides keyboard)
    if (gSerialController.isConnected()) {
        // Reset the SerialController's joystick flags after checking, to ensure
        // we only respond to fresh inputs each frame
        if (gSerialController.isUpPressed()) {
            try_dir = UP;
            std::cout << "Setting try_dir to UP from joystick" << std::endl;
        }
        else if (gSerialController.isDownPressed()) {
            try_dir = DOWN;
            std::cout << "Setting try_dir to DOWN from joystick" << std::endl;
        }
        else if (gSerialController.isRightPressed()) {
            try_dir = RIGHT;
            std::cout << "Setting try_dir to RIGHT from joystick" << std::endl;
        }
        else if (gSerialController.isLeftPressed()) {
            try_dir = LEFT;
            std::cout << "Setting try_dir to LEFT from joystick" << std::endl;
        }
    }

    // If we have a direction to try, check if it's valid
    if (try_dir != NONE) {

        // Check if we can move in this direction
        if (!PlayerTileCollision(try_dir, gState.player->pos) && !InTunnel(gState.player->pos)) {

            // Change direction
            gState.player->cur_dir = try_dir;

            // Only need to corner if not opposite direction
            if (try_dir != opposite_dir[old_dir]) {
                gState.player->cornering = true;
                gState.player->correction = GetCorrection(try_dir, gState.player->pos);
            }

            // Ensure player is not stopped
            gState.player->stopped = false;
        }
    }

    // After handling input, reset the controller's joystick flags
    // This ensures we only respond to fresh movements
    if (gSerialController.isConnected()) {
        // Send command to reset joystick flags in SerialController
        gSerialController.resetJoystickFlags();
    }

    // Continue with movement based on current direction
    if (!gState.player->stopped) {
        gState.player->pos += dir_addition[gState.player->cur_dir] * player_speed;
    }

    if (gState.player->cornering) {
        Cornering();
    }

    // Check for collision in the current direction
    if (PlayerTileCollision(gState.player->cur_dir, gState.player->pos)) {
        ResolveCollision();
        gState.player->stopped = true;
    }

    // tunneling
    if (gState.player->pos.x < -1) {
        gState.player->pos.x += 29;
        std::cout << "Tunneling right to left" << std::endl;
    }
    else if (gState.player->pos.x >= 29) {
        gState.player->pos.x -= 29;
        std::cout << "Tunneling left to right" << std::endl;
    }
}