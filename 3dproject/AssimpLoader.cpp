#include "AssimpLoader.h"
#include "Util.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <vector>
#include <stdexcept>
#include "Helper2D.h"

static Mesh makeMeshFromInterleaved(const std::vector<float>& data) {
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

LoadedModel loadModelAssimp(const std::string& path,
    const std::string& diffuseTexturePath,
    const glm::u8vec4& fallbackColor)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(
        path,
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_JoinIdenticalVertices |
        aiProcess_ImproveCacheLocality |
        aiProcess_OptimizeMeshes
        // (po potrebi) | aiProcess_FlipUVs
    );

    if (!scene || !scene->HasMeshes()) {
        throw std::runtime_error(std::string("Assimp failed: ") + importer.GetErrorString());
    }

    const aiMesh* mesh = scene->mMeshes[0];

    std::vector<float> data;
    data.reserve(mesh->mNumFaces * 3 * 8);

    auto pushV = [&](unsigned idx) {
        const aiVector3D& p = mesh->mVertices[idx];

        aiVector3D n(0, 1, 0);
        if (mesh->HasNormals()) n = mesh->mNormals[idx];

        aiVector3D uv(0, 0, 0);
        if (mesh->HasTextureCoords(0)) uv = mesh->mTextureCoords[0][idx];

        data.push_back(p.x);  data.push_back(p.y);  data.push_back(p.z);
        data.push_back(n.x);  data.push_back(n.y);  data.push_back(n.z);
        data.push_back(uv.x); data.push_back(uv.y);
        };

    for (unsigned f = 0; f < mesh->mNumFaces; f++) {
        const aiFace& face = mesh->mFaces[f];
        if (face.mNumIndices != 3) continue;
        pushV(face.mIndices[0]);
        pushV(face.mIndices[1]);
        pushV(face.mIndices[2]);
    }

    LoadedModel out;
    out.mesh = makeMeshFromInterleaved(data);

    if (!diffuseTexturePath.empty()) {
        out.tex = loadImageToTexture(diffuseTexturePath.c_str());
        glBindTexture(GL_TEXTURE_2D, out.tex);
        setTextureParamsClamp();
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    else {
        // fallback - obojeni "materijal" preko solid teksture
        out.tex = makeSolidTexture(fallbackColor.r, fallbackColor.g, fallbackColor.b, fallbackColor.a);
    }

    return out;
}