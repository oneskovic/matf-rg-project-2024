//#shader vertex
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;
out vec2 TexCoords;
void main() {
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos, 0.0, 1.0);
}

//#shader fragment
#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D bloomTex;
uniform float bloomIntensity;
uniform float exposure;
uniform float gamma;

// ACES tone map
vec3 aces(vec3 x) {
    return clamp((x*(2.51*x + 0.03)) / (x*(2.43*x + 0.59) + 0.14), 0.0, 1.0);
}

void main(){
    vec3 sceneHdr = texture(screenTexture, TexCoords).rgb;
    vec3 bloomHdr = texture(bloomTex, TexCoords).rgb;

    vec3 hdr = sceneHdr + bloomHdr * bloomIntensity;
    hdr *= exposure;
    vec3 mapped = aces(hdr);
    FragColor = vec4(mapped, 1.0);
}