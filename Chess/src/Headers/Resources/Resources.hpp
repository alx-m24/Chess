#pragma once
#include <SFML/Graphics.hpp>

extern sf::Vector2u windowSize;
extern sf::RenderWindow window;
extern bool left;
extern bool lastLeft;

void handleEvents();
char getSign(char num);