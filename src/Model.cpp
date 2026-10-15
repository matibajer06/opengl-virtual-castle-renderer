#include "Model.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <filesystem>
#include <iostream>
#include <stdexcept>

Model::Model(std::string debugNameIn) : debugName(std::move(debugNameIn)) {}

bool Model::loadFromFile(const std::string& path) {
    debugName = path;

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(
        path,
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_JoinIdenticalVertices |
        aiProcess_ImproveCacheLocality |
        aiProcess_OptimizeMeshes |
        aiProcess_FlipUVs
    );

    if (!scene || !scene->mRootNode || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)) {
        std::cerr << "Assimp could not load model: " << path << "\n"
                  << importer.GetErrorString() << "\n";
        return false;
    }

    meshes.clear();
    processNode(scene->mRootNode, scene);
    return !meshes.empty();
}

void Model::addMesh(Mesh mesh) {
    meshes.push_back(std::move(mesh));
}

void Model::draw() const {
    for (const Mesh& mesh : meshes) {
        mesh.draw();
    }
}

void Model::processNode(const aiNode* node, const aiScene* scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }

    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(const aiMesh* mesh, const aiScene*) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    vertices.reserve(mesh->mNumVertices);
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        Vertex vertex;
        vertex.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);

        if (mesh->HasNormals()) {
            vertex.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        }

        if (mesh->mTextureCoords[0]) {
            vertex.texCoord = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        }

        vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        const aiFace& face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j) {
            indices.push_back(face.mIndices[j]);
        }
    }

    return Mesh(vertices, indices, mesh->mName.C_Str());
}

std::shared_ptr<Model> loadModelOrFallback(const std::string& path, std::shared_ptr<Model> fallback, const std::string& label) {
    if (std::filesystem::exists(path)) {
        auto model = std::make_shared<Model>(label);
        if (model->loadFromFile(path)) {
            std::cout << "Loaded model: " << path << "\n";
            return model;
        }
    }

    std::cout << "Using procedural placeholder for: " << label << " because model file was not found: " << path << "\n";
    return fallback;
}
