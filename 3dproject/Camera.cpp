#include "Camera.h"
#include "constants.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

void Camera::onMouse(double xpos, double ypos) {
    if (firstMouse) {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }

    float xoffset = (float)xpos - lastX;
    float yoffset = lastY - (float)ypos; // invert Y

    lastX = (float)xpos;
    lastY = (float)ypos;

    xoffset *= Cfg::MOUSE_SENS;
    yoffset *= Cfg::MOUSE_SENS;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)  pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 dir;
    dir.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    dir.y = sin(glm::radians(pitch));
    dir.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(dir);
}

void updateCameraFPS(GLFWwindow* w, Camera& cam, float dt)
{
    // mouse look (koristi state iz cam)
    double mx, my;
    glfwGetCursorPos(w, &mx, &my);
    cam.onMouse(mx, my);

    // movement
    float moveSpeed = Cfg::MOVE_SPEED;

    glm::vec3 right = glm::normalize(glm::cross(cam.front, cam.up));
    glm::vec3 up = cam.up;

    if (glfwGetKey(w, GLFW_KEY_W) == GLFW_PRESS) cam.pos += cam.front * moveSpeed * dt;
    if (glfwGetKey(w, GLFW_KEY_S) == GLFW_PRESS) cam.pos -= cam.front * moveSpeed * dt;
    if (glfwGetKey(w, GLFW_KEY_A) == GLFW_PRESS) cam.pos -= right * moveSpeed * dt;
    if (glfwGetKey(w, GLFW_KEY_D) == GLFW_PRESS) cam.pos += right * moveSpeed * dt;
    if (glfwGetKey(w, GLFW_KEY_Q) == GLFW_PRESS) cam.pos += up * moveSpeed * dt;
    if (glfwGetKey(w, GLFW_KEY_E) == GLFW_PRESS) cam.pos -= up * moveSpeed * dt;
}

void Camera::onScroll(double yoffset) {
    fov -= (float)yoffset;
    if (fov < 1.0f)  fov = 1.0f;
    if (fov > 75.0f) fov = 75.0f; // ili 45, kako želiš
}