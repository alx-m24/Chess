#pragma once
#include <array>
#include <iostream>
#include <SFML/Graphics.hpp>
#include "Resources/Resources.hpp"

constexpr unsigned int BLACK = 6;

enum Pieces
{
	NONE,
	Pawn,
	Knight,
	Bishop,
	Rook,
	Queen,
	King
};

extern std::array<sf::Texture, 13> textures; // Indexes are same a enum

void displayPiece(const char& piece, sf::Vector2f position, sf::Vector2f size, bool centerOffset = false);