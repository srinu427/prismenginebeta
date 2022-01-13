#version 460
#define MAX_LIGHTS 4

struct GPULight {
	vec4 pos;
    vec4 color;
    vec4 dir;
    mat4 viewproj;
};

layout(std140, set = 0, binding = 0) uniform LightBuffer { GPULight lights[MAX_LIGHTS]; } lightBuffer;

layout(push_constant) uniform lpconst {
    ivec4 idx;
    mat4 viewproj;
} lightPC;

struct ObjectData{
	mat4 model;
};

layout(std140,set = 1, binding = 0) readonly buffer ObjectBuffer{
	ObjectData objects[];
} objectBuffer;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec4 fragPos;
layout(location = 1) out vec4 lightPos;

void main() {
    lightPos = lightBuffer.lights[lightPC.idx.x].pos;
    fragPos = objectBuffer.objects[gl_BaseInstance].model * vec4(inPosition, 1.0f);
    vec4 cpos = lightPC.viewproj * lightBuffer.lights[lightPC.idx.x].viewproj * fragPos;
    gl_Position = cpos;
}