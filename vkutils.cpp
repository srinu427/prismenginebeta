#include "vkutils.h"

#include <set>
#include <fstream>
#include <iostream>
#include <algorithm>

bool checkValidationLayerSupport(const std::vector<const char*> vlayer_list) {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, NULL);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const VkLayerProperties vlp : availableLayers) {
		std::cout << vlp.layerName << std::endl;
	}

	for (const char* layerName : vlayer_list) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL GPUDebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != NULL) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

VkDebugUtilsMessengerCreateInfoEXT populateDebugMessengerCreateInfo() {
	VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = GPUDebugCallback;

	return createInfo;
}

VkDebugUtilsMessengerEXT setupDebugMessenger(VkInstance instance) {
	VkDebugUtilsMessengerCreateInfoEXT createInfo = populateDebugMessengerCreateInfo();

	VkDebugUtilsMessengerEXT debugMessenger;

	if (CreateDebugUtilsMessengerEXT(instance, &createInfo, NULL, &debugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug messenger!");
	}

	return debugMessenger;
}

void vkutils::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != NULL) {
		func(instance, debugMessenger, pAllocator);
	}
}

bool checkExtensionSupport(std::vector<const char*> ext_list)
{
	//Get Supported extensions
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensions.data());

	for (uint32_t i = 0; i < ext_list.size(); i++) {
		bool found = false;
		for (const auto& ext : extensions) {
			if (!strcmp(ext_list[i], ext.extensionName)) {
				found = true;
				break;
			}
		}
		if (!found) return false;
	}

	return true;
}

std::vector<const char*> getRequiredExtensions(bool enableValidationLayers) {
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

VkInstance vkutils::createVKInstance(
	VkApplicationInfo appInfo,
	bool enableValidationLayers,
	std::vector<const char*> validationLayers)
{
	//Check Validation Layer support (only used if debug build)
	if (enableValidationLayers && !checkValidationLayerSupport(validationLayers)) {
		throw std::runtime_error("validation layers requested, but not available!");
	}

	//Query extensions needed for glfw
	std::vector<const char*> extensions = getRequiredExtensions(enableValidationLayers);

	//Verify if extentions are supported
	if (!checkExtensionSupport(extensions)) {
		throw std::runtime_error("Extentions required by glfw not supported");
	}

	//Struct to create instance
	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = extensions.size();
	createInfo.ppEnabledExtensionNames = extensions.data();

	if (enableValidationLayers) {
		createInfo.enabledLayerCount = validationLayers.size();
		createInfo.ppEnabledLayerNames = validationLayers.data();
		VkDebugUtilsMessengerCreateInfoEXT dmcreateinfo = populateDebugMessengerCreateInfo();
		createInfo.pNext = &dmcreateinfo;
	}
	else {
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = NULL;
	}

	//Create instance
	VkInstance instance;
	VkResult result = vkCreateInstance(&createInfo, NULL, &instance);

	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to create instance!");
	}

	return instance;
}

VkDebugUtilsMessengerEXT vkutils::setupDebugMessenger(VkInstance instance)
{
	VkDebugUtilsMessengerEXT debugMessenger;

	//Struct to create debug messenger
	VkDebugUtilsMessengerCreateInfoEXT createInfo = populateDebugMessengerCreateInfo();

	if (CreateDebugUtilsMessengerEXT(instance, &createInfo, NULL, &debugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug messenger!");
	}
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device, std::vector<const char*> requiredExtensions) {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, NULL);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, availableExtensions.data());

	std::set<std::string> leftExtensions(requiredExtensions.begin(), requiredExtensions.end());

	for (const auto& extension : availableExtensions) {
		leftExtensions.erase(extension.extensionName);
	}
	return leftExtensions.empty();
}

QueueFamilyIndices vkutils::findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
	QueueFamilyIndices indices;
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) indices.graphicsFamily = i;
		else if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) indices.transferFamily = i;
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
		if (presentSupport) indices.presentFamily = i;
		if (indices.isComplete()) break;
		i++;
	}
	return indices;
}

SwapChainSupportDetails vkutils::querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, NULL);

	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, NULL);

	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

bool isDeviceSuitable(
	VkPhysicalDevice device,
	VkSurfaceKHR surface,
	std::vector<const char*> requiredExtensions,
	bool allow_integrated
)
{
	VkPhysicalDeviceProperties supportedProperties;
	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceProperties(device, &supportedProperties);
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

	QueueFamilyIndices indices = vkutils::findQueueFamilies(device, surface);

	bool extensionsSupported = checkDeviceExtensionSupport(device, requiredExtensions);

	bool swapChainAdequate = false;
	if (extensionsSupported) {
		SwapChainSupportDetails swapChainSupport = vkutils::querySwapChainSupport(device, surface);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	return (allow_integrated || supportedProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) &&
		supportedFeatures.geometryShader && indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

VkPhysicalDevice vkutils::pickPhysicalDevice(
	VkInstance instance,
	VkSurfaceKHR surface,
	std::vector<const char*> requiredExtensions,
	bool allowIntegrated
) {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	bool integrated_status = false;
	if (deviceCount == 1) integrated_status = true;

	VkPhysicalDevice physicalDevice{};
	for (const auto& device : devices) {
		if (isDeviceSuitable(device, surface, requiredExtensions, integrated_status)) {
			physicalDevice = device;
			break;
		}
	}
	if (physicalDevice == VK_NULL_HANDLE) throw std::runtime_error("failed to find a suitable GPU!");
	return physicalDevice;
}

VkSurfaceFormatKHR vkutils::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}
	return availableFormats[0];
}

VkPresentModeKHR vkutils::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D vkutils::chooseSwapExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities) {
	if (capabilities.currentExtent.width != UINT32_MAX) {
		return capabilities.currentExtent;
	}
	else {
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

GPUImage vkutils::createGPUImage(VkDevice device, VkImage image, VkImageViewCreateInfo imageViewInfo)
{
	GPUImage timg;
	timg._image = image;
	timg._imageViewInfo = imageViewInfo;
	if (vkCreateImageView(device, &timg._imageViewInfo, nullptr, &timg._imageView) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image view!");
	}
	return timg;
}

GPUImage vkutils::createGPUImage(VkDevice device, VkImage image, VkImageViewType viewType, VkFormat format, VkImageAspectFlags aspectFlag)
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = viewType;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlag;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	if (viewType == VK_IMAGE_VIEW_TYPE_CUBE) viewInfo.subresourceRange.layerCount = 6;
	if (format == VK_FORMAT_R32_SFLOAT) viewInfo.components = { VK_COMPONENT_SWIZZLE_R };

	return createGPUImage(device, image, viewInfo);
}

GPUImage vkutils::createGPUImage(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usageFlag, VkMemoryPropertyFlags memFlag, VkImageViewType viewType, VkImageAspectFlags aspectFlag)
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usageFlag;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkImage tImg;
	if (vkCreateImage(device, &imageInfo, NULL, &tImg) != VK_SUCCESS) throw std::runtime_error("failed to create image!");

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device, tImg, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, memFlag);

	VkDeviceMemory tImgMem;
	if (vkAllocateMemory(device, &allocInfo, NULL, &tImgMem) != VK_SUCCESS) throw std::runtime_error("failed to allocate image memory!");

	vkBindImageMemory(device, tImg, tImgMem, 0);

	GPUImage res = createGPUImage(device, tImg, viewType, format, aspectFlag);
	res._imageMemory = tImgMem;
	return res;
}

GPUImage vkutils::createGPUImage(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, uint32_t layerCount, VkImageCreateFlags imageFlag, VkImageUsageFlags usageFlag, VkMemoryPropertyFlags memFlag, VkImageViewType viewType, VkImageAspectFlags aspectFlag)
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = layerCount;
	imageInfo.flags = imageFlag;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usageFlag;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkImage tImg;
	if (vkCreateImage(device, &imageInfo, NULL, &tImg) != VK_SUCCESS) throw std::runtime_error("failed to create image!");

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device, tImg, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, memFlag);

	VkDeviceMemory tImgMem;
	if (vkAllocateMemory(device, &allocInfo, NULL, &tImgMem) != VK_SUCCESS) throw std::runtime_error("failed to allocate image memory!");

	vkBindImageMemory(device, tImg, tImgMem, 0);

	GPUImage res = createGPUImage(device, tImg, viewType, format, aspectFlag);
	res._imageMemory = tImgMem;
	return res;
}

GPUImage vkutils::createGPUImage(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usageFlag, VkMemoryPropertyFlags memFlag, VkImageViewCreateInfo imageViewInfo)
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usageFlag;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkImage tImg;
	if (vkCreateImage(device, &imageInfo, NULL, &tImg) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image!");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device, tImg, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, memFlag);

	VkDeviceMemory tImgMem;
	if (vkAllocateMemory(device, &allocInfo, NULL, &tImgMem) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate image memory!");
	}

	vkBindImageMemory(device, tImg, tImgMem, 0);

	GPUImage res = createGPUImage(device, tImg, imageViewInfo);
	res._imageMemory = tImgMem;
}

GPUImage vkutils::createGPUImage(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, uint32_t layerCount, VkImageCreateFlags imageFlag, VkImageUsageFlags usageFlag, VkMemoryPropertyFlags memFlag, VkImageViewCreateInfo imageViewInfo)
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = layerCount;
	imageInfo.flags = imageFlag;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usageFlag;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkImage tImg;
	if (vkCreateImage(device, &imageInfo, NULL, &tImg) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image!");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device, tImg, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, memFlag);

	VkDeviceMemory tImgMem;
	if (vkAllocateMemory(device, &allocInfo, NULL, &tImgMem) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate image memory!");
	}

	vkBindImageMemory(device, tImg, tImgMem, 0);

	GPUImage res = createGPUImage(device, tImg, imageViewInfo);
	res._imageMemory = tImgMem;
}

void vkutils::destroyGPUImage(VkDevice device, GPUImage image, bool memory_already_freed)
{
	vkDestroyImageView(device, image._imageView, NULL);
	vkDestroyImage(device, image._image, NULL);
	if (!memory_already_freed) vkFreeMemory(device, image._imageMemory, NULL);
}

VkCommandBuffer vkutils::createCmdBuffer(VkDevice device, VkCommandPool cmdPool) {
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = cmdPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer cmdBuffer;
	if (vkAllocateCommandBuffers(device, &allocInfo, &cmdBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}
	return cmdBuffer;
}

VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool cmdPool) {

	VkCommandBuffer commandBuffer = vkutils::createCmdBuffer(device, cmdPool);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	return commandBuffer;
}

void endSingleTimeCommands(VkDevice device, VkCommandPool cmdPool, VkCommandBuffer commandBuffer, VkQueue queue) {
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(queue);

	vkFreeCommandBuffers(device, cmdPool, 1, &commandBuffer);
}

void vkutils::transitionImageLayout(VkCommandBuffer cmdBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImageSubresourceRange subresourceRange, uint32_t srcQFI, uint32_t dstQFI)
{
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = srcQFI;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;

	barrier.subresourceRange = subresourceRange;

	switch (oldLayout)
	{
	case VK_IMAGE_LAYOUT_UNDEFINED:
		barrier.srcAccessMask = 0;
		break;

	case VK_IMAGE_LAYOUT_PREINITIALIZED:
		barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	default:
		throw std::invalid_argument("unsupported layout transition!");
		break;
	}

	switch (newLayout)
	{
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		barrier.dstAccessMask = barrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		if (hasStencilComponent(format)) {
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
		break;

	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		if (barrier.srcAccessMask == 0)
		{
			barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		}
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	default:
		throw std::invalid_argument("unsupported layout transition!");
		break;
	}

	vkCmdPipelineBarrier(
		cmdBuffer,
		srcStageMask, dstStageMask,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

}

void vkutils::transitionImageLayout(VkDevice device, VkCommandPool cmdPool, VkQueue queue, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImageSubresourceRange subresourceRange, uint32_t srcQFI, uint32_t dstQFI)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, cmdPool);

	transitionImageLayout(commandBuffer, image, format, oldLayout, newLayout, srcStageMask, dstStageMask, subresourceRange);

	endSingleTimeCommands(device, cmdPool, commandBuffer, queue);
}

VkFramebuffer vkutils::createFrameBuffer(VkDevice device, VkRenderPass renderPass, std::vector<GPUImage> attachments, uint32_t width, uint32_t height, uint32_t layers)
{
	std::vector<VkImageView> attachment_iviews;
	for (GPUImage gi : attachments) attachment_iviews.push_back(gi._imageView);

	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = renderPass;
	framebufferInfo.attachmentCount = attachments.size();
	framebufferInfo.pAttachments = attachment_iviews.data();
	framebufferInfo.width = width;
	framebufferInfo.height = height;
	framebufferInfo.layers = layers;

	VkFramebuffer fBuffer;
	if (vkCreateFramebuffer(device, &framebufferInfo, NULL, &fBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create framebuffer!");
	}
	return fBuffer;
}

void vkutils::beginRenderPass(VkRenderPass rPass, VkFramebuffer fBuffer, VkExtent2D rpExtent, VkCommandBuffer cmdBuffer, std::vector<VkClearValue> clearValues) {
	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = rPass;
	renderPassInfo.framebuffer = fBuffer;
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = rpExtent;
	renderPassInfo.clearValueCount = clearValues.size();
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

bool vkutils::hasStencilComponent(VkFormat format) {
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

VkFormat findSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
	for (VkFormat format : candidates) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);
		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			return format;
		}

	}
	throw std::runtime_error("failed to find supported format!");
}

VkFormat vkutils::findDepthFormat(VkPhysicalDevice physicalDevice) {
	return findSupportedFormat(
		physicalDevice,
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

VkDescriptorSetLayout vkutils::createDescriptorSetLayout(
	VkDevice device,
	uint32_t binding,
	VkDescriptorType dType,
	uint32_t dCount,
	VkShaderStageFlags stageFlag,
	VkSampler* pImmutableSamplers
) {
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = binding;
	uboLayoutBinding.descriptorType = dType;
	uboLayoutBinding.descriptorCount = dCount;
	uboLayoutBinding.stageFlags = stageFlag;
	uboLayoutBinding.pImmutableSamplers = pImmutableSamplers;

	VkDescriptorSetLayoutCreateInfo ubolayoutInfo{};
	ubolayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	ubolayoutInfo.bindingCount = 1;
	ubolayoutInfo.pBindings = &uboLayoutBinding;

	VkDescriptorSetLayout dSetLayout;
	if (vkCreateDescriptorSetLayout(device, &ubolayoutInfo, NULL, &dSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}
	return dSetLayout;
}

GPUBuffer vkutils::createBuffer(
	VkDevice device,
	VkPhysicalDevice physicalDevice,
	VkDeviceSize size,
	VkBufferUsageFlags usage,
	VkMemoryPropertyFlags properties
) {
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	GPUBuffer gBuff;
	if (vkCreateBuffer(device, &bufferInfo, NULL, &gBuff._buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, gBuff._buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(device, &allocInfo, NULL, &gBuff._bufferMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	vkBindBufferMemory(device, gBuff._buffer, gBuff._bufferMemory, 0);

	return gBuff;
}

void vkutils::copyBuffer(VkDevice device, VkCommandPool cmdPool, VkQueue queue, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
	VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, cmdPool);

	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0; // Optional
	copyRegion.dstOffset = 0; // Optional
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	endSingleTimeCommands(device, cmdPool, commandBuffer, queue);
}

VkDescriptorSet vkutils::createSingleDSet(
	VkDevice device,
	VkDescriptorPool dPool,
	VkDescriptorSetLayout dSetLayout,
	VkDescriptorType dType,
	GPUBuffer dBuff,
	VkDeviceSize dBuffSize
) {
	VkDescriptorSetAllocateInfo dSetAllocInfo{};
	dSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	dSetAllocInfo.descriptorPool = dPool;
	dSetAllocInfo.descriptorSetCount = 1;
	dSetAllocInfo.pSetLayouts = &dSetLayout;

	VkDescriptorSet dSet;

	if (vkAllocateDescriptorSets(device, &dSetAllocInfo, &dSet) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor set!");
	}

	VkDescriptorBufferInfo dBufferInfo{};
	dBufferInfo.buffer = dBuff._buffer;
	dBufferInfo.offset = 0;
	dBufferInfo.range = dBuffSize;

	VkWriteDescriptorSet dSetWrite{};

	dSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	dSetWrite.dstSet = dSet;
	dSetWrite.dstBinding = 0;
	dSetWrite.dstArrayElement = 0;
	dSetWrite.descriptorType = dType;
	dSetWrite.descriptorCount = 1;
	dSetWrite.pBufferInfo = &dBufferInfo;

	vkUpdateDescriptorSets(device, 1, &dSetWrite, 0, NULL);

	return dSet;
}

VkDescriptorSet vkutils::createImageDSet(VkDevice device, VkDescriptorPool dPool, VkDescriptorSetLayout dSetLayout, std::vector<GPUImage> images, VkImageLayout imageLayout, VkSampler sampler)
{
	VkDescriptorSetAllocateInfo dSetAllocInfo{};
	dSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	dSetAllocInfo.descriptorPool = dPool;
	dSetAllocInfo.descriptorSetCount = 1;
	dSetAllocInfo.pSetLayouts = &dSetLayout;

	VkDescriptorSet dSet;
	VkResult vkr = vkAllocateDescriptorSets(device, &dSetAllocInfo, &dSet);
	if (vkr != VK_SUCCESS) throw std::runtime_error("failed to allocate descriptor set!");

	std::vector<VkDescriptorImageInfo> imageInfos;
	for (int i = 0; i < images.size(); i++) {
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = imageLayout;
		imageInfo.imageView = images[i]._imageView;
		imageInfo.sampler = sampler;
		imageInfos.push_back(imageInfo);
	}

	VkWriteDescriptorSet dSetWrite{};
	dSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	dSetWrite.dstSet = dSet;
	dSetWrite.dstBinding = 0;
	dSetWrite.dstArrayElement = 0;
	dSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	dSetWrite.descriptorCount = imageInfos.size();
	dSetWrite.pImageInfo = imageInfos.data();

	vkUpdateDescriptorSets(device, 1, &dSetWrite, 0, NULL);
	return dSet;
}

GPUBuffer vkutils::createBuffer(
	VkDevice device,
	VkPhysicalDevice physicalDevice,
	VkDeviceSize buffSize,
	void* data,
	VkBufferUsageFlags usage,
	VkMemoryPropertyFlags properties,
	VkCommandPool cmdPool,
	VkQueue queue
) {
	GPUBuffer stageBuffer = createBuffer(
		device,
		physicalDevice,
		buffSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);

	void* temp;
	vkMapMemory(device, stageBuffer._bufferMemory, 0, buffSize, 0, &temp);
	memcpy(temp, data, (size_t)buffSize);
	vkUnmapMemory(device, stageBuffer._bufferMemory);

	GPUBuffer resBuffer;
	resBuffer = createBuffer(
		device,
		physicalDevice,
		buffSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage,
		properties
	);

	copyBuffer(device, cmdPool, queue, stageBuffer._buffer, resBuffer._buffer, buffSize);
	destroyBuffer(device, stageBuffer);
	return resBuffer;
}

void vkutils::writeToBuffer(VkDevice device, VkPhysicalDevice physicalDevice, GPUBuffer buffer, VkDeviceSize buffSize, void* data, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkCommandPool cmdPool, VkQueue queue)
{
	GPUBuffer stageBuffer = createBuffer(
		device,
		physicalDevice,
		buffSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);

	void* temp;
	vkMapMemory(device, stageBuffer._bufferMemory, 0, buffSize, 0, &temp);
	memcpy(temp, data, (size_t)buffSize);
	vkUnmapMemory(device, stageBuffer._bufferMemory);

	copyBuffer(device, cmdPool, queue, stageBuffer._buffer, buffer._buffer, buffSize);
	destroyBuffer(device, stageBuffer);
}

void vkutils::destroyBuffer(VkDevice device, GPUBuffer buffer)
{
	vkDestroyBuffer(device, buffer._buffer, NULL);
	vkFreeMemory(device, buffer._bufferMemory, NULL);
}

void vkutils::copyDataToImage(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool cmdPool, VkQueue queue, VkDeviceSize dataSize, void* data, GPUImage image, VkOffset3D imgOffset, VkExtent3D imgExtent)
{
	GPUBuffer stageBuffer = createBuffer(
		device,
		physicalDevice,
		dataSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);

	void* temp;
	vkMapMemory(device, stageBuffer._bufferMemory, 0, dataSize, 0, &temp);
	memcpy(temp, data, (size_t)dataSize);
	vkUnmapMemory(device, stageBuffer._bufferMemory);

	VkCommandBuffer cmdBuffer = beginSingleTimeCommands(device, cmdPool);
	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = imgOffset;
	region.imageExtent = imgExtent;
	vkCmdCopyBufferToImage(
		cmdBuffer,
		stageBuffer._buffer,
		image._image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);
	endSingleTimeCommands(device, cmdPool, cmdBuffer, queue);

	destroyBuffer(device, stageBuffer);
}

GPUSetBuffer vkutils::createSetBuffer(
	VkDevice device,
	VkPhysicalDevice physicalDevice,
	VkDeviceSize buffSize,
	VkBufferUsageFlags buffUsage,
	VkMemoryPropertyFlags buffMemProps,
	VkDescriptorPool dPool,
	VkDescriptorSetLayout dSetLayout,
	VkDescriptorType dType
) {
	GPUSetBuffer sBuffer;
	sBuffer._gBuffer = createBuffer(device, physicalDevice, buffSize, buffUsage, buffMemProps);
	sBuffer._dSet = createSingleDSet(device, dPool, dSetLayout, dType, sBuffer._gBuffer, buffSize);
	return sBuffer;
}

void vkutils::destroySetBuffer(
	VkDevice device,
	VkDescriptorPool dPool,
	GPUSetBuffer sBuffer
){
	vkFreeDescriptorSets(device, dPool, 1, &sBuffer._dSet);
	destroyBuffer(device, sBuffer._gBuffer);
}

VkShaderModule vkutils::createShader(VkDevice device, std::string shaderFilePath)
{
	std::ifstream file(shaderFilePath, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();
	char* buffer = (char *)malloc(sizeof(char) * fileSize);

	file.seekg(0);
	file.read(buffer, fileSize);
	file.close();

	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = fileSize;
	createInfo.pCode = reinterpret_cast<const uint32_t*>(buffer);

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, NULL, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}