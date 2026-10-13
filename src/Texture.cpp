#include "Texture.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <cctype>

namespace {
std::string nextToken(std::istream& in) {
    std::string token;
    char c = 0;
    while (in.get(c)) {
        if (std::isspace(static_cast<unsigned char>(c))) {
            continue;
        }
        if (c == '#') {
            std::string ignored;
            std::getline(in, ignored);
            continue;
        }
        token.push_back(c);
        break;
    }
    while (in.get(c)) {
        if (std::isspace(static_cast<unsigned char>(c))) {
            break;
        }
        token.push_back(c);
    }
    return token;
}
}

GLuint loadTexturePPM(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open texture file: " + path);
    }

    const std::string magic = nextToken(file);
    const int width = std::stoi(nextToken(file));
    const int height = std::stoi(nextToken(file));
    const int maxValue = std::stoi(nextToken(file));

    if (width <= 0 || height <= 0 || maxValue != 255) {
        throw std::runtime_error("Unsupported PPM texture format: " + path);
    }

    std::vector<unsigned char> pixels(static_cast<size_t>(width) * static_cast<size_t>(height) * 3u);

    if (magic == "P6") {
        file.read(reinterpret_cast<char*>(pixels.data()), static_cast<std::streamsize>(pixels.size()));
        if (file.gcount() != static_cast<std::streamsize>(pixels.size())) {
            throw std::runtime_error("Incomplete PPM texture data: " + path);
        }
    } else if (magic == "P3") {
        for (unsigned char& component : pixels) {
            component = static_cast<unsigned char>(std::stoi(nextToken(file)));
        }
    } else {
        throw std::runtime_error("Unsupported texture magic in " + path + ": " + magic);
    }

    GLuint texture = 0;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    std::cout << "Loaded texture: " << path << " (" << width << "x" << height << ")\n";
    return texture;
}

void deleteTexture(GLuint texture) {
    if (texture != 0) {
        glDeleteTextures(1, &texture);
    }
}
