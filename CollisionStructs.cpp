#include "CollisionStructs.h"
#include <cmath>


void collutils::ConvexPolyPlane::init_polydata()
{
	sides = points.size();
	for (size_t i = 0; i < points.size(); i++) rays.push_back(glm::normalize(points[(i + 1) % sides] - points[i]));
	n = glm::cross(rays[0], rays[1]);
	for (size_t i = 0; i < points.size(); i++) perps.push_back(glm::cross(n, rays[i]));
	equation = glm::vec4(n, -glm::dot(n, points[0]));
}

collutils::ConvexPolyPlane::ConvexPolyPlane(std::vector<glm::vec3> polyPoints, float thickness, float planeFriction)
{
	points = polyPoints;
	height = thickness;
	friction = planeFriction;
	init_polydata();
}

collutils::ConvexPolyPlane::ConvexPolyPlane(glm::vec3 rectCenter, glm::vec3 u, glm::vec3 v, float lenU, float lenV, float thickness, float planeFriction)
{
	glm::vec3 ua = 0.5f * lenU * glm::normalize(u);
	glm::vec3 va = 0.5f * lenU * glm::normalize(v);
	points.push_back(rectCenter - ua - va);
	points.push_back(rectCenter + ua - va);
	points.push_back(rectCenter + ua + va);
	points.push_back(rectCenter - ua + va);
	height = thickness;
	friction = planeFriction;
	init_polydata();
}

Mesh collutils::ConvexPolyPlane::gen_mesh() {
	std::vector<Vertex> vlist = std::vector<Vertex>((sides - 2) * 3);
	glm::vec3 uaxn = glm::normalize(rays[0]);
	glm::vec3 vaxn = glm::cross(rays[0], n);
	for (int ti = 0; ti < sides - 2; ti++) {
		vlist[3 * ti].pos = points[0];
		vlist[3 * ti].normal = n;
		vlist[3 * ti].texCoord = glm::vec2(0, 0);

		vlist[(3 * ti) + 1].pos = points[ti + 1];
		vlist[(3 * ti) + 1].normal = n;
		vlist[(3 * ti) + 1].texCoord = glm::vec2(glm::dot(uaxn, points[ti + 1] - points[0]), glm::dot(vaxn, points[ti + 1] - points[0]));

		vlist[(3 * ti) + 2].pos = points[ti + 2];
		vlist[(3 * ti) + 2].normal = n;
		vlist[(3 * ti) + 2].texCoord = glm::vec2(glm::dot(uaxn, points[ti + 2] - points[0]), glm::dot(vaxn, points[ti + 2] - points[0]));
	}

	Mesh mout;
	mout.add_vertices(vlist);

	return mout;
}

int collutils::ConvexPolyPlane::point_status(glm::vec3 p, float pdist)
{
	float ndist = glm::dot(glm::vec4(p, 1), equation);
	glm::vec3 q = p - ndist * n;
	bool in_region = false;
	for (size_t i = 0; i < sides; i++) {
		if (glm::dot(q - points[(i + 1) % sides], perps[i]) < 0) return 0;
	}
	return (0 <= ndist && ndist <= height) ? 1 : 2;
}

collutils::CollPoint collutils::ConvexPolyPlane::check_point_future(glm::vec3 ploc, glm::vec3 pvel, glm::vec3 pacc, float until)
{
	CollPoint out;
	if (pacc == glm::vec3(0)) {
		if (pvel == glm::vec3(0)) {
			out.point = ploc;
			out.time = 0;
			out.will_collide = (point_status(ploc, abs(glm::dot(glm::vec4(ploc, 1), equation))) == 1);
			return out;
		}
		else {
			float d = glm::dot(glm::vec4(ploc, 1), equation);
			float udn = glm::dot(pvel, n);

			out.time = -d / udn;
			out.point = ploc + out.time * pvel;
			out.will_collide = (out.time <= until && point_status(ploc, abs(d)) == 1);
			return out;
		}
	}
	else {
		float dqeA = glm::dot(pacc, n);
		float qeB = glm::dot(pvel, n);
		float qeC = glm::dot(glm::vec4(ploc, 1), equation);
		float qdelta = qeB * qeB - 2 * dqeA * qeC;
		if (qdelta < 0) {
			out.will_collide = false;
			return out;
		}
		else {
			double rdelta = sqrt(qdelta);
			float lpr = (-qeB - rdelta) / dqeA;
			if (lpr < 0) lpr = (-qeB + rdelta) / dqeA;
			if (lpr < 0 || lpr > until) {
				out.will_collide = false;
				return out;
			}
			else {
				out.time = lpr;
				out.point = ploc + (pvel * lpr) + (pacc * lpr * lpr * 0.5f);
				float opd = glm::dot(glm::vec4(out.point, 1), equation);
				if (qeC * opd < 0) {
					out.point -= opd * n;
				}
				out.will_collide = (point_status(out.point, 0) == 1);
				return out;
			}
		}
	}
}

collutils::CirclePlane::CirclePlane(glm::vec3 planeNormal, glm::vec3 planeCenter, float circleRadius, float thickness)
{
	n = glm::normalize(planeNormal);
	height = abs(thickness);
	radius = circleRadius;
	center = planeCenter;
	equation = glm::vec4(n, -glm::dot(n, planeCenter));
}

int collutils::CirclePlane::point_status(glm::vec3 p)
{
	float ndist = glm::dot(glm::vec4(p, 1), equation);
	glm::vec3 q = p - ndist * n;
	if (glm::length(q - center) <= radius) return (0 <= ndist && ndist <= height) ? 1 : 2;
	else return 0;
}

collutils::Sphere::Sphere(glm::vec3 sphereCenter, float sphereRadius)
{
	center = sphereCenter;
	radius = sphereRadius;
}

int collutils::Sphere::point_status(glm::vec3 p)
{
	return (glm::length(p - center) <= radius) ? 1 : 0;
}

glm::vec3 collutils::project_vec_on_plane(glm::vec3 vToProj, glm::vec3 planeN) {
	if (abs(glm::dot(glm::normalize(vToProj), glm::normalize(planeN))) > 0.99) return glm::vec3(0);
	glm::vec3 tmp = glm::cross(planeN, vToProj);
	tmp = glm::normalize(glm::cross(tmp, planeN));
	return glm::dot(vToProj, tmp) * tmp;
}

glm::vec3 collutils::apply_bound_planes(glm::vec3 vec_to_bound, std::vector<ConvexPolyPlane> touching_planes) {
	if (touching_planes.size() == 0) return vec_to_bound;
	if (glm::length(vec_to_bound) < 0.001) return glm::vec3(0);
	for (int cdi = 0; cdi < touching_planes.size(); cdi++) {
		float vcomp = glm::dot(vec_to_bound, touching_planes[cdi].n);
		glm::vec3 vtc = (vcomp >= 0) ? vec_to_bound : project_vec_on_plane(vec_to_bound, touching_planes[cdi].n);

		bool accept_graze = true;
		for (int cdj = 0; cdj < touching_planes.size(); cdj++) {
			if (glm::dot(vtc, touching_planes[cdj].n) < 0) {
				accept_graze = false;
				break;
			}
		}
		if (accept_graze) {
			return vtc;
		}
	}
	return glm::vec3(0);
}

