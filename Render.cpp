#include "Render.h"
#include "Animate.h"
#include <string>
#include <vector>
#include <sstream>
#include <iostream>

static RenderItems RItems;
static Textures RTextures;

void InitRender()
{
	InitTextures();
	InitWalls();
	RItems.pellet_va.setPrimitiveType(sf::Quads);
	RItems.sprite_va.setPrimitiveType(sf::Quads);
	RItems.text_va.setPrimitiveType(sf::Quads);

	RItems.wall_map.setTexture(RTextures.wall_map_t);
	RItems.wall_map.setScale({ 0.5,0.5 });

	InitPellets();

	for (int i = 0; i < 4; i++)
	{
		RItems.ghosts[i].setTexture(RTextures.sprites);
		RItems.ghosts[i].setScale({ 0.5,0.5 });
		RItems.ghosts[i].setOrigin({ 16,16 });
	}

	RItems.player.setTexture(RTextures.sprites);
	RItems.player.setScale({ 0.5,0.5 });
	RItems.player.setOrigin({ 15,15 });

	RItems.float_score.setTexture(RTextures.sprites);
	RItems.float_score.setScale({ 0.5,0.5 });
	RItems.float_score.setOrigin({ 16,16 });

	RItems.buzzy.setTexture(RTextures.buzzy_sprite);
	RItems.buzzy.setScale({ 0.5, 0.5 });
	RItems.buzzy.setOrigin({ 64, 64 });

	RItems.buzzyfriends.setTexture(RTextures.buzzy_friends);
	RItems.buzzyfriends.setScale({ 0.4, 0.4 });
	RItems.buzzyfriends.setOrigin({ 256, 256 });

	RItems.flower.setTexture(RTextures.flower_t);
	RItems.flower.setScale({ 0.5, 0.5 });
	RItems.flower.setOrigin({ 128, 128 });

}
void MakeQuad(sf::VertexArray& va, float x, float y, int w, int h, sf::Color color, sf::FloatRect t_rect)
{
	sf::Vertex vert;
	vert.color = color;
	vert.position = { x,y };
	vert.texCoords = { t_rect.left,t_rect.top };
	va.append(vert);
	vert.position = { x + w,y };
	vert.texCoords = { t_rect.left + t_rect.width,t_rect.top };
	va.append(vert);
	vert.position = { x + w,y + h };
	vert.texCoords = { t_rect.left + t_rect.width,t_rect.top + t_rect.height };
	va.append(vert);
	vert.position = { x,y + h };
	vert.texCoords = { t_rect.left,t_rect.top + t_rect.height };
	va.append(vert);
}
void InitWalls()
{
	RItems.wall_va.setPrimitiveType(sf::Quads);
	for (int y = 0; y < gState.board.size(); y++) {
		for (int x = 0; x < gState.board.at(y).size(); x++) {
			if (gState.board.at(y).at(x) == '|')
				MakeQuad(RItems.wall_va, x * TSIZE, y * TSIZE + YOFFSET, TSIZE, TSIZE, { 150,150,150 });
		}
	}
}
void InitTextures()
{
	RTextures.pellets.loadFromFile("textures/dots.png");
	RTextures.sprites.loadFromFile("textures/sprites.png");
	RTextures.wall_map_t.loadFromFile("textures/map.png");
	RTextures.font.loadFromFile("textures/font.png");
	RTextures.wall_map_t_white.loadFromFile("textures/map.png");
	RTextures.wall_map_t_white.loadFromFile("textures/map_white.png");
	RTextures.powered_pacman.loadFromFile("textures/poweredbees.png");
	RTextures.buzzy_sprite.loadFromFile("textures/buzzy.png");
	RTextures.buzzy_friends.loadFromFile("textures/buzzy_and_friends.png");
	RTextures.flower_t.loadFromFile("textures/flower.png");

}

void InitPellets()
{
	RItems.pellet_va.clear();
	int VA_Index = 0;
	int Pow_index = 0;
	for (int y = 0; y < gState.board.size(); y++) {
		for (int x = 0; x < gState.board.at(y).size(); x++) {
			char temp = GetTile(x, y);
			if (temp == '.') {
				MakeQuad(RItems.pellet_va, x * TSIZE, y * TSIZE + YOFFSET, TSIZE, TSIZE, { 255,255,255 }, pel_r);
				RItems.pellet_va_indicies.insert({ y * 28 + x, VA_Index });
				VA_Index += 4;
			}
			else if (temp == 'o') {
				MakeQuad(RItems.pellet_va, x * TSIZE, y * TSIZE + YOFFSET, TSIZE, TSIZE, { 255,255,255 }, pow_r);
				RItems.pellet_va_indicies.insert({ y * 28 + x, VA_Index });
				RItems.pow_indicies[Pow_index] = VA_Index;
				Pow_index++;
				VA_Index += 4;
			}
		}
	}
}
void ResetPellets()
{
	for (int i = 0; i < RItems.pellet_va.getVertexCount(); i++) {
		RItems.pellet_va[i].color = { 255,255,255,255 };
	}

	RItems.wall_map.setTexture(RTextures.wall_map_t);
}
void RemovePellet(int x, int y)
{
	int va_idx = RItems.pellet_va_indicies.at(y * 28 + x);
	sf::Vertex* vert = &RItems.pellet_va[va_idx];
	vert[0].color = { 0,0,0,0 };
	vert[1].color = { 0,0,0,0 };
	vert[2].color = { 0,0,0,0 };
	vert[3].color = { 0,0,0,0 };
}
void DrawGameUI()
{
	ClearText();

	MakeText("HIGH SCORE", 9, 0, { 204, 85, 0 });

	if (gState.game_state == GAMESTART)
		MakeText("READY!", 11, 20, { 204, 85, 0 });
	else if (gState.game_state == GAMEOVER)
		MakeText("GAME OVER", 10.5, 20, { 204, 85, 0 });

	std::string score = std::to_string(gState.game_score);
	if (score.size() == 1)
		score.insert(score.begin(), '0');
	MakeText(score, 7 - score.size(), 1, { 204, 85, 0 });

	score = std::to_string(gState.high_score);
	if (score.size() == 1)
		score.insert(score.begin(), '0');
	MakeText(score, 17 - score.size(), 1, { 204, 85, 0 });

	RItems.player.setTextureRect({ 256,32,30,30 });
	for (int i = 0; i < gState.player_lives; i++) {
		RItems.player.setPosition({ 24.f + 16 * i,35 * TSIZE });
		gState.window->draw(RItems.player);
	}
}
void FlashPPellets()
{
	sf::Uint8 new_alpha = (RItems.pow_is_off) ? 255 : 1;

	for (int i = 0; i < 4; i++) {
		int index = RItems.pow_indicies[i];
		sf::Vertex* vert = &RItems.pellet_va[index];
		// I am using alpha 0 to hide pellets, so for flashing, Ill just use alpha 1
		if (vert->color.a == 0)
			continue;
		vert[0].color.a = new_alpha;
		vert[1].color.a = new_alpha;
		vert[2].color.a = new_alpha;
		vert[3].color.a = new_alpha;
	}
}
void ResetPPelletFlash()
{
	RItems.pow_is_off = true;
	FlashPPellets();
}
void DrawTriviaQuestion()
{
	ClearText();

	// Debug output
	std::cout << "Current selected answer: " << gState.selected_trivia_answer << std::endl;

	// Get the full question
	std::string full_question = gState.current_trivia_question.question;

	// Maximum characters per line
	const int MAX_LINE_LENGTH = 26;

	// Maximum number of lines to display
	const int MAX_QUESTION_LINES = 5;

	// Wrap the question text
	std::vector<std::string> question_lines = WrapText(full_question, MAX_LINE_LENGTH);

	// Limit the number of lines displayed
	if (question_lines.size() > MAX_QUESTION_LINES) {
		question_lines.resize(MAX_QUESTION_LINES);
	}

	// Draw question lines
	for (size_t i = 0; i < question_lines.size(); i++) {
		MakeText(question_lines[i], 1, 1 + i * 2, { 204, 85, 0 });
	}

	// Calculate starting Y for answers based on question lines
	int answers_start_y = 1 + question_lines.size() * 2 + 2;

	// Draw answers with full text and line breaking
	for (int i = 0; i < gState.current_trivia_question.answers.size(); i++) {
		std::string answer_text = std::to_string(i + 1) + ": " +
			gState.current_trivia_question.answers[i];

		// Break long answers into multiple lines
		std::vector<std::string> answer_lines = WrapText(answer_text, MAX_LINE_LENGTH);

		// Limit the number of answer lines
		const int MAX_ANSWER_LINES = 3;
		if (answer_lines.size() > MAX_ANSWER_LINES) {
			answer_lines.resize(MAX_ANSWER_LINES);
		}

		// Debug output for each answer
		std::cout << "Answer " << i << " selected: "
			<< (i == gState.selected_trivia_answer ? "YES" : "NO") << std::endl;

		// Determine color based on selection
		sf::Color answer_color = (i == gState.selected_trivia_answer)
			? sf::Color::Blue  // blue for selected answer
			: sf::Color({ 204, 85, 0 });  // burnt orange for unselected answer

		// Draw each line of the answer
		for (size_t j = 0; j < answer_lines.size(); j++) {
			// Adjusted vertical spacing to create more consistent layout
			int line_y = answers_start_y + (i * 7) + j * 2;
			MakeText(answer_lines[j], 5, line_y, answer_color);
		}
	}
}

