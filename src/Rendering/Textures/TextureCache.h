#pragma once
#include <string>
#include <memory>
#include <unordered_map>

#include <IO/stb_image.h>
#include <IO/tinyexr.h>

#include "TextureData.h"

class TextureCache {
    std::string workingDir;
    std::unordered_map<std::string, std::shared_ptr<const TextureData>> cache;
public:
    static TextureCache& Instance() {
        static TextureCache instance;
        return instance;
    }

    void SetWorkingDir(std::string dir) { workingDir = dir; }
    
    std::shared_ptr<const TextureData> Get(std::string filename) {
        std::string key = workingDir + "\\" + filename;
        if (cache.find(key) == cache.end()) {
            TextureData data;
            std::string imageFileFormat = filename.substr(filename.find_last_of('.') + 1);

            if (imageFileFormat == "exr") {
                float* out; // width * height * RGBA
                const char* err;

                int ret = LoadEXR(&out, &data.width, &data.height, key.c_str(), &err);
                if (ret != 0) {
                    std::cerr << err << std::endl;
                    return nullptr;
                }

                data.pixels = std::move(std::shared_ptr<Color>((Color*)out));

            } else {
                int comp;
                auto imgData = stbi_loadf(key.c_str(), &data.width, &data.height, &comp, 4);

                if (!imgData) {
                    std::cerr << stbi_failure_reason() << std::endl;
                    return nullptr;
                }

                data.pixels = std::move(std::shared_ptr<Color>((Color*)imgData));
            }
            
            cache[key] = std::make_shared<const TextureData>(data);
        }

        return cache.find(key)->second;
    }
};