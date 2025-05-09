#ifndef SOUND_H
#define SOUND_H
#include <SFML/Audio.hpp>
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include "Buzzy.h"

struct Sounds
{
	// Original sound buffers
	sf::SoundBuffer munch1;
	sf::SoundBuffer munch2;
	sf::SoundBuffer game_start;
	sf::SoundBuffer death_1;
	sf::SoundBuffer death_2;
	sf::SoundBuffer eat_ghost;
	sf::SoundBuffer retreating;
	sf::SoundBuffer power_pellet;
	sf::SoundBuffer siren;

	// New sound buffers
	sf::SoundBuffer button_press;
	sf::SoundBuffer answer_correct;
	sf::SoundBuffer answer_wrong;
	sf::SoundBuffer instruction_ambient;
	sf::SoundBuffer gameplay_ambient;
	sf::SoundBuffer win_game;
	sf::SoundBuffer lose_game;

	// Original sound players
	sf::Sound munch;
	sf::Sound ghost_eat;
	sf::Sound background;
	sf::Sound death1;
	sf::Sound death2;

	// New sound players
	sf::Sound ambient;
	sf::Sound button_sfx;
	sf::Sound correct_sfx;
	sf::Sound wrong_sfx;
	sf::Sound win_sfx;
	sf::Sound lose_sfx;
};

enum BkState
{
	NO_SOUND,
	SIREN,
	PPELLET,
	RETREAT,
};

struct SoundState
{
	bool first_munch = true;
	bool playing_death = false;
	int death_timer = 0;
	BkState bk_state = NO_SOUND;
};

// Original sound functions
void InitSounds();
void PlayMunch();
void PlayEatGhost();
void PlayDeathSound();
void PlayGameStart();
void UpdateGameSounds(int ms_elapsed);
void StopSounds();

// New sound functions
void PlayButtonSound();
void PlayCorrectAnswerSound();
void PlayWrongAnswerSound();
void PlayInstructionAmbient();
void PlayGameplayAmbient();
void PlayWinSound();
void PlayLoseSound();

#endif // !SOUND_H