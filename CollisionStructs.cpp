#include "CollisionStructs.h"
#include <set>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>


collutils::isecLine collutils::find_isec_of_planes(glm::vec4 planeeq1, glm::vec4 planeeq2, float thickness, bool normalized)
{
	glm::vec3 pn1, pn2;
	glm::vec4 npe1 = planeeq1, npe2 = planeeq2;
	pn1 = glm::vec3(planeeq1.x, planeeq1.y, planeeq1.z);
	pn2 = glm::vec3(planeeq2.x, planeeq2.y, planeeq2.z);

	if (!normalized) {
		npe1 = npe1/glm::length(pn1);
		npe2 = npe2/glm::length(pn2);

		pn1 = glm::normalize(pn1);
		pn2 = glm::normalize(pn2);
	}

	isecLine outdata;
	if (abs(glm::dot(pn1, pn2)) > 0.9999) {
		outdata.parallel = true;
		outdata.will_isec = (abs(abs(npe1.w) - abs(npe2.w)) < thickness) ? true : false;
		return outdata;
	}
	else {
		outdata.parallel = false;
		outdata.will_isec = true;
		outdata.ldir = glm::cross(pn1, pn2);
		glm::vec3 temp = glm::cross(outdata.ldir, pn1);
		outdata.point = (-npe1.w * pn1) + ((npe1.w * glm::dot(pn1, pn2) / glm::dot(temp, pn2)) * temp) ;
		return outdata;
	}
}

collutils::isecPoint collutils::find_isec_of_lines(glm::vec3 lpoint1, glm::vec3 ldir1, glm::vec3 lpoint2, glm::vec3 ldir2, bool normalized)
{
	glm::mat2 cmatrix = { glm::vec2(ldir1.x, ldir1.y), glm::vec2(-ldir2.x, -ldir2.y) };
	glm::vec2 omatrix = glm::vec2(lpoint2.x - lpoint1.x, lpoint2.y - lpoint1.y);
	
	if (glm::determinant(cmatrix) == 0) {
		cmatrix = { glm::vec2(ldir1.x, -ldir2.x), glm::vec2(ldir1.z, -ldir2.z) };
		omatrix = glm::vec2(lpoint2.x - lpoint1.x, lpoint2.z - lpoint1.z);
	}

	isecPoint outdata;

	if (glm::determinant(cmatrix) == 0) {
		outdata.parallel = true;
		outdata.will_isec = (abs(glm::dot(glm::normalize(lpoint2 - lpoint1), glm::normalize(ldir1))) > 0.9999) ? true : false;
	}
	else {
		outdata.parallel = false;
		glm::vec2 solmat = glm::inverse(cmatrix) * omatrix;
		if (glm::length((lpoint1 + solmat.x * ldir1) - (lpoint2 + solmat.y * ldir2)) < 0.001) {
			outdata.will_isec = true;
			outdata.point = lpoint1 + solmat.x * ldir1;
			return outdata;
		}
		else {
			outdata.will_isec = false;
			return outdata;
		}
	}
}

collutils::isecPoint collutils::find_isec_of_linesegments(glm::vec3 l1a, glm::vec3 l1b, glm::vec3 l2a, glm::vec3 l2b)
{
	glm::vec3 l1dir = l1b - l1a;
	glm::vec3 l2dir = l2b - l2a;
	glm::mat2 cmatrix = { glm::vec2(l1dir.x, l1dir.y), glm::vec2(-l2dir.x, -l2dir.y) };
	glm::vec2 omatrix = glm::vec2(l2a.x - l1a.x, l2a.y - l1a.y);

	if (glm::determinant(cmatrix) == 0) {
		cmatrix = { glm::vec2(l1dir.x, -l2dir.x), glm::vec2(l1dir.z, -l2dir.z) };
		omatrix = glm::vec2(l2a.x - l1a.x, l2a.z - l1a.z);
	}

	isecPoint outdata;

	if (glm::determinant(cmatrix) == 0) {
		outdata.parallel = true;
		outdata.will_isec = (abs(glm::dot(glm::normalize(l2a - l1a), glm::normalize(l1dir))) > 0.9999) ? true : false;
	}
	else {
		outdata.parallel = false;
		glm::vec2 solmat = glm::inverse(cmatrix) * omatrix;
		if (glm::length((l1a + solmat.x * l1dir) - (l2a + solmat.y * l2dir)) < 0.001 && solmat.x >= 0 && solmat.y >= 0 && solmat.x <= 1 && solmat.y <= 0) {
			outdata.will_isec = true;
			outdata.point = l1a + solmat.x * l1dir;
			return outdata;
		}
		else {
			outdata.will_isec = false;
			return outdata;
		}
	}
}

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
	glm::vec3 va = 0.5f * lenV * glm::normalize(v);
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
	glm::vec3 vaxn = glm::cross(n, rays[0]);
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

void collutils::ConvexPolyPlane::apply_displacement(glm::vec3 disp)
{
	for (int i = 0; i < points.size(); i++) points[i] += disp;
	equation = glm::vec4(n, -glm::dot(n, points[0]));
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
			out.will_collide = (out.time <= until && point_status(out.point, abs(d)) == 1);
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
					out.time -= opd / glm::dot(pvel + pacc * out.time, n);
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

collutils::CollMesh::CollMesh()
{
	vertices = {};
	planes = {};
	edges = {};
}

collutils::CollMesh::CollMesh(ConvexPolyPlane cnvpp)
{
	vertices.insert(vertices.end(), cnvpp.points.begin(), cnvpp.points.end());
	planes.push_back(cnvpp);
	for (int i = 0; i < vertices.size(); i++) {
		edges.push_back(glm::ivec4(i, (i+1) % vertices.size(), 0, -1));
	}
}

void collutils::CollMesh::apply_displacement(glm::vec3 disp)
{
	for (int i = 0; i < vertices.size(); i++) {
		vertices[i] += disp;
	}
	for (int i = 0; i < planes.size(); i++) {
		planes[i].apply_displacement(disp);
	}
}

//Outdated Fn
collutils::CollPoint collutils::check_lines_future(glm::vec3 l1point, glm::vec3 l1dir, glm::vec3 l2point, glm::vec3 l2dir, glm::vec3 l2vel, glm::vec3 l2acc, float until)
{
	CollPoint outdata;

	glm::vec3 np = glm::normalize(glm::cross(l1dir, l2dir));

	float dqeA = glm::dot(l2acc, np);
	float qeB = glm::dot(l2vel, np);
	float qeC = glm::dot((l1point - l2point), np);
	float qdelta = qeB * qeB - 2 * dqeA * qeC;

	if (qdelta < 0) {
		outdata.will_collide = false;
		return outdata;
	}
	else {
		double rdelta = sqrt(qdelta);
		float lpr = (-qeB - rdelta) / dqeA;
		if (lpr < 0) lpr = (-qeB + rdelta) / dqeA;
		if (lpr < 0 || lpr > until) {
			outdata.will_collide = false;
			return outdata;
		}
		else {
			outdata.time = lpr;
			glm::vec3 nlp2 = glm::cross(np, glm::normalize(l2dir));
			float lt1 = -(glm::dot(l1point, nlp2) - glm::dot(l2point, nlp2))/glm::dot(glm::normalize(l1dir),nlp2);
			outdata.point = l1point + lt1 * glm::normalize(l1dir);
			outdata.will_collide = true;
			return outdata;
		}
	}
}

collutils::CollPoint collutils::check_lineseg_future(glm::vec3 l1a, glm::vec3 l1b, glm::vec3 l2a, glm::vec3 l2b, glm::vec3 l2vel, glm::vec3 l2acc, float until)
{
	CollPoint outdata;
	glm::vec3 l1dir = l1b - l1a;
	glm::vec3 l2dir = l2b - l2a;

	glm::vec3 np;

	if (abs(glm::dot(glm::normalize(l1dir), glm::normalize(l2dir))) > 0.9999) {
		if (abs(glm::dot(glm::normalize(l2b - l1a), glm::normalize(l1b - l1a))) > 0.9999) {
			float t1 = glm::dot(l2a - l1a, l1dir);
			float t2 = glm::dot(l2b - l1a, l1dir);

			if ((t1 <= 0 && t2 <= 0) || (t1 >= 1 && t2 >= 1)) {
				outdata.will_collide = false;
				return outdata;
			}
			else {
				outdata.time = 0;
				outdata.will_collide = true;
				return outdata;
			}
		}
		else {
			np = glm::normalize(glm::cross(glm::cross(l2a - l1a, l2dir), l2dir));
		}
	}
	else {
		np = glm::normalize(glm::cross(l1dir, l2dir));
	}

	float dqeA = glm::dot(l2acc, np);
	float qeB = glm::dot(l2vel, np);
	float qeC = glm::dot((l2a - l1a), np);
	float qdelta = qeB * qeB - 2 * dqeA * qeC;

	if (qdelta < 0) {
		outdata.will_collide = false;
		return outdata;
	}
	else {
		float lpr;
		if (dqeA == 0) {
			if (qeB == 0) {
				if (abs(qeC) < 0.05) {
					lpr = 0;
				}
				else {
					outdata.will_collide = false;
					return outdata;
				}
			}
			else {
				lpr = -qeC / qeB;
			}
		}
		else {
			double rdelta = sqrt(qdelta);
			lpr = (-qeB - rdelta) / dqeA;
			if (lpr < 0) lpr = (-qeB + rdelta) / dqeA;
		}
		
		if (lpr < 0 || lpr > until) {
			outdata.will_collide = false;
			return outdata;
		}
		else {
			outdata.time = lpr;
			glm::vec3 nlp2 = glm::cross(np, glm::normalize(l2dir));
			glm::vec3 fl2a = l2a + l2vel * lpr + l2acc * lpr * lpr * 0.5f;
			float lt1 = (glm::dot(fl2a - l1a, nlp2)) / glm::dot(l1dir, nlp2);
			outdata.point = l1a + lt1 * l1dir;
			float lt2 = glm::dot(outdata.point - fl2a, l2dir) / glm::dot(l2dir, l2dir);
			outdata.will_collide = (0 <= lt1 && lt1 <= 1 && 0 <= lt2 && lt2 <= 1);
			//outdata.will_collide = true;
			outdata.point -= l2a + l2dir * lt2; //+ 0.03f * np;
			return outdata;
		}
	}
}

collutils::SolidCollData collutils::check_polyplanes_future(ConvexPolyPlane pp1, ConvexPolyPlane pp2, glm::vec3 pvel, glm::vec3 pacc, float until)
{
	SolidCollData outdata;
	outdata.time = until;

	int pp1s = pp1.points.size();
	int pp2s = pp2.points.size();

	for (int pidx = 0; pidx < pp1s; pidx++) {
		CollPoint tmp1 = pp2.check_point_future(pp1.points[pidx], -pvel, -pacc, until);
		if (tmp1.will_collide && (!outdata.will_collide || tmp1.time < outdata.time)) {
			outdata.time = tmp1.time;
			outdata.will_collide = true;
			outdata.disp = (pvel * outdata.time) + (0.5f * pacc * outdata.time * outdata.time);
		}
	}
	for (int pidx = 0; pidx < pp2s; pidx++) {
		CollPoint tmp1 = pp1.check_point_future(pp2.points[pidx], pvel, pacc, until);
		if (tmp1.will_collide && (!outdata.will_collide || tmp1.time < outdata.time)) {
			outdata.time = tmp1.time;
			outdata.will_collide = true;
			outdata.disp = (pvel * outdata.time) + (0.5f * pacc * outdata.time * outdata.time);
		}
	}
	for (int pidx = 0; pidx < pp1s; pidx++) {
		for (int pidy = 0; pidy < pp2s; pidy++) {
			CollPoint tmp1 = check_lineseg_future(pp1.points[(pidx + 1) % pp1s], pp1.points[pidx], pp2.points[(pidy + 1) % pp2s], pp2.points[pidy], pvel, pacc, until);
			if (tmp1.will_collide && (!outdata.will_collide || tmp1.time < outdata.time)) {
				outdata.time = tmp1.time;
				outdata.will_collide = true;
				outdata.disp = (pvel * outdata.time) + (0.5f * pacc * outdata.time * outdata.time);
			}
		}
	}

	return outdata;
}

collutils::SolidCollData collutils::check_mesh_future(CollMesh cm1, CollMesh cm2, glm::vec3 mvel, glm::vec3 macc, float until)
{
	SolidCollData outdata;
	outdata.time = until;

	for (uint32_t vidx = 0; vidx < cm2.vertices.size(); vidx++) {
		for (uint32_t pidx = 0; pidx < cm1.planes.size(); pidx++) {
			CollPoint tmp1 = cm1.planes[pidx].check_point_future(cm2.vertices[vidx], mvel, macc, until);
			if (tmp1.will_collide && (!outdata.will_collide || tmp1.time < outdata.time)) {
				outdata.will_collide = true;
				outdata.pl_id = pidx;
				outdata.time = tmp1.time;
				//outdata.bound_dir = cm1.planes[pidx].n;
				outdata.disp = mvel * outdata.time + (0.5f * macc * outdata.time * outdata.time);
			}
		}
	}

	for (uint32_t vidx = 0; vidx < cm1.vertices.size(); vidx++) {
		for (uint32_t pidx = 0; pidx < cm2.planes.size(); pidx++) {
			CollPoint tmp1 = cm2.planes[pidx].check_point_future(cm1.vertices[vidx], -mvel, -macc, until);
			if (tmp1.will_collide && (!outdata.will_collide || tmp1.time < outdata.time)) {
				outdata.will_collide = true;
				outdata.pl_id = pidx;
				outdata.time = tmp1.time;
				//outdata.bound_dir = cm2.planes[pidx].n;
				outdata.disp = mvel * outdata.time + (0.5f * macc * outdata.time * outdata.time);
			}
		}
	}

	bool edge_coll = false;
	for (uint32_t eidx = 0; eidx < cm1.edges.size(); eidx++) {
		for (uint32_t eidy = 0; eidy < cm2.edges.size(); eidy++) {
			CollPoint tmp1 = check_lineseg_future(cm1.vertices[cm1.edges[eidx].x], cm1.vertices[cm1.edges[eidx].y], cm2.vertices[cm2.edges[eidy].x], cm2.vertices[cm2.edges[eidy].y], mvel, macc, until);
			if (tmp1.will_collide && (!outdata.will_collide || tmp1.time <= outdata.time)) {
				outdata.will_collide = true;
				outdata.pl_id = -1;
				outdata.time = tmp1.time;
				outdata.disp = tmp1.point;
				outdata.disp = mvel * outdata.time + (0.5f * macc * outdata.time * outdata.time);
				edge_coll = true;
			}
		}
	}

	bool found_dplane = false;
	for (int pli = 0; pli < cm1.planes.size(); pli++) {
		bool dir_sign = true;
		bool dir_set = false;
		bool this_dplane = true;
		for (int vxi = 0; vxi < cm1.vertices.size(); vxi++) {
			float d = glm::dot(cm1.planes[pli].equation, glm::vec4(cm1.vertices[vxi], 1));
			if (!dir_set && d != 0) {
				dir_set = true;
				dir_sign = (d >= 0);
			}
			if (dir_sign ^ (d >= 0)) {
				this_dplane = false;
				break;
			}
		}
		for (int vxi = 0; vxi < cm2.vertices.size(); vxi++) {
			float d = glm::dot(cm1.planes[pli].equation, glm::vec4(cm2.vertices[vxi], 1));
			if (dir_sign ^ (d <= 0)) {
				this_dplane = false;
				break;
			}
		}
		if (this_dplane) {
			outdata.bound_dir = cm1.planes[pli].n;
			found_dplane = true;
			break;
		}
	}
	if (!found_dplane) {
		for (int pli = 0; pli < cm2.planes.size(); pli++) {
			bool dir_sign = true;
			bool dir_set = false;
			bool this_dplane = true;
			for (int vxi = 0; vxi < cm1.vertices.size(); vxi++) {
				float d = glm::dot(cm2.planes[pli].equation, glm::vec4(cm1.vertices[vxi], 1));
				if (!dir_set && d != 0) {
					dir_set = true;
					dir_sign = (d > 0);
				}
				if (dir_sign ^ (d >= 0)) {
					this_dplane = false;
					break;
				}
			}
			for (int vxi = 0; vxi < cm2.vertices.size(); vxi++) {
				float d = glm::dot(cm2.planes[pli].equation, glm::vec4(cm2.vertices[vxi], 1));
				if (dir_sign ^ (d <= 0)) {
					this_dplane = false;
					break;
				}
			}
			if (this_dplane) {
				outdata.bound_dir = -cm2.planes[pli].n;
				break;
			}
		}
	}
	
	return outdata;
}

glm::vec3 collutils::project_vec_on_plane(glm::vec3 vToProj, glm::vec3 planeN) {
	if (abs(glm::dot(glm::normalize(vToProj), glm::normalize(planeN))) > 0.99) return glm::vec3(0);
	glm::vec3 tmp = glm::cross(planeN, vToProj);
	tmp = glm::normalize(glm::cross(tmp, planeN));
	return glm::dot(vToProj, tmp) * tmp;
}

glm::vec3 collutils::apply_bound_planes(glm::vec3 vec_to_bound, std::vector<ConvexPolyPlane> touching_planes) {
	if (touching_planes.size() == 0) return vec_to_bound;
	if (glm::length(vec_to_bound) < 0.0001) return glm::vec3(0);
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

glm::vec3 collutils::apply_bound_dirs(glm::vec3 vec_to_bound, std::vector<glm::vec3> bound_dirs) {
	if (bound_dirs.size() == 0) return vec_to_bound;
	if (glm::length(vec_to_bound) < 0.0001) return glm::vec3(0);
	for (int cdi = 0; cdi < bound_dirs.size(); cdi++) {
		float vcomp = glm::dot(vec_to_bound, bound_dirs[cdi]);
		glm::vec3 vtc = (vcomp >= 0) ? vec_to_bound : project_vec_on_plane(vec_to_bound, bound_dirs[cdi]);

		bool accept_graze = true;
		for (int cdj = 0; cdj < bound_dirs.size(); cdj++) {
			if (glm::dot(vtc, bound_dirs[cdj]) < 0) {
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

collutils::KineSolidObj collutils::progress_solid_kinematics(KineSolidObj kso, std::vector<CollMesh> smeshes, int nfPlaneIdx, float fwd_time)
{
	KineSolidObj outkso = kso;
	//out.vel += input_vel;

	int coll_count = 0;
	float remain_time = fwd_time;
	float next_coll_time = 0;

	while (remain_time > 0.001) {
		// Graze check
		std::vector<glm::vec3> bound_dirs;
		std::vector<ConvexPolyPlane> touching_planes;
		std::vector<SolidCollData> graze_data = std::vector<SolidCollData>(smeshes.size());
		int nfTpi = -1;

		for (int cdi = 0; cdi < smeshes.size(); cdi++) {
			//std::cout << outData.pos.x << ',' << outData.pos.y << ',' << outData.pos.z << '\n';
			graze_data[cdi] = check_mesh_future(smeshes[cdi], outkso._cmesh, glm::vec3(0), glm::vec3(0), remain_time);
			if (graze_data[cdi].will_collide && graze_data[cdi].time == 0) {
				if (cdi == nfPlaneIdx) nfTpi = touching_planes.size();
				if (graze_data[cdi].pl_id >= 0) touching_planes.push_back(smeshes[cdi].planes[graze_data[cdi].pl_id]);
				bound_dirs.push_back(graze_data[cdi].bound_dir);
			}
		}

		// Bound velocity & acc
		int bdl = bound_dirs.size();
		glm::vec3 bound_vel = apply_bound_dirs(outkso._vel, bound_dirs);
		glm::vec3 bound_acc = apply_bound_dirs(outkso._acc, bound_dirs);

		// Apply friction
		float friction_factor = 0;
		glm::vec3 friction = glm::vec3(0.0f);
		for (int tpi = 0; tpi < touching_planes.size(); tpi++) {
			if ((glm::dot(touching_planes[tpi].n, outkso._acc) < 0.0 && glm::dot(touching_planes[tpi].n, outkso._vel) <= 0.0) && tpi != nfTpi) { // && glm::length(bound_vel) > 0.00f
				friction_factor += touching_planes[tpi].friction;
			}
		}

		outkso._vel = bound_vel;

		if (glm::length(outkso._vel) > 0) {
			if (glm::length(bound_acc) < friction_factor) {
				bound_acc = -friction_factor * glm::normalize(outkso._vel);
			}
			else bound_acc -= glm::normalize(outkso._vel) * friction_factor;
		}
		else {
			if (glm::length(bound_acc) <= friction_factor) bound_acc = glm::vec3(0);
			else {
				bound_acc -= friction_factor * glm::normalize(bound_acc);
			}
		}

		float friction_time = remain_time;
		if (friction_factor != 0) {
			float temptime = -glm::length(outkso._vel) / glm::dot(glm::normalize(outkso._vel), bound_acc);
			if (temptime >= 0) {
				friction_time = std::min(temptime, remain_time);
			}
		}

		// Calculate collisions before frictions stop obj
		float min_time = friction_time;
		glm::vec3 min_coll_disp = glm::vec3(0);
		int cplane_i = -1;
		for (int cdi = 0; cdi < smeshes.size(); cdi++) {
			if (!(graze_data[cdi].will_collide && graze_data[cdi].time == 0)) {
				SolidCollData coll_data = check_mesh_future(smeshes[cdi], outkso._cmesh,outkso._vel, outkso._acc, remain_time);
				if (coll_data.will_collide && coll_data.time < min_time) {
					min_time = coll_data.time;
					min_coll_disp = coll_data.disp;
					cplane_i = cdi;
				}
			}
		}
		if (cplane_i == -1) {
			glm::vec3 fdisp = (outkso._vel * friction_time) + (bound_acc * friction_time * friction_time * 0.5f);

			outkso._cmesh.apply_displacement(fdisp);
			outkso._center += fdisp;
			outkso._vel += bound_acc * friction_time;
			if (friction_time < remain_time) {
				outkso._vel = glm::vec3(0);
			}
			remain_time -= friction_time;
		}
		else {
			glm::vec3 fdisp = min_coll_disp;

			outkso._cmesh.apply_displacement(fdisp);
			outkso._center += fdisp;
			outkso._vel += bound_acc * min_time;
			remain_time -= min_time;
		}
	}
	//std::cout << outData.pos.x << ',' << outData.pos.y << ',' << outData.pos.z << '\n';
	//std::cout << outData.vel.x << ',' << outData.vel.y << ',' << outData.vel.z << '\n';
	
	return outkso;
}

collutils::CollMesh collutils::gen_cube_bplanes(glm::vec3 ccenter, glm::vec3 uax, glm::vec3 vax, float ulen, float vlen, float tlen, float face_thickness, float face_friction)
{
	CollMesh cmesh;

	glm::vec3 uaxn = glm::normalize(uax);
	glm::vec3 vaxn = glm::normalize(vax);
	glm::vec3 taxn = glm::cross(uaxn, vaxn);

	cmesh.planes.push_back(ConvexPolyPlane(ccenter + (0.5f * ulen * uaxn), vaxn, taxn, vlen, tlen, face_thickness, face_friction));
	cmesh.planes.push_back(ConvexPolyPlane(ccenter - (0.5f * ulen * uaxn), vaxn, -taxn, vlen, tlen, face_thickness, face_friction));
	cmesh.planes.push_back(ConvexPolyPlane(ccenter - (0.5f * vlen * vaxn), taxn, -uaxn, tlen, ulen, face_thickness, face_friction));
	cmesh.planes.push_back(ConvexPolyPlane(ccenter - (0.5f * tlen * taxn), uaxn, -vaxn, ulen, vlen, face_thickness, face_friction));
	cmesh.planes.push_back(ConvexPolyPlane(ccenter + (0.5f * vlen * vaxn), taxn, uaxn, tlen, ulen, face_thickness, face_friction));
	cmesh.planes.push_back(ConvexPolyPlane(ccenter + (0.5f * tlen * taxn), uaxn, vaxn, ulen, vlen, face_thickness, face_friction));
	
	cmesh.vertices.push_back(ccenter + 0.5f * (ulen * uaxn - vlen * vaxn + tlen * taxn));
	cmesh.vertices.push_back(ccenter + 0.5f * (ulen * uaxn - vlen * vaxn - tlen * taxn));
	cmesh.vertices.push_back(ccenter + 0.5f * (ulen * uaxn + vlen * vaxn - tlen * taxn));
	cmesh.vertices.push_back(ccenter + 0.5f * (ulen * uaxn + vlen * vaxn + tlen * taxn));
	
	cmesh.vertices.push_back(ccenter + 0.5f * (-ulen * uaxn - vlen * vaxn + tlen * taxn));
	cmesh.vertices.push_back(ccenter + 0.5f * (-ulen * uaxn - vlen * vaxn - tlen * taxn));
	cmesh.vertices.push_back(ccenter + 0.5f * (-ulen * uaxn + vlen * vaxn - tlen * taxn));
	cmesh.vertices.push_back(ccenter + 0.5f * (-ulen * uaxn + vlen * vaxn + tlen * taxn));

	for (int i = 0; i < 4; i++) {
		cmesh.edges.push_back(glm::ivec4(i, (i + 1) % 4, 0, 2 + i));
		cmesh.edges.push_back(glm::ivec4(4 + i, 4 + ((i + 1) % 4), 1, 2 + i));
		cmesh.edges.push_back(glm::ivec4(i, 4 + i, 2 + i, 2 + (3 + i)% 4));
	}
	return cmesh;
}

collutils::CollMesh collutils::gen_cube_bplanes(ConvexPolyPlane pplane, float tlen)
{
	CollMesh cmesh;
	cmesh.planes.push_back(pplane);
	cmesh.vertices.insert(cmesh.vertices.end(),pplane.points.begin(), pplane.points.end());
	std::vector<glm::vec3> of_points = pplane.points;
	for (int i = 0; i < of_points.size(); i++) {
		of_points[i] = (pplane.points[i] - (pplane.n * tlen));
	}
	cmesh.vertices.insert(cmesh.vertices.end(), of_points.begin(), of_points.end());
	int ppls = pplane.points.size();
	for (int ppi = 0; ppi < ppls; ppi++) {
		cmesh.planes.push_back(ConvexPolyPlane({ pplane.points[ppi], of_points[ppi], of_points[(ppi + 1) % ppls], pplane.points[(ppi + 1) % ppls] }, pplane.height, pplane.friction));
	}
	for (int ppi = 0; ppi < ppls; ppi++) {
		cmesh.edges.push_back(glm::ivec4(ppi, (ppi + 1) % ppls, 0, 1 + ppi));
		cmesh.edges.push_back(glm::ivec4(ppls + ppi, ppls + ((ppi + 1) % ppls), cmesh.planes.size(), 1 + ppi));
		cmesh.edges.push_back(glm::ivec4(ppi, ppi + ppls, 1 + ppi, 1 + (ppls + ppi - 1) % ppls));
		//cmesh.edges.push_back(glm::ivec4((ppi + 1) % ppls, ppls + ((ppi + 1) % ppls), ));
	}
	std::reverse(of_points.begin(), of_points.end());
	cmesh.planes.push_back(ConvexPolyPlane(of_points, pplane.height, pplane.friction));
	return cmesh;
}

/*
std::vector<collutils::ConvexPolyPlane> collutils::gen_cube_bplanes(ConvexPolyPlane pplane, float face_thickness, float face_friction)
{
	std::vector<ConvexPolyPlane> plout;
	plout.push_back(pplane);
	std::vector<glm::vec3> of_points;
	int ppls = pplane.points.size();
	for (int ppi = 0; ppi < ppls; ppi++) {
		of_points.push_back(pplane.points[ppi] + (pplane.n * face_thickness));
		plout.push_back(ConvexPolyPlane({ pplane.points[ppi], pplane.points[(ppi + 1) % ppls], pplane.points[(ppi + 1) % ppls] + (pplane.n * face_thickness), pplane.points[ppi] + (pplane.n * face_thickness) }, pplane.height, pplane.friction));
	}
	std::reverse(of_points.begin(), of_points.end());
	plout.push_back(ConvexPolyPlane(of_points, pplane.height, pplane.friction));
	return plout;
}
*/