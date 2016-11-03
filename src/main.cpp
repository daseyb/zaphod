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

#include <Alembic/AbcGeom/All.h>
#include <Alembic/AbcCoreAbstract/All.h>
#include <Alembic/AbcCoreFactory/All.h>
#include <Alembic/Util/All.h>
#include <Alembic/Abc/TypedPropertyTraits.h>


std::string get_time_string() {
  auto t = std::time(nullptr);
  auto tm = *std::localtime(&t);
  std::stringstream file_name;
  file_name << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");
  return file_name.str();
}

#ifndef HEADLESS
#include <SFML/Graphics.hpp>
int display_window(int width, int height, const Raytracer &rt) {
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
  }

  return 0;
}
#endif

const std::string USAGE = "<width> <height> <spp> <tile size> <thread count> "
                          "<scene file> <integrator>";

int main(int argc, char **argv) {

  if(argc == 2)
  {
    Alembic::AbcCoreFactory::IFactory factory;
    factory.setPolicy(Alembic::AbcGeom::ErrorHandler::kQuietNoopPolicy);
	Alembic::AbcGeom::IArchive archive = factory.getArchive(argv[1]);

    if (archive) {
      std::cout << "AbcEcho for "
                << Alembic::AbcCoreAbstract::GetLibraryVersion() << std::endl;
      ;

      std::string appName;
      std::string libraryVersionString;
      Alembic::Util::uint32_t libraryVersion;
      std::string whenWritten;
      std::string userDescription;
      GetArchiveInfo(archive, appName, libraryVersionString, libraryVersion,
                     whenWritten, userDescription);

      if (appName != "") {
        std::cout << "  file written by: " << appName << std::endl;
        std::cout << "  using Alembic : " << libraryVersionString << std::endl;
        std::cout << "  written on : " << whenWritten << std::endl;
        std::cout << "  user description : " << userDescription << std::endl;
        std::cout << std::endl;
      } else {
        std::cout << argv[1] << std::endl;
        std::cout << "  (file doesn't have any ArchiveInfo)" << std::endl;
        std::cout << std::endl;
      }
    }
  }

  if (argc != 8) {
    std::cout << "Wrong number of arguments!" << std::endl;
    std::cout << USAGE << std::endl;
    return -1;
  }

  int width = std::stoi(argv[1]);
  int height = std::stoi(argv[2]);
  int spp = std::stoi(argv[3]);
  int tile_size = std::stoi(argv[4]);
  int thread_count = std::stoi(argv[5]);

  const char *scene_file = argv[6];

  // Initialize the Raytracer class with width, height and horizontal FOV
  Raytracer rt;
  if (!rt.Initialize(width, height, argv[7], spp, tile_size, thread_count,
                     scene_file)) {
    std::cout << "Failed to initialized the renderer!" << std::endl;
    return -1;
  }

  // Update the pixel array
  rt.Render();

#ifndef HEADLESS
  display_window(width, height, rt);
#else
  rt.Wait();
  stbi_write_hdr((get_time_string() + ".hdr").c_str(), width, height, 4,
                 (float *)rt.GetRawPixels());
#endif
  rt.Shutdown();

  return 0;
}