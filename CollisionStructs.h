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
		glm::vec3 point = glm::vec3(0);
		float time = 0;
	};

	struct SolidCollData {
		bool will_collide = false;
		int pl_id = -1;
		glm::vec3 disp = glm::vec3(0);
		glm::vec3 bound_dir = glm::vec3(0);
		float time = 0;
	};

	struct isecPoint {
		bool will_isec = false;
		bool parallel = false;
		glm::vec3 point = glm::vec3(0);
	};

	struct isecLine {
		bool will_isec = false;
		bool parallel = false;
		glm::vec3 point = glm::vec3(0);
		glm::vec3 ldir = glm::vec3(0);
	};

	isecLine find_isec_of_planes(glm::vec4 planeeq1, glm::vec4 planeeq2, float thickness = 0.1, bool normalized=true);

	isecPoint find_isec_of_lines(glm::vec3 lpoint1, glm::vec3 ldir1, glm::vec3 lpoint2, glm::vec3 ldir2, bool normalized=true);
	
	isecPoint find_isec_of_linesegments(glm::vec3 l1a, glm::vec3 l1b, glm::vec3 l2a, glm::vec3 l2b);

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
		ConvexPolyPlane(std::vector<glm::vec3> polyPoints, float thickness = 0, float planeFriction = 0.0f);
		ConvexPolyPlane(glm::vec3 rectCenter, glm::vec3 u, glm::vec3 v, float lenU, float lenV, float thickness = 0, float planeFriction = 0.0f);

		Mesh gen_mesh();
		void addToRenderer();

		void apply_displacement(glm::vec3 disp);
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

	struct CollMesh {
		std::vector<glm::vec3> vertices;
		std::vector<ConvexPolyPlane> planes;
		std::vector<glm::ivec4> edges;

		CollMesh();
		CollMesh(ConvexPolyPlane cnvpp);
		void apply_displacement(glm::vec3 disp);
	};

	struct KineSolidObj
	{
		CollMesh _cmesh;
		glm::vec3 _center = glm::vec3(0);
		glm::vec3 _vel = glm::vec3(0);
		glm::vec3 _acc = glm::vec3(0);
	};

	CollPoint check_lines_future(glm::vec3 l1point, glm::vec3 l1dir, glm::vec3 l2point, glm::vec3 l2dir, glm::vec3 l2vel, glm::vec3 l2acc, float until = 10);

	CollPoint check_lineseg_future(glm::vec3 l1a, glm::vec3 l1b, glm::vec3 l2a, glm::vec3 l2b, glm::vec3 l2vel, glm::vec3 l2acc, float until = 10);

	SolidCollData check_polyplanes_future(ConvexPolyPlane pp1, ConvexPolyPlane pp2, glm::vec3 pvel, glm::vec3 pacc, float until = 10);

	SolidCollData check_mesh_future(CollMesh cm1, CollMesh cm2, glm::vec3 mvel, glm::vec3 macc, float until = 0);

	glm::vec3 project_vec_on_plane(glm::vec3 vToProj, glm::vec3 planeN);

	glm::vec3 apply_bound_planes(glm::vec3 vec_to_bound, std::vector<ConvexPolyPlane> touching_planes);
	glm::vec3 apply_bound_dirs(glm::vec3 vec_to_bound, std::vector<glm::vec3> bound_dirs);

	KinePointObj progress_kinematics(KinePointObj kpo, std::vector<ConvexPolyPlane>* planes, int nfPlaneIdx, float fwd_time);

	KineSolidObj progress_solid_kinematics(KineSolidObj kso, std::vector<CollMesh> smeshes, int nfPlaneIdx, float fwd_time);

	CollMesh gen_cube_bplanes(glm::vec3 ccenter, glm::vec3 uax, glm::vec3 vax, float ulen, float vlen, float tlen, float face_thickness = 0.1, float face_friction = 1);

	CollMesh gen_cube_bplanes(ConvexPolyPlane pplane, float tlen = 0.1);
}
