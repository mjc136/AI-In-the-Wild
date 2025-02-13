#include "Bird.h"
#include "Game.h"
#include "Brain.h"
#include <iostream>
#include <fstream>

using namespace std;

Bird::Bird()
{
}


Bird::~Bird()
{
}

void Bird::init(sf::Color birdColour)
{
	texture.loadFromFile("assets/flying_dragon-gold.png");
	rectSourceSprite = sf::IntRect(0, 128, 144, 128);
	dragon = sf::Sprite(texture, rectSourceSprite);
	dragon.scale(sf::Vector2f(.5, .5));

	x = 200;
//	y = rand() % Game::height;
	y = Game::height / 2 - 20;	// All Birds start in the same place
	
	// Size of Bird. A nice square Bird. 
	// I have added a dragon sprite sheet fort visual appeal but left the retangle as the collision object.
	w = 40;
	h = 40;
	colour = birdColour;

	fitness = 0;
	score = 0;

	alive = true;

	dragon.setPosition(x, y);
	rectangle.setPosition(x, y);
	rectangle.setSize(sf::Vector2f(w, h));
	rectangle.setFillColor(colour);

	brain.init(5, 4, 1);	// Give the Bird a Brain

}

//the game loop method
void Bird::update(Pillar pillar)
{
	y += 3;	// Gravity
	if (!Game::player)
		jump = think(pillar.x, pillar.upperH, pillar.lowerY);	// Make a decision on whether to flap or not.
	if (jump)
	{
		y -= 6;	// Flap!
	}
	rectangle.setPosition(x, y);

	score += (1.0 / 60.0);	// Score increases as time pases. The longer the Brid is alive the higher the score.
	fitness = score;		// Why do I need score? Must refactor.
	// If the bird leaves the screen it dies.
	if (y < 0 || y > Game::height)
	{
		fitness = score;	// Record how long it lived.
		alive = false;
		score = 0.0;
	}
	// If the bird hits the upcoming pillar.
	if (pillar.lowerRectangle.getGlobalBounds().intersects(this->rectangle.getGlobalBounds()))
	{
		fitness = score;
		alive = false;
		score = 0.0;
	}
	if (pillar.upperRectangle.getGlobalBounds().intersects(this->rectangle.getGlobalBounds()))
	{
		fitness = score;
		alive = false;
		score = 0.0;
	}
} 

// Decide on what to do.
// The 5 inputs to the Neural Network are :
// 1. X – coordinate of the front – most “pillar”.
// 2. Y – Coordinate of the lower part of the Upper “Pillar” i.e., (0 + height of Upper pillar)
// 3. Y – Coordinate of the upper part of the Lower “Pillar”
// 4. X – Coordinate of the Bird / Species itself
// 5. y – Coordinate of the Bird / Species itself
bool Bird::think(int pillarx, int upperPillarH, int lowerPillarY)
{
	bool jump = false;
	float inputs[5] = { pillarx, upperPillarH, lowerPillarY, x, y };

	jump = brain.FeedForward(inputs);

	return jump;
}

void Bird::reset() 
{
//	y = Game::height / 2 - 20;
	y = rand() % Game::height;
	alive = true;

}

void Bird::draw(sf::RenderWindow& game_window, Pillar pillar)
{
	if(jump)	// Only flap if we are going up. Visually this will look like he stops flaping when going down.
		if (clock.getElapsedTime().asSeconds() > 0.25f)	// Change animation every half second.
		{
			if (rectSourceSprite.left == 144 * 2)
				rectSourceSprite.left = 0;
			else
				rectSourceSprite.left += 144;
			dragon.setTextureRect(rectSourceSprite);
			clock.restart();
		}
	dragon.setScale(.5, .5);
	dragon.setPosition(x, y);
	sf::RectangleShape rectangle(sf::Vector2f(15.0f, 15.0f));
	rectangle.setPosition(x, y);
	rectangle.setFillColor(colour);
	game_window.draw(rectangle);
	game_window.draw(dragon);

	// Draw the connecting lines from Bird to pillar corners.
	line1[0] = sf::Vector2f(dragon.getPosition().x + dragon.getLocalBounds().width * dragon.getScale().x / 2, dragon.getPosition().y + dragon.getLocalBounds().height * dragon.getScale().y / 2);
	line1[0].color = colour;
	line1[1] = sf::Vector2f(pillar.x, pillar.upperH);
	line1[1].color = colour;

	line2[0] = sf::Vector2f(dragon.getPosition().x + dragon.getLocalBounds().width * dragon.getScale().x / 2, dragon.getPosition().y + dragon.getLocalBounds().height * dragon.getScale().y / 2);
	line2[0].color = colour;
	line2[1] = sf::Vector2f(pillar.x, pillar.lowerY);
	line2[1].color = colour;

// Old code when I just had a boring rectangle displaying.
//	line1[0] = sf::Vector2f(x + rectangle.getSize().x / 2, y + rectangle.getSize().y / 2);
//	line1[0].color = colour;
//	line1[1] = sf::Vector2f(pillar.x, pillar.upperH);
//	line1[1].color = colour;

//	line2[0] = sf::Vector2f(x + rectangle.getSize().x / 2, y + rectangle.getSize().y / 2);
//	line2[0].color = colour;
//	line2[1] = sf::Vector2f(pillar.x, pillar.lowerY);
//	line2[1].color = colour;

	game_window.draw(line1, 2, sf::Lines);
	game_window.draw(line2, 2, sf::Lines);

}

