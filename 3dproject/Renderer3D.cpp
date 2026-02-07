#include "Renderer3D.h"
#include "Util.h"
#include "GameState.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

static glm::mat4 makeM(const Transform& t) {
    glm::mat4 M(1.0f);
    M = glm::translate(M, t.pos);
    M = glm::rotate(M, t.rot.y, glm::vec3(0, 1, 0));
    M = glm::rotate(M, t.rot.x, glm::vec3(1, 0, 0));
    M = glm::rotate(M, t.rot.z, glm::vec3(0, 0, 1));
    M = glm::scale(M, t.scale);
    return M;
}

void Renderer3D::init() {
    shader = createShader("phong.vert", "phong.frag");
}

void Renderer3D::begin(const Camera& cam, int w, int h) {
    glUseProgram(shader);

    glUniform1f(glGetUniformLocation(shader, "uHalfW"), 0.35f);
    glUniform1f(glGetUniformLocation(shader, "uHalfH"), 0.50f);
    glUniform1f(glGetUniformLocation(shader, "uRadius"), 0.08f);
    glUniform1f(glGetUniformLocation(shader, "uLightIntensity"), 1.4f);

    glm::mat4 V = glm::lookAt(cam.pos, cam.pos + cam.front, cam.up);
    glm::mat4 P = glm::perspective(glm::radians(cam.fov), (float)w / (float)h, 0.1f, 100.0f);

    glUniformMatrix4fv(glGetUniformLocation(shader, "uV"), 1, GL_FALSE, glm::value_ptr(V));
    glUniformMatrix4fv(glGetUniformLocation(shader, "uP"), 1, GL_FALSE, glm::value_ptr(P));

    glUniform3fv(glGetUniformLocation(shader, "uViewPos"), 1, glm::value_ptr(cam.pos));

    glUniform1i(glGetUniformLocation(shader, "uLightEnabled"), gLightEnabled ? 1 : 0);
    glUniform3f(glGetUniformLocation(shader, "uLightPos"), 0.0f, 1.1f, -0.4f);
    glUniform3f(glGetUniformLocation(shader, "uLightColor"), 1.0f, 1.0f, 1.0f);

    glUniform3f(glGetUniformLocation(shader, "uKa"), 0.25f, 0.25f, 0.25f);
    glUniform3f(glGetUniformLocation(shader, "uKd"), 0.9f, 0.9f, 0.9f);
    glUniform3f(glGetUniformLocation(shader, "uKs"), 0.5f, 0.5f, 0.5f);
    glUniform1f(glGetUniformLocation(shader, "uShine"), 32.0f);

    glUniform1i(glGetUniformLocation(shader, "uTex"), 0);
}

void setInt(GLuint prog, const char* name, int v) {
    glUniform1i(glGetUniformLocation(prog, name), v);
}

void Renderer3D::draw(const Mesh& mesh, unsigned int tex, const glm::mat4& M, const Camera& cam) {
    glUseProgram(shader);
    glUniformMatrix4fv(glGetUniformLocation(shader, "uM"), 1, GL_FALSE, glm::value_ptr(M));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);

    glBindVertexArray(mesh.vao);
    glDrawArrays(GL_TRIANGLES, 0, mesh.vertexCount);
    glBindVertexArray(0);
}


