#include "TableRender.h"
#include "Renderer3D.h"
#include "Camera.h"
#include "Util.h"
#include "Helper2D.h"
#include <glm/gtc/matrix_transform.hpp>

Mesh MakeChipTopMesh(int segments, float r, float halfT);
Mesh MakeChipBottomMesh(int segments, float r, float halfT);
Mesh MakeChipSideMesh(int segments, float r, float halfT);

static glm::mat4 makeModel(const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scale) {
    glm::mat4 M(1.0f);
    M = glm::translate(M, pos);
    M = glm::rotate(M, rot.y, glm::vec3(0, 1, 0));
    M = glm::rotate(M, rot.x, glm::vec3(1, 0, 0));
    M = glm::rotate(M, rot.z, glm::vec3(0, 0, 1));
    M = glm::scale(M, scale);
    return M;
}

void TableRender::init(int segments,
    float mainRadius, float mainThickness,
    const char* feltTexturePath,
    float baseRadius, float baseHeight)
{

    mainThickness_ = mainThickness;
    baseHeight_ = baseHeight;

    // ===== gornja ploča =====
    float halfMainT = mainThickness * 0.5f;

    top = MakeChipTopMesh(segments, mainRadius, halfMainT);
    bottom = MakeChipBottomMesh(segments, mainRadius, halfMainT);
    side = MakeChipSideMesh(segments, mainRadius, halfMainT);

    texFelt = loadImageToTexture(feltTexturePath);
    glBindTexture(GL_TEXTURE_2D, texFelt);
    setTextureParamsRepeat();
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    texSide = makeSolidTexture(200, 0, 0, 255); // crvena ivica

    // ===== baza =====
    float halfBaseH = baseHeight * 0.5f;

    baseTop = MakeChipTopMesh(segments, baseRadius, halfBaseH);
    baseBottom = MakeChipBottomMesh(segments, baseRadius, halfBaseH);
    baseSide = MakeChipSideMesh(segments, baseRadius, halfBaseH);

    texBase = makeSolidTexture(35, 35, 35, 255); // tamno siva 
}


void TableRender::draw(Renderer3D& r3d, const Camera& cam) {
    glm::mat4 M = makeModel(pos, rot, scale);

    glUseProgram(r3d.shader);
    glUniform1i(glGetUniformLocation(r3d.shader, "uIsCard"), 0);
    glUniform1i(glGetUniformLocation(r3d.shader, "uUseAlpha"), 0);

    // 1) gornji sto (kao do sad)
    glFrontFace(GL_CW);
    r3d.draw(top, texFelt, M, cam);
    glFrontFace(GL_CCW);
    r3d.draw(bottom, texFelt, M, cam);
    r3d.draw(side, texSide, M, cam);

    // 2) baza: center postolja ide ispod ploče
   // ploča je centrirana oko y=0 sa halfMainT, baza oko svog centra
    float halfMainT = mainThickness_ * 0.5f;
    float halfBaseH = baseHeight_ * 0.5f;

    glm::mat4 Mbase = M;
    Mbase = glm::translate(Mbase, glm::vec3(0.0f, -(halfMainT + halfBaseH), 0.0f));

    glFrontFace(GL_CW);
    r3d.draw(baseTop, texBase, Mbase, cam);
    glFrontFace(GL_CCW);
    r3d.draw(baseBottom, texBase, Mbase, cam);
    r3d.draw(baseSide, texBase, Mbase, cam);
}

void TableRender::destroy() {
    top.destroy(); bottom.destroy(); side.destroy();
    baseTop.destroy(); baseBottom.destroy(); baseSide.destroy();

    if (texFelt) glDeleteTextures(1, &texFelt);
    if (texSide) glDeleteTextures(1, &texSide);
    if (texBase) glDeleteTextures(1, &texBase);

    texFelt = texSide = texBase = 0;
}
