#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vector>
#include "CAVertexBuffer.h"
#include "CAIndexBuffer.h"
#include "CAUniformBuffer.h"

class CAModel;

class CAVulkanState
{
public:
	CAVulkanState(GLFWwindow* window);
	~CAVulkanState();
	void windowResized(int width, int height);
	void draw();
	void setModel(CAModel* model);

	// Métodos de gestión de buffers
	void createVertexBuffer(size_t size, const void* data, CAVertexBuffer* vbo);
	void destroyVertexBuffer(CAVertexBuffer vbo);
	void createIndexBuffer(size_t size, const void* data, CAIndexBuffer* ibo);
	void destroyIndexBuffer(CAIndexBuffer ibo);
	void createUniformBuffer(size_t size, CAUniformBuffer* ubo);
	void updateUniformBuffer(size_t size, const void* data, CAUniformBuffer ubo);
	void destroyUniformBuffer(CAUniformBuffer ubo);
	void createDescriptorSets(VkDescriptorPool* descriptorPool, std::vector<VkDescriptorSet> *descriptorSets, CAUniformBuffer** buffers,size_t* bufferSizes, size_t size);
	void destroyDescriptorSets(VkDescriptorPool descriptorPool, std::vector<VkDescriptorSet> descriptorSets);
	VkPipelineLayout getPipelineLayout();

private:
	// Campos internos
	int wWidth;
	int wHeight;
	uint32_t imageCount;
	uint32_t frameCount;

	CAModel* model;
	GLFWwindow* window;
	VkSurfaceKHR surface;
	VkInstance instance;
	VkPhysicalDevice physicalDevice;
	VkPhysicalDeviceMemoryProperties memProperties;
	VkDevice device;
	uint32_t graphicsQueueFamilyIndex;
	uint32_t presentQueueFamilyIndex;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImageView> swapChainImageViews;
	VkRenderPass renderPass;
	VkDescriptorSetLayout descriptorSetLayout;
	VkPipeline graphicsPipeline;
	VkPipelineLayout pipelineLayout;
	std::vector<VkImage> depthImages;
	std::vector<VkDeviceMemory> depthImageMemories;
	std::vector<VkImageView> depthImageViews;
	std::vector<VkFramebuffer> swapChainFramebuffers;
	VkCommandPool commandPool;
	std::vector<VkCommandBuffer> commandBuffers;
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	std::vector<VkFence> imagesInFlight;
	size_t currentFrame = 0;
	uint32_t currentImage = 0;
	bool framebufferResized = false;

	// Métodos de inicialización de Vulkan
	void createInstance();
	void createSurface(GLFWwindow* window);
	void pickPhysicalDevice();
	void createLogicalDevice();
	void createSwapChain();
	void createImageViews();
	void createRenderPass();
	void createPipelineLayout();
	void createGraphicsPipeline();
	void createDepthBuffers();
	void createFramebuffers();
	void createCommandPool();
	void createCommandBuffers();
	void createSyncObjects();
	void recreateSwapChain();
	void fillCommandBuffers();

	// Métodos de definición del pipeline de renderizado
	void createVertexShaderStageCreateInfo(VkShaderModule* vertShaderModule, VkPipelineShaderStageCreateInfo* vertShaderStageInfo);
	void createFragmentShaderStageCreateInfo(VkShaderModule* fragShaderModule, VkPipelineShaderStageCreateInfo* fragShaderStageInfo);
	void createPipelineVertexInputStateCreateInfo(VkPipelineVertexInputStateCreateInfo* vertexInputInfo, VkVertexInputBindingDescription* bindingDescriptions, VkVertexInputAttributeDescription* attributeDescriptions);
	void createPipelineInputAssemblyStateCreateInfo(VkPipelineInputAssemblyStateCreateInfo* inputAssembly);
	void createPipelineViewportStateCreateInfo(VkPipelineViewportStateCreateInfo* viewportState, VkViewport* viewport, VkRect2D* scissor);
	void createPipelineRasterizationStateCreateInfo(VkPipelineRasterizationStateCreateInfo* rasterizer);
	void createPipelineMultisampleStateCreateInfo(VkPipelineMultisampleStateCreateInfo* multisampling);
	void createPipelineDepthStencilStateCreateInfo(VkPipelineDepthStencilStateCreateInfo* depthStencil);
	void createPipelineColorBlendStateCreateInfo(VkPipelineColorBlendAttachmentState* colorBlendAttachment, VkPipelineColorBlendStateCreateInfo* colorBlending);

	// Métodos de generación de la imagen
	void waitForNextImage();
	void submitGraphicsCommands();
	void submitPresentCommands();

	// Métodos auxiliares
	bool isDeviceSuitable(VkPhysicalDevice pDevice);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	VkShaderModule createShaderModule(const std::vector<char>& code);
	std::vector<char> getFileFromResource(int resource);
	VkFormat findDepthFormat();
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
};