void DrawTriviaExplanationScreen(bool was_correct)
{
	gState.window->clear(sf::Color(255, 214, 135));
	ClearText();

	// Draw the Buzzy sprite
	RItems.buzzy.setPosition(14 * TSIZE, 6 * TSIZE + YOFFSET);
	gState.window->draw(RItems.buzzy);

	// Set title color based on correct/incorrect
	sf::Color titleColor = was_correct ? sf::Color::Green : sf::Color::Red;

	// Draw title
	std::string titleText = was_correct ? "CORRECT!" : "NOT QUITE!";
	MakeText(titleText, 11, 3, titleColor);

	// Get explanation
	std::string explanation = gState.current_explanation;

	// Break the explanation into lines for display
	std::vector<std::string> explanation_lines = WrapText(explanation, 26);

	// Draw explanation text
	for (size_t i = 0; i < explanation_lines.size() && i < 10; i++) {
		MakeText(explanation_lines[i], 5, 16 + i * 2, sf::Color::Black);
	}

	// Draw instruction at bottom
	if (IsPulse()) {
		MakeText("Press button to continue", 5, 32, {204, 85, 0});
	}

	gState.window->draw(RItems.text_va, &RTextures.font);
	gState.window->display();
}

std::vector<std::string> WrapText(const std::string& text, size_t line_length) {
	std::vector<std::string> wrapped_lines;
	std::istringstream iss(text);
	std::string word, current_line;

	while (iss >> word) {
		if (current_line.empty() || current_line.length() + word.length() + 1 <= line_length) {
			if (!current_line.empty()) current_line += " ";
			current_line += word;
		}
		else {
			wrapped_lines.push_back(current_line);
			current_line = word;
		}
	}

	if (!current_line.empty()) {
		wrapped_lines.push_back(current_line);
	}

	return wrapped_lines;
}
void DrawFrame()
{
	gState.window->clear(sf::Color(255, 214, 135));
	DrawGameUI();

	if (RItems.pow_is_off != IsPulse()) {
		FlashPPellets();
		RItems.pow_is_off = !RItems.pow_is_off;
	}

	static bool white_texture = false;
	if (gState.game_state == GAMEWIN) {
		if (IsPulse() && !white_texture) {
			RItems.wall_map.setTexture(RTextures.wall_map_t_white);
			white_texture = true;
		}
		else if (!IsPulse() && white_texture) {
			RItems.wall_map.setTexture(RTextures.wall_map_t);
			white_texture = false;
		}
	}

	// Ensure the game background is not drawn during trivia mode
	if (gState.game_state != TRIVIA_MODE) {
		gState.window->draw(RItems.wall_map);
		gState.window->draw(RItems.pellet_va, &RTextures.pellets);
	}

	if (gState.game_state == TRIVIA_MODE) {
		DrawTriviaQuestion();
	}

	gState.window->draw(RItems.text_va, &RTextures.font);

	for (int i = 0; i < 4; i++) {
		if (!gState.ghosts[i]->enable_draw)
			continue;
		RItems.ghosts[i].setPosition(gState.ghosts[i]->pos.x * TSIZE, gState.ghosts[i]->pos.y * TSIZE + YOFFSET);
		RItems.ghosts[i].setTextureRect(GetGhostFrame(gState.ghosts[i]->type, gState.ghosts[i]->target_state, gState.ghosts[i]->cur_dir));
		if (gState.game_state != TRIVIA_MODE) {
			gState.window->draw(RItems.ghosts[i]);
		}
	}

	if (gState.player->enable_draw) {
		RItems.player.setPosition(gState.player->pos.x * TSIZE, gState.player->pos.y * TSIZE + YOFFSET);

		// Check if player is powered up
		bool isPowered = (gState.energizer_time > 0);

		// Set the appropriate texture and frame
		if (isPowered && !animate.death_animation) {
			// Use powered texture only when not in death animation
			RItems.player.setTexture(RTextures.powered_pacman);
			RItems.player.setTextureRect(GetPoweredPacManFrame(gState.player->cur_dir));
		}
		else {
			// Always use regular sprites for death animation or normal state
			RItems.player.setTexture(RTextures.sprites);
			RItems.player.setTextureRect(GetPacManFrame(gState.player->cur_dir));
		}

		if (gState.game_state != TRIVIA_MODE) {
			gState.window->draw(RItems.player);
		}
	}

	if (gState.player_eat_ghost) {
		RItems.float_score.setPosition(gState.player->pos.x * TSIZE, gState.player->pos.y * TSIZE + YOFFSET);
		RItems.float_score.setTextureRect({ (gState.ghosts_eaten_in_powerup - 1) * 32,256,32,32 });

		if (gState.game_state != TRIVIA_MODE) {
			gState.window->draw(RItems.float_score);
		}
	}

	gState.window->display();
}

