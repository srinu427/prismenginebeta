#pragma once
#include "PrismRenderer.h"

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <vector>
#include <unordered_map>
#include <string>

struct LRS {
	glm::vec3 location = glm::vec3(0.0f);
	glm::vec3 scale = glm::vec3{ 1.0f };
	glm::mat4 rotate = glm::mat4{ 1.0f };

	glm::mat4 getTMatrix();
};

struct BoneAnimStep {
	int stepduration_ms;
	int curr_time = 0;
	glm::vec3 initPos = glm::vec3(0.0f);
	glm::vec3 finalPos = glm::vec3(0.0f);
	glm::vec3 rotAxis = { 0.0f, 1.0f, 0.0f };
	float initAngle = 0;
	float finalAngle = 0;
	glm::vec3 initScale = glm::vec3(1.0f);
	glm::vec3 finalScale = glm::vec3(1.0f);
};

class BoneAnimData {
public:
	std::string name;
	std::vector<BoneAnimStep> steps;
	int curr_time = 0;
	int total_time;
	int curr_step = 0;

	LRS transformAfterGap(int gap_ms);
};

class ObjectLogicData
{
public:
	std::string id;
	std::string modelFilePath;
	std::string texFilePath;
	LRS objLRS;
	std::unordered_map<std::string, BoneAnimData> anims;

	void pushToRenderer(PrismRenderer* renderer);
	void updateAnim(std::string animName, int gap_ms);
};
