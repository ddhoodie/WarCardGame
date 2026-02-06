#pragma once
#include <glm/glm.hpp>

struct GLFWwindow; 

struct Camera {
    glm::vec3 pos{ 0.0f, 1.2f, 2.5f };
    glm::vec3 front{ 0.0f, -0.25f, -1.0f };
    glm::vec3 up{ 0.0f, 1.0f, 0.0f };

    float yaw = -90.0f;
    float pitch = -15.0f;
    float fov = 45.0f;

    bool  firstMouse = true;
    float lastX = 0.0f, lastY = 0.0f;

    void onMouse(double xpos, double ypos);
    void onScroll(double yoffset);
};

void updateCameraFPS(GLFWwindow* w, Camera& cam, float dt);