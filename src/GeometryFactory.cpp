#include "GeometryFactory.hpp"

#include <cmath>
#include <vector>

namespace {
constexpr float PI = 3.14159265358979323846f;

std::shared_ptr<Model> singleMeshModel(const std::string& name, std::vector<Vertex> vertices, std::vector<unsigned int> indices) {
    auto model = std::make_shared<Model>(name);
    model->addMesh(Mesh(std::move(vertices), std::move(indices), name + "_mesh"));
    return model;
}

void appendTransformedBox(std::vector<Vertex>& vertices,
                          std::vector<unsigned int>& indices,
                          const glm::vec3& center,
                          const glm::vec3& size,
                          const std::string&) {
    const unsigned int baseVertex = static_cast<unsigned int>(vertices.size());
    const float x = size.x * 0.5f;
    const float y = size.y * 0.5f;
    const float z = size.z * 0.5f;

    const glm::vec3 p[8] = {
        center + glm::vec3(-x, -y,  z), center + glm::vec3( x, -y,  z),
        center + glm::vec3( x,  y,  z), center + glm::vec3(-x,  y,  z),
        center + glm::vec3(-x, -y, -z), center + glm::vec3( x, -y, -z),
        center + glm::vec3( x,  y, -z), center + glm::vec3(-x,  y, -z)
    };

    const glm::vec3 normals[6] = {
        {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, -1.0f},
        {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}
    };

    const unsigned int faces[6][4] = {
        {0, 1, 2, 3}, {5, 4, 7, 6}, {4, 0, 3, 7},
        {1, 5, 6, 2}, {3, 2, 6, 7}, {4, 5, 1, 0}
    };

    const glm::vec2 uvs[4] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};

    for (int face = 0; face < 6; ++face) {
        for (int corner = 0; corner < 4; ++corner) {
            Vertex v;
            v.position = p[faces[face][corner]];
            v.normal = normals[face];
            v.texCoord = uvs[corner];
            vertices.push_back(v);
        }
        const unsigned int start = baseVertex + static_cast<unsigned int>(face * 4);
        indices.insert(indices.end(), {start, start + 1, start + 2, start, start + 2, start + 3});
    }
}
}

std::shared_ptr<Model> makeBoxModel(const std::string& name, const glm::vec3& size) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    appendTransformedBox(vertices, indices, glm::vec3(0.0f), size, name);
    return singleMeshModel(name, std::move(vertices), std::move(indices));
}

std::shared_ptr<Model> makeCylinderModel(const std::string& name, float radius, float height, int segments) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    const float halfHeight = height * 0.5f;

    for (int i = 0; i <= segments; ++i) {
        const float angle = 2.0f * PI * static_cast<float>(i) / static_cast<float>(segments);
        const float x = std::cos(angle) * radius;
        const float z = std::sin(angle) * radius;
        const glm::vec3 normal = glm::normalize(glm::vec3(x, 0.0f, z));

        Vertex bottom;
        bottom.position = glm::vec3(x, -halfHeight, z);
        bottom.normal = normal;
        bottom.texCoord = glm::vec2(static_cast<float>(i) / segments, 0.0f);
        vertices.push_back(bottom);

        Vertex top;
        top.position = glm::vec3(x, halfHeight, z);
        top.normal = normal;
        top.texCoord = glm::vec2(static_cast<float>(i) / segments, 1.0f);
        vertices.push_back(top);
    }

    for (int i = 0; i < segments; ++i) {
        const unsigned int start = static_cast<unsigned int>(i * 2);
        indices.insert(indices.end(), {start, start + 1, start + 3, start, start + 3, start + 2});
    }

    const unsigned int topCenter = static_cast<unsigned int>(vertices.size());
    vertices.push_back({glm::vec3(0.0f, halfHeight, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.5f)});
    const unsigned int bottomCenter = static_cast<unsigned int>(vertices.size());
    vertices.push_back({glm::vec3(0.0f, -halfHeight, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.5f)});

    for (int i = 0; i < segments; ++i) {
        const float a0 = 2.0f * PI * static_cast<float>(i) / static_cast<float>(segments);
        const float a1 = 2.0f * PI * static_cast<float>(i + 1) / static_cast<float>(segments);

        const unsigned int t0 = static_cast<unsigned int>(vertices.size());
        vertices.push_back({glm::vec3(std::cos(a0) * radius, halfHeight, std::sin(a0) * radius), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f)});
        const unsigned int t1 = static_cast<unsigned int>(vertices.size());
        vertices.push_back({glm::vec3(std::cos(a1) * radius, halfHeight, std::sin(a1) * radius), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f)});
        indices.insert(indices.end(), {topCenter, t0, t1});

        const unsigned int b0 = static_cast<unsigned int>(vertices.size());
        vertices.push_back({glm::vec3(std::cos(a0) * radius, -halfHeight, std::sin(a0) * radius), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f)});
        const unsigned int b1 = static_cast<unsigned int>(vertices.size());
        vertices.push_back({glm::vec3(std::cos(a1) * radius, -halfHeight, std::sin(a1) * radius), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f)});
        indices.insert(indices.end(), {bottomCenter, b1, b0});
    }

    return singleMeshModel(name, std::move(vertices), std::move(indices));
}

std::shared_ptr<Model> makeFlagModel(const std::string& name, float width, float height, int segmentsX, int segmentsY) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    for (int y = 0; y <= segmentsY; ++y) {
        for (int x = 0; x <= segmentsX; ++x) {
            const float u = static_cast<float>(x) / static_cast<float>(segmentsX);
            const float v = static_cast<float>(y) / static_cast<float>(segmentsY);
            Vertex vertex;
            vertex.position = glm::vec3(u * width, (v - 0.5f) * height, 0.0f);
            vertex.normal = glm::vec3(0.0f, 0.0f, 1.0f);
            vertex.texCoord = glm::vec2(u, v);
            vertices.push_back(vertex);
        }
    }

    const int row = segmentsX + 1;
    for (int y = 0; y < segmentsY; ++y) {
        for (int x = 0; x < segmentsX; ++x) {
            const unsigned int i0 = static_cast<unsigned int>(y * row + x);
            const unsigned int i1 = i0 + 1;
            const unsigned int i2 = i0 + static_cast<unsigned int>(row);
            const unsigned int i3 = i2 + 1;
            indices.insert(indices.end(), {i0, i1, i3, i0, i3, i2});
            indices.insert(indices.end(), {i3, i1, i0, i2, i3, i0});
        }
    }

    return singleMeshModel(name, std::move(vertices), std::move(indices));
}

std::shared_ptr<Model> makeLowPolyAnimalModel(const std::string& name) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    appendTransformedBox(vertices, indices, glm::vec3(0.0f, 0.45f, 0.0f), glm::vec3(0.9f, 0.55f, 0.45f), name);
    appendTransformedBox(vertices, indices, glm::vec3(0.55f, 0.60f, 0.0f), glm::vec3(0.32f, 0.32f, 0.32f), name);
    appendTransformedBox(vertices, indices, glm::vec3(-0.35f, 0.10f, 0.16f), glm::vec3(0.12f, 0.32f, 0.12f), name);
    appendTransformedBox(vertices, indices, glm::vec3(0.25f, 0.10f, 0.16f), glm::vec3(0.12f, 0.32f, 0.12f), name);
    appendTransformedBox(vertices, indices, glm::vec3(-0.35f, 0.10f, -0.16f), glm::vec3(0.12f, 0.32f, 0.12f), name);
    appendTransformedBox(vertices, indices, glm::vec3(0.25f, 0.10f, -0.16f), glm::vec3(0.12f, 0.32f, 0.12f), name);
    return singleMeshModel(name, std::move(vertices), std::move(indices));
}

std::shared_ptr<Model> makeLowPolyKnightModel(const std::string& name) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    appendTransformedBox(vertices, indices, glm::vec3(0.0f, 1.05f, 0.0f), glm::vec3(0.45f, 0.85f, 0.25f), name);
    appendTransformedBox(vertices, indices, glm::vec3(0.0f, 1.70f, 0.0f), glm::vec3(0.35f, 0.35f, 0.35f), name);
    appendTransformedBox(vertices, indices, glm::vec3(-0.33f, 0.92f, 0.0f), glm::vec3(0.14f, 0.65f, 0.14f), name);
    appendTransformedBox(vertices, indices, glm::vec3(0.33f, 0.92f, 0.0f), glm::vec3(0.14f, 0.65f, 0.14f), name);
    appendTransformedBox(vertices, indices, glm::vec3(-0.13f, 0.35f, 0.0f), glm::vec3(0.16f, 0.72f, 0.16f), name);
    appendTransformedBox(vertices, indices, glm::vec3(0.13f, 0.35f, 0.0f), glm::vec3(0.16f, 0.72f, 0.16f), name);
    appendTransformedBox(vertices, indices, glm::vec3(0.55f, 1.05f, 0.0f), glm::vec3(0.08f, 1.25f, 0.08f), name);
    return singleMeshModel(name, std::move(vertices), std::move(indices));
}
