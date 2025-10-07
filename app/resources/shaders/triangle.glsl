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

uniform vec3 objectColor;
uniform vec3 ambientColor;

uniform vec3 dirLightDir;
uniform vec3 dirLightColor;

uniform vec3 pointLightPos;
uniform vec3 pointLightColor;
uniform float pointKc;
uniform float pointKl;
uniform float pointKq;

void main()
{
    vec3 N = normalize(Normal);

    vec3 Ld = normalize(dirLightDir);
    float diffDir = max(dot(N, Ld), 0.0);
    vec3 dirTerm = diffDir * dirLightColor;

    vec3 toPoint = pointLightPos - FragPos;
    float dist   = length(toPoint);
    vec3 Lp      = toPoint / max(dist, 1e-6);
    float diffPt = max(dot(N, Lp), 0.0);
    float atten  = 1.0 / (pointKc + pointKl * dist + pointKq * dist * dist);
    vec3 pointTerm = diffPt * pointLightColor * atten;

    vec3 lighting = ambientColor + dirTerm + pointTerm;
    vec3 color = lighting * objectColor;

    FragColor = vec4(color, 1.0);
}