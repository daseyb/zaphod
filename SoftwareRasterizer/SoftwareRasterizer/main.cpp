#include <SFML/Graphics.hpp>
#include <stdlib.h>
#include "Raytracer.h"

const int WIDTH = 850;
const int HEIGHT = 480;

int GetRand(int _min, int _max)
{
	return _min + ( rand() % (_max - _min + 1));
}

int main()
{
	srand( (unsigned) time(NULL) ) ; 

    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "CPU Raytracer");

	sf::Uint8* pixels = new sf::Uint8[WIDTH * HEIGHT * 4];

	sf::Texture tex;
	tex.create(WIDTH, HEIGHT);

	sf::Sprite renderSprite;
	renderSprite.setTexture(tex);

	Raytracer rt;
	rt.Initialize(WIDTH, HEIGHT, 90);
	
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();

		rt.Render();

		tex.update(rt.GetPixels());
		
		window.draw(renderSprite);
        window.display();
    }

    return 0;
}