#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Camera.hpp"
#include "GeometryFactory.hpp"
#include "Model.hpp"
#include "Shader.hpp"
#include "Texture.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

constexpr unsigned int SCR_WIDTH = 1280;
constexpr unsigned int SCR_HEIGHT = 720;

Camera camera;
bool keys[1024]{};
bool firstMouse = true;
float lastX = static_cast<float>(SCR_WIDTH) * 0.5f;
float lastY = static_cast<float>(SCR_HEIGHT) * 0.5f;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool drawbridgeRaised = false;
bool bWasPressed = false;
bool flashlightEnabled = true;
bool fWasPressed = false;
bool rWasPressed = false;
float drawbridgeAngle = 0.0f;

struct SceneObject {
    std::string name;
    std::shared_ptr<Model> model;
    glm::vec3 color{1.0f, 1.0f, 1.0f};
    float shininess = 32.0f;
    int objectType = 0;       // 0 - normalny obiekt, 1 - flaga animowana w shaderze
    bool emissive = false;    // true dla widocznego płomienia
    GLuint texture = 0;
    std::function<glm::mat4(float)> transform;
};

void framebufferSizeCallback(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouseCallback(GLFWwindow*, double xpos, double ypos) {
    if (firstMouse) {
        lastX = static_cast<float>(xpos);
        lastY = static_cast<float>(ypos);
        firstMouse = false;
    }

    const float xOffset = static_cast<float>(xpos) - lastX;
    const float yOffset = lastY - static_cast<float>(ypos);
    lastX = static_cast<float>(xpos);
    lastY = static_cast<float>(ypos);

    camera.processMouse(xOffset, yOffset);
}

void keyCallback(GLFWwindow* window, int key, int, int action, int) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            keys[key] = true;
        } else if (action == GLFW_RELEASE) {
            keys[key] = false;
        }
    }
}

void processInput(GLFWwindow*) {
    camera.processKeyboard(keys[GLFW_KEY_W], keys[GLFW_KEY_S], keys[GLFW_KEY_A], keys[GLFW_KEY_D], deltaTime);

    if (keys[GLFW_KEY_B] && !bWasPressed) {
        drawbridgeRaised = !drawbridgeRaised;
    }
    bWasPressed = keys[GLFW_KEY_B];

    if (keys[GLFW_KEY_F] && !fWasPressed) {
        flashlightEnabled = !flashlightEnabled;
    }
    fWasPressed = keys[GLFW_KEY_F];

    if (keys[GLFW_KEY_R] && !rWasPressed) {
        camera.reset();
    }
    rWasPressed = keys[GLFW_KEY_R];
}

GLFWwindow* createWindow() {
    if (!glfwInit()) {
        throw std::runtime_error("GLFW initialization failed");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Virtual Castle - OpenGL + GLM", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        throw std::runtime_error("Could not create GLFW window");
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glewExperimental = GL_TRUE;
    const GLenum glewResult = glewInit();
    if (glewResult != GLEW_OK) {
        glfwTerminate();
        throw std::runtime_error(reinterpret_cast<const char*>(glewGetErrorString(glewResult)));
    }

    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glEnable(GL_DEPTH_TEST);
    return window;
}

std::string pointLightUniform(int index, const std::string& field) {
    return "pointLights[" + std::to_string(index) + "]." + field;
}

glm::mat4 transformTRS(const glm::vec3& pos,
                       const glm::vec3& scale,
                       float yawDegrees = 0.0f,
                       const glm::vec3& rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f)) {
    glm::mat4 m(1.0f);
    m = glm::translate(m, pos);
    if (std::abs(yawDegrees) > 0.001f) {
        m = glm::rotate(m, glm::radians(yawDegrees), rotationAxis);
    }
    m = glm::scale(m, scale);
    return m;
}

glm::mat4 translateScale(const glm::vec3& pos, const glm::vec3& scale) {
    return transformTRS(pos, scale);
}

void drawSceneObject(const SceneObject& obj, Shader& shader, float time) {
    const glm::mat4 modelMatrix = obj.transform ? obj.transform(time) : glm::mat4(1.0f);
    const glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, obj.texture);

    shader.setMat4("model", modelMatrix);
    shader.setMat3("normalMatrix", normalMatrix);
    shader.setVec3("material.baseColor", obj.color);
    shader.setFloat("material.shininess", obj.shininess);
    shader.setInt("uObjectType", obj.objectType);
    shader.setBool("uEmissive", obj.emissive);
    shader.setBool("uUseTexture", obj.texture != 0);
    shader.setInt("diffuseTexture", 0);
    obj.model->draw();
}

void setupLights(Shader& shader, const std::array<glm::vec3, 6>& torches) {
    shader.setVec3("dirLight.direction", glm::vec3(-0.35f, -1.0f, -0.25f));
    shader.setVec3("dirLight.color", glm::vec3(0.95f, 0.88f, 0.75f));
    shader.setFloat("dirLight.intensity", 0.25f);

    shader.setInt("pointLightCount", static_cast<int>(torches.size()));
    for (int i = 0; i < static_cast<int>(torches.size()); ++i) {
        shader.setVec3(pointLightUniform(i, "position"), torches[static_cast<size_t>(i)]);
        shader.setVec3(pointLightUniform(i, "color"), glm::vec3(1.0f, 0.55f, 0.18f));
        shader.setFloat(pointLightUniform(i, "intensity"), 2.5f);
        shader.setFloat(pointLightUniform(i, "constant"), 1.0f);
        shader.setFloat(pointLightUniform(i, "linear"), 0.09f);
        shader.setFloat(pointLightUniform(i, "quadratic"), 0.032f);
    }

    shader.setVec3("spotLight.position", camera.position);
    shader.setVec3("spotLight.direction", camera.front);
    shader.setVec3("spotLight.color", glm::vec3(0.85f, 0.90f, 1.0f));
    shader.setFloat("spotLight.intensity", 0.733333f);
    shader.setFloat("spotLight.cutOff", std::cos(glm::radians(12.5f)));
    shader.setFloat("spotLight.outerCutOff", std::cos(glm::radians(18.0f)));
    shader.setBool("spotLight.enabled", flashlightEnabled);
}

int main() {
    std::vector<GLuint> ownedTextures;

    try {
        GLFWwindow* window = createWindow();
        Shader shader("shaders/castle.vert", "shaders/castle.frag");

        std::unordered_map<std::string, GLuint> tex;
        auto addTexture = [&](const std::string& key, const std::string& path) {
            GLuint handle = loadTexturePPM(path);
            tex[key] = handle;
            ownedTextures.push_back(handle);
            return handle;
        };

        addTexture("stone", "assets/textures/stone.ppm");
        addTexture("wood", "assets/textures/wood.ppm");
        addTexture("grass", "assets/textures/grass.ppm");
        addTexture("water", "assets/textures/water.ppm");
        addTexture("metal", "assets/textures/metal.ppm");
        addTexture("cloth", "assets/textures/cloth_blue.ppm");
        addTexture("straw", "assets/textures/straw.ppm");
        addTexture("flame", "assets/textures/flame.ppm");

        const auto unitBox = makeBoxModel("unit_box", glm::vec3(1.0f));
        const auto groundBox = makeBoxModel("ground_box", glm::vec3(1.0f));
        const auto flagMesh = makeFlagModel("flag_mesh", 1.35f, 0.82f, 22, 8);
        const auto flameMarker = makeBoxModel("flame_marker", glm::vec3(0.24f));

        auto wallModel = loadModelOrFallback("assets/models/wall/wall.obj", makeBoxModel("wall_placeholder", glm::vec3(4.0f, 2.4f, 0.55f)), "castle wall");
        auto towerModel = loadModelOrFallback("assets/models/tower/tower.obj", makeCylinderModel("tower_placeholder", 1.0f, 1.0f, 36), "castle tower");
        auto keepModel = loadModelOrFallback("assets/models/keep/keep.obj", makeBoxModel("keep_placeholder", glm::vec3(4.2f, 5.2f, 4.2f)), "castle keep");
        auto gatehouseModel = loadModelOrFallback("assets/models/gatehouse/gatehouse.obj", makeBoxModel("gatehouse_placeholder", glm::vec3(5.6f, 3.6f, 2.2f)), "gatehouse");
        auto bridgeModel = loadModelOrFallback("assets/models/drawbridge/drawbridge.obj", makeBoxModel("drawbridge_placeholder", glm::vec3(4.8f, 0.28f, 6.2f)), "drawbridge");
        auto barracksModel = loadModelOrFallback("assets/models/barracks/barracks.obj", makeBoxModel("barracks_placeholder", glm::vec3(4.6f, 1.8f, 2.4f)), "barracks");
        auto stableModel = loadModelOrFallback("assets/models/stable/stable.obj", makeBoxModel("stable_placeholder", glm::vec3(4.2f, 2.2f, 2.8f)), "stable");
        auto bedModel = loadModelOrFallback("assets/models/bed/bed.obj", makeBoxModel("bed_placeholder", glm::vec3(1.3f, 0.45f, 0.65f)), "soldier bed");
        auto axeModel = loadModelOrFallback("assets/models/axe/axe.obj", makeBoxModel("axe_placeholder", glm::vec3(0.16f, 1.1f, 0.12f)), "wall axe");
        auto torchModel = loadModelOrFallback("assets/models/torch/torch.obj", makeBoxModel("torch_placeholder", glm::vec3(0.25f, 1.2f, 0.25f)), "wall torch");

        std::vector<SceneObject> objects;
        objects.reserve(100);

        auto addStatic = [&](const std::string& name,
                             std::shared_ptr<Model> model,
                             const glm::vec3& color,
                             GLuint texture,
                             const glm::mat4& transform,
                             float shininess = 24.0f,
                             int objectType = 0,
                             bool emissive = false) {
            objects.push_back(SceneObject{name, model, color, shininess, objectType, emissive, texture, [transform](float) { return transform; }});
        };


        addStatic("terrain", groundBox, glm::vec3(1.0f), tex["grass"], translateScale(glm::vec3(0.0f, -0.08f, 0.0f), glm::vec3(60.0f, 0.12f, 60.0f)), 8.0f);
        addStatic("courtyard", groundBox, glm::vec3(0.92f, 0.88f, 0.82f), tex["stone"], translateScale(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(17.2f, 0.08f, 15.2f)), 12.0f);
        addStatic("front_moat", groundBox, glm::vec3(0.65f, 0.78f, 1.0f), tex["water"], translateScale(glm::vec3(0.0f, 0.02f, 12.7f), glm::vec3(25.0f, 0.08f, 4.6f)), 96.0f);
        addStatic("back_moat", groundBox, glm::vec3(0.65f, 0.78f, 1.0f), tex["water"], translateScale(glm::vec3(0.0f, 0.02f, -11.5f), glm::vec3(25.0f, 0.08f, 4.6f)), 96.0f);
        addStatic("left_moat", groundBox, glm::vec3(0.65f, 0.78f, 1.0f), tex["water"], translateScale(glm::vec3(-11.7f, 0.02f, 0.6f), glm::vec3(4.4f, 0.08f, 20.0f)), 96.0f);
        addStatic("right_moat", groundBox, glm::vec3(0.65f, 0.78f, 1.0f), tex["water"], translateScale(glm::vec3(11.7f, 0.02f, 0.6f), glm::vec3(4.4f, 0.08f, 20.0f)), 96.0f);

        addStatic("front_wall_left", wallModel, glm::vec3(1.0f), tex["stone"], transformTRS(glm::vec3(-5.2f, 0.0f, 8.1f), glm::vec3(1.85f, 1.18f, 1.0f)));
        addStatic("front_wall_right", wallModel, glm::vec3(1.0f), tex["stone"], transformTRS(glm::vec3(5.2f, 0.0f, 8.1f), glm::vec3(1.85f, 1.18f, 1.0f)));
        addStatic("back_wall", wallModel, glm::vec3(1.0f), tex["stone"], transformTRS(glm::vec3(0.0f, 0.0f, -8.1f), glm::vec3(4.65f, 1.18f, 1.0f)));
        addStatic("left_wall", wallModel, glm::vec3(1.0f), tex["stone"], transformTRS(glm::vec3(-9.0f, 0.0f, 0.0f), glm::vec3(4.2f, 1.18f, 1.0f), 90.0f));
        addStatic("right_wall", wallModel, glm::vec3(1.0f), tex["stone"], transformTRS(glm::vec3(9.0f, 0.0f, 0.0f), glm::vec3(4.2f, 1.18f, 1.0f), 90.0f));

        const std::array<glm::vec3, 4> towerPositions = {
            glm::vec3(-9.0f, 0.0f, 8.1f), glm::vec3(9.0f, 0.0f, 8.1f),
            glm::vec3(-9.0f, 0.0f, -8.1f), glm::vec3(9.0f, 0.0f, -8.1f)
        };

        for (size_t i = 0; i < towerPositions.size(); ++i) {
            addStatic("tower_" + std::to_string(i + 1), towerModel, glm::vec3(0.96f), tex["stone"], transformTRS(towerPositions[i], glm::vec3(1.05f)), 28.0f);
        }

        addStatic("gatehouse", gatehouseModel, glm::vec3(0.98f), tex["stone"], transformTRS(glm::vec3(0.0f, 0.0f, 8.15f), glm::vec3(1.05f)), 30.0f);
        addStatic("main_keep", keepModel, glm::vec3(0.98f), tex["stone"], transformTRS(glm::vec3(0.0f, 0.0f, -2.0f), glm::vec3(1.0f)), 28.0f);

        objects.push_back(SceneObject{
            "drawbridge",
            bridgeModel,
            glm::vec3(1.0f),
            18.0f,
            0,
            false,
            tex["wood"],
            [](float) {
                glm::mat4 m(1.0f);
                m = glm::translate(m, glm::vec3(0.0f, 0.36f, 9.15f));
                m = glm::rotate(m, glm::radians(-drawbridgeAngle), glm::vec3(1.0f, 0.0f, 0.0f));
                m = glm::translate(m, glm::vec3(0.0f, 0.0f, 3.1f));
                return m;
            }
        });

        for (size_t i = 0; i < towerPositions.size(); ++i) {
            glm::mat4 flagTransform(1.0f);
            flagTransform = glm::translate(flagTransform, towerPositions[i] + glm::vec3(-0.12f, 5.18f, 0.0f));
            flagTransform = glm::rotate(flagTransform, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            addStatic("animated_flag_" + std::to_string(i + 1), flagMesh, glm::vec3(0.72f, 0.82f, 1.0f), tex["cloth"], flagTransform, 20.0f, 1);

            glm::mat4 pole(1.0f);
            pole = glm::translate(pole, towerPositions[i] + glm::vec3(-0.2f, 4.72f, 0.0f));
            pole = glm::scale(pole, glm::vec3(0.08f, 1.75f, 0.08f));
            addStatic("flag_pole_" + std::to_string(i + 1), unitBox, glm::vec3(0.9f, 0.76f, 0.56f), tex["wood"], pole, 18.0f);
        }

        addStatic("barracks", barracksModel, glm::vec3(1.0f), tex["wood"], transformTRS(glm::vec3(-5.3f, 0.0f, -2.7f), glm::vec3(1.0f)), 22.0f);
        addStatic("stable", stableModel, glm::vec3(1.0f), tex["wood"], transformTRS(glm::vec3(5.7f, 0.0f, -2.2f), glm::vec3(0.98f)), 18.0f);

        for (int i = 0; i < 4; ++i) {
            glm::mat4 bedTransform(1.0f);
            bedTransform = glm::translate(bedTransform, glm::vec3(-6.7f + i * 0.9f, 0.05f, -0.9f));
            bedTransform = glm::rotate(bedTransform, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            addStatic("bed_" + std::to_string(i + 1), bedModel, glm::vec3(1.0f), tex["straw"], bedTransform, 16.0f);
        }


        for (int i = 0; i < 3; ++i) {
            glm::mat4 axeTransform(1.0f);
            axeTransform = glm::translate(axeTransform, glm::vec3(-3.0f + i * 3.0f, 1.85f, -7.68f));
            axeTransform = glm::rotate(axeTransform, glm::radians(12.0f + i * 8.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            addStatic("wall_axe_" + std::to_string(i + 1), axeModel, glm::vec3(1.0f), tex["metal"], axeTransform, 64.0f);
        }


        const std::array<glm::vec3, 6> torchPositions = {
            glm::vec3(-3.0f, 2.38f, 7.62f), glm::vec3(3.0f, 2.38f, 7.62f),
            glm::vec3(-8.55f, 2.2f, 2.8f), glm::vec3(8.55f, 2.2f, 2.8f),
            glm::vec3(-2.5f, 2.1f, -7.55f), glm::vec3(2.5f, 2.1f, -7.55f)
        };

        for (size_t i = 0; i < torchPositions.size(); ++i) {
            glm::mat4 torchTransform(1.0f);
            torchTransform = glm::translate(torchTransform, torchPositions[i] - glm::vec3(0.0f, 1.05f, 0.0f));
            torchTransform = glm::scale(torchTransform, glm::vec3(0.62f));
            addStatic("torch_body_" + std::to_string(i + 1), torchModel, glm::vec3(0.9f, 0.7f, 0.45f), tex["wood"], torchTransform, 20.0f);
            addStatic("visible_flame_" + std::to_string(i + 1), flameMarker, glm::vec3(1.0f, 0.85f, 0.28f), tex["flame"], translateScale(torchPositions[i], glm::vec3(1.0f)), 8.0f, 0, true);
        }

        while (!glfwWindowShouldClose(window)) {
            const float currentFrame = static_cast<float>(glfwGetTime());
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            processInput(window);

            const float targetAngle = drawbridgeRaised ? 72.0f : 0.0f;
            const float speed = 65.0f * deltaTime;
            if (drawbridgeAngle < targetAngle) {
                drawbridgeAngle = std::min(drawbridgeAngle + speed, targetAngle);
            } else if (drawbridgeAngle > targetAngle) {
                drawbridgeAngle = std::max(drawbridgeAngle - speed, targetAngle);
            }

            glClearColor(0.08f, 0.11f, 0.14f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            shader.use();
            const glm::mat4 projection = glm::perspective(glm::radians(60.0f), static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT), 0.1f, 120.0f);
            const glm::mat4 view = camera.viewMatrix();
            shader.setMat4("projection", projection);
            shader.setMat4("view", view);
            shader.setVec3("viewPos", camera.position);
            shader.setFloat("uTime", currentFrame);
            setupLights(shader, torchPositions);

            for (const SceneObject& obj : objects) {
                drawSceneObject(obj, shader, currentFrame);
            }

            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        for (GLuint texture : ownedTextures) {
            deleteTexture(texture);
        }
        glfwTerminate();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        for (GLuint texture : ownedTextures) {
            deleteTexture(texture);
        }
        glfwTerminate();
        return 1;
    }
}
