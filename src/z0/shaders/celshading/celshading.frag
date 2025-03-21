/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
#version 450

#include "utils.glsl"

layout (location = 0) in VertexOut fs_in;
layout (location = 0) out vec4 COLOR;

float stepmix(float edge0, float edge1, float E, float x)  {
    float T = clamp(0.5 * (x - edge0 + E) / E, 0.0, 1.0);
    return mix(edge0, edge1, T);
}

float detectEdges(vec3 normal, vec3 viewDir) {
    float edgeFactor = dot(normal, viewDir);
    return smoothstep(0.2, 0.5, edgeFactor);
}


void main() {
    Material material = materials.material[pushConstants.materialIndex];
    Texture textures = textures.texture[pushConstants.materialIndex];

    vec4 color = material.albedoColor;
    if (textures.diffuseTexture.index != -1) {
        color *= texture(texSampler[textures.diffuseTexture.index], uvTransform(textures.diffuseTexture, fs_in.UV));
    }
    if (((material.transparency == TRANSPARENCY_SCISSOR) || (material.transparency == TRANSPARENCY_SCISSOR_ALPHA)) && (color.a < material.alphaScissor)) {
        discard;
    }
    const float transparency = (material.transparency == TRANSPARENCY_ALPHA || material.transparency == TRANSPARENCY_SCISSOR_ALPHA) ? color.a : 1.0f;

    vec3 N = fs_in.NORMAL;
    vec3 V =  fs_in.VIEW_DIRECTION; //normalize(-global.cameraPosition);

    vec3 diffuse = vec3(0.0f);
    vec3 specular =  vec3(0.0f);
    for (uint i = 0; i < global.lightsCount; i++) {
        Light light = lights.light[i];

        // https://prideout.net/blog/old/blog/index.html@p=22.html
        vec3 L = normalize(-light.direction);
        vec3 H = normalize(L + V);

        float df = max(0.0, dot(N, L));
        float sf = max(0.0, dot(N, H));
        sf = pow(sf, 32.0); // TODO add specular parameter to material

        const float A = 0.1;
        const float B = 0.3;
        const float C = 0.6;
        const float D = 1.0;
        float E = fwidth(df);

        if      (df > A - E && df < A + E) df = stepmix(A, B, E, df);
        else if (df > B - E && df < B + E) df = stepmix(B, C, E, df);
        else if (df > C - E && df < C + E) df = stepmix(C, D, E, df);
        else if (df < A) df = 0.0;
        else if (df < B) df = B;
        else if (df < C) df = C;
        else df = D;

        E = fwidth(sf);
        if (sf > 0.5 - E && sf < 0.5 + E)  {
            sf = smoothstep(0.5 - E, 0.5 + E, sf);
        }
        else {
            sf = step(0.5, sf);
        }

        diffuse += df * light.color.rgb * light.color.w;
        specular += sf * light.color.rgb * light.color.w;
    }

    vec3 ambient = color.rgb * global.ambient.w * global.ambient.rgb ;
//    vec3 finalColor = ambient + (diffuse + specular) * color.rgb;
    vec3 finalColor = vec3(1.0);


    vec3 viewDir = normalize(global.cameraPosition - fs_in.POSITION);
    float edge = 1.0 - smoothstep(0.0, 0.01, abs(dot(N, fs_in.VIEW_DIRECTION)));
    vec3 outlineColor = vec3(0.0);
    COLOR = vec4(mix(finalColor, outlineColor, edge), transparency);

//    COLOR = vec4(fs_in.UV.x, fs_in.UV.y, 1.0, 1.0);
}

/*
#version 330 core

in vec3 FragPos;       // Position du fragment en espace monde
in vec3 Normal;        // Normale du fragment en espace monde
in vec2 TexCoords;     // Coordonnées de texture
in vec4 FragPosLightSpace; // Optionnel, si ombres

out vec4 FragColor;

// Uniforms pour l'éclairage
uniform vec3 lightDir;     // Direction de la lumière directionnelle (normalisée)
uniform vec3 lightColor;   // Couleur de la lumière
uniform vec3 ambientColor; // Couleur de la lumière ambiante

// Texture
uniform sampler2D texture1;

// Buffers pour la détection de contours
uniform sampler2D normalBuffer;  // Contient les normales de la scène
uniform sampler2D depthBuffer;   // Contient les profondeurs
uniform vec2 screenSize;         // Taille de l'écran pour récupérer les pixels voisins

// Fonction de quantification pour l'effet toon sur la texture
vec3 quantizeColor(vec3 color, int levels) {
    return floor(color * levels) / levels;
}

// Détection des contours via Sobel filter sur les normales
float detectEdges(vec2 uv) {
    float offset = 1.0 / screenSize.x; // Adapté à la taille de l'écran

    vec3 n[9];
    n[0] = texture(normalBuffer, uv + vec2(-offset, -offset)).rgb;
    n[1] = texture(normalBuffer, uv + vec2(0.0, -offset)).rgb;
    n[2] = texture(normalBuffer, uv + vec2(offset, -offset)).rgb;
    n[3] = texture(normalBuffer, uv + vec2(-offset, 0.0)).rgb;
    n[4] = texture(normalBuffer, uv).rgb; // Centre
    n[5] = texture(normalBuffer, uv + vec2(offset, 0.0)).rgb;
    n[6] = texture(normalBuffer, uv + vec2(-offset, offset)).rgb;
    n[7] = texture(normalBuffer, uv + vec2(0.0, offset)).rgb;
    n[8] = texture(normalBuffer, uv + vec2(offset, offset)).rgb;

    // Gradient Sobel
    vec3 sobelX = n[2] + 2.0 * n[5] + n[8] - (n[0] + 2.0 * n[3] + n[6]);
    vec3 sobelY = n[0] + 2.0 * n[1] + n[2] - (n[6] + 2.0 * n[7] + n[8]);

    float edge = length(sobelX) + length(sobelY);
    return edge > 0.2 ? 1.0 : 0.0; // Seuil pour accentuer les bords
}

void main()
{
    vec3 norm = normalize(Normal);

    // Calcul de l'éclairage directionnel (Toon shading)
    float diff = max(dot(norm, -lightDir), 0.0);
    float levels[4] = float[4](0.1, 0.4, 0.7, 1.0);
    float toonFactor = 0.0;
    for (int i = 0; i < 4; i++) {
        if (diff >= levels[i]) {
            toonFactor = levels[i];
        }
    }

    // Récupération et quantification de la couleur de la texture
    vec3 texColor = texture(texture1, TexCoords).rgb;
    texColor = quantizeColor(texColor, 4); // Réduit les couleurs

    // Application de l'éclairage toon
    vec3 finalColor = ambientColor + toonFactor * lightColor * texColor;

    // Détection des contours
    float edge = detectEdges(TexCoords);

    // Mélange : noir si contour, sinon couleur toon
    vec3 outlineColor = vec3(0.0);
    vec3 outputColor = mix(finalColor, outlineColor, edge);

    FragColor = vec4(outputColor, 1.0);
}
*/