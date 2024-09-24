#include "Board.hpp"

Board::Board()
{
	this->reset();
	this->loadTextures();
}

void Board::set(std::string FEN)
{
	int x = 0, y = 0;
	for (const char& c : FEN) {
		if (x > 7) {
			x = 0;
			++y;
		}

		if (c == '/') {
			continue;
		}

		switch (c)
		{
		case 'p':
			board[x][y].first = -Pawn;
			++x;
			break;
		case 'n':
			board[x][y].first = -Knight;
			++x;
			break;
		case 'b':
			board[x][y].first = -Bishop;
			++x;
			break;
		case 'r':
			board[x][y].first = -Rook;
			++x;
			break;
		case 'q':
			board[x][y].first = -Queen;
			++x;
			break;
		case 'k':
			board[x][y].first = -King;
			++x;
			break;
		case 'P':
			board[x][y].first = Pawn;
			++x;
			break;
		case 'N':
			board[x][y].first = Knight;
			++x;
			break;
		case 'B':
			board[x][y].first = Bishop;
			++x;
			break;
		case 'R':
			board[x][y].first = Rook;
			++x;
			break;
		case 'Q':
			board[x][y].first = Queen;
			++x;
			break;
		case 'K':
			board[x][y].first = King;
			++x;
			break;
		default:
			if (!isalpha(c)) {
				int num = c - '0';
				for (int i = 0; i < num; ++i) {
					board[x][y].first = NONE;
					x += 1;
				}
			}
			break;
		}
	}

	initMoveMaps();
	setFlags();
}

bool Board::isChecked(bool white)
{
	return white ? (flags & 0b01000000) : (flags & 0b10000000);
}

void Board::reset()
{
	// Starting Pos
	this->set("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");

	// Puzzle
	//this->set("k1n3qr/p1pR1bpp/QrP5/1N/6/6P1/4bp1P/PPP2P2/1KR5");

	// Stalemate
	//this->set("7k/8/8/6Q1/8/8/8/K7");

	// Checkmate
	//this->set("8/5K1k/8/6Q1/8/8/8/8");
}

void Board::update()
{
#pragma region Condition
	if (gamestate != PLAYING) return;
#pragma endregion

#pragma region Input
	sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition(window));

	if (left && !lastLeft) {
		if (selected.piece == NONE) {
			char* piece = getPiece(mousePos);
			if (piece != nullptr) {
				if (((*piece < 0) && !whiteToMove) || ((*piece > 0) && whiteToMove)) {
					selected.piece = *piece;
					selected.pos = getIdx(mousePos);
					*piece = NONE;

					getMoves(selected.piece, sf::Vector2i(selected.pos));
				}
			}
		}
	}
	else if (lastLeft && !left) {
		char* piece = getPiece(mousePos);
		if (piece == nullptr) {
			if (selected.piece != NONE)
				board[selected.pos.x][selected.pos.y].first = selected.piece;
		}
		else if (board[getIdx(mousePos).x][getIdx(mousePos).y].second == false) {
			board[selected.pos.x][selected.pos.y].first = selected.piece;
		}
		else {
			*piece = selected.piece;

			// Updating flags
			if (selected.piece == Rook) {
				// Rook 1
				if (selected.pos.x == 0) {
					flags &= 0b11111011;
				}
				// Rook 2
				else {
					flags &= 0b11110111;
				}
			}
			else if (selected.piece == -Rook) {
				// Rook 1
				if (selected.pos.x == 0) {
					flags &= 0b11101111;
				}
				// Rook 2
				else {
					flags &= 0b11011111;
				}
			}
			else if (selected.piece == King) {
				flags &= 0b11111110;
			}
			else if (selected.piece == -King) {
				flags &= 0b11111101;
			}

			whiteToMove = !whiteToMove;

			updateMoveMap(true);
			updateMoveMap(false);

			setCheckedFlag();
			detectEndGame();

			if (gamestate == STALEMATE) std::cout << "\nDraw" << std::endl;
			else if (gamestate == WHITEWIN) std::cout << "\nWhite Win" << std::endl;
			else if (gamestate == BLACKWIN) std::cout << "\Black Win" << std::endl;
			else if (gamestate == PLAYING) std::cout << "\nPlaying" << std::endl;
		}
		selected = { NONE, {-1.0f, -1.0f} };
	}
#pragma endregion
}

void Board::display()
{
#pragma region Variables
	sf::Vector2f float_winSize = { static_cast<float>(windowSize.x), static_cast<float>(windowSize.y) };

	boardSize.x = float_winSize.y * (9.0f / 10.0f);
	boardSize.y = boardSize.x;

	slotSize = sf::Vector2f(boardSize) / 8.0f;
	origin = (float_winSize / 2.0f) - (boardSize / 2.0f);
	sf::Vector2f position = origin;
#pragma endregion

#pragma region Outline
	float outlineThickness = 8.0f;

	sf::RectangleShape outline(boardSize + sf::Vector2f(2.0f * outlineThickness, 2.0f * outlineThickness));

	outline.setPosition(origin - sf::Vector2f(outlineThickness, outlineThickness));
	outline.setFillColor(sf::Color(138, 71, 33));

	window.draw(outline);
#pragma endregion

#pragma region Board
	bool black = false;
	for (std::array<std::pair<char, bool>, 8>&row : board) {
		for (std::pair<char, bool>& slot : row) {
			sf::RectangleShape rect(slotSize);
			sf::Color color = black ? sf::Color(0, 124, 133) : sf::Color::White;

			if (slot.first == King) {
				if (isChecked(true))
					color = sf::Color::Red;
			}
			else if (slot.first == -King) {
				if (isChecked(false))
					color = sf::Color::Red;
			}

			rect.setPosition(position);
			rect.setFillColor(color);

			window.draw(rect);

			displayPiece(slot.first, position, slotSize);

			if (slot.second == true) {
				float radius = slotSize.x / 9.0f;
				sf::CircleShape selected(radius);

				selected.setOrigin(radius, radius);
				selected.setPosition(position + (slotSize / 2.0f));
				selected.setFillColor(sf::Color(38, 255, 38, 200));

				window.draw(selected);
			}

			black = !black;
			position.y += slotSize.y;
		}
		black = !black;

		position.y = origin.y;
		position.x += slotSize.x;
	}

	if (selected.piece != NONE) {
		displayPiece(selected.piece, sf::Vector2f(sf::Mouse::getPosition(window)), slotSize, true);
	}
#pragma endregion
}

void Board::setFlags()
{
	flags = 0b00111111;
}

void Board::debugDisplay()
{
	std::cout << "\n";
	for (int i = 0; i < board.size(); ++i) {
		std::cout << i + 1 << ": ";
		for (int j = 0; j < board.size(); ++j) {
			if (board[j][i].first < 0) {
				std::cout << '-' << char(-board[j][i].first + '0') << ' ';
			}
			else {
				std::cout << ' ' << char(board[j][i].first + '0') << ' ';
			}
		}
		std::cout << "\n";
	}

	std::cout << "\n    ";
	for (int i = 0; i < board.size(); ++i) {
		std::cout << char('a' + i) << "  ";
	}

	std::cout << std::endl;

	std::cout << "WhiteMoves: " << std::endl;
	for (int i = 0; i < board.size(); ++i) {
		std::cout << i + 1 << ": ";
		for (int j = 0; j < board.size(); ++j) {
			std::cout << getSquareFromMap({ j, i }, whiteMoveMap) << " ";
		}
		std::cout << "\n";
	}

	std::cout << "\nBlackMoves: " << std::endl;
	for (int i = 0; i < board.size(); ++i) {
		std::cout << i + 1 << ": ";
		for (int j = 0; j < board.size(); ++j) {
			std::cout << getSquareFromMap({ j, i }, blackMoveMap) << " ";
		}
		std::cout << "\n";
	}
}

void Board::initMoveMaps()
{
	updateMoveMap(true);
	updateMoveMap(false);
}

void Board::detectEndGame()
{
	// store initial board
	std::array<std::array<std::pair<char, bool>, 8>, 8> temp_board = board;
	unsigned long long temp_whiteMoveMap = whiteMoveMap;
	unsigned long long temp_blackMoveMap = blackMoveMap;
	char temp_flags = flags;

	bool canWhiteMove = false;
	bool canBlackMove = false;

	for (int i = 0; i < 8; ++i) {
		for (int j = 0; j < 8; ++j) {
			temp_board[i][j].second = false; // reset moves
			char piece = temp_board[i][j].first;
			if (piece < 0) {
				if (canBlackMove) continue;

				selected.pos = sf::Vector2f(i, j);
				if (getMoves(piece, { i, j })) canBlackMove = true;
			}
			else if (piece > 0) {
				if (canWhiteMove) continue;

				selected.pos = sf::Vector2f(i, j);
				if (getMoves(piece, { i, j })) canWhiteMove = true;
			}
		}
	}
	
	// restore initial board
	board = temp_board;
	whiteMoveMap = temp_whiteMoveMap;
	blackMoveMap = temp_blackMoveMap;
	flags = temp_flags;

	bool isWhiteChecked = isChecked(true);
	bool isBlackChecked = isChecked(false);

	std::cout << "\nCan black move: " << canBlackMove << std::endl;

	if		(!canWhiteMove && isWhiteChecked) gamestate = BLACKWIN;
	else if (!canBlackMove && isBlackChecked) gamestate = WHITEWIN;
	else if (!canWhiteMove && !isWhiteChecked) gamestate = STALEMATE;
	else if (!canBlackMove && !isBlackChecked) gamestate = STALEMATE;
	else gamestate = PLAYING;
}

void Board::setCheckedFlag()
{
	for (int i = 0; i < 8; ++i) {
		for (int j = 0; j < 8; ++j) {
			// White
			if (board[i][j].first == King) {
				if (getSquareFromMap({ i, j }, blackMoveMap)) {
					flags |= 0b01000000;
				}
				else {
					flags &= 0b10111111;
				}
			}
			// Black
			if (board[i][j].first == -King) {
				if (getSquareFromMap({ i, j }, whiteMoveMap)) {
					flags |= 0b10000000;
				}
				else {
					flags &= 0b01111111;
				}
			}
		}
	}
}

bool Board::isValidIdx(int i, int j)
{
	return i >= 0 && i < 8 && j >= 0 && j < 8;;
}

bool Board::getMoves(char piece, const sf::Vector2i& idx)
{
	bool canMove = false;

#pragma region Sliding Pieces
	char unsignedPiece = (piece < 0) ? -piece : piece;

	if (unsignedPiece == Rook || unsignedPiece == Queen) {
		// Left
		for (int i = idx.x - 1; i >= 0; --i) {
			std::pair<char, bool>& square = board[i][idx.y];
			if (square.first == NONE) {
				if (!isThisMovedChecked(piece, square)) {
					square.second = true;
					canMove = true;
				}
				continue;
			}
			if (getSign(piece) == getSign(square.first)) {
				break;
			}
			else {
				if (!isThisMovedChecked(piece, square)) {
					square.second = true;
					canMove = true;
				}
				break;
			}
		}
		// Right
		for (int i = idx.x + 1; i < 8; ++i) {
			std::pair<char, bool>& square = board[i][idx.y];
			if (square.first == NONE) {
				if (!isThisMovedChecked(piece, square)) {
					square.second = true;
					canMove = true;
				}
				continue;
			}
			if (getSign(piece) == getSign(square.first)) {
				break;
			}
			else {
				if (!isThisMovedChecked(piece, square)) {
					square.second = true;
					canMove = true;
				}
				break;
			}
		}
		// Up
		for (int i = idx.y - 1; i >= 0; --i) {
			std::pair<char, bool>& square = board[idx.x][i];
			if (square.first == NONE) {
				if (!isThisMovedChecked(piece, square)) {
					square.second = true;
					canMove = true;
				}
				continue;
			}
			if (getSign(piece) == getSign(square.first)) {
				break;
			}
			else {
				if (!isThisMovedChecked(piece, square)) {
					square.second = true;
					canMove = true;
				}
				break;
			}
		}
		// Down
		for (int i = idx.y + 1; i < 8; ++i) {
			std::pair<char, bool>& square = board[idx.x][i];
			if (square.first == NONE) {
				if (!isThisMovedChecked(piece, square)) {
					square.second = true;
					canMove = true;
				}
				continue;
			}
			if (getSign(piece) == getSign(square.first)) {
				break;
			}
			else {
				if (!isThisMovedChecked(piece, square)) {
					square.second = true;
					canMove = true;
				}
				break;
			}
		}
	}
	if (unsignedPiece == Bishop || unsignedPiece == Queen) {
		// Left and Up
		for (int i = -1; isValidIdx(idx.x + i, idx.y + i); --i) {
			std::pair<char, bool>& square = board[idx.x + i][idx.y + i];
			if (square.first == NONE) {
				if (!isThisMovedChecked(piece, square)) {
					square.second = true;
					canMove = true;
				}
				continue;
			}
			if (getSign(piece) == getSign(square.first)) {
				break;
			}
			else {
				if (!isThisMovedChecked(piece, square)) {
					square.second = true;
					canMove = true;
				}
				break;
			}
		}
		// Right and Down
		for (int i = 1; isValidIdx(idx.x + i, idx.y + i); ++i) {
			std::pair<char, bool>& square = board[idx.x + i][idx.y + i];
			if (square.first == NONE) {
				if (!isThisMovedChecked(piece, square)) {
					square.second = true;
					canMove = true;
				}
				continue;
			}
			if (getSign(piece) == getSign(square.first)) {
				break;
			}
			else {
				if (!isThisMovedChecked(piece, square)) {
					square.second = true;
					canMove = true;
				}
				break;
			}
		}
		// Left and Down
		for (int i = -1; isValidIdx(idx.x + i, idx.y - i); --i) {
			std::pair<char, bool>& square = board[idx.x + i][idx.y - i];
			if (square.first == NONE) {
				if (!isThisMovedChecked(piece, square)) {
					square.second = true;
					canMove = true;
				}
				continue;
			}
			if (getSign(piece) == getSign(square.first)) {
				break;
			}
			else {
				if (!isThisMovedChecked(piece, square)) {
					square.second = true;
					canMove = true;
				}
				break;
			}
		}
		// Right and Up
		for (int i = 1; isValidIdx(idx.x + i, idx.y - i); ++i) {
			std::pair<char, bool>& square = board[idx.x + i][idx.y - i];
			if (square.first == NONE) {
				if (!isThisMovedChecked(piece, square)) {
					square.second = true;
					canMove = true;
				}
				continue;
			}
			if (getSign(piece) == getSign(square.first)) {
				break;
			}
			else {
				if (!isThisMovedChecked(piece, square)) {
					square.second = true;
					canMove = true;
				}
				break;
			}
		}
	}
#pragma endregion

#pragma region Pawns
	// White
	if (piece == Pawn) {
		int max = (idx.y == 6) ? 2 : 1;
		for (int i = idx.y - 1; i >= idx.y - max; --i) {
			std::pair<char, bool>& square = board[idx.x][i];
			if (square.first != NONE) {
				break;
			}
			if (!isThisMovedChecked(piece, square)) {
				square.second = true;
				canMove = true;
			}
		}
		// Attack
		if (idx.y - 1 >= 0) {
			if (idx.x - 1 >= 0) {
				std::pair<char, bool>& square = board[idx.x - 1][idx.y - 1];
				if (square.first != NONE) {
					if (getSign(piece) != getSign(square.first)) {
						if (!isThisMovedChecked(piece, square)) {
							square.second = true;
							canMove = true;
						}
					}
				}
			}
			if (idx.x + 1 < 8) {
				std::pair<char, bool>& square = board[idx.x + 1][idx.y - 1];
				if (square.first != NONE) {
					if (getSign(piece) != getSign(square.first)) {
						if (!isThisMovedChecked(piece, square)) {
							square.second = true;
							canMove = true;
						}
					}
				}
			}
		}
	}
	// Black
	else if (piece == -Pawn) {
		int max = (idx.y == 1) ? 2 : 1;
		for (int i = idx.y + 1; i <= idx.y + max; ++i) {
			std::pair<char, bool>& square = board[idx.x][i];
			if (square.first != NONE) {
				break;
			}
			if (!isThisMovedChecked(piece, square)) {
				square.second = true;
				canMove = true;
			}
		}
		// Attack
		if (idx.y + 1 < 8) {
			if (idx.x - 1 >= 0) {
				std::pair<char, bool>& square = board[idx.x - 1][idx.y + 1];
				if (square.first != NONE) {
					if (getSign(piece) != getSign(square.first)) {
						if (!isThisMovedChecked(piece, square)) {
							square.second = true;
							canMove = true;
						}
					}
				}
			}
			if (idx.x + 1 < 8) {
				std::pair<char, bool>& square = board[idx.x + 1][idx.y + 1];
				if (square.first != NONE) {
					if (getSign(piece) != getSign(square.first)) {
						if (!isThisMovedChecked(piece, square)) {
							square.second = true;
							canMove = true;
						}
					}
				}
			}
		}
	}
#pragma endregion

#pragma region Knights
	if (unsignedPiece == Knight) {

		if (idx.x - 1 >= 0) {
			if (idx.y - 2 >= 0) {
				std::pair<char, bool>& square = board[idx.x - 1][idx.y - 2];
				if (square.first == NONE || (getSign(piece) != getSign(square.first))) {
					if (!isThisMovedChecked(piece, square)) {
						square.second = true;
						canMove = true;
					}
				}
			}
			if (idx.y + 2 < 8) {
				std::pair<char, bool>& square = board[idx.x - 1][idx.y + 2];
				if (square.first == NONE || (getSign(piece) != getSign(square.first))) {
					if (!isThisMovedChecked(piece, square)) {
						square.second = true;
						canMove = true;
					}
				}
			}
		}
		if (idx.x + 1 < 8) {
			if (idx.y - 2 >= 0) {
				std::pair<char, bool>& square = board[idx.x + 1][idx.y - 2];
				if (square.first == NONE || (getSign(piece) != getSign(square.first))) {
					if (!isThisMovedChecked(piece, square)) {
						square.second = true;
						canMove = true;
					}
				}
			}
			if (idx.y + 2 < 8) {
				std::pair<char, bool>& square = board[idx.x + 1][idx.y + 2];
				if (square.first == NONE || (getSign(piece) != getSign(square.first))) {
					if (!isThisMovedChecked(piece, square)) {
						square.second = true;
						canMove = true;
					}
				}
			}
		}

		if (idx.x - 2 >= 0) {
			if (idx.y - 1 >= 0) {
				std::pair<char, bool>& square = board[idx.x - 2][idx.y - 1];
				if (square.first == NONE || (getSign(piece) != getSign(square.first))) {
					if (!isThisMovedChecked(piece, square)) {
						square.second = true;
						canMove = true;
					}
				}
			}
			if (idx.y + 1 < 8) {
				std::pair<char, bool>& square = board[idx.x - 2][idx.y + 1];
				if (square.first == NONE || (getSign(piece) != getSign(square.first))) {
					if (!isThisMovedChecked(piece, square)) {
						square.second = true;
						canMove = true;
					}
				}
			}
		}
		if (idx.x + 2 < 8) {
			if (idx.y - 1 >= 0) {
				std::pair<char, bool>& square = board[idx.x + 2][idx.y - 1];
				if (square.first == NONE || (getSign(piece) != getSign(square.first))) {
					if (!isThisMovedChecked(piece, square)) {
						square.second = true;
						canMove = true;
					}
				}
			}
			if (idx.y + 1 < 8) {
				std::pair<char, bool>& square = board[idx.x + 2][idx.y + 1];
				if (square.first == NONE || (getSign(piece) != getSign(square.first))) {
					if (!isThisMovedChecked(piece, square)) {
						square.second = true;
						canMove = true;
					}
				}
			}
		}
	}
#pragma endregion

#pragma region king
	if (unsignedPiece == King) {
		unsigned long long& map = (piece < 0) ? whiteMoveMap : blackMoveMap;

		if (idx.y - 1 >= 0) {
			std::pair<char, bool>& square = board[idx.x][idx.y - 1];
			if (square.first == NONE || (getSign(piece) != getSign(square.first))) {
				if (!getSquareFromMap({ idx.x, idx.y - 1 }, map)) {
					if (!isThisMovedChecked(piece, square)) {
						square.second = true;
						canMove = true;
					}
				}
			}
			if (idx.x - 1 >= 0) {
				std::pair<char, bool>&  square = board[idx.x - 1][idx.y - 1];
				if (square.first == NONE || (getSign(piece) != getSign(square.first))) {
					if (!getSquareFromMap({ idx.x - 1, idx.y - 1 }, map)) {
						if (!isThisMovedChecked(piece, square)) {
							square.second = true;
							canMove = true;
						}
					}
				}
			}
			if (idx.x + 1 < 8) {
				std::pair<char, bool>& square = board[idx.x + 1][idx.y - 1];
				if (square.first == NONE || (getSign(piece) != getSign(square.first))) {
					if (!getSquareFromMap({ idx.x + 1, idx.y - 1 }, map)) {
						if (!isThisMovedChecked(piece, square)) {
							square.second = true;
							canMove = true;
						}
					}
				}
			}
		}
		if (idx.y + 1 < 8) {
			std::pair<char, bool>& square = board[idx.x][idx.y + 1];
			if (square.first == NONE || (getSign(piece) != getSign(square.first))) {
				if (!getSquareFromMap({ idx.x, idx.y + 1 }, map)) {
					if (!isThisMovedChecked(piece, square)) {
						square.second = true;
						canMove = true;
					}
				}
			}

			if (idx.x - 1 >= 0) {
				std::pair<char, bool>& square = board[idx.x - 1][idx.y + 1];
				if (square.first == NONE || (getSign(piece) != getSign(square.first))) {
					if (!getSquareFromMap({ idx.x - 1, idx.y + 1 }, map)) {
						if (!isThisMovedChecked(piece, square)) {
							square.second = true;
							canMove = true;
						}
					}
				}
			}
			if (idx.x + 1 < 8) {
				std::pair<char, bool>& square = board[idx.x + 1][idx.y + 1];
				if (square.first == NONE || (getSign(piece) != getSign(square.first))) {
					if (!getSquareFromMap({ idx.x + 1, idx.y + 1 }, map)) {
						if (!isThisMovedChecked(piece, square)) {
							square.second = true;
							canMove = true;
						}
					}
				}
			}
		}
		if (idx.x - 1 >= 0) {
			std::pair<char, bool>& square = board[idx.x - 1][idx.y];
			if (square.first == NONE || (getSign(piece) != getSign(square.first))) {
				if (!getSquareFromMap({ idx.x - 1, idx.y }, map)) {
					if (!isThisMovedChecked(piece, square)) {
						square.second = true;
						canMove = true;
					}
				}
			}
		}
		if (idx.x + 1 < 8) {
			std::pair<char, bool>& square = board[idx.x + 1][idx.y];
			if (square.first == NONE || (getSign(piece) != getSign(square.first))) {
				if (!getSquareFromMap({ idx.x + 1, idx.y }, map)) {
					if (!isThisMovedChecked(piece, square)) {
						square.second = true;
						canMove = true;
					}
				}
			}
		}
	}
#pragma endregion

	return canMove;
}

bool Board::isThisMovedChecked(char piece, std::pair<char, bool>& square)
{
	bool ans = false;
	bool white = piece > 0;

	// store initial conditions(board, flags & maps)
	std::array<std::array<std::pair<char, bool>, 8>, 8> temp_board = board;
	unsigned long long temp_whiteMoveMap = whiteMoveMap;
	unsigned long long temp_blackMoveMap = blackMoveMap;
	char temp_flags = flags;

	// set piece to new position
	board[selected.pos.x][selected.pos.y].first = NONE;
	square.first = piece;

	// check if still checked
	updateMoveMap(!white);
	setCheckedFlag();
	ans = isChecked(white);

	// restore initial conditions
	board = temp_board;
	whiteMoveMap = temp_whiteMoveMap;
	blackMoveMap = temp_blackMoveMap;
	flags = temp_flags;

	return ans;
}

void Board::getAttacks(char piece, const sf::Vector2i& idx, unsigned long long& map)
{
#pragma region Sliding Pieces
	char unsignedPiece = (piece < 0) ? -piece : piece;

	if (unsignedPiece == Rook || unsignedPiece == Queen) {
		// Left
		for (int i = idx.x - 1; i >= 0; --i) {
			std::pair<char, bool>& square = board[i][idx.y];
			if (square.first == NONE) {
				setSquareFromMap(1, { i, idx.y }, map);
				continue;
			}
			if (getSign(piece) == getSign(square.first)) {
				break;
			}
			else {
				setSquareFromMap(1, { i, idx.y }, map);
				break;
			}
		}
		// Right
		for (int i = idx.x + 1; i < 8; ++i) {
			std::pair<char, bool>& square = board[i][idx.y];
			if (square.first == NONE) {
				setSquareFromMap(1, { i, idx.y }, map);
				continue;
			}
			if (getSign(piece) == getSign(square.first)) {
				break;
			}
			else {
				setSquareFromMap(1, { i, idx.y }, map);
				break;
			}
		}
		// Up
		for (int i = idx.y - 1; i >= 0; --i) {
			std::pair<char, bool>& square = board[idx.x][i];
			if (square.first == NONE) {
				setSquareFromMap(1, { idx.x, i }, map);
				continue;
			}
			if (getSign(piece) == getSign(square.first)) {
				break;
			}
			else {
				setSquareFromMap(1, { idx.x, i }, map);
				break;
			}
		}
		// Down
		for (int i = idx.y + 1; i < 8; ++i) {
			std::pair<char, bool>& square = board[idx.x][i];
			if (square.first == NONE) {
				setSquareFromMap(1, { idx.x, i }, map);
				continue;
			}
			if (getSign(piece) == getSign(square.first)) {
				break;
			}
			else {
				setSquareFromMap(1, { idx.x, i }, map);
				break;
			}
		}
	}
	if (unsignedPiece == Bishop || unsignedPiece == Queen) {
		// Left and Up
		for (int i = -1; isValidIdx(idx.x + i, idx.y + i); --i) {
			std::pair<char, bool>& square = board[idx.x + i][idx.y + i];
			if (square.first == NONE) {
				setSquareFromMap(1, { idx.x + i, idx.y + i }, map);
				continue;
			}
			if (getSign(piece) == getSign(square.first)) {
				break;
			}
			else {
				setSquareFromMap(1, { idx.x + i, idx.y + i }, map);
				break;
			}
		}
		// Right and Down
		for (int i = 1; isValidIdx(idx.x + i, idx.y + i); ++i) {
			std::pair<char, bool>& square = board[idx.x + i][idx.y + i];
			if (square.first == NONE) {
				setSquareFromMap(1, { idx.x + i, idx.y + i }, map);
				continue;
			}
			if (getSign(piece) == getSign(square.first)) {
				break;
			}
			else {
				setSquareFromMap(1, { idx.x + i, idx.y + i }, map);
				break;
			}
		}
		// Left and Down
		for (int i = -1; isValidIdx(idx.x + i, idx.y - i); --i) {
			std::pair<char, bool>& square = board[idx.x + i][idx.y - i];
			if (square.first == NONE) {
				setSquareFromMap(1, { idx.x + i, idx.y - i }, map);
				continue;
			}
			if (getSign(piece) == getSign(square.first)) {
				break;
			}
			else {
				setSquareFromMap(1, { idx.x + i, idx.y - i }, map);
				break;
			}
		}
		// Right and Up
		for (int i = 1; isValidIdx(idx.x + i, idx.y - i); ++i) {
			std::pair<char, bool>& square = board[idx.x + i][idx.y - i];
			if (square.first == NONE) {
				setSquareFromMap(1, { idx.x + i, idx.y - i }, map);
				continue;
			}
			if (getSign(piece) == getSign(square.first)) {
				break;
			}
			else {
				setSquareFromMap(1, { idx.x + i, idx.y - i }, map);
				break;
			}
		}
	}
#pragma endregion

#pragma region Pawns
	// White
	if (piece == Pawn) {
		// Attack
		if (idx.y - 1 >= 0) {
			if (idx.x - 1 >= 0) {
				//std::pair<char, bool>& square = board[idx.x - 1][idx.y - 1];
				setSquareFromMap(1, { idx.x - 1, idx.y - 1 }, map);
			}
			if (idx.x + 1 < 8) {
				//std::pair<char, bool>& square = board[idx.x + 1][idx.y - 1];
				setSquareFromMap(1, { idx.x + 1, idx.y - 1 }, map);
			}
		}
	}
	// Black
	else if (piece == -Pawn) {
		// Attack
		if (idx.y + 1 < 8) {
			if (idx.x - 1 >= 0) {
				//std::pair<char, bool>& square = board[idx.x - 1][idx.y + 1];
				setSquareFromMap(1, { idx.x - 1, idx.y + 1 }, map);
			}
			if (idx.x + 1 < 8) {
				//std::pair<char, bool>& square = board[idx.x + 1][idx.y + 1];
				setSquareFromMap(1, { idx.x + 1, idx.y + 1 }, map);
			}
		}
	}
#pragma endregion

#pragma region Knights
	if (unsignedPiece == Knight) {

		if (idx.x - 1 >= 0) {
			if (idx.y - 2 >= 0) {
				std::pair<char, bool>& square = board[idx.x - 1][idx.y - 2];
				if (square.first == NONE || (getSign(piece) != getSign(square.first))) {
					setSquareFromMap(1, { idx.x - 1, idx.y - 2 }, map);
				}
			}
			if (idx.y + 2 < 8) {
				std::pair<char, bool>& square = board[idx.x - 1][idx.y + 2];
				if (square.first == NONE || (getSign(piece) != getSign(square.first))) {
					setSquareFromMap(1, { idx.x - 1, idx.y + 2 }, map);
				}
			}
		}
		if (idx.x + 1 < 8) {
			if (idx.y - 2 >= 0) {
				std::pair<char, bool>& square = board[idx.x + 1][idx.y - 2];
				if (square.first == NONE || (getSign(piece) != getSign(square.first))) {
					setSquareFromMap(1, { idx.x + 1, idx.y - 2 }, map);
				}
			}
			if (idx.y + 2 < 8) {
				std::pair<char, bool>& square = board[idx.x + 1][idx.y + 2];
				if (square.first == NONE || (getSign(piece) != getSign(square.first))) {
					setSquareFromMap(1, { idx.x - 1, idx.y + 2 }, map);
				}
			}
		}

		if (idx.x - 2 >= 0) {
			if (idx.y - 1 >= 0) {
				std::pair<char, bool>& square = board[idx.x - 2][idx.y - 1];
				if (square.first == NONE || (getSign(piece) != getSign(square.first))) {
					setSquareFromMap(1, { idx.x - 2, idx.y - 1 }, map);
				}
			}
			if (idx.y + 1 < 8) {
				std::pair<char, bool>& square = board[idx.x - 2][idx.y + 1];
				if (square.first == NONE || (getSign(piece) != getSign(square.first))) {
					setSquareFromMap(1, { idx.x - 2, idx.y + 1 }, map);
				}
			}
		}
		if (idx.x + 2 < 8) {
			if (idx.y - 1 >= 0) {
				std::pair<char, bool>& square = board[idx.x + 2][idx.y - 1];
				if (square.first == NONE || (getSign(piece) != getSign(square.first))) {
					setSquareFromMap(1, { idx.x + 2, idx.y - 1 }, map);
				}
			}
			if (idx.y + 1 < 8) {
				std::pair<char, bool>& square = board[idx.x + 2][idx.y + 1];
				if (square.first == NONE || (getSign(piece) != getSign(square.first))) {
					setSquareFromMap(1, { idx.x + 2, idx.y + 1 }, map);
				}
			}
		}
	}
#pragma endregion

#pragma region king
	if (unsignedPiece == King) {
		if (idx.y - 1 >= 0) {
			std::pair<char, bool>& square = board[idx.x][idx.y - 1];
			if (square.first == NONE || (getSign(piece) != getSign(square.first))) {
				setSquareFromMap(1, { idx.x, idx.y - 1 }, map);
			}

			if (idx.x - 1 >= 0) {
				std::pair<char, bool>& square = board[idx.x - 1][idx.y - 1];
				if (square.first == NONE || (getSign(piece) != getSign(square.first))) {
					setSquareFromMap(1, { idx.x - 1, idx.y - 1 }, map);
				}
			}
			if (idx.x + 1 < 8) {
				std::pair<char, bool>& square = board[idx.x + 1][idx.y - 1];
				if (square.first == NONE || (getSign(piece) != getSign(square.first))) {
					setSquareFromMap(1, { idx.x + 1, idx.y - 1 }, map);
				}
			}
		}
		if (idx.y + 1 < 8) {
			std::pair<char, bool>& square = board[idx.x][idx.y + 1];
			if (square.first == NONE || (getSign(piece) != getSign(square.first))) {
				setSquareFromMap(1, { idx.x, idx.y + 1 }, map);
			}

			if (idx.x - 1 >= 0) {
				std::pair<char, bool>& square = board[idx.x - 1][idx.y + 1];
				if (square.first == NONE || (getSign(piece) != getSign(square.first))) {
					setSquareFromMap(1, { idx.x - 1, idx.y + 1 }, map);
				}
			}
			if (idx.x + 1 < 8) {
				std::pair<char, bool>& square = board[idx.x + 1][idx.y + 1];
				if (square.first == NONE || (getSign(piece) != getSign(square.first))) {
					setSquareFromMap(1, { idx.x + 1, idx.y + 1 }, map);
				}
			}
		}
		if (idx.x - 1 >= 0) {
			std::pair<char, bool>& square = board[idx.x - 1][idx.y];
			if (square.first == NONE || (getSign(piece) != getSign(square.first))) {
				setSquareFromMap(1, { idx.x - 1, idx.y }, map);
			}
		}
		if (idx.x + 1 < 8) {
			std::pair<char, bool>& square = board[idx.x + 1][idx.y];
			if (square.first == NONE || (getSign(piece) != getSign(square.first))) {
				setSquareFromMap(1, { idx.x + 1, idx.y }, map);
			}
		}
	}
#pragma endregion
}

char* Board::getPiece(sf::Vector2f worldposition)
{
	sf::Vector2f idx = getIdx(worldposition);

	if (idx.x >= 0.0f && idx.x < 8.0f && idx.y >= 0.0f && idx.y < 8.0f) {
		return &board[idx.x][idx.y].first;
	}
	
	return nullptr;
}

bool Board::getSquareFromMap(sf::Vector2i idx, const unsigned long long& map)
{
	int positionalVal = idx.x + 8 * idx.y;
	return static_cast<bool>(map & static_cast<unsigned long long>(pow(2, positionalVal)));
}

void Board::setSquareFromMap(bool val, sf::Vector2i idx, unsigned long long& map)
{
	int positionalVal = idx.x + 8 * idx.y;
	unsigned long long num = static_cast<unsigned long long>(pow(2, positionalVal));
	if (val) {
		map = map | num;
	}
	else {
		num = ~num;
		map = map & num;
	}
}

sf::Vector2f Board::getIdx(sf::Vector2f worldPosition)
{
	sf::Vector2f relativeMousePos = worldPosition - origin;
	return sf::Vector2f(relativeMousePos.x / slotSize.x, relativeMousePos.y / slotSize.y);
}

void Board::updateMoveMap(bool white)
{
	// Getting map
	unsigned long long& map = white ? whiteMoveMap : blackMoveMap;
	map = 0u;

	// Getting all possible moves
	for (int i = 0; i < 8; ++i) {
		for (int j = 0; j < 8; ++j) {
			char piece = board[i][j].first;
			if (piece == NONE) continue; // If no piece
			if (white) {
				if (piece < 0) continue; // If we are checking white but piece is black
			}
			else {
				if (piece > 0) continue; // Vice-versa
			}
			getAttacks(piece, { i, j }, map);
		}
	}
}

void Board::loadTextures()
{
	const std::string path = "C:\\Users\\alexa\\OneDrive\\Coding\\C++\\Chess\\Chess\\res\\Textures\\";

	textures[NONE] = sf::Texture(); // Empty Texture
	textures[Pawn].loadFromFile(path + "/white-pawn.png");
	textures[Knight].loadFromFile(path + "/white-knight.png");
	textures[Bishop].loadFromFile(path + "/white-bishop.png");
	textures[Rook].loadFromFile(path + "/white-rook.png");
	textures[Queen].loadFromFile(path + "/white-queen.png");
	textures[King].loadFromFile(path + "/white-king.png");

	textures[Pawn + BLACK].loadFromFile(path + "/black-pawn.png");
	textures[Knight + BLACK].loadFromFile(path + "/black-knight.png");
	textures[Bishop + BLACK].loadFromFile(path + "/black-bishop.png");
	textures[Rook + BLACK].loadFromFile(path + "/black-rook.png");
	textures[Queen + BLACK].loadFromFile(path + "/black-queen.png");
	textures[King + BLACK].loadFromFile(path + "/black-king.png");

	for (auto& tex : textures) {
		tex.setSmooth(true);
	}
}
