#pragma once

#include <glm/glm.hpp>

class Camera {
public:
    glm::vec3 position{0.0f, 2.0f, 14.0f};
    glm::vec3 front{0.0f, 0.0f, -1.0f};
    glm::vec3 up{0.0f, 1.0f, 0.0f};
    glm::vec3 right{1.0f, 0.0f, 0.0f};
    glm::vec3 worldUp{0.0f, 1.0f, 0.0f};

    float yaw = -90.0f;
    float pitch = 0.0f;
    float movementSpeed = 7.5f;
    float mouseSensitivity = 0.08f;

    Camera();

    glm::mat4 viewMatrix() const;
    void processKeyboard(bool forward, bool backward, bool left, bool rightMove, float dt);
    void processMouse(float xOffset, float yOffset);
    void reset();

private:
    void updateVectors();
};
