#include <ctime>
#include <string>
#include <memory>
#include <chrono>
#include <thread>
#include <iomanip>
#include <sstream>
#include <iostream>

#include "IO/stb_image_write.h"
#include "Rendering/Raytracer.h"

int FrameIndex = 0;
int FrameEnd = 10;

bool batchmode = false;

const char *scene_file;

std::string get_time_string() {
  auto t = std::time(nullptr);
  auto tm = *std::localtime(&t);
  std::stringstream file_name;
  file_name << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");
  return file_name.str();
}

void padTo(std::string &str, const size_t num, const char paddingChar = ' ')
{
	if (num > str.size())
		str.insert(0, num - str.size(), paddingChar);
}

#ifndef HEADLESS
#include <SFML/Graphics.hpp>
int display_window(int width, int height, Raytracer &rt) {
  // Initialize the window
  sf::RenderWindow window(sf::VideoMode(width, height), "zaphod");

  // The texture we write the pixels into
  sf::Texture tex;
  tex.create(width, height);

  // We need a sprite for rendering the texture with SFML
  sf::Sprite renderSprite;
  renderSprite.setTexture(tex);

  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed)
        window.close();

      if (event.type == sf::Event::KeyPressed &&
          event.key.code == sf::Keyboard::O) {
        stbi_write_hdr((get_time_string() + ".hdr").c_str(), width, height, 4,
                       (float *)rt.GetRawPixels());
      }
    }

    // Write the updated pixels to the texture
    tex.update(rt.GetPixels());

    window.draw(renderSprite);
    window.display();
    std::this_thread::sleep_for(std::chrono::milliseconds(16));

    if (FrameIndex <= FrameEnd && rt.FrameDone()) {
      std::cout << "=========== Rendered frame " + std::to_string(FrameIndex) +
                       " ===========\n";

			std::string frameIndexStr = std::to_string(FrameIndex);
			padTo(frameIndexStr, 5, '0');

            auto filename = std::string(scene_file) + " " + frameIndexStr;
      stbi_write_hdr(
          (filename + ".hdr")
              .c_str(),
          width, height, 4, (float *)rt.GetRawPixels());

      auto img = tex.copyToImage();
      img.saveToFile(filename + ".png");

      FrameIndex++;
      if (FrameIndex <= FrameEnd) {
        rt.Render(FrameIndex);
      } else if(batchmode) {
          window.close();
      }
    }
  }

  return 0;
}
#endif

const std::string USAGE = "<width> <height> <spp> <tile size> <thread count> "
                          "<scene file> <start frame> <end frame> <integrator>";

int main(int argc, char **argv) {

  if (argc == 2) {
  }

  if (argc == 11) {
      batchmode = true;
      argc--;
  }

  if (argc != 10) {
    std::cout << "Wrong number of arguments!" << std::endl;
    std::cout << USAGE << std::endl;
    return -1;
  }

  int width = std::stoi(argv[1]);
  int height = std::stoi(argv[2]);
  int spp = std::stoi(argv[3]);
  int tile_size = std::stoi(argv[4]);
  int thread_count = std::stoi(argv[5]);

  scene_file = argv[6];

  FrameIndex = std::stoi(argv[7]);
  FrameEnd = std::stoi(argv[8]);

  const char *integrator = argv[9];

  // Initialize the Raytracer class with width, height and horizontal FOV
  Raytracer rt;
  if (!rt.Initialize(width, height, integrator, spp, tile_size, thread_count,
                     scene_file)) {
    std::cout << "Failed to initialized the renderer!" << std::endl;
    return -1;
  }

  // Update the pixel array
  rt.Render(FrameIndex);

#ifndef HEADLESS
  display_window(width, height, rt);
#else
  while (true) {
    rt.Wait();
    std::cout << "=========== Rendered frame " + std::to_string(FrameIndex) +
                     " ===========\n";

		std::string frameIndexStr = std::to_string(FrameIndex);
		padTo(frameIndexStr, 5, '0');

		stbi_write_hdr(
			(std::string(scene_file) + " " + frameIndexStr + ".hdr")
			.c_str(),
			width, height, 4, (float *)rt.GetRawPixels());

    if (FrameIndex < FrameEnd) {
      FrameIndex++;
      rt.Render(FrameIndex);
    } else {
      break;
    }
  }

#endif
  rt.Shutdown();

  return 0;
}