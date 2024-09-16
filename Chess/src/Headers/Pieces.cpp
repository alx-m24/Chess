#include "Pieces.hpp"

std::array<sf::Texture, 13> textures;

void displayPiece(const char& piece, sf::Vector2f position, sf::Vector2f size, bool centerOffset)
{
	const unsigned int idx = piece < 0 ? ((piece * -1) + BLACK) : piece;
	const sf::Texture& tex = textures[idx];

	sf::Vector2f scale = { size.x / tex.getSize().x, size.y / tex.getSize().y };

	sf::Sprite display_piece(tex);

	display_piece.setScale(scale);
	if (centerOffset) {
		position -= size / 2.0f;
	}
	display_piece.setPosition(position);

	window.draw(display_piece);
}
