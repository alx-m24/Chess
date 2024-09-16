#pragma once
#include <set>
#include <array>
#include <string>
#include <iostream>
#include "Pieces.hpp"
#include "Resources/Resources.hpp"

/*
* Board:
*	- 8x8
*	- Each case/position has 7 possible states:
*		- Empty
*		- Occupied by:
*			- Pawn
*			- Rook
*			- etc
*/

struct Selected {
	char piece = NONE;
	sf::Vector2f pos = {-1.0f, -1.0f};
};

class Board
{
private:
	std::array<std::array<std::pair<char, bool>, 8>, 8> board; // Negative if black
	sf::Vector2f boardSize;
	sf::Vector2f slotSize;
	sf::Vector2f origin;

private:
	Selected selected;

public:
	bool whiteToMove = true;

public:
	Board();

public:
	void set(std::string FEN);
	void reset();
	void update();
	void display();
	void debugDisplay();
	bool isValidIdx(int i, int j);
	void setSelected(char piece, const sf::Vector2i& idx);
	char* getPiece(sf::Vector2f worldposition);
	sf::Vector2f getIdx(sf::Vector2f worldPosition);

private:
	void loadTextures();
};
