#pragma once

#include "vkutils.h"

#include <mutex>
#include <vector>

class PrismRenderer
{
public:
	bool framebufferResized = false;
	VkExtent2D swapChainExtent = { 1280, 720 };
	VkExtent2D plight_smap_extent = { 1024, 1024 };
	VkExtent2D dlight_smap_extent = { 1024, 1024 };

	GPUSceneData currentScene;
	GPUCameraData currentCamera;
	std::vector<RenderObject> renderObjects;
	std::vector<GPULight> pointLights;
	std::vector<GPULight> directionalLights;

	VkRenderPass finalRenderPass;
	VkRenderPass shadowRenderPass;
	std::mutex spawn_mut;

	PrismRenderer(GLFWwindow* glfwWindow, void(*nextFrameCallback)(float framedeltat, PrismRenderer* renderer));
	void (*uboUpdateCallback) (float framedeltat, PrismRenderer* renderer);
	void run();
	void addRenderObj(
		std::string id,
		std::string meshFilePath,
		std::string texFilePath,
		std::string texSamplerType,
		glm::mat4 initTransform,
		bool include_in_final_render = true,
		bool include_in_shadow_map = true
	);
	void addRenderObj(
		std::string id,
		Mesh meshData,
		std::string texFilePath,
		std::string texSamplerType,
		glm::mat4 initTransform,
		bool include_in_final_render = true,
		bool include_in_shadow_map = true
	);
	void removeRenderObj(std::string id);
private:
#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif
	std::vector<const char*> validationLayers;
	std::vector<const char*> deviceExtensions;

	GLFWwindow* window;
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	QueueFamilyIndices queueFamilyIndices;
	VkQueue graphicsQueue, presentQueue, transferQueue;
	VkSurfaceKHR surface;

	unsigned int MAX_FRAMES_IN_FLIGHT = 3;

	VkSwapchainKHR swapChain;
	VkFormat swapChainImageFormat;

	const unsigned int MAX_OBJECTS = 1000;
	const unsigned int MAX_POINT_LIGHTS = 4;
	const unsigned int MAX_DIRECTIONAL_LIGHTS = 10;

	VkDescriptorPool descriptorPool;
	std::unordered_map<std::string, VkDescriptorSetLayout> dSetLayouts;
	std::unordered_map<std::string, GPUPipeline> pipelines;
	std::unordered_map<std::string, Mesh> meshes;
	std::unordered_map<std::string, GPUImage> textures;
	std::unordered_map<std::string, VkSampler> texSamplers;

	std::vector<GPUBuffer> uniformBuffers;

	std::vector<GPUFrameData> frameDatas;
	size_t currentFrame = 0;
	bool time_refresh = true;
	std::chrono::steady_clock::time_point startTime;
	std::chrono::steady_clock::time_point lastFrameTime;

	VkFormat depthFormat;
	GPUImage depthImage;

	VkCommandPool uploadCmdPool;
	std::vector<VkFence> imagesInFlight;

	void getVkInstance();
	void createSurface();
	void getVkLogicalDevice();
	void createSwapChain(SwapChainSupportDetails swapChainSupport);
	void makeBasicCmdPools();
	void makePLightMaps();
	void createDepthImage();
	void createShadowRenderPass();
	void createFinalRenderPass();
	void createDescriptorPool();
	void addDsetLayout(std::string name, uint32_t binding, VkDescriptorType dType, uint32_t dCount, VkShaderStageFlags stageFlag);
	void makeBasicDSetLayouts();
	void makeBasicDSets();
	void addSimplePipeline(
		std::string name,
		VkRenderPass rPass,
		std::unordered_map<VkShaderStageFlagBits, std::string> stage_shader_map,
		std::vector<std::string> reqDSetLayouts,
		VkOffset2D scissorOffset,
		VkExtent2D scissorExtent,
		float VPWidth, float VPHeight,
		bool invert_VP_Y = true,
		std::vector<VkPushConstantRange> pushConstantRanges = {}
	);
	void makeFinalPipeline();
	void makeShadowPipeline();
	void createFinalFrameBuffers();
	void createShadowFrameBuffers();
	void addPLightCmds(VkCommandBuffer cmdBuffer, int frameNo);
	void addFinalMeshCmds(VkCommandBuffer cmdBuffer, int frameNo);
	void createFinalCmdBuffers();
	void refreshFinalCmdBuffers();
	void createSyncObjects();

	void initVulkan();
	void recreateSwapChain();
	void updateUBOs(uint32_t curr_img);
	void drawFrame();
	void mainLoop();

	void createBasicSamplers();
	Mesh* addMesh(std::string meshFilePath);
	GPUImage* loadTexture(std::string texturePath);

	void cleanupSwapChain(bool destroy_only_swapchain);
	void cleanup();
};