collutils::KinePointObj collutils::progress_kinematics(KinePointObj kpo, std::vector<ConvexPolyPlane>* planes, int nfPlaneIdx, float fwd_time) {
	KinePointObj outData = kpo;
	//out.vel += input_vel;

	int coll_count = 0;
	float remain_time = fwd_time;
	float next_coll_time = 0;

	while (remain_time > 0.001) {
		// Graze check
		std::vector<CollPoint> graze_data = std::vector<CollPoint>(planes->size());
		std::vector<ConvexPolyPlane> touching_planes;
		std::vector<bool> frict_data = std::vector<bool>(planes->size(), false);
		for (int cdi = 0; cdi < graze_data.size(); cdi++) {
			//std::cout << outData.pos.x << ',' << outData.pos.y << ',' << outData.pos.z << '\n';
			graze_data[cdi] = (*planes)[cdi].check_point_future(outData.pos, glm::vec3(0), glm::vec3(0), remain_time);
			if (graze_data[cdi].will_collide && graze_data[cdi].time == 0) {
				touching_planes.push_back((*planes)[cdi]);
			}
		}

		// Bound velocity & acc
		glm::vec3 bound_vel = apply_bound_planes(outData.vel, touching_planes);
		glm::vec3 bound_acc = apply_bound_planes(outData.acc, touching_planes);

		// Apply friction
		float friction_factor = 0;
		glm::vec3 friction = glm::vec3(0.0f);
		for (int cdi = 0; cdi < graze_data.size(); cdi++) {
			if (graze_data[cdi].will_collide && graze_data[cdi].time == 0 && (glm::dot((*planes)[cdi].n, outData.acc) < 0.0 && glm::dot((*planes)[cdi].n, outData.vel) <= 0.0) && cdi != nfPlaneIdx) { // && glm::length(bound_vel) > 0.00f
				friction_factor += (*planes)[cdi].friction;
			}
		}

		outData.vel = bound_vel;

		if (glm::length(outData.vel) > 0) {
			if (glm::length(bound_acc) < friction_factor) {
				bound_acc = -friction_factor * glm::normalize(outData.vel);
			}
			else bound_acc -= glm::normalize(outData.vel) * friction_factor;
		}
		else {
			if (glm::length(bound_acc) <= friction_factor) bound_acc = glm::vec3(0);
			else {
				bound_acc -= friction_factor * glm::normalize(bound_acc);
			}
		}

		float friction_time = remain_time;
		if (friction_factor != 0) {
			float temptime = -glm::length(outData.vel) / glm::dot(glm::normalize(outData.vel), bound_acc);
			if (temptime >= 0) {
				friction_time = std::min(temptime, remain_time);
			}
		}

		// Calculate collisions before frictions stop obj
		float min_time = friction_time;
		glm::vec3 min_coll_point = glm::vec3(0);
		int cplane_i = -1;
		for (int cdi = 0; cdi < graze_data.size(); cdi++) {
			if (!(graze_data[cdi].will_collide && graze_data[cdi].time == 0)) {
				CollPoint coll_data = (*planes)[cdi].check_point_future(outData.pos, outData.vel, bound_acc, friction_time);
				if (coll_data.will_collide && coll_data.time < min_time) {
					min_time = coll_data.time;
					min_coll_point = coll_data.point;
					cplane_i = cdi;
				}
			}
		}
		if (cplane_i == -1) {
			outData.pos = outData.pos + (outData.vel * friction_time) + (bound_acc * friction_time * friction_time * 0.5f);
			outData.vel += bound_acc * friction_time;
			if (friction_time < remain_time) {
				outData.vel = glm::vec3(0);
			}
			remain_time -= friction_time;
		}
		else {
			outData.pos = min_coll_point;
			outData.vel += bound_acc * min_time;
			remain_time -= min_time;
		}
	}
	//std::cout << outData.pos.x << ',' << outData.pos.y << ',' << outData.pos.z << '\n';
	//std::cout << outData.vel.x << ',' << outData.vel.y << ',' << outData.vel.z << '\n';
	return outData;
}

std::vector<collutils::ConvexPolyPlane> collutils::gen_cube_bplanes(glm::vec3 ccenter, glm::vec3 uax, glm::vec3 vax, float ulen, float vlen, float tlen, float face_thickness)
{
	std::vector<ConvexPolyPlane> plout;

	glm::vec3 uaxn = glm::normalize(uax);
	glm::vec3 vaxn = glm::normalize(vax);
	glm::vec3 taxn = glm::cross(uaxn, vaxn);

	plout.push_back(ConvexPolyPlane(ccenter + (0.5f * ulen * uaxn), vaxn, taxn, vlen, tlen, face_thickness, 0.0f));
	plout.push_back(ConvexPolyPlane(ccenter - (0.5f * ulen * uaxn), vaxn, -taxn, vlen, tlen, face_thickness, 0.0f));
	plout.push_back(ConvexPolyPlane(ccenter + (0.5f * vlen * vaxn), taxn, uaxn, tlen, ulen, face_thickness, 0.0f));
	plout.push_back(ConvexPolyPlane(ccenter - (0.5f * vlen * vaxn), taxn, -uaxn, tlen, ulen, face_thickness, 0.0f));
	plout.push_back(ConvexPolyPlane(ccenter + (0.5f * tlen * taxn), uaxn, vaxn, ulen, vlen, face_thickness, 0.0f));
	plout.push_back(ConvexPolyPlane(ccenter - (0.5f * tlen * taxn), uaxn, -vaxn, ulen, vlen, face_thickness, 0.0f));

	return plout;
}