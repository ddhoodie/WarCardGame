#include "Helper2D.h"
#include "Camera.h"

#include <cmath> 
#include <algorithm>
#include <string>
#include <glm/gtc/matrix_transform.hpp>

unsigned int VAO_FOLD = 0;

void setTextureParams()
{
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void setTextureParamsRepeat()
{
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void setTextureParamsClamp()
{
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void drawDigit(unsigned int shader,
    unsigned int texDigit,
    float x, float y,
    float w, float h,
    unsigned int VAO)
{
    glUseProgram(shader);

    glUniform2f(glGetUniformLocation(shader, "uPos"), x, y);
    glUniform2f(glGetUniformLocation(shader, "uScale"), w, h);
    glUniform4f(glGetUniformLocation(shader, "uUV"),
        0.0f, 0.0f,
        1.0f, 1.0f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texDigit);
    glUniform1i(glGetUniformLocation(shader, "uTexture"), 0);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void drawNumber(unsigned int shader,
    int number,
    float x, float y,
    float size,
    unsigned int VAO)
{
    std::string s = std::to_string(number);
    float spacing = size * 1.6f;

    for (int i = 0; i < s.size(); i++)
    {
        int digit = s[i] - '0';
        drawDigit(shader,
            digitTextures[digit],
            x + i * spacing,
            y,
            size,
            size,
            VAO);
    }
}

bool isHover(float mouseX, float mouseY, float centerX, float centerY, float scaleX, float scaleY) {
    return mouseX >= (centerX - scaleX) && mouseX <= (centerX + scaleX) &&
        mouseY >= (centerY - scaleY) && mouseY <= (centerY + scaleY);
}

void drawQuad(unsigned int shader, unsigned int texture,
    float x, float y, float sx, float sy,
    unsigned int VAO, bool shadow)
{
    glUseProgram(shader);

    glUniform2f(glGetUniformLocation(shader, "uPos"), x, y);
    glUniform2f(glGetUniformLocation(shader, "uScale"), sx, sy);
    glUniform1i(glGetUniformLocation(shader, "uShadow"), shadow ? 1 : 0);
    glUniform4f(glGetUniformLocation(shader, "uUV"), 0.0f, 0.0f, 1.0f, 1.0f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(glGetUniformLocation(shader, "uTexture"), 0);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

float easeOutCubic(float t) {
    return 1.0f - powf(1.0f - t, 3.0f);
}

void cardIDtoSuitRank(int cardID, int& outSuit, int& outRank) {
    outSuit = cardID / 13;
    outRank = (cardID % 13) + 1;
}

float toNDC_X(double x, int w)
{
    return float((x / w) * 2.0 - 1.0);
}

float toNDC_Y(double y, int h)
{
    return float(-((y / h) * 2.0 - 1.0));
}

void initFoldVAO()
{
    glGenVertexArrays(1, &VAO_FOLD);
}

bool mousePressedOnce(GLFWwindow* w, int button)
{
    static int last[8] = {};
    int now = glfwGetMouseButton(w, button);
    bool pressed = (now == GLFW_PRESS && last[button] == GLFW_RELEASE);
    last[button] = now;
    return pressed;
}

bool keyPressedOnce(GLFWwindow* w, int key)
{
    static int last[512] = {};
    int now = glfwGetKey(w, key);
    bool pressed = (now == GLFW_PRESS && last[key] == GLFW_RELEASE);
    last[key] = now;
    return pressed;
}

float expApproach(float cur, float target, float k, float dt)
{
    float a = 1.0f - std::exp(-k * dt);
    return cur + (target - cur) * a;
}

bool hoverWorldPointPx(GLFWwindow* window,
    const Camera& cam,
    const glm::vec3& worldPos,
    int ww, int wh,
    float radiusPx)
{
    double mx, my;
    glfwGetCursorPos(window, &mx, &my);

    glm::mat4 V = glm::lookAt(cam.pos, cam.pos + cam.front, cam.up);
    glm::mat4 P = glm::perspective(glm::radians(cam.fov), (float)ww / (float)wh, 0.1f, 100.0f);

    glm::vec3 sp = glm::project(worldPos, V, P, glm::vec4(0, 0, ww, wh));

    float sx = sp.x;
    float sy = (float)wh - sp.y;

    float dx = (float)mx - sx;
    float dy = (float)my - sy;
    return (dx * dx + dy * dy) <= (radiusPx * radiusPx);
}

void limitFPS(double targetFps)
{
    static double lastTimeForRefresh = 0.0;
    if (lastTimeForRefresh == 0.0) lastTimeForRefresh = glfwGetTime();

    while (glfwGetTime() < lastTimeForRefresh + 1.0 / targetFps) {}
    lastTimeForRefresh += 1.0 / targetFps;
}


unsigned int digitTextures[10] = { 0 };
