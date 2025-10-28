//#shader vertex
#version 330 core
layout(location=0) in vec2 aPos; layout(location=1) in vec2 aUV;
out vec2 vUV; void main(){ vUV=aUV; gl_Position=vec4(aPos,0,1); }

//#shader fragment
#version 330 core
out vec4 FragColor; in vec2 vUV;
uniform sampler2D image;
uniform int horizontal;
const float w[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
void main(){
    vec2 texel = 1.0 / vec2(textureSize(image, 0));
    vec3 sum = texture(image, vUV).rgb * w[0];
    for (int i=1; i<5; i++) {
        vec2 off = (horizontal==1 ? vec2(texel.x*i,0) : vec2(0,texel.y*i));
        sum += texture(image, vUV + off).rgb * w[i];
        sum += texture(image, vUV - off).rgb * w[i];
    }
    FragColor = vec4(sum, 1.0);
}
