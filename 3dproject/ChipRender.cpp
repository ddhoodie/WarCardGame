#include "ChipRender.h"
#include "Util.h"
#include "Helper2D.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec4.hpp> 

static glm::mat4 makeModel(const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scale) {
    glm::mat4 M(1.0f);
    M = glm::translate(M, pos);
    M = glm::rotate(M, rot.y, glm::vec3(0, 1, 0));
    M = glm::rotate(M, rot.x, glm::vec3(1, 0, 0));
    M = glm::rotate(M, rot.z, glm::vec3(0, 0, 1));
    M = glm::scale(M, scale);
    return M;
}

void ChipRender::init(int segments, float radius, float thickness,
    const char* facePngPath,
    const glm::u8vec4& sideColor)
{
    float halfT = thickness * 0.5f;

    top = MakeChipTopMesh(segments, radius, halfT);
    bottom = MakeChipBottomMesh(segments, radius, halfT);
    side = MakeChipSideMesh(segments, radius, halfT);

    texFace = loadImageToTexture(facePngPath);
    glBindTexture(GL_TEXTURE_2D, texFace);
    setTextureParamsClamp();
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    texSide = makeSolidTexture(sideColor.r, sideColor.g, sideColor.b, sideColor.a);
}

void ChipRender::draw(Renderer3D& r3d, const Camera& cam) {
    glm::mat4 M = makeModel(pos, rot, scale);

    glUseProgram(r3d.shader);

    glUniform1i(glGetUniformLocation(r3d.shader, "uIsCard"), 0);

    glUniform1i(glGetUniformLocation(r3d.shader, "uIsChip"), 1);
    glUniform1f(glGetUniformLocation(r3d.shader, "uChipRadius"), 0.18f);

    glUniform1i(glGetUniformLocation(r3d.shader, "uUseAlpha"), 0);

    r3d.draw(top, texFace, M, cam);
    r3d.draw(bottom, texFace, M, cam);

    glUniform1i(glGetUniformLocation(r3d.shader, "uUseAlpha"), 0);
    r3d.draw(side, texSide, M, cam);

    glUniform1i(glGetUniformLocation(r3d.shader, "uIsChip"), 0);
}

void ChipRender::destroy() {
    top.destroy();
    bottom.destroy();
    side.destroy();
    if (texFace) glDeleteTextures(1, &texFace);
    if (texSide) glDeleteTextures(1, &texSide);
    texFace = texSide = 0;
}
