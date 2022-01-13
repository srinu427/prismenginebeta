#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "vkstructs.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>

void Mesh::add_vertices(std::vector<Vertex> verts)
{
	std::unordered_map<Vertex, uint32_t> uniqueVertices{};
	for (int i = 0; i < verts.size(); i++) {
		Vertex v = verts[i];
		if (uniqueVertices.count(v) == 0) {
			uniqueVertices[v] = static_cast<uint32_t>(_vertices.size());
			_vertices.push_back(v);
		}
		_indices.push_back(uniqueVertices[v]);
	}
}

bool Mesh::load_from_obj(const char* filename)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename)) {
		std::cout << (warn + err) << std::endl;
		return false;
	}

	std::unordered_map<Vertex, uint32_t> uniqueVertices{};

	for (const auto& shape : shapes) {
		for (const auto& index : shape.mesh.indices) {
			Vertex vertex{};

			vertex.pos = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			vertex.normal = {
				attrib.normals[3 * index.normal_index + 0],
				attrib.normals[3 * index.normal_index + 1],
				attrib.normals[3 * index.normal_index + 2]
			};

			vertex.texCoord = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};

			vertex.color = { 1.0f, 1.0f, 1.0f };

			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = static_cast<uint32_t>(_vertices.size());
				_vertices.push_back(vertex);
			}

			_indices.push_back(uniqueVertices[vertex]);
		}

	}
	return true;
}

void GPUPipeline::bindPipeline(VkCommandBuffer cmdBuffer)
{
	vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);
}

void GPUPipeline::bindPipelineDSets(VkCommandBuffer cmdBuffer, std::vector<VkDescriptorSet> dSets, std::vector<GPUPushConstant> pushConstants)
{
	vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, dSets.size(), dSets.data(), 0, NULL);
	for (int i = 0; i < pushConstants.size(); i++) vkCmdPushConstants(cmdBuffer, _pipelineLayout, pushConstants[i].PCRange.stageFlags, pushConstants[i].PCRange.offset, pushConstants[i].PCRange.size, pushConstants[i].PCData);
}

void RenderObject::drawMesh(VkCommandBuffer cmdBuffer, int obj_idx)
{
	vkCmdDrawIndexed(cmdBuffer, static_cast<uint32_t>(mesh->_indices.size()), 1, 0, 0, obj_idx);
}
