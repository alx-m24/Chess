// SFML
#include <SFML/Graphics.hpp>
// My Headers
#include "Headers/Resources/Resources.hpp"
#include "Headers/Board.hpp"
// Other
#include <iostream>
#include <array>

/* Chess:
*	- Board:
*		- 8x8
*		- Each case/position has 7 possible states:
*			- Empty
*			- Occupied by:
*				- Pawn
*				- Rook
*				- etc
*	- Pieces:
*		- 6 pieces
*/

int main() {
	window.create(sf::VideoMode(windowSize.x, windowSize.y), "Chess", sf::Style::Default);

	Board board;

	board.debugDisplay();

	while (window.isOpen()) {
		handleEvents();

		board.update();

		window.clear(board.whiteToMove ? sf::Color(28, 28, 28) : sf::Color::Black);
		board.display();

		window.display();
	}

	return EXIT_SUCCESS;
}