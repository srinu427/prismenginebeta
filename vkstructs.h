#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <array>
#include <optional>

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;
	std::optional<uint32_t> transferFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value() && transferFamily.has_value();
	}
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

struct MeshPushConstants {
	int tidx;
	glm::vec4 data;
	glm::mat4 render_matrix;
};

struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

struct Vertex {
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec3 color;
	glm::vec2 texCoord;

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, normal);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, color);

		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[3].offset = offsetof(Vertex, texCoord);

		return attributeDescriptions;
	}

	bool operator==(const Vertex& other) const {
		return pos == other.pos && color == other.color && texCoord == other.texCoord;
	}
};

namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^
				(hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}

struct GPUPushConstant {
	VkPushConstantRange PCRange;
	void* PCData;
};

struct GPUPipeline {
	VkPipeline _pipeline;
	VkPipelineLayout _pipelineLayout;

	void bindPipeline(VkCommandBuffer cmdBuffer);
	void bindPipelineDSets(VkCommandBuffer cmdBuffer, std::vector<VkDescriptorSet> dSets, std::vector<GPUPushConstant> pushConstants);
};

struct RunnableGPUPipeline {
	GPUPipeline gPipeline;
	std::vector<VkDescriptorSet> dSets;
	std::vector<GPUPushConstant> pushConstants;
};

struct GPUBuffer {
	VkBuffer _buffer;
	VkDeviceMemory _bufferMemory;
};

struct GPUSetBuffer {
	GPUBuffer _gBuffer;
	VkDescriptorSet _dSet;
};

struct Mesh {
	std::vector<Vertex> _vertices;
	std::vector<uint32_t> _indices;

	GPUBuffer _vertexBuffer;
	GPUBuffer _indexBuffer;
	VkSampler _textureSampler;

	void add_vertices(std::vector<Vertex> verts);
	bool load_from_obj(const char* filename);
};

struct GPUImage {
	VkImage _image;
	VkDeviceMemory _imageMemory;
	VkImageViewCreateInfo _imageViewInfo;
	VkImageView _imageView;
};

struct GPUTexture2d {
	GPUImage _gImage;
	VkDescriptorSet _dSet;
};

struct GPUCameraData {
	glm::vec4 camPos;
	glm::vec4 camDir;
	glm::mat4 viewproj;
};

struct GPUObjectData {
	glm::mat4 model = glm::mat4{ 1.0f };
};

struct GPUSceneData {
	glm::vec4 fogColor; // w is for exponent
	glm::vec4 fogDistances; //x for min, y for max, zw unused.
	glm::vec4 ambientColor;
	glm::vec4 sunlightPosition;
	glm::vec4 sunlightDirection; //w for sun power
	glm::vec4 sunlightColor;
};

struct GPULight{
	glm::vec4 pos = glm::vec4(0);
	glm::vec4 color = glm::vec4(0);
	glm::vec4 dir = glm::vec4(0);
	glm::mat4 viewproj = glm::mat4(1);
};

struct GPULightPC {
	glm::ivec4 idx;
	glm::mat4 viewproj;
};

class RenderObject {
public:
	std::string id;
	Mesh* mesh;
	GPUTexture2d* texture;
	GPUObjectData uboData;
	bool renderable = true;
	bool shadowcasting = true;

	void drawMesh(VkCommandBuffer cmdBuffer, int obj_idx = 0);
};

struct GPUFrameData {
	std::unordered_map<std::string, GPUSetBuffer> setBuffers;

	GPUImage shadowMapTemp;
	GPUImage shadowDepthImage;
	VkFramebuffer shadowFrameBuffer;

	VkCommandPool commandPool;
	VkCommandBuffer commandBuffer;
	std::vector<GPUImage> shadow_cube_maps;
	VkDescriptorSet shadow_cube_dset;
	std::vector<GPUImage> shadow_dir_maps;
	VkDescriptorSet shadow_dir_dset;

	GPUImage swapChainImage;
	VkFramebuffer swapChainFrameBuffer;

	VkSemaphore presentSemaphore, renderSemaphore;
	VkFence renderFence;
};