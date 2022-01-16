#include "LogicManager.h"

#include <math.h>
#include <chrono>
#include <iostream>
#include <thread>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace collutils;

LogicManager::LogicManager(PrismInputs* ipmgr, PrismAudioManager* audman, int logicpolltime_ms)
{
	inputmgr = ipmgr;
    audiomgr = audman;
    if (logicpolltime_ms != 0) logicPollTime = logicpolltime_ms;
    init();
}

void LogicManager::init()
{
    sunlightDir = currentCamEye;

    ObjectLogicData obama;
    obama.id = "obama";
    obama.modelFilePath = "models/obamaprisme.obj";
    obama.texFilePath = "textures/obama_prime.jpg";
    obama.objLRS.location = glm::vec3(0.0f, 0.5f, 0.1f);
    obama.objLRS.scale = glm::vec3{ 0.1f };
    BoneAnimStep rotateprism;
    rotateprism.stepduration_ms = 2000;
    rotateprism.rotAxis = { 0,1,0 };
    rotateprism.initAngle = 0;
    rotateprism.finalAngle = 2 * glm::pi<float>();
    BoneAnimData rotateanim;
    rotateanim.name = "rotateprism";
    rotateanim.total_time = 1000;
    rotateanim.steps.push_back(rotateprism);
    obama.anims["rotateprism"] = rotateanim;

    ObjectLogicData room;
    room.id = "room";
    room.modelFilePath = "models/untitled.obj";
    room.texFilePath = "textures/viking_room.png";
    room.objLRS.location = glm::vec3(0.0f, 0.0f, 0.0f);
    room.objLRS.scale = glm::vec3{ 1.0f };

    //ObjectLogicData floor;
    //floor.id = "floor";
    //floor.modelFilePath = "models/floor.obj";
    //floor.texFilePath = "textures/floor_tile_2.png";

    ObjectLogicData light;
    light.id = "light";
    light.modelFilePath = "models/obamaprisme_rn.obj";
    light.texFilePath = "textures/obama_prime.jpg";
    light.objLRS.location = currentCamEye;
    light.objLRS.scale = glm::vec3{ 0.04f };

    lObjects["obama"] = obama;
    newObjQueue.push_back("obama");

    lObjects["room"] = room;
    newObjQueue.push_back("room");

    /*
    lObjects["floor"] = floor;
    newObjQueue.push_back("floor");

    ObjectLogicData wall1;
    wall1.id = "wall1";
    wall1.modelFilePath = "models/wall.obj";
    wall1.texFilePath = "textures/basic_tile.jpg";
    wall1.objLRS.location = glm::vec3(5, 5, 0);
    wall1.objLRS.scale = glm::vec3(0.5f, 0.5f, 0.5f);

    lObjects["wall1"] = wall1;
    newObjQueue.push_back("wall1");

    ObjectLogicData wall2;
    wall2.id = "wall2";
    wall2.modelFilePath = "models/wall.obj";
    wall2.texFilePath = "textures/basic_tile.jpg";
    wall2.objLRS.location = glm::vec3(0, 5, -5);
    wall2.objLRS.scale = glm::vec3(0.5f, 0.5f, 0.5f);
    wall2.objLRS.rotate = glm::rotate(glm::mat4(1), glm::radians(90.0f), glm::vec3(0, 1, 0));

    lObjects["wall2"] = wall2;
    newObjQueue.push_back("wall2");

    ObjectLogicData wall3;
    wall3.id = "wall3";
    wall3.modelFilePath = "models/wall.obj";
    wall3.texFilePath = "textures/basic_tile.jpg";
    wall3.objLRS.location = glm::vec3(-5, 5, 0);
    wall3.objLRS.scale = glm::vec3(0.5f, 0.5f, 0.5f);
    wall3.objLRS.rotate = glm::rotate(glm::mat4(1), glm::radians(180.0f), glm::vec3(0, 1, 0));

    lObjects["wall3"] = wall3;
    newObjQueue.push_back("wall3");

    ObjectLogicData wall4;
    wall4.id = "wall4";
    wall4.modelFilePath = "models/wall.obj";
    wall4.texFilePath = "textures/basic_tile.jpg";
    wall4.objLRS.location = glm::vec3(0, 5, 5);
    wall4.objLRS.scale = glm::vec3(0.5f, 0.5f, 0.5f);
    wall4.objLRS.rotate = glm::rotate(glm::mat4(1), glm::radians(-90.0f), glm::vec3(0, 1, 0));

    lObjects["wall4"] = wall4;
    newObjQueue.push_back("wall4");

    ObjectLogicData wall5;
    wall5.id = "wall5";
    wall5.modelFilePath = "models/wall.obj";
    wall5.texFilePath = "textures/basic_tile.jpg";
    wall5.objLRS.location = glm::vec3(0, 2.5, 5);
    wall5.objLRS.scale = glm::vec3(0.1f, 0.5f, 0.5f);
    wall5.objLRS.rotate = glm::rotate(glm::mat4(1), glm::radians(-90.0f), glm::vec3(0, 1, 0)) * glm::rotate(glm::mat4(1), glm::radians(-45.0f), glm::vec3(0, 0, 1));

    lObjects["wall5"] = wall5;
    newObjQueue.push_back("wall5");

    */

    lObjects["light"] = light;
    //newObjQueue.push_back("light");
    plights.resize(2);
    plights[0] = glm::vec4(1.5, 1.5, 1.5, 1.0);
    plights[1] = glm::vec4(1.5, 1.5, 1.5, 1.0);

    //add ground
    ConvexPolyPlane ground = ConvexPolyPlane(glm::vec3(0, 0, 0), glm::vec3(0, 0, 1), glm::vec3(1, 0, 0), 30, 30, 0.1, 10);
    static_bounds.push_back(ground);
    ConvexPolyPlane wall_col = ConvexPolyPlane(glm::vec3(0, 0, 5), glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0), 30, 30, 0.1);
    static_bounds.push_back(wall_col);
    wall_col = ConvexPolyPlane(glm::vec3(0, 0, -5), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), 30, 30, 0.1);
    static_bounds.push_back(wall_col);
    wall_col = ConvexPolyPlane(glm::vec3(5, 0, 0), glm::vec3(0, 0, 1), glm::vec3(0, 1, 0), 30, 30, 0.1);
    static_bounds.push_back(wall_col);
    wall_col = ConvexPolyPlane(glm::vec3(-5, 0, 0), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0), 30, 30, 0.1);
    static_bounds.push_back(wall_col);
    wall_col = ConvexPolyPlane(glm::vec3(0, 2.5, 5), glm::vec3(-1, 0, 0), glm::vec3(0, 1, 1), 30, 30, 0.1, 10);
    static_bounds.push_back(wall_col);
    std::vector<ConvexPolyPlane> cube1 = gen_cube_bplanes(glm::vec3(-2, 0.25, -2), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), 1, 0.5, 1, 0.1, 10);
    static_bounds.insert(static_bounds.begin(), cube1.begin(), cube1.end());
    cube1 = gen_cube_bplanes(glm::vec3(-2, 0.75, -1), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), 1, 0.5, 1, 0.1, 10);
    static_bounds.insert(static_bounds.begin(), cube1.begin(), cube1.end());
    cube1 = gen_cube_bplanes(glm::vec3(-2, 1.25, 0), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), 1, 0.5, 1, 0.1, 10);
    static_bounds.insert(static_bounds.begin(), cube1.begin(), cube1.end());
    cube1 = gen_cube_bplanes(glm::vec3(-3, 1.25, 0), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), 1, 0.5, 1, 0.1, 10);
    static_bounds.insert(static_bounds.begin(), cube1.begin(), cube1.end());
    cube1 = gen_cube_bplanes(glm::vec3(-4, 1.75, 0), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), 1, 0.5, 1, 0.1, 10);
    static_bounds.insert(static_bounds.begin(), cube1.begin(), cube1.end());

    for (int sbi = 0; sbi < static_bounds.size(); sbi++) {
        new_bp_meshes.push_back(static_bounds[sbi].gen_mesh());
    }
    
    //add player
    player.pos = glm::vec3(1, 1, 1);
    player.vel = glm::vec3(0, 0, 0);
    player.acc = glm::vec3(0, -10, 0);
    in_air = true;

    audiomgr->add_aud_buffer("jump", "sounds/jump1.wav");
    audiomgr->add_aud_buffer("fall", "sounds/fall1.wav");
    audiomgr->add_aud_source("player");
    audiomgr->add_aud_source("1");
    audiomgr->update_listener(player.pos, player.vel, currentCamDir, currentCamUp);
}

void LogicManager::run()
{
	std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
	std::chrono::milliseconds interval = std::chrono::milliseconds(logicPollTime);
	std::chrono::system_clock::time_point wait_until = start + interval;

	while (!shouldStop) {
		std::chrono::milliseconds gap = interval;
		std::chrono::system_clock::time_point curr_time = std::chrono::system_clock::now();
		if (wait_until < curr_time) {
			std::chrono::milliseconds offset = ((std::chrono::ceil<std::chrono::milliseconds>(curr_time - wait_until).count() / std::chrono::ceil<std::chrono::milliseconds>(interval).count()) + 1) * interval;
			wait_until += offset;
			gap += offset;
		}
		std::this_thread::sleep_until(wait_until);
		computeLogic(wait_until, gap);
		wait_until += interval;
	}
}

void LogicManager::pushToRenderer(PrismRenderer* renderer)
{
    rpush_mut.lock();
    for (std::string it : newObjQueue) {
        lObjects[it].pushToRenderer(renderer);
    }
    for (int sbi = 0; sbi < new_bp_meshes.size(); sbi++) {
        renderer->addRenderObj(
            "sbound-" + std::to_string(sbi),
            new_bp_meshes[sbi],
            "textures/basic_tile.jpg",
            "linear",
            glm::mat4(1)
            );
    }
    newObjQueue.clear();
    new_bp_meshes.clear();

    //currentCamDir = glm::vec3(0.0, 0.0, -1.0);

    renderer->currentScene.sunlightPosition = glm::vec4(sunlightDir, 1.0f);
    renderer->pointLights[0].pos = plights[0];
    renderer->pointLights[0].color = glm::vec4(1, 1, 1, 1);
    renderer->pointLights[1].pos = plights[1];
    renderer->pointLights[1].color = glm::vec4(1, 1, 1, 1);

    glm::mat4 view = glm::lookAt(
        currentCamEye,
        currentCamEye + currentCamDir,
        currentCamUp
    );
    glm::mat4 proj = glm::perspective(
        glm::radians(90.0f),
        renderer->swapChainExtent.width / (float)renderer->swapChainExtent.height,
        0.01f,
        20.0f);
    renderer->currentCamera.viewproj = proj * view;
    renderer->currentCamera.camPos = {currentCamEye , 0.0};
    renderer->currentCamera.camDir = {currentCamDir, 0.0};

    size_t robjCount = renderer->renderObjects.size();
    for (size_t it = 0; it < robjCount; it++) {
        std::string objid = renderer->renderObjects[it].id;
        renderer->renderObjects[it].uboData.model = lObjects[objid].objLRS.getTMatrix();
    }
    rpush_mut.unlock();
}

