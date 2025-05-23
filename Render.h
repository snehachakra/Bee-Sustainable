#ifndef RENDER_H
#define RENDER_H
#include "SFML/Graphics.hpp"
#include "Buzzy.h"
#include <map>

struct Textures
{
	sf::Texture pellets;
	sf::Texture sprites;
	sf::Texture wall_map_t;
	sf::Texture wall_map_t_white;
	sf::Texture font;
	sf::Texture powered_pacman;
	sf::Texture buzzy_sprite;
	sf::Texture buzzy_friends;
	sf::Texture flower_t;
};
struct RenderItems
{
	sf::VertexArray pellet_va;
	sf::VertexArray sprite_va;
	sf::VertexArray wall_va;
	sf::Sprite wall_map;

	sf::Sprite float_score;

	sf::Sprite player;
	sf::Sprite ghosts[4];

	sf::Sprite buzzy;
	sf::Sprite buzzyfriends;
	sf::Sprite flower;

	// instead of rebuilding pellet vertex array every frame, store va index of 
	// each pellet location then only delete the one eaten
	std::map<int, int> pellet_va_indicies;

	// also keep index of power up pellets so they can be flashed
	int pow_indicies[4];

	sf::VertexArray text_va;

	bool pow_is_off = false;
};

const int font_width = 14;

// pellet rects
const sf::FloatRect pel_r = { 0,0,16,16 };
const sf::FloatRect pow_r = { 16,0,16,16 };

void InitRender();
void InitWalls();
void InitTextures();
void InitPellets();
void ResetPellets();
void RemovePellet(int x, int y);
void MakeQuad(sf::VertexArray& va, float x, float y, int w, int h,
	sf::Color color = { 255,255,255 }, sf::FloatRect tex_rect = { 0,0,0,0 });
void DrawGameUI();
void DrawFrame();
void DrawTriviaQuestion();
void DrawTriviaExplanationScreen(bool was_correct);

std::vector<std::string> WrapText(const std::string& text, size_t line_length);

void FlashPPellets();
void ResetPPelletFlash();

void DrawInstructionScreen1();
void DrawInstructionScreen2();
void DrawInstructionScreen3();
void DrawInstructionScreen4();
void DrawFinalInstructionScreen();

// Text drawing
// Ive tried using SFML's text class in the past but it ends up
// looking blurry at low resolution, this is a simple way of drawing text
// using the original lettering
void ClearText();
void MakeText(std::string string, float x, float y, sf::Color f_color);


#endif // !RENDER_H