#define PUGIXML_HEADER_ONLY
#include <pugixml.hpp>
#include "vkstructs.h"

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <vector>

template <typename T>
std::vector<T> tokenize_data(std::string str_data) {
	std::string temp;
	std::vector<T> res;
	for (char c : str_data) {
		if (c == ' ' && !temp.empty()) {
			res.push_back(std::atof(temp.c_str()));
			temp = "";
		}
		else temp += c;
	}
	return res;
}

std::vector<Mesh> parse_dae_file(std::string dae_fname) {
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(dae_fname.c_str());

	if (result) {
		std::vector<Mesh> meshes_out;
		for (pugi::xml_node geom : doc.child("COLLADA").child("library_geometries").children()) {
			std::string gid = geom.attribute("id").value();
			std::string gname = geom.attribute("id").name();

			pugi::xml_node meshnode = geom.child("mesh");
			
			std::vector<float> plist = tokenize_data<float>(meshnode.find_child_by_attribute("id", (gid + "-positions").c_str()).value());
			std::vector<float> nlist = tokenize_data<float>(meshnode.find_child_by_attribute("id", (gid + "-normals").c_str()).value());
			std::vector<float> mlist = tokenize_data<float>(meshnode.find_child_by_attribute("id", (gid + "-map-0").c_str()).value());
			std::vector<int> tlist = tokenize_data<int>(meshnode.child("triangles").child("p").value());

			uint8_t vert_offset;
			uint8_t norm_offset;
			uint8_t tex_offset;
			uint8_t offset_count = 0;

			uint32_t tcount = unsigned int(meshnode.child("triangles").attribute("count").value());
			for (pugi::xml_node i : meshnode.child("triangles").children("input")) {
				if (i.attribute("semantic").value() == "VERTEX") vert_offset = uint8_t(i.attribute("offset").value());
				if (i.attribute("semantic").value() == "NORMAL") norm_offset = uint8_t(i.attribute("offset").value());
				if (i.attribute("semantic").value() == "TEXCOORD") tex_offset = uint8_t(i.attribute("offset").value());
				offset_count++;
			}
			
			std::vector<Vertex> v_arr;
			for (int idx = 0; idx < tcount * 3; idx++) {
				Vertex temp;
				temp.pos = {
					plist[3 * tlist[offset_count * idx + vert_offset]],
					plist[3 * tlist[offset_count * idx + vert_offset] + 1],
					plist[3 * tlist[offset_count * idx + vert_offset] + 2] };
				temp.normal = {
					nlist[3 * tlist[offset_count * idx + norm_offset]],
					nlist[3 * tlist[offset_count * idx + norm_offset] + 1],
					nlist[3 * tlist[offset_count * idx + norm_offset] + 2] };
				temp.texCoord = {
					mlist[2 * tlist[offset_count * idx + tex_offset]],
					mlist[2 * tlist[offset_count * idx + tex_offset] + 1] };
				v_arr.push_back(temp);
			}
			Mesh mtemp;
			mtemp.add_vertices(v_arr);
			meshes_out.push_back(mtemp);
		}
		return meshes_out;
	}
}