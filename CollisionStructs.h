#pragma once
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <vector>
#include "vkstructs.h"
namespace collutils {

	struct KinePointObj
	{
		glm::vec3 pos = glm::vec3(0);
		glm::vec3 vel = glm::vec3(0);
		glm::vec3 acc = glm::vec3(0);
	};

	struct CollPoint {
		bool will_collide = false;
		glm::vec3 point;
		KinePointObj collide_kpo;
		float time;
	};

	struct ConvexPolyPlane
	{
		std::vector<glm::vec3> points;
		int sides;
		glm::vec4 equation;
		glm::vec3 n;
		std::vector<glm::vec3> rays;
		std::vector<glm::vec3> perps;
		float height;
		float friction;

		void init_polydata();
		ConvexPolyPlane(std::vector<glm::vec3> polyPoints, float thickness = 0, float planeFriction = 1.0f);
		ConvexPolyPlane(glm::vec3 rectCenter, glm::vec3 u, glm::vec3 v, float lenU, float lenV, float thickness = 0, float planeFriction = 1.0f);

		Mesh gen_mesh();
		void addToRenderer();

		int point_status(glm::vec3 p, float pdist = 0);
		CollPoint check_point_future(glm::vec3 ploc, glm::vec3 pvel, glm::vec3 pacc, float until = 10);
	};

	struct CirclePlane
	{
		glm::vec4 equation;
		glm::vec3 n;
		glm::vec3 center;
		float height, radius;

		CirclePlane(glm::vec3 planeNormal, glm::vec3 planeCenter, float circleRadius, float thickness = 0);
		int point_status(glm::vec3 p);
	};

	struct Sphere
	{
		glm::vec3 center;
		float radius;

		Sphere(glm::vec3 sphereCenter, float sphereRadius);
		int point_status(glm::vec3 p);
	};

	glm::vec3 project_vec_on_plane(glm::vec3 vToProj, glm::vec3 planeN);

	glm::vec3 apply_bound_planes(glm::vec3 vec_to_bound, std::vector<ConvexPolyPlane> touching_planes);

	KinePointObj progress_kinematics(KinePointObj kpo, std::vector<ConvexPolyPlane>* planes, int nfPlaneIdx, float fwd_time);

	std::vector<ConvexPolyPlane> gen_cube_bplanes(glm::vec3 ccenter, glm::vec3 uax, glm::vec3 vax, float ulen, float vlen, float tlen, float face_thickness = 0.1);
}
