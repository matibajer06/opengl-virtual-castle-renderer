#pragma once

#include "Model.hpp"

#include <glm/glm.hpp>
#include <memory>
#include <string>

std::shared_ptr<Model> makeBoxModel(const std::string& name, const glm::vec3& size);
std::shared_ptr<Model> makeCylinderModel(const std::string& name, float radius, float height, int segments);
std::shared_ptr<Model> makeFlagModel(const std::string& name, float width, float height, int segmentsX, int segmentsY);
std::shared_ptr<Model> makeLowPolyAnimalModel(const std::string& name);
std::shared_ptr<Model> makeLowPolyKnightModel(const std::string& name);
