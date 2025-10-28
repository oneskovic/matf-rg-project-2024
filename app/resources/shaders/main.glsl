//#shader vertex
#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoords = aTexCoords;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}

//#shader fragment
#version 330 core

out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_emissive1;
uniform bool useEmissive;

uniform float uTile; // tiling factor

// ambient light
uniform vec3 ambientLight;

// directional light
uniform vec3 dirLightDir;
uniform vec3 dirLightColor;

// point lights
struct PointLight {
    vec3 position;
    vec3 color;
};

#define MAX_POINT_LIGHTS 16
uniform int uNumPointLights;
uniform PointLight uPointLights[MAX_POINT_LIGHTS];

// shared attenuation for all point lights
uniform float pointKc;
uniform float pointKl;
uniform float pointKq;

void main()
{
    vec3 N = normalize(Normal);
    vec3 Ld = normalize(dirLightDir);
    float diffDir = max(dot(N, Ld), 0.0);
    vec3 dirTerm = diffDir * dirLightColor;

    vec3 pointAccum = vec3(0.0); // Total accumulated light

    for (int i = 0; i < uNumPointLights; ++i) {
        vec3 toPoint = uPointLights[i].position - FragPos;
        float dist   = length(toPoint);
        vec3 Lp      = toPoint / max(dist, 1e-6);

        float diffPt = max(dot(N, Lp), 0.0);

        float atten  = 1.0 / (pointKc + pointKl * dist + pointKq * dist * dist);

        pointAccum += diffPt * uPointLights[i].color * atten;
    }

    vec3 lighting = ambientLight + dirTerm + pointAccum;

    // Do tiling
    vec2 tiledUV = TexCoords * uTile;
    vec3 albedo = texture(texture_diffuse1, tiledUV).rgb;
    vec3 emissive = useEmissive ? texture(texture_emissive1, tiledUV).rgb : vec3(0.0);

    vec3 color = lighting * albedo + 2*emissive;
    FragColor = vec4(color, 1.0);
}