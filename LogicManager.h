#pragma once
#include "PrismInputs.h"
#include "PrismRenderer.h"
#include "PrismAudioManager.h"
#include "ObjectLogicData.h"
#include "CollisionStructs.h"

#include <regex>


class LogicManager
{
public:
	LogicManager(PrismInputs* ipmgr, PrismAudioManager* audman, int logicpolltime_ms=1);
	void run();
	void stop();
	void parseCollDataFile(std::string cfname);
	void pushToRenderer(PrismRenderer* renderer);
private:
	PrismInputs* inputmgr;
	PrismAudioManager* audiomgr;
	int logicPollTime = 1;
	bool shouldStop = false;
	float langle = 0;

	glm::vec3 currentCamEye = glm::vec3(1.5f, 1.5f, 1.5f);
	glm::vec3 currentCamDir = glm::normalize(glm::vec3(0.0f, 0.0f, 0.0f) - currentCamEye);
	glm::vec3 currentCamUp = glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f));

	float MOUSE_SENSITIVITY_X = -0.1f;
	float MOUSE_SENSITIVITY_Y = -0.1f;
	float CAM_SPEED = 1.0f;

	glm::vec3 sunlightDir = (glm::vec3(0.0f, 0.2f, 0.0f));

	std::vector<collutils::CollMesh> static_bounds;

	collutils::KinePointObj player_point;
	collutils::KineSolidObj player;
	bool in_air = false;

	std::unordered_map<std::string, ObjectLogicData> lObjects;
	std::vector<glm::vec4> plights;
	std::vector<std::string> newObjQueue;
	std::vector<Mesh> new_bp_meshes;
	
	void init();
	void computeLogic(std::chrono::system_clock::time_point curr_time, std::chrono::milliseconds gap);
	std::chrono::system_clock::time_point lastLogicComputeTime;
	bool started = false;
	std::mutex rpush_mut;
};
