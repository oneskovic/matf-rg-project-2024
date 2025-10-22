//#shader vertex
#version 330 core
layout(location=0) in vec2 aPos; layout(location=1) in vec2 aUV;
out vec2 vUV; void main(){ vUV=aUV; gl_Position=vec4(aPos,0,1); }

//#shader fragment
#version 330 core
out vec4 FragColor; in vec2 vUV;
uniform sampler2D hdrTex;
uniform float threshold;
void main(){
    vec3 c = texture(hdrTex, vUV).rgb;
    float luma = max(max(c.r, c.g), c.b);
    vec3 bright = (luma > threshold) ? c : vec3(0.0);
    FragColor = vec4(bright, 1.0);
}