#version 330 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;

out vec2 vUV;

uniform vec2 uPos;
uniform vec2 uScale;
uniform vec4 uUV;   

void main()
{
    vec2 pos = aPos * uScale + uPos;
    gl_Position = vec4(pos, 0.0, 1.0);

    vUV = mix(uUV.xy, uUV.zw, aUV);
}