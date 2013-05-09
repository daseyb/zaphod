#pragma once
#include <SFML\Graphics.hpp>

class Vertex
{
public:
	sf::Vector3f Position;
	sf::Color Color;

	Vertex(sf::Vector3f _pos, sf::Color _col) : Position(_pos) , Color(_col){}
	Vertex() : Position(0, 0, 0) , Color(sf::Color::White){}
	Vertex(sf::Vector3f _pos) : Position(_pos), Color(sf::Color::White){}
	~Vertex(void);
};

