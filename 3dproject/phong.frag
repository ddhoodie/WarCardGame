#version 330 core
in vec3 vPos;
in vec3 vNor;
in vec2 vUV;
in vec3 vLocal;

out vec4 outCol;

uniform int uIsCard;      // 1 = karta (radi rounded), 0 = ostalo (sto)
uniform int uUseAlpha;    // 1 = koristi alpha discard, 0 = ne

uniform int uIsChip;        // 1=chip, 0=ostalo
uniform float uChipRadius;  // isti radius kao u meshu (npr 0.18)

uniform sampler2D uTex;

uniform vec3 uViewPos;

uniform bool uLightEnabled;
uniform vec3 uLightPos;
uniform vec3 uLightColor;
uniform float uLightIntensity;



uniform vec3 uKa;
uniform vec3 uKd;
uniform vec3 uKs;
uniform float uShine;

uniform float uHalfW;
uniform float uHalfH;
uniform float uRadius;

void main() {
    if (uIsChip == 1) {
        float rr = length(vec2(vLocal.x, vLocal.z));
        if (rr > uChipRadius + 0.0005) discard; // mali eps
    }

    if (uIsCard == 1) {
        vec2 p = vec2(vLocal.x, vLocal.z);
        vec2 b = vec2(uHalfW - uRadius, uHalfH - uRadius);
        vec2 q = abs(p) - b;
        float d = length(max(q, vec2(0.0))) + min(max(q.x, q.y), 0.0) - uRadius;
        float eps = 0.002;          
        if (d > eps) discard;
    }

    vec3 N = normalize(vNor);
    vec3 L = normalize(uLightPos - vPos);
    vec3 V = normalize(uViewPos - vPos);
    vec3 R = reflect(-L, N);

    vec4 t = texture(uTex, vUV);

    if (uUseAlpha == 1) {
        if (t.a < 0.5) discard;
    }

    vec3 tex = t.rgb;

       vec3 ambient = uKa * tex;
    if (!uLightEnabled) {
        outCol = vec4(ambient, 1.0);
        return;
    }

    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = uKd * diff * tex;

    float spec = pow(max(dot(R, V), 0.0), uShine);
    vec3 specular = uKs * spec;

    vec3 color = ambient + (diffuse + specular) * uLightColor * uLightIntensity;
    outCol = vec4(color, 1.0);
}
