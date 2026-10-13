#pragma once

#include <GL/glew.h>
#include <string>

GLuint loadTexturePPM(const std::string& path);
void deleteTexture(GLuint texture);
