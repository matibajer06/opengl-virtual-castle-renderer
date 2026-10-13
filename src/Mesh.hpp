#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

struct Vertex {
    glm::vec3 position{0.0f};
    glm::vec3 normal{0.0f, 1.0f, 0.0f};
    glm::vec2 texCoord{0.0f};
};

class Mesh {
public:
    Mesh() = default;
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::string debugName = "mesh");

    void draw() const;
    const std::string& name() const { return debugName; }

private:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::string debugName;

    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;

    void setup();
};
