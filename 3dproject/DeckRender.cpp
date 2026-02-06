#include "DeckRender.h"
#include "Renderer3D.h"
#include "Camera.h"
#include "Util.h"
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <vector>
#include <cmath>
#include "Helper2D.h"

static glm::mat4 makeModel(const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scale) {
    glm::mat4 M(1.0f);
    M = glm::translate(M, pos);
    M = glm::rotate(M, rot.y, glm::vec3(0, 1, 0));
    M = glm::rotate(M, rot.x, glm::vec3(1, 0, 0));
    M = glm::rotate(M, rot.z, glm::vec3(0, 0, 1));
    M = glm::scale(M, scale);
    return M;
}

static Mesh makeMeshFromData(const std::vector<float>& data) {
    Mesh m{};
    glGenVertexArrays(1, &m.vao);
    glGenBuffers(1, &m.vbo);

    glBindVertexArray(m.vao);
    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(data.size() * sizeof(float)), data.data(), GL_STATIC_DRAW);

    int stride = (3 + 3 + 2) * sizeof(float);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    m.vertexCount = (int)(data.size() / 8);
    return m;
}

static void pushV(std::vector<float>& d,
    float x, float y, float z,
    float nx, float ny, float nz,
    float u, float v)
{
    d.push_back(x); d.push_back(y); d.push_back(z);
    d.push_back(nx); d.push_back(ny); d.push_back(nz);
    d.push_back(u);  d.push_back(v);
}

static std::vector<glm::vec2> makeRoundedRectOutline(float hw, float hz, float r, int cornerSeg) {
    std::vector<glm::vec2> pts;
    pts.reserve(4 * (cornerSeg + 1));

    auto addArc = [&](glm::vec2 c, float a0, float a1) {
        for (int i = 0; i <= cornerSeg; i++) {
            float s = (float)i / (float)cornerSeg;
            float a = a0 + (a1 - a0) * s;
            pts.push_back(c + glm::vec2(std::cos(a), std::sin(a)) * r);
        }
        };

    glm::vec2 cTR(hw - r, hz - r);
    glm::vec2 cTL(-hw + r, hz - r);
    glm::vec2 cBL(-hw + r, -hz + r);
    glm::vec2 cBR(hw - r, -hz + r);

    addArc(cTR, 0.0f, glm::half_pi<float>());              
    addArc(cTL, glm::half_pi<float>(), glm::pi<float>());    
    addArc(cBL, glm::pi<float>(), glm::pi<float>() * 1.5f);   
    addArc(cBR, glm::pi<float>() * 1.5f, glm::two_pi<float>()); 

    return pts;
}

static Mesh makeDeckTop(float hw, float hz, float halfT) {
    const float y = +halfT;
    std::vector<float> d;
    d.reserve(6 * 8);

    pushV(d, -hw, y, -hz, 0, 1, 0, 0, 0);
    pushV(d, -hw, y, +hz, 0, 1, 0, 0, 1);
    pushV(d, +hw, y, +hz, 0, 1, 0, 1, 1);

    pushV(d, -hw, y, -hz, 0, 1, 0, 0, 0);
    pushV(d, +hw, y, +hz, 0, 1, 0, 1, 1);
    pushV(d, +hw, y, -hz, 0, 1, 0, 1, 0);

    return makeMeshFromData(d);
}

static Mesh makeDeckBottom(float hw, float hz, float halfT) {
    const float y = -halfT;
    std::vector<float> d;
    d.reserve(6 * 8);

    pushV(d, -hw, y, -hz, 0, -1, 0, 0, 0);
    pushV(d, +hw, y, +hz, 0, -1, 0, 1, 1);
    pushV(d, -hw, y, +hz, 0, -1, 0, 0, 1);

    pushV(d, -hw, y, -hz, 0, -1, 0, 0, 0);
    pushV(d, +hw, y, -hz, 0, -1, 0, 1, 0);
    pushV(d, +hw, y, +hz, 0, -1, 0, 1, 1);

    return makeMeshFromData(d);
}

static Mesh makeDeckSide(float hw, float hz, float halfT, float cornerR, int cornerSeg) {
    std::vector<glm::vec2> pts = makeRoundedRectOutline(hw, hz, cornerR, cornerSeg);

    std::vector<float> data;
    data.reserve(pts.size() * 6 * 8);

    auto pushSideV = [&](float x, float y, float z, const glm::vec3& n, float u, float v) {
        pushV(data, x, y, z, n.x, n.y, n.z, u, v);
        };

    float uAccum = 0.0f;
    float totalLen = 0.0f;

    for (size_t i = 0; i < pts.size(); i++) {
        glm::vec2 p0 = pts[i];
        glm::vec2 p1 = pts[(i + 1) % pts.size()];
        totalLen += glm::length(p1 - p0);
    }

    for (size_t i = 0; i < pts.size(); i++) {
        glm::vec2 p0 = pts[i];
        glm::vec2 p1 = pts[(i + 1) % pts.size()];

        glm::vec2 e = p1 - p0;
        float len = glm::length(e);
        if (len < 1e-6f) continue;

        glm::vec2 d = e / len;

        glm::vec3 n = glm::normalize(glm::vec3(d.y, 0.0f, -d.x));

        float u0 = (totalLen > 0.0f) ? (uAccum / totalLen) : 0.0f;
        float u1 = (totalLen > 0.0f) ? ((uAccum + len) / totalLen) : 1.0f;

        float y0 = -halfT;
        float y1 = +halfT;

        pushSideV(p0.x, y0, p0.y, n, u0, 0.0f);
        pushSideV(p1.x, y1, p1.y, n, u1, 1.0f);
        pushSideV(p0.x, y1, p0.y, n, u0, 1.0f);

        pushSideV(p0.x, y0, p0.y, n, u0, 0.0f);
        pushSideV(p1.x, y0, p1.y, n, u1, 0.0f);
        pushSideV(p1.x, y1, p1.y, n, u1, 1.0f);

        uAccum += len;
    }

    return makeMeshFromData(data);
}

void DeckRender::init(float thickness, const char* backTexturePath) {
    const float hw = 0.35f;
    const float hz = 0.50f;
    const float cornerR = 0.08f;
    const int cornerSeg = 12;

    float halfT = thickness * 0.5f;

    top = makeDeckTop(hw, hz, halfT);
    bottom = makeDeckBottom(hw, hz, halfT);
    side = makeDeckSide(hw, hz, halfT, cornerR, cornerSeg);

    texBack = loadImageToTexture(backTexturePath);
    glBindTexture(GL_TEXTURE_2D, texBack);
    setTextureParamsClamp();
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    texSide = makeSolidTexture(230, 230, 230, 255);
}

void DeckRender::draw(Renderer3D& r3d, const Camera& cam) {
    glm::mat4 M = makeModel(pos, rot, scale);

    glDisable(GL_CULL_FACE);

    glUseProgram(r3d.shader);

    glUniform1i(glGetUniformLocation(r3d.shader, "uIsCard"), 1);   // <<< BITNO
    glUniform1i(glGetUniformLocation(r3d.shader, "uUseAlpha"), 0);

    r3d.draw(top, texBack, M, cam);
    r3d.draw(bottom, texBack, M, cam);
    r3d.draw(side, texSide, M, cam);
}


void DeckRender::destroy() {
    top.destroy();
    bottom.destroy();
    side.destroy();
    if (texBack) glDeleteTextures(1, &texBack);
    if (texSide) glDeleteTextures(1, &texSide);
    texBack = texSide = 0;
}
