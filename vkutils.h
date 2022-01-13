#pragma once

#include "vkstructs.h"

#include <vector>
#include <array>

namespace vkutils {

	VkInstance createVKInstance(
		VkApplicationInfo appInfo,
		bool enableValidationLayers,
		std::vector<const char*> validationLayers
	);

	VkDebugUtilsMessengerEXT setupDebugMessenger(VkInstance instance);
	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
	
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

	VkPhysicalDevice pickPhysicalDevice(
		VkInstance instance,
		VkSurfaceKHR surface,
		std::vector<const char*> requiredExtensions,
		bool allowIntegrated
	);

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

	VkExtent2D chooseSwapExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities);

	GPUImage createGPUImage(VkDevice device, VkImage image, VkImageViewCreateInfo imageViewInfo);
	GPUImage createGPUImage(
		VkDevice device,
		VkImage image,
		VkImageViewType viewType,
		VkFormat format,
		VkImageAspectFlags aspectFlag
	);
	GPUImage createGPUImage(
		VkDevice device,
		VkPhysicalDevice physicalDevice,
		uint32_t width, uint32_t height,
		VkFormat format,
		VkImageTiling tiling,
		uint32_t layerCount,
		VkImageCreateFlags imageFlag,
		VkImageUsageFlags usageFlag,
		VkMemoryPropertyFlags memFlag,
		VkImageViewType viewType,
		VkImageAspectFlags aspectFlag
	);
	GPUImage createGPUImage(
		VkDevice device,
		VkPhysicalDevice physicalDevice,
		uint32_t width, uint32_t height,
		VkFormat format,
		VkImageTiling tiling,
		VkImageUsageFlags usageFlag,
		VkMemoryPropertyFlags memFlag,
		VkImageViewType viewType,
		VkImageAspectFlags aspectFlag
	);
	GPUImage createGPUImage(
		VkDevice device,
		VkPhysicalDevice physicalDevice,
		uint32_t width, uint32_t height,
		VkFormat format,
		VkImageTiling tiling,
		uint32_t layerCount,
		VkImageCreateFlags imageFlag,
		VkImageUsageFlags usageFlag,
		VkMemoryPropertyFlags memFlag,
		VkImageViewCreateInfo imageViewInfo
	);
	GPUImage createGPUImage(
		VkDevice device,
		VkPhysicalDevice physicalDevice,
		uint32_t width, uint32_t height,
		VkFormat format,
		VkImageTiling tiling,
		VkImageUsageFlags usageFlag,
		VkMemoryPropertyFlags memFlag,
		VkImageViewCreateInfo imageViewInfo
	);

	void destroyGPUImage(VkDevice device, GPUImage image, bool memory_already_freed = false);

	VkCommandBuffer createCmdBuffer(VkDevice device, VkCommandPool cmdPool);

	void transitionImageLayout(
		VkDevice device,
		VkCommandPool cmdPool,
		VkQueue queue,
		VkImage image,
		VkFormat format,
		VkImageLayout oldLayout,
		VkImageLayout newLayout,
		VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VkImageSubresourceRange subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
		uint32_t srcQFI = VK_QUEUE_FAMILY_IGNORED,
		uint32_t dstQFI = VK_QUEUE_FAMILY_IGNORED
	);

	void transitionImageLayout(
		VkCommandBuffer cmdBuffer,
		VkImage image,
		VkFormat format,
		VkImageLayout oldLayout,
		VkImageLayout newLayout,
		VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VkImageSubresourceRange subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
		uint32_t srcQFI = VK_QUEUE_FAMILY_IGNORED,
		uint32_t dstQFI = VK_QUEUE_FAMILY_IGNORED
	);

	VkFramebuffer createFrameBuffer(
		VkDevice device,
		VkRenderPass renderPass,
		std::vector<GPUImage> attachments,
		uint32_t width,
		uint32_t height,
		uint32_t layers=1
	);

	void beginRenderPass(
		VkRenderPass rPass,
		VkFramebuffer fBuffer,
		VkExtent2D rpExtent,
		VkCommandBuffer cmdBuffer,
		std::vector<VkClearValue> clearValues
	);

	bool hasStencilComponent(VkFormat format);

	VkFormat findDepthFormat(VkPhysicalDevice physicalDevice);

	VkDescriptorSetLayout createDescriptorSetLayout(
		VkDevice device,
		uint32_t binding,
		VkDescriptorType dType,
		uint32_t dCount,
		VkShaderStageFlags stageFlag,
		VkSampler* pImmutableSamplers=NULL
	);

	GPUBuffer createBuffer(
		VkDevice device,
		VkPhysicalDevice physicalDevice,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties
	);
	void copyBuffer(
		VkDevice device,
		VkCommandPool cmdPool,
		VkQueue queue,
		VkBuffer srcBuffer, VkBuffer dstBuffer,
		VkDeviceSize size
	);
	GPUBuffer createBuffer(
		VkDevice device,
		VkPhysicalDevice physicalDevice,
		VkDeviceSize buffSize,
		void* data,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkCommandPool cmdPool,
		VkQueue queue
	);
	void writeToBuffer(
		VkDevice device,
		VkPhysicalDevice physicalDevice,
		GPUBuffer buffer,
		VkDeviceSize buffSize,
		void* data,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkCommandPool cmdPool,
		VkQueue queue
	);
	void destroyBuffer(VkDevice device, GPUBuffer buffer);

	void copyDataToImage(
		VkDevice device,
		VkPhysicalDevice physicalDevice,
		VkCommandPool cmdPool,
		VkQueue queue,
		VkDeviceSize dataSize,
		void* data,
		GPUImage image,
		VkOffset3D imgOffset,
		VkExtent3D imgExtent
	);

	VkDescriptorSet createSingleDSet(
		VkDevice device,
		VkDescriptorPool dPool,
		VkDescriptorSetLayout dSetLayout,
		VkDescriptorType dType,
		GPUBuffer dBuff,
		VkDeviceSize dBuffSize
	);
	VkDescriptorSet createImageDSet(
		VkDevice device,
		VkDescriptorPool dPool,
		VkDescriptorSetLayout dSetLayout,
		std::vector<GPUImage> images,
		VkImageLayout imageLayout,
		VkSampler sampler
	);

	GPUSetBuffer createSetBuffer(
		VkDevice device,
		VkPhysicalDevice physicalDevice,
		VkDeviceSize buffSize,
		VkBufferUsageFlags buffUsage,
		VkMemoryPropertyFlags buffMemProps,
		VkDescriptorPool dPool,
		VkDescriptorSetLayout dSetLayout,
		VkDescriptorType dType
	);
	void destroySetBuffer(VkDevice device, VkDescriptorPool dPool, GPUSetBuffer sBuffer);

	VkShaderModule createShader(VkDevice device, std::string shaderFilePath);
}