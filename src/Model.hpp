#pragma once

#include "Mesh.hpp"

#include <memory>
#include <string>
#include <vector>

class Model {
public:
    Model() = default;
    explicit Model(std::string debugName);

    bool loadFromFile(const std::string& path);
    void addMesh(Mesh mesh);
    void draw() const;

    const std::string& name() const { return debugName; }
    bool empty() const { return meshes.empty(); }

private:
    std::string debugName = "model";
    std::vector<Mesh> meshes;

    void processNode(const struct aiNode* node, const struct aiScene* scene);
    Mesh processMesh(const struct aiMesh* mesh, const struct aiScene* scene);
};

std::shared_ptr<Model> loadModelOrFallback(const std::string& path, std::shared_ptr<Model> fallback, const std::string& label);
