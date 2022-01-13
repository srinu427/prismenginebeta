#version 460

layout(location = 0) in vec4 fragPos;
layout(location = 1) in vec4 lightPos;

layout(location = 0) out float outColor;

void main() {
    outColor = length(fragPos.xyz - lightPos.xyz);
}