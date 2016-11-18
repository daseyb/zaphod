#pragma once
#include <string>
#include <memory>
#include <unordered_map>

#include <IO/stb_image.h>

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
            int comp;
            auto imgData = stbi_loadf(key.c_str(), &data.width, &data.height, &comp, 4);
            
            if (!imgData) return nullptr;

            data.pixels = std::move(std::shared_ptr<Color>((Color*)imgData));

            cache[key] = std::make_shared<const TextureData>(data);
        }

        return cache.find(key)->second;
    }
};