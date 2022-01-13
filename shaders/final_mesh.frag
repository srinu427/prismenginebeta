#version 460
//#extension GL_KHR_vulkan_glsl: enable
#define MAX_LIGHTS 4
#define MAX_PLIGHT_SHADOW_DISTANCE 20.0
#define SHADOW_MUL_EPSILON 1.00
#define SHADOW_ADD_EPSILON 0.01
#define SHADOW_BLUR_EPSILON 0.008
#define SHADOW_BLUR_SCALING 20
#define SHADOW_BLUR_FALLOFF 0.6
#define SHADOW_BLUR_SAMPLES 45

#define SHADOW_SOFT_MUL_EPSILON 1.00
#define SHADOW_SOFT_ADD_EPSILON 0.01
#define SHADOW_SOFT_BLUR_EPSILON 0.5
#define SHADOW_SOFT_BLUR_SCALING 10
#define SHADOW_SOFT_BLUR_FALLOFF 0.8
#define SHADOW_SOFT_BLUR_SAMPLES 45

#define SHADOW_ANTIBLUR_EPSILON 0.001
#define PI 3.1415926538

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragColor;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 fragNormal;

layout(set = 2, binding = 0) uniform GPUSceneData {
	vec4 fogColor; // w is for exponent
	vec4 fogDistances; //x for min, y for max, zw unused.
	vec4 ambientColor;
	vec4 sunlightPosition;
	vec4 sunlightDirection; //w for sun power
	vec4 sunlightColor;
} sceneData;

struct GPULight {
	vec4 pos;
	vec4 color;
	vec4 dir;
	mat4 viewproj;
};

layout(set = 3, binding = 0) uniform sampler2D texSampler;
layout(std140, set = 4, binding = 0) uniform LightBuffer {GPULight lights[MAX_LIGHTS];} lightBuffer;
layout(set = 5, binding = 0) uniform samplerCube shadowCubes[MAX_LIGHTS];

layout(location = 0) out vec4 outColor;

float compute_soft_shadow(int lidx, vec3 light_vec){
	vec3 v1, v2;

	float lvl = length(light_vec);
	if (lvl > MAX_PLIGHT_SHADOW_DISTANCE) return 1;
	vec3 lvn = normalize(light_vec);

	if (lvn.y == 0 && lvn.z == 0) {v1=vec3(0, 1, 0);}
	else {v1 = vec3(1,0,0);}

	v2 = cross(lvn, v1);
	v1 = cross(lvn, v2);
	float light_samples = 0;
	float sharp_depth = texture(shadowCubes[lidx], light_vec).r;
	if(sharp_depth*SHADOW_SOFT_MUL_EPSILON >= lvl){light_samples += 1;}
	vec3 sample_vec;
	float brad = max(SHADOW_SOFT_BLUR_EPSILON*(1-sharp_depth/lvl), SHADOW_BLUR_EPSILON*lvl);
	float sample_weight = SHADOW_SOFT_BLUR_FALLOFF/(SHADOW_SOFT_BLUR_SCALING*brad+1);
	float theta = PI/SHADOW_SOFT_BLUR_SAMPLES;
	for (int i = 0; i < SHADOW_SOFT_BLUR_SAMPLES; i++){
		sample_vec = v1*cos(i*theta) + v2*sin(i*theta);
		sample_vec *= brad;
		if(texture(shadowCubes[lidx], light_vec + sample_vec).r*SHADOW_SOFT_MUL_EPSILON + SHADOW_SOFT_ADD_EPSILON >= lvl){light_samples += sample_weight;}
		if(texture(shadowCubes[lidx], light_vec - sample_vec).r*SHADOW_SOFT_MUL_EPSILON + SHADOW_SOFT_ADD_EPSILON >= lvl){light_samples += sample_weight;}
	}
	return 0.3 + 0.7*(light_samples/(1 + 2*SHADOW_SOFT_BLUR_SAMPLES*sample_weight));
}

float compute_shadow(int lidx, vec3 light_vec){
	vec3 v1, v2;

	float lvl = length(light_vec);
	if (lvl > MAX_PLIGHT_SHADOW_DISTANCE) return 1;
	vec3 lvn = normalize(light_vec);

	if (lvn.y == 0 && lvn.z == 0) {v1=vec3(0, 1, 0);}
	else {v1 = vec3(1,0,0);}

	v2 = cross(lvn, v1);
	v1 = cross(lvn, v2);
	float light_samples = 0;
	if(texture(shadowCubes[lidx], light_vec).r*SHADOW_MUL_EPSILON >= lvl){light_samples += 1;}
	vec3 sample_vec;
	float brad = SHADOW_BLUR_EPSILON*lvl;
	float sample_weight = SHADOW_BLUR_FALLOFF/(SHADOW_BLUR_SCALING*brad+1);
	float theta = PI/SHADOW_BLUR_SAMPLES;
	for (int i = 0; i < SHADOW_BLUR_SAMPLES; i++){
		sample_vec = v1*cos(i*theta) + v2*sin(i*theta);
		sample_vec *= brad;
		if(texture(shadowCubes[lidx], light_vec + sample_vec).r*SHADOW_MUL_EPSILON + SHADOW_ADD_EPSILON >= lvl){light_samples += sample_weight;}
		if(texture(shadowCubes[lidx], light_vec - sample_vec).r*SHADOW_MUL_EPSILON + SHADOW_ADD_EPSILON >= lvl){light_samples += sample_weight;}
	}
	return 0.3 + 0.7*(light_samples/(1 + 2*SHADOW_BLUR_SAMPLES*sample_weight));
}

void main() {
    vec4 texColor = texture(texSampler, fragTexCoord);
	float shadow = 1;
	float diffuse = 0;
	for (int i = 0; i < MAX_LIGHTS; i++){
		if (lightBuffer.lights[i].color.z > 0.1){
			vec3 lightVec = lightBuffer.lights[i].pos.xyz - fragPosition;
			shadow *= compute_shadow(i, lightVec);
			diffuse = max(dot(fragNormal, normalize(lightVec)), diffuse);
		}
	}
    outColor = texColor * diffuse * shadow;
}