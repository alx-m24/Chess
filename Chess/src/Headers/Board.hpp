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

enum GameState {
	PLAYING,
	STALEMATE,
	WHITEWIN,
	BLACKWIN
};

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
	GameState gamestate = PLAYING;
	unsigned long long whiteMoveMap;
	unsigned long long blackMoveMap;
	/* 8 - bit:
	*	8: Black in check
	*	7: White in check
	*	6: B_Rook2 first move
	*	5: B_Rook1 first move
	*	4: W_Rook2 first move
	*	3: W_Rook1 first move
	*	2: B_King first move
	*	1: W_King first move
	*/
	char flags;

public:
	bool whiteToMove = true;

public:
	Board();

public:
	void reset();
	void update();
	void display();
	void setFlags();
	void debugDisplay();
	void initMoveMaps();
	void detectEndGame();
	void setCheckedFlag();
	void set(std::string FEN);
	bool isChecked(bool white);
	bool isValidIdx(int i, int j);
	void updateMoveMap(bool white);
	char* getPiece(sf::Vector2f worldposition);
	sf::Vector2f getIdx(sf::Vector2f worldPosition);
	bool getMoves(char piece, const sf::Vector2i& idx);
	bool isThisMovedChecked(char piece, std::pair<char, bool>& square);
	bool getSquareFromMap(sf::Vector2i idx, const unsigned long long& map);
	void setSquareFromMap(bool val, sf::Vector2i idx, unsigned long long& map);
	void getAttacks(char piece, const sf::Vector2i& idx, unsigned long long& map);

private:
	void loadTextures();
};