void DrawInstructionScreen1()
{
	gState.window->clear(sf::Color(255, 214, 135)); // Same background as menu

	ClearText();

	// Draw the Buzzy sprite
	// Position in the upper part of the screen, centered horizontally
	RItems.buzzy.setPosition(14 * TSIZE, 1.5 * TSIZE + YOFFSET);
	gState.window->draw(RItems.buzzy);

	// Display instruction text with word wrapping
	std::string line1 = "Hi there! I'm Buzzy, the busy worker";
	std::string line2 = "bee. Worker bees have many important";
	std::string line3 = "jobs to help the hive. We collect";
	std::string line4 = "nectar and pollen, build and protect";
	std::string line5 = "the hive, make honey, take care of";
	std::string line6 = "the babies, and more!";
	std::string line7_1 = "Today, I'm on \t\t\t\t\t\t\t\t\t\t\t duty.";
	std::string line7_2 = "POLLINATION";
	std::string line8 = "Can you help me pollinate all the";
	std::string line9 = "flowers?";

	MakeText(line1, 0, 9, sf::Color::Black);
	MakeText(line2, 0, 11, sf::Color::Black);
	MakeText(line3, 1, 13, sf::Color::Black);
	MakeText(line4, 0, 15, sf::Color::Black);
	MakeText(line5, 0.5, 17, sf::Color::Black);
	MakeText(line6, 4, 19, sf::Color::Black);
	MakeText(line7_1, 2, 23, sf::Color::Black);
	MakeText(line7_2, 12.5, 23, { 52, 194, 48 });
	MakeText(line8, 1, 27, sf::Color::Black);
	MakeText(line9, 10, 29, sf::Color::Black);

	// pulse text
	if (IsPulse()) {
		MakeText("Press the big yellow button", 3, 32, { 204, 85, 0 });
		MakeText("to continue!", 9, 34, { 204, 85, 0 });
	}

	gState.window->draw(RItems.text_va, &RTextures.font);
	gState.window->display();
}

void DrawInstructionScreen2()
{
	gState.window->clear(sf::Color(255, 214, 135));

	ClearText();

	// Draw joystick instruction with bee sprite
	RItems.flower.setPosition(14 * TSIZE, 8 * TSIZE + YOFFSET);
	gState.window->draw(RItems.flower);

	// Draw instruction text
	std::string line1 = "Use the joystick to help me fly";
	std::string line2 = "to a flower and pollinate it!";

	MakeText(line1, 2, 20, sf::Color::Black);
	MakeText(line2, 2, 22, sf::Color::Black);

	// Display button prompt - pulse this text
	if (IsPulse()) {
		MakeText("Press to continue", 7, 30, { 204, 85, 0 });
	}

	gState.window->draw(RItems.text_va, &RTextures.font);
	gState.window->display();
}

void DrawInstructionScreen3()
{
	gState.window->clear(sf::Color(255, 214, 135));

	ClearText();

	// Draw trivia instruction with flower icon
	RItems.buzzyfriends.setPosition(14 * TSIZE, 8 * TSIZE + YOFFSET);
	gState.window->draw(RItems.buzzyfriends);

	// Draw instruction text
	std::string line1 = "Answer a question correctly to get";
	std::string line2 = "a power-up! Then, my friends will";
	std::string line3 = "show up to help me pollinate!";

	MakeText(line1, 0.75, 18, sf::Color::Black);
	MakeText(line2, 1, 20, sf::Color::Black);
	MakeText(line3, 2, 22, sf::Color::Black);

	// Display button prompt - pulse this text
	if (IsPulse()) {
		MakeText("Press to continue", 7, 30, { 204, 85, 0 });
	}

	gState.window->draw(RItems.text_va, &RTextures.font);
	gState.window->display();
}

void DrawInstructionScreen4()
{
	gState.window->clear(sf::Color(255, 214, 135));

	ClearText();

	// Draw attention-grabbing text
	std::string line1 = "One last important detail...";

	MakeText(line1, 3, 15, sf::Color::Red);

	// Display button prompt - pulse this text
	if (IsPulse()) {
		MakeText("Press to continue", 7, 30, { 204, 85, 0 });
	}

	gState.window->draw(RItems.text_va, &RTextures.font);
	gState.window->display();
}

void DrawFinalInstructionScreen()
{
	// Draw the original menu screen but with warning text
	gState.window->clear(sf::Color(255, 214, 135));

	ClearText();

	// Draw warning text at top
	MakeText("STAY AWAY from the murder hornets.", 1, 3, sf::Color::Red);
	MakeText("They will try to eat me!", 4, 5, sf::Color::Red);
	MakeText("I can ONLY fight back with the help", 0.5, 7, sf::Color::Red);
	MakeText("of my friends!", 8, 9, sf::Color::Red);

	// Draw original menu content
	MakeText("-RIZZ LORD", 9, 12, { 255, 0, 0 });
	MakeText("-SKIBIDI TOILET", 9, 15, { 255, 181, 255 });
	MakeText("-QUANDALE DINGLE", 9, 18, { 10, 255, 255 });
	MakeText("-THE ALPHA", 9, 21, { 255, 187, 85 });
	MakeText("-BUZZY", 9, 24, { 204, 85, 0 });

	// Draw start prompt at bottom
	if (IsPulse()) {
		MakeText("Press the button to start Buzzy's day!", 0, 30, { 204, 85, 0 });
	}
	// Draw ghosts for the menu (hornets)
	for (int i = 0; i < 4; i++) {
		gState.ghosts[i]->enable_draw = true;
		gState.ghosts[i]->pos = { 8, 5.5f + i * 3.f + (float) 4.2};
		RItems.ghosts[i].setPosition(gState.ghosts[i]->pos.x * TSIZE, gState.ghosts[i]->pos.y * TSIZE + YOFFSET);
		RItems.ghosts[i].setTextureRect(GetGhostFrame(gState.ghosts[i]->type, gState.ghosts[i]->target_state, gState.ghosts[i]->cur_dir));
		gState.window->draw(RItems.ghosts[i]);
	}

	// Draw player (bee)
	gState.player->enable_draw = true;
	gState.player->pos = { 8, 17.5f + 4.2};
	RItems.player.setPosition(gState.player->pos.x * TSIZE, gState.player->pos.y * TSIZE + YOFFSET);
	RItems.player.setTextureRect(GetPacManFrame(gState.player->cur_dir));
	gState.window->draw(RItems.player);

	gState.window->draw(RItems.text_va, &RTextures.font);
	gState.window->display();
}

void ClearText()
{
	RItems.text_va.clear();
}
void MakeText(std::string string, float x, float y, sf::Color color)
{
	const int CHAR_WIDTH = 46;   // Width of each character cell in spritesheet
	const int CHAR_HEIGHT = 65;  // Height of each character cell in spritesheet

	// Increased character display size (previously 8x8)
	const int DISPLAY_WIDTH = 14;  // Doubled width for larger text
	const int DISPLAY_HEIGHT = 14; // Doubled height for larger text

	// Reduced spacing factor (previously multiplied by DISPLAY_WIDTH)
	const int SPACING = 6;  // Tighter spacing between characters

	// Direct mapping of characters to their positions in the spritesheet
	std::map<char, std::pair<int, int>> charMap;

	// First row (index 0,0 to 0,13)
	charMap['!'] = { 0, 0 };
	charMap['"'] = { 0, 1 };
	charMap['#'] = { 0, 2 };
	charMap['$'] = { 0, 3 };
	charMap['%'] = { 0, 4 };
	charMap['&'] = { 0, 5 };
	charMap['\''] = { 0, 6 };
	charMap['['] = { 0, 7 };
	charMap[']'] = { 0, 8 };
	charMap['*'] = { 0, 9 };
	charMap['+'] = { 0, 10 };
	charMap[','] = { 0, 11 };
	charMap['-'] = { 0, 12 };
	charMap['.'] = { 0, 13 };

	// Second row (index 1,0 to 1,13)
	charMap['/'] = { 1, 0 };
	charMap['0'] = { 1, 1 };
	charMap['1'] = { 1, 2 };
	charMap['2'] = { 1, 3 };
	charMap['3'] = { 1, 4 };
	charMap['4'] = { 1, 5 };
	charMap['5'] = { 1, 6 };
	charMap['6'] = { 1, 7 };
	charMap['7'] = { 1, 8 };
	charMap['8'] = { 1, 9 };
	charMap['9'] = { 1, 10 };
	charMap[':'] = { 1, 11 };
	charMap[';'] = { 1, 12 };
	charMap['<'] = { 1, 13 };

	// Third row
	charMap['='] = { 2, 0 };
	charMap['>'] = { 2, 1 };
	charMap['?'] = { 2, 2 };
	charMap['@'] = { 2, 3 };
	charMap['A'] = { 2, 4 };
	charMap['B'] = { 2, 5 };
	charMap['C'] = { 2, 6 };
	charMap['D'] = { 2, 7 };
	charMap['E'] = { 2, 8 };
	charMap['F'] = { 2, 9 };
	charMap['G'] = { 2, 10 };
	charMap['H'] = { 2, 11 };
	charMap['I'] = { 2, 12 };
	charMap['J'] = { 2, 13 };

	// Fourth row
	charMap['K'] = { 3, 0 };
	charMap['L'] = { 3, 1 };
	charMap['M'] = { 3, 2 };
	charMap['N'] = { 3, 3 };
	charMap['O'] = { 3, 4 };
	charMap['P'] = { 3, 5 };
	charMap['Q'] = { 3, 6 };
	charMap['R'] = { 3, 7 };
	charMap['S'] = { 3, 8 };
	charMap['T'] = { 3, 9 };
	charMap['U'] = { 3, 10 };
	charMap['V'] = { 3, 11 };
	charMap['W'] = { 3, 12 };
	charMap['X'] = { 3, 13 };

	// Fifth row
	charMap['Y'] = { 4, 0 };
	charMap['Z'] = { 4, 1 };
	charMap['a'] = { 4, 2 };
	charMap['b'] = { 4, 3 };
	charMap['c'] = { 4, 4 };
	charMap['d'] = { 4, 5 };
	charMap['e'] = { 4, 6 };
	charMap['f'] = { 4, 7 };
	charMap['g'] = { 4, 8 };
	charMap['h'] = { 4, 9 };
	charMap['i'] = { 4, 10 };
	charMap['j'] = { 4, 11 };
	charMap['k'] = { 4, 12 };
	charMap['l'] = { 4, 13 };

	// Sixth row
	charMap['m'] = { 5, 0 };
	charMap['n'] = { 5, 1 };
	charMap['o'] = { 5, 2 };
	charMap['p'] = { 5, 3 };
	charMap['q'] = { 5, 4 };
	charMap['r'] = { 5, 5 };
	charMap['s'] = { 5, 6 };
	charMap['t'] = { 5, 7 };
	charMap['u'] = { 5, 8 };
	charMap['v'] = { 5, 9 };
	charMap['w'] = { 5, 10 };
	charMap['x'] = { 5, 11 };
	charMap['y'] = { 5, 12 };
	charMap['z'] = { 5, 13 };

	// Special handling for space
	charMap[' '] = { 0, 0 };

	for (int i = 0; i < string.size(); i++) {
		char letter = string.at(i);

		// Handle spaces with reduced width
		if (letter == ' ') {
			continue; // Skip rendering, but position advances with spacing below
		}

		// Check if we have a mapping for this character
		if (charMap.find(letter) != charMap.end()) {
			sf::FloatRect font_let = { 0, 0, CHAR_WIDTH, CHAR_HEIGHT };

			// Get the row and column for this character
			int row = charMap[letter].first;
			int col = charMap[letter].second;

			// Calculate position in the spritesheet
			font_let.left = col * CHAR_WIDTH;
			font_let.top = row * CHAR_HEIGHT;

			// Draw the character with closer spacing
			MakeQuad(RItems.text_va, x * TSIZE + SPACING * i, y * TSIZE, DISPLAY_WIDTH, DISPLAY_HEIGHT, color, font_let);
		}
	}
}