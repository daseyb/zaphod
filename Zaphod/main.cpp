#include <SFML/Graphics.hpp>
#include "Rendering/Raytracer.h"
#include <ctime>
#include <string>
#include <memory>
#include <chrono>
#include <thread>

const int WIDTH = 1280;
const int HEIGHT = 720;

int main()
{
  //Initialize the window
  sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "CPU Raytracer");

  //The texture we write the pixels into
  sf::Texture tex;
  tex.create(WIDTH, HEIGHT);

  //We need a sprite for rendering the texture with SFML
  sf::Sprite renderSprite;
  renderSprite.setTexture(tex);

  //Initialize the Raytracer class with width, height and horizontal FOV
  Raytracer rt;
  rt.Initialize(WIDTH, HEIGHT, 90);

  //Update the pixel array
  rt.Render();

  time_t now = time(0);

  while (window.isOpen())
  {
    sf::Event event;
    while (window.pollEvent(event))
    {
      if (event.type == sf::Event::Closed)
        window.close();

      if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::O) {
        tex.copyToImage().saveToFile("Output.png");
      }
    }

    //window.clear();

    //Write the updated pixels to the texture
    tex.update(rt.GetPixels());

    window.draw(renderSprite);
    window.display();
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
  }

  rt.Shutdown();

  return 0;
}