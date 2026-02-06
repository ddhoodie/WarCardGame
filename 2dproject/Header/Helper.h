#pragma once
#include <GL/glew.h>

void setTextureParams();
bool isHover(float mouseX, float mouseY, float centerX, float centerY, float scaleX, float scaleY);
void drawQuad(unsigned int shader, unsigned int texture,
    float x, float y, float sx, float sy,
    unsigned int VAO, bool shadow = false);
float easeOutCubic(float t);
void cardIDtoSuitRank(int cardID, int& outSuit, int& outRank);
unsigned int getCardTextureByID(int cardID);
void drawNumber(unsigned int shader,
    int number,
    float x, float y,
    float size,
    unsigned int VAO);
inline float lerp(float a, float b, float t){return a + (b - a) * t;};
float toNDC_X(double x, int w);
float toNDC_Y(double y, int h);
void initFoldVAO();

extern unsigned int VAO_FOLD;
extern unsigned int cardFaceTextures[4][14];
extern unsigned int digitTextures[10];