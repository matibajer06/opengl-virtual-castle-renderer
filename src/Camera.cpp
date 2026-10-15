#include "Camera.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

Camera::Camera() {
    updateVectors();
}

glm::mat4 Camera::viewMatrix() const {
    return glm::lookAt(position, position + front, up);
}

void Camera::processKeyboard(bool forward, bool backward, bool leftMove, bool rightMove, float dt) {
    const float velocity = movementSpeed * dt;
    glm::vec3 horizontalFront = glm::normalize(glm::vec3(front.x, 0.0f, front.z));
    glm::vec3 horizontalRight = glm::normalize(glm::vec3(right.x, 0.0f, right.z));

    if (forward) {
        position += horizontalFront * velocity;
    }
    if (backward) {
        position -= horizontalFront * velocity;
    }
    if (leftMove) {
        position -= horizontalRight * velocity;
    }
    if (rightMove) {
        position += horizontalRight * velocity;
    }

    position.y = 2.0f;
}

void Camera::processMouse(float xOffset, float yOffset) {
    xOffset *= mouseSensitivity;
    yOffset *= mouseSensitivity;

    yaw += xOffset;
    pitch += yOffset;
    pitch = std::clamp(pitch, -84.0f, 84.0f);

    updateVectors();
}

void Camera::reset() {
    position = glm::vec3(0.0f, 2.0f, 14.0f);
    yaw = -90.0f;
    pitch = 0.0f;
    updateVectors();
}

void Camera::updateVectors() {
    glm::vec3 f;
    f.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    f.y = sin(glm::radians(pitch));
    f.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(f);
    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
}
