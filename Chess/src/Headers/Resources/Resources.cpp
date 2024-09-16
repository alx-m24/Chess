#include "Resources.hpp"

sf::Vector2u windowSize = sf::Vector2u(1000, 600);
sf::RenderWindow window;
bool left = false;
bool lastLeft = false;

void handleEvents() {
	sf::Event events;
	while (window.pollEvent(events)) {
		switch (events.type)
		{
		case sf::Event::Closed:
			window.close();
			break;
		case sf::Event::Resized:
			window.setView(sf::View(sf::FloatRect(0.0f, 0.0f, events.size.width, events.size.height)));
			windowSize = window.getSize();
			break;
		default:
			break;
		}
	}

	lastLeft = left;
	left = sf::Mouse::isButtonPressed(sf::Mouse::Left);
}

char getSign(char num)
{
	if (num < 0) return -1;
	return 1;
}
