#include "Mesh.h"
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

void Mesh::destroy() {
    if (vbo) glDeleteBuffers(1, &vbo);
    if (vao) glDeleteVertexArrays(1, &vao);
    vbo = vao = 0;
    vertexCount = 0;
}


static Mesh makeQuad(float y, float nx, float ny, float nz, bool flipUV) {
    float u0 = flipUV ? 1.0f : 0.0f;
    float u1 = flipUV ? 0.0f : 1.0f;

    float v[] = {
        // x     y     z     nx ny nz   u   v
        -0.35f, y, -0.50f,  nx,ny,nz,  u0, 0,
        -0.35f, y,  0.50f,  nx,ny,nz,  u0, 1,
         0.35f, y,  0.50f,  nx,ny,nz,  u1, 1,

        -0.35f, y, -0.50f,  nx,ny,nz,  u0, 0,
         0.35f, y,  0.50f,  nx,ny,nz,  u1, 1,
         0.35f, y, -0.50f,  nx,ny,nz,  u1, 0,
    };

    Mesh m{};
    glGenVertexArrays(1, &m.vao);
    glGenBuffers(1, &m.vbo);

    glBindVertexArray(m.vao);
    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);

    int stride = (3 + 3 + 2) * sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    m.vertexCount = 6;
    return m;
}

static void pushFace(std::vector<float>& v,
    const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3,
    const glm::vec3& n,
    bool flipUV = false)
{
    float u0 = flipUV ? 1.0f : 0.0f;
    float u1 = flipUV ? 0.0f : 1.0f;

    auto add = [&](const glm::vec3& p, float u, float t) {
        v.push_back(p.x); v.push_back(p.y); v.push_back(p.z);
        v.push_back(n.x); v.push_back(n.y); v.push_back(n.z);
        v.push_back(u);   v.push_back(t);
        };

    add(p0, u0, 0); add(p1, u0, 1); add(p2, u1, 1);
    add(p0, u0, 0); add(p2, u1, 1); add(p3, u1, 0);
}

static Mesh makeMeshFromVerts(const float* v, size_t bytes, int vertexCount)
{
    Mesh m{};
    glGenVertexArrays(1, &m.vao);
    glGenBuffers(1, &m.vbo);

    glBindVertexArray(m.vao);
    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
    glBufferData(GL_ARRAY_BUFFER, bytes, v, GL_STATIC_DRAW);

    int stride = (3 + 3 + 2) * sizeof(float);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    m.vertexCount = vertexCount;
    return m;
}

Mesh MakeCardTopMesh()
{
    const float hw = 0.35f;
    const float hz = 0.50f;
    const float t = 0.006f;

    const float v[] = {
        -hw, +t, -hz,   0,1,0,   0,0,
        -hw, +t, +hz,   0,1,0,   0,1,
        +hw, +t, +hz,   0,1,0,   1,1,

        -hw, +t, -hz,   0,1,0,   0,0,
        +hw, +t, +hz,   0,1,0,   1,1,
        +hw, +t, -hz,   0,1,0,   1,0,
    };

    return makeMeshFromVerts(v, sizeof(v), 6);
}

Mesh MakeCardBottomMesh()
{
    const float hw = 0.35f;
    const float hz = 0.50f;
    const float t = 0.006f;

    const float v[] = {
        -hw, -t, -hz,   0,-1,0,  0,0,
        +hw, -t, +hz,   0,-1,0,  1,1,
        -hw, -t, +hz,   0,-1,0,  0,1,

        -hw, -t, -hz,   0,-1,0,  0,0,
        +hw, -t, -hz,   0,-1,0,  1,0,
        +hw, -t, +hz,   0,-1,0,  1,1,
    };

    return makeMeshFromVerts(v, sizeof(v), 6);
}

Mesh MakeCardSideMesh(int cornerSeg = 12)
{
    const float hw = 0.35f;
    const float hz = 0.50f;
    const float t = 0.006f;   // half thickness 
    const float r = 0.08f;    // radius 

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

    std::vector<float> data;
    data.reserve(pts.size() * 6 * 8);

    auto pushV = [&](float x, float y, float z, const glm::vec3& n) {
        data.push_back(x); data.push_back(y); data.push_back(z);
        data.push_back(n.x); data.push_back(n.y); data.push_back(n.z);
        data.push_back(0.0f); data.push_back(0.0f); 
        };

    for (size_t i = 0; i < pts.size(); i++) {
        glm::vec2 p0 = pts[i];
        glm::vec2 p1 = pts[(i + 1) % pts.size()];

        glm::vec2 d = p1 - p0;
        float len = std::sqrt(d.x * d.x + d.y * d.y);
        if (len < 1e-6f) continue;
        d /= len;

        glm::vec3 n = glm::normalize(glm::vec3(d.y, 0.0f, -d.x));

        pushV(p0.x, +t, p0.y, n);
        pushV(p0.x, -t, p0.y, n);
        pushV(p1.x, -t, p1.y, n);

        pushV(p0.x, +t, p0.y, n);
        pushV(p1.x, -t, p1.y, n);
        pushV(p1.x, +t, p1.y, n);
    }

    Mesh m{};
    glGenVertexArrays(1, &m.vao);
    glGenBuffers(1, &m.vbo);

    glBindVertexArray(m.vao);
    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);

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

Mesh MakeTableMesh() {
    // Sto = veliki quad
    float v[] = {
        // pos                 normal   uv
        -3.f, 0.f, -3.f,      0,1,0,   0,0,
        -3.f, 0.f,  3.f,      0,1,0,   0,4,
         3.f, 0.f,  3.f,      0,1,0,   4,4,

        -3.f, 0.f, -3.f,      0,1,0,   0,0,
         3.f, 0.f,  3.f,      0,1,0,   4,4,
         3.f, 0.f, -3.f,      0,1,0,   4,0,
    };

    Mesh m{};
    glGenVertexArrays(1, &m.vao);
    glGenBuffers(1, &m.vbo);

    glBindVertexArray(m.vao);
    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);

    int stride = (3 + 3 + 2) * sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    m.vertexCount = 6;
    return m;
}

#include <cmath>

static Mesh makeMeshFromData(const std::vector<float>& data) {
    Mesh m{};
    glGenVertexArrays(1, &m.vao);
    glGenBuffers(1, &m.vbo);

    glBindVertexArray(m.vao);
    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);

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
    d.push_back(u); d.push_back(v);
}

Mesh MakeChipTopMesh(int segments, float r, float halfT) {
    std::vector<float> d;
    d.reserve(segments * 3 * 8);

    const float y = +halfT;
    const float nx = 0, ny = 1, nz = 0;

    for (int i = 0; i < segments; i++) {
        float a0 = (float)i / segments * 2.0f * 3.14159265f;
        float a1 = (float)(i + 1) / segments * 2.0f * 3.14159265f;

        float x0 = r * cosf(a0), z0 = r * sinf(a0);
        float x1 = r * cosf(a1), z1 = r * sinf(a1);

        float uc = 0.5f, vc = 0.5f;
        float u0 = 0.5f + x0 / (2 * r), v0 = 0.5f + z0 / (2 * r);
        float u1 = 0.5f + x1 / (2 * r), v1 = 0.5f + z1 / (2 * r);

        pushV(d, 0, y, 0, nx, ny, nz, uc, vc);
        pushV(d, x0, y, z0, nx, ny, nz, u0, v0);
        pushV(d, x1, y, z1, nx, ny, nz, u1, v1);
    }

    return makeMeshFromData(d);
}

Mesh MakeChipBottomMesh(int segments, float r, float halfT) {
    std::vector<float> d;
    d.reserve(segments * 3 * 8);

    const float y = -halfT;
    const float nx = 0, ny = -1, nz = 0;

    for (int i = 0; i < segments; i++) {
        float a0 = (float)i / segments * 2.0f * 3.14159265f;
        float a1 = (float)(i + 1) / segments * 2.0f * 3.14159265f;

        float x0 = r * cosf(a0), z0 = r * sinf(a0);
        float x1 = r * cosf(a1), z1 = r * sinf(a1);

        float uc = 0.5f, vc = 0.5f;
        // flip UV na bottom da izgleda isto
        float u0 = 0.5f + x0 / (2 * r), v0 = 0.5f - z0 / (2 * r);
        float u1 = 0.5f + x1 / (2 * r), v1 = 0.5f - z1 / (2 * r);

        // winding za donju stranu (da normal ide na dole)
        pushV(d, 0, y, 0, nx, ny, nz, uc, vc);
        pushV(d, x1, y, z1, nx, ny, nz, u1, v1);
        pushV(d, x0, y, z0, nx, ny, nz, u0, v0);
    }

    return makeMeshFromData(d);
}

Mesh MakeChipSideMesh(int segments, float r, float halfT) {
    std::vector<float> d;
    d.reserve(segments * 6 * 8);

    float y0 = -halfT;
    float y1 = +halfT;

    for (int i = 0; i < segments; i++) {
        float a0 = (float)i / segments * 2.0f * 3.14159265f;
        float a1 = (float)(i + 1) / segments * 2.0f * 3.14159265f;

        float x0 = r * cosf(a0), z0 = r * sinf(a0);
        float x1 = r * cosf(a1), z1 = r * sinf(a1);

        float nx0 = cosf(a0), nz0 = sinf(a0);
        float nx1 = cosf(a1), nz1 = sinf(a1);

        float u0 = (float)i / segments;
        float u1 = (float)(i + 1) / segments;

        // dva trougla: (p0b, p1t, p0t) + (p0b, p1b, p1t)
        pushV(d, x0, y0, z0, nx0, 0, nz0, u0, 0);
        pushV(d, x1, y1, z1, nx1, 0, nz1, u1, 1);
        pushV(d, x0, y1, z0, nx0, 0, nz0, u0, 1);

        pushV(d, x0, y0, z0, nx0, 0, nz0, u0, 0);
        pushV(d, x1, y0, z1, nx1, 0, nz1, u1, 0);
        pushV(d, x1, y1, z1, nx1, 0, nz1, u1, 1);
    }

    return makeMeshFromData(d);
}

Mesh MakeBoxMesh(float hx, float hy, float hz)
{
    std::vector<float> d;
    d.reserve(36 * 8);

    auto quad = [&](glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 n) {
        auto push = [&](glm::vec3 p, float u, float v) {
            d.push_back(p.x); d.push_back(p.y); d.push_back(p.z);
            d.push_back(n.x); d.push_back(n.y); d.push_back(n.z);
            d.push_back(u);   d.push_back(v);
            };
        // 2 triangles
        push(p0, 0, 0); push(p1, 1, 0); push(p2, 1, 1);
        push(p0, 0, 0); push(p2, 1, 1); push(p3, 0, 1);
        };

    // Front (+Z)
    quad({ -hx,-hy,+hz }, { +hx,-hy,+hz }, { +hx,+hy,+hz }, { -hx,+hy,+hz }, { 0,0,1 });
    // Back (-Z)
    quad({ +hx,-hy,-hz }, { -hx,-hy,-hz }, { -hx,+hy,-hz }, { +hx,+hy,-hz }, { 0,0,-1 });
    // Right (+X)
    quad({ +hx,-hy,+hz }, { +hx,-hy,-hz }, { +hx,+hy,-hz }, { +hx,+hy,+hz }, { 1,0,0 });
    // Left (-X)
    quad({ -hx,-hy,-hz }, { -hx,-hy,+hz }, { -hx,+hy,+hz }, { -hx,+hy,-hz }, { -1,0,0 });
    // Top (+Y)
    quad({ -hx,+hy,+hz }, { +hx,+hy,+hz }, { +hx,+hy,-hz }, { -hx,+hy,-hz }, { 0,1,0 });
    // Bottom (-Y)
    quad({ -hx,-hy,-hz }, { +hx,-hy,-hz }, { +hx,-hy,+hz }, { -hx,-hy,+hz }, { 0,-1,0 });

    return makeMeshFromData(d);
}