void LogicManager::computeLogic(std::chrono::system_clock::time_point curr_time, std::chrono::milliseconds gap)
{
    rpush_mut.lock();
    float logicDeltaT = std::chrono::duration<float, std::chrono::seconds::period>(gap).count();
    
    langle = (langle + (logicDeltaT * 0.5));
    glm::vec3 crelx = glm::normalize(glm::cross(currentCamDir, currentCamUp));
    glm::vec3 crely = currentCamUp;
    glm::vec3 crelz = glm::cross(crelx, crely);

    if (inputmgr->wasKeyPressed(GLFW_KEY_F)) {
        plights[0] = glm::vec4(currentCamEye, 1.0);
    }
    if (inputmgr->wasKeyPressed(GLFW_KEY_G)) {
        plights[1] = glm::vec4(currentCamEye, 1.0);
    }

    bool ground_touch = false;
    int ground_plane = -1;
    for (int pli = 0; pli < static_bounds.size(); pli++) if (static_bounds[pli].check_point_future(player.pos, glm::vec3(0), glm::vec3(0), 0).will_collide && glm::dot(static_bounds[pli].n, glm::vec3(0, 1, 0)) > 0.1) {
        ground_touch = true;
        ground_plane = pli;
        break;
    };
    glm::vec3 inp_vel = glm::vec3(0);

    if (inputmgr->wasKeyPressed(GLFW_KEY_W)) inp_vel -= crelz;
    if (inputmgr->wasKeyPressed(GLFW_KEY_S)) inp_vel += crelz;
    if (inputmgr->wasKeyPressed(GLFW_KEY_A)) inp_vel -= crelx;
    if (inputmgr->wasKeyPressed(GLFW_KEY_D)) inp_vel += crelx;
    if (glm::length(inp_vel) > 0) {
        inp_vel = glm::normalize(inp_vel) * 2.0f;
        if (ground_touch) {
            //if (glm::dot(player.vel, glm::normalize(inp_vel)) < glm::length(inp_vel)) {
            //    player.vel += glm::vec3(0, player.vel.y, 0) + glm::vec3(inp_vel.x, 0, inp_vel.z);
            //}
            player.vel = glm::vec3(0, player.vel.y, 0) + glm::vec3(inp_vel.x, 0, inp_vel.z);
            player.acc.x = 0;
            player.acc.z = 0;
        }
        else {
            if (glm::dot(player.vel, glm::normalize(inp_vel)) < glm::length(inp_vel) * 1.0f) {
                player.acc = glm::vec3(0, player.acc.y, 0) + glm::vec3(inp_vel.x, 0, inp_vel.z);
            }
        }
    }

    else {
        player.acc.x = 0;
        player.acc.z = 0;
    }

    if (ground_touch) {
        if (in_air) {
            audiomgr->play_aud_buffer_from_source("player", "fall");
        }
        in_air = false;
    }
    else {
        in_air = true;
    }

    if (inputmgr->wasKeyPressed(GLFW_KEY_SPACE) and ground_touch) {
        player.vel.y = 5;
        //audiomgr->update_aud_source("1", player.pos, glm::vec3(0));
        audiomgr->play_aud_buffer_from_source("player", "jump");
    }
    //if (inputmgr->wasKeyPressed(GLFW_KEY_LEFT_CONTROL)) playVel = playVel - crely * (CAM_SPEED * logicDeltaT);

    //player = progress_kinematics(player, &static_bounds, (glm::length(inp_vel) > 0) ? ground_plane : -1, 0.05);
    player = progress_kinematics(player, &static_bounds, (glm::length(inp_vel) > 0) ? ground_plane: -1, logicDeltaT);
    //std::cout << player.vel.x << ',' << player.vel.y << ',' << player.vel.z << '\n';
    currentCamEye = player.pos + glm::vec3(0, 0.5, 0);
    currentCamDir = glm::vec3(glm::rotate(glm::mat4(1.0f),
        glm::radians(MOUSE_SENSITIVITY_X * float(inputmgr->dmx)),
        crely) * glm::vec4(currentCamDir, 0.0f));

    float nrb = glm::dot(glm::normalize(currentCamUp), glm::normalize(currentCamDir));
    if (!(abs(nrb) > 0.9 && nrb*inputmgr->dmy < 0)) {
        currentCamDir = glm::vec3(glm::rotate(glm::mat4(1.0f),
            glm::radians(MOUSE_SENSITIVITY_Y * float(inputmgr->dmy)),
            crelx) * glm::vec4(currentCamDir, 0.0f));
    }
    audiomgr->update_aud_source("player", player.pos, player.vel);
    audiomgr->update_listener(player.pos, player.vel, currentCamDir, currentCamUp);
    
    lObjects["obama"].updateAnim("rotateprism", int(logicDeltaT * 1000));

    inputmgr->clearMOffset();
    rpush_mut.unlock();
}

void LogicManager::stop()
{
    rpush_mut.lock();
    shouldStop = true;
    rpush_mut.unlock();
}
