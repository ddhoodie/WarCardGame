#version 330 core

in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uTexture;
uniform bool uShadow;
uniform bool uUseColor = false;
uniform vec4 uColorFill = vec4(0.0, 0.0, 0.0, 1.0);
uniform float uAlphaMul = 1.0;


void main()
{
    if (uUseColor) {
        FragColor = uColorFill;
        return;
    }

    vec4 texColor = texture(uTexture, vUV);

    if (uShadow)
    {
        float shadowAlpha = texColor.a * 0.45;
        FragColor = vec4(0.0, 0.0, 0.0, shadowAlpha);
        return;
    }

    FragColor = vec4(texColor.rgb, texColor.a * uAlphaMul);
}