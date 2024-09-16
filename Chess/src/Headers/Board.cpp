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
}

void Board::reset()
{
	this->set("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
}

void Board::update()
{
#pragma region test
	/*this->reset();

	sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition(window));
	char* piece = getPiece(mousePos);
	if (piece != nullptr) *piece = King;*/
#pragma endregion

#pragma region Selected Piece
	if (selected.piece != NONE) {
		setSelected(selected.piece, sf::Vector2i(selected.pos));
	}
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

			whiteToMove = !whiteToMove;

			std::cout << "\033[2J\033[1;1H";
			debugDisplay();
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

			rect.setPosition(position);
			rect.setFillColor(black ? sf::Color(0, 124, 133) : sf::Color::White);

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

			slot.second = false; // Resetting the square to NOT be selected
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
}

bool Board::isValidIdx(int i, int j)
{
	return i >= 0 && i < 8 && j >= 0 && j < 8;;
}

void Board::setSelected(char piece, const sf::Vector2i& idx)
{
#pragma region Sliding Pieces
	char unsignedPiece = (piece < 0) ? -piece : piece;

	if (unsignedPiece == Rook || unsignedPiece == Queen) {
		// Left
		for (int i = idx.x - 1; i >= 0; --i) {
			std::pair<char, bool>& square = board[i][idx.y];
			if (square.first == NONE) {
				square.second = true;
				continue;
			}
			if (getSign(piece) == getSign(square.first)) {
				break;
			}
			else {
				square.second = true;
				break;
			}
		}
		// Right
		for (int i = idx.x + 1; i < 8; ++i) {
			std::pair<char, bool>& square = board[i][idx.y];
			if (square.first == NONE) {
				square.second = true;
				continue;
			}
			if (getSign(piece) == getSign(square.first)) {
				break;
			}
			else {
				square.second = true;
				break;
			}
		}
		// Up
		for (int i = idx.y - 1; i >= 0; --i) {
			std::pair<char, bool>& square = board[idx.x][i];
			if (square.first == NONE) {
				square.second = true;
				continue;
			}
			if (getSign(piece) == getSign(square.first)) {
				break;
			}
			else {
				square.second = true;
				break;
			}
		}
		// Down
		for (int i = idx.y + 1; i < 8; ++i) {
			std::pair<char, bool>& square = board[idx.x][i];
			if (square.first == NONE) {
				square.second = true;
				continue;
			}
			if (getSign(piece) == getSign(square.first)) {
				break;
			}
			else {
				square.second = true;
				break;
			}
		}
	}
	if (unsignedPiece == Bishop || unsignedPiece == Queen) {
		// Left and Up
		for (int i = -1; isValidIdx(idx.x + i, idx.y + i); --i) {
			std::pair<char, bool>& square = board[idx.x + i][idx.y + i];
			if (square.first == NONE) {
				square.second = true;
				continue;
			}
			if (getSign(piece) == getSign(square.first)) {
				break;
			}
			else {
				square.second = true;
				break;
			}
		}
		// Right and Down
		for (int i = 1; isValidIdx(idx.x + i, idx.y + i); ++i) {
			std::pair<char, bool>& square = board[idx.x + i][idx.y + i];
			if (square.first == NONE) {
				square.second = true;
				continue;
			}
			if (getSign(piece) == getSign(square.first)) {
				break;
			}
			else {
				square.second = true;
				break;
			}
		}
		// Left and Down
		for (int i = -1; isValidIdx(idx.x + i, idx.y - i); --i) {
			std::pair<char, bool>& square = board[idx.x + i][idx.y - i];
			if (square.first == NONE) {
				square.second = true;
				continue;
			}
			if (getSign(piece) == getSign(square.first)) {
				break;
			}
			else {
				square.second = true;
				break;
			}
		}
		// Right and Up
		for (int i = 1; isValidIdx(idx.x + i, idx.y - i); ++i) {
			std::pair<char, bool>& square = board[idx.x + i][idx.y - i];
			if (square.first == NONE) {
				square.second = true;
				continue;
			}
			if (getSign(piece) == getSign(square.first)) {
				break;
			}
			else {
				square.second = true;
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
			square.second = true;
		}
		// Attack
		if (idx.y - 1 >= 0) {
			if (idx.x - 1 >= 0) {
				std::pair<char, bool>& square = board[idx.x - 1][idx.y - 1];
				if (square.first != NONE) {
					if (getSign(piece) != getSign(square.first)) {
						square.second = true;
					}
				}
			}
			if (idx.x + 1 < 8) {
				std::pair<char, bool>& square = board[idx.x + 1][idx.y - 1];
				if (square.first != NONE) {
					if (getSign(piece) != getSign(square.first)) {
						square.second = true;
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
			square.second = true;
		}
		// Attack
		if (idx.y + 1 < 8) {
			if (idx.x - 1 >= 0) {
				std::pair<char, bool>& square = board[idx.x - 1][idx.y + 1];
				if (square.first != NONE) {
					if (getSign(piece) != getSign(square.first)) {
						square.second = true;
					}
				}
			}
			if (idx.x + 1 < 8) {
				std::pair<char, bool>& square = board[idx.x + 1][idx.y + 1];
				if (square.first != NONE) {
					if (getSign(piece) != getSign(square.first)) {
						square.second = true;
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
					square.second = true;
				}
			}
			if (idx.y + 2 < 8) {
				std::pair<char, bool>& square = board[idx.x - 1][idx.y + 2];
				if (square.first == NONE || (getSign(piece) != getSign(square.first))) {
					square.second = true;
				}
			}
		}
		if (idx.x + 1 < 8) {
			if (idx.y - 2 >= 0) {
				std::pair<char, bool>& square = board[idx.x + 1][idx.y - 2];
				if (square.first == NONE || (getSign(piece) != getSign(square.first))) {
					square.second = true;
				}
			}
			if (idx.y + 2 < 8) {
				std::pair<char, bool>& square = board[idx.x + 1][idx.y + 2];
				if (square.first == NONE || (getSign(piece) != getSign(square.first))) {
					square.second = true;
				}
			}
		}

		if (idx.x - 2 >= 0) {
			if (idx.y - 1 >= 0) {
				std::pair<char, bool>& square = board[idx.x - 2][idx.y - 1];
				if (square.first == NONE || (getSign(piece) != getSign(square.first))) {
					square.second = true;
				}
			}
			if (idx.y + 1 < 8) {
				std::pair<char, bool>& square = board[idx.x - 2][idx.y + 1];
				if (square.first == NONE || (getSign(piece) != getSign(square.first))) {
					square.second = true;
				}
			}
		}
		if (idx.x + 2 < 8) {
			if (idx.y - 1 >= 0) {
				std::pair<char, bool>& square = board[idx.x + 2][idx.y - 1];
				if (square.first == NONE || (getSign(piece) != getSign(square.first))) {
					square.second = true;
				}
			}
			if (idx.y + 1 < 8) {
				std::pair<char, bool>& square = board[idx.x + 2][idx.y + 1];
				if (square.first == NONE || (getSign(piece) != getSign(square.first))) {
					square.second = true;
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
				square.second = true;
			}

			if (idx.x - 1 >= 0) {
				std::pair<char, bool>&  square = board[idx.x - 1][idx.y - 1];
				if (square.first == NONE || (getSign(piece) != getSign(square.first))) {
					square.second = true;
				}
			}
			if (idx.x + 1 < 8) {
				std::pair<char, bool>& square = board[idx.x + 1][idx.y - 1];
				if (square.first == NONE || (getSign(piece) != getSign(square.first))) {
					square.second = true;
				}
			}
		}
		if (idx.y + 1 < 8) {
			std::pair<char, bool>& square = board[idx.x][idx.y + 1];
			if (square.first == NONE || (getSign(piece) != getSign(square.first))) {
				square.second = true;
			}

			if (idx.x - 1 >= 0) {
				std::pair<char, bool>& square = board[idx.x - 1][idx.y + 1];
				if (square.first == NONE || (getSign(piece) != getSign(square.first))) {
					square.second = true;
				}
			}
			if (idx.x + 1 < 8) {
				std::pair<char, bool>& square = board[idx.x + 1][idx.y + 1];
				if (square.first == NONE || (getSign(piece) != getSign(square.first))) {
					square.second = true;
				}
			}
		}
		if (idx.x - 1 >= 0) {
			std::pair<char, bool>& square = board[idx.x - 1][idx.y];
			if (square.first == NONE || (getSign(piece) != getSign(square.first))) {
				square.second = true;
			}
		}
		if (idx.x + 1 < 8) {
			std::pair<char, bool>& square = board[idx.x + 1][idx.y];
			if (square.first == NONE || (getSign(piece) != getSign(square.first))) {
				square.second = true;
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

sf::Vector2f Board::getIdx(sf::Vector2f worldPosition)
{
	sf::Vector2f relativeMousePos = worldPosition - origin;
	return sf::Vector2f(relativeMousePos.x / slotSize.x, relativeMousePos.y / slotSize.y);
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
