#include "PrismInputs.h"

#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>

PrismInputs::PrismInputs()
{
	valid_keys = {
		GLFW_KEY_W,
		GLFW_KEY_S,
		GLFW_KEY_A,
		GLFW_KEY_D,
		GLFW_KEY_F,
		GLFW_KEY_G,
		GLFW_KEY_SPACE,
		GLFW_KEY_LEFT_CONTROL,
		GLFW_MOUSE_BUTTON_LEFT,
		GLFW_MOUSE_BUTTON_RIGHT
	};
	pressed_keys = std::vector<bool>(valid_keys.size());
}

void PrismInputs::newMpos(double xpos, double ypos)
{
	if (!nodata) {
		dmx += xpos - lmx;
		dmy += ypos - lmy;
	}
	else nodata = false;

	lmx = xpos;
	lmy = ypos;
}

void PrismInputs::updateKeyPressState(int key, int scancode, int action, int mods)
{
	int kind = 0;
	bool kfound = false;

	while (kind < valid_keys.size()) {
		if (valid_keys[kind] == key) {
			kfound = true;
			break;
		}
		kind++;
	}

	if (kfound) {
		if (action == GLFW_PRESS) pressed_keys[kind] = true;
		else if (action == GLFW_RELEASE) pressed_keys[kind] = false;
	}
}

bool PrismInputs::wasKeyPressed(int key)
{
	int kind = 0;
	bool kfound = false;

	while (kind < valid_keys.size()) {
		if (valid_keys[kind] == key) {
			kfound = true;
			break;
		}
		kind++;
	}

	if (kfound) return pressed_keys[kind];

	return false;
}

void PrismInputs::clearMOffset() {
	dmx = 0;
	dmy = 0;
}
