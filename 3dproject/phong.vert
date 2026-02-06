#version 330 core
layout(location=0) in vec3 inPos;
layout(location=1) in vec3 inNor;
layout(location=2) in vec2 inUV;

uniform mat4 uM;
uniform mat4 uV;
uniform mat4 uP;

out vec3 vPos;
out vec3 vNor;
out vec3 vLocal;
out vec2 vUV;

void main() {
    vec4 world = uM * vec4(inPos, 1.0);
    vLocal = inPos;
    vPos = world.xyz;
    vNor = mat3(transpose(inverse(uM))) * inNor;
    vUV  = inUV;
    gl_Position = uP * uV * world;
}
