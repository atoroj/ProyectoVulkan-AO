#include "CAVulkanState.h"

#include "CAModel.h"
#include "CAVertex.h"
#include "CATransform.h"
#include "resource.h"
#include <windows.h>
#include <glm/common.hpp>
#include <iostream>
#include <vector>

///////////////////////////////////////////////////////////////////////////////////////////
/////                                                                                 /////
/////                              Métodos públicos                                   /////
/////                                                                                 /////
///////////////////////////////////////////////////////////////////////////////////////////

//
// FUNCIÓN: CAVulkanState::CAVulkanState(GLFWwindow* window)
//
// PROPÓSITO: Crea el estado de Vulkan
//
CAVulkanState::CAVulkanState(GLFWwindow* window)
{
	glfwGetFramebufferSize(window, &wWidth, &wHeight);
	this->window = window;
	createInstance();
	createSurface(window);
	pickPhysicalDevice();
	createLogicalDevice();
	createSwapChain();
	createImageViews();
	createRenderPass();
	createPipelineLayout();
	createGraphicsPipeline();
	createDepthBuffers();
	createFramebuffers();
	createCommandPool();
	createCommandBuffers();
	createSyncObjects();
}

//
// FUNCIÓN: CAVulkanState::~CAVulkanState()
//
// PROPÓSITO: Destruye el estado de Vulkan
//
CAVulkanState::~CAVulkanState()
{
	for (size_t i = 0; i < frameCount; i++)
	{
		vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(device, inFlightFences[i], nullptr);
	}
	vkFreeCommandBuffers(device, commandPool, imageCount, commandBuffers.data());
	vkDestroyCommandPool(device, commandPool, nullptr);
	for (uint32_t i = 0; i < imageCount; i++)
	{
		vkDestroyImageView(device, depthImageViews[i], nullptr);
		vkFreeMemory(device, depthImageMemories[i], nullptr);
		vkDestroyImage(device, depthImages[i], nullptr);
		vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
		vkDestroyImageView(device, swapChainImageViews[i], nullptr);
	}
	vkDestroyPipeline(device, graphicsPipeline, nullptr);
	vkDestroyRenderPass(device, renderPass, nullptr);
	vkDestroySwapchainKHR(device, swapChain, nullptr);
	vkDestroyDevice(device, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);
}

//
// FUNCIÓN: CAVulkanState::windowResized(int width, int height)
//
// PROPÓSITO: Actualiza el estado de Vulkan al redimensionar el
// 	          tamaño de la ventana
//
void CAVulkanState::windowResized(int width, int height)
{
	wWidth = width;
	wHeight = height;
	framebufferResized = true;
}

//
// FUNCIÓN: CAVulkanState::setModel(CAModel* model)
//
// PROPÓSITO: Asigna el modelo
//
void CAVulkanState::setModel(CAModel* model)
{
	this->model = model;
	double aspect = (double)wWidth / (double)wHeight;
	this->model->aspect_ratio(aspect);
	fillCommandBuffers();
}

//
// FUNCIÓN: CAVulkanState::draw()
//
// PROPÓSITO: Lanza la generación del dibujo
//
void CAVulkanState::draw()
{
	waitForNextImage();
	model->update();
	submitGraphicsCommands();
	submitPresentCommands();
}

///////////////////////////////////////////////////////////////////////////////////////////
/////                                                                                 /////
/////                    Métodos públicos de gestión de buffers                       /////
/////                                                                                 /////
///////////////////////////////////////////////////////////////////////////////////////////
 
//
// FUNCIÓN: CAVulkanState::createVertexBuffer(size_t vertexSize, void * vertexData, CAVertexBuffer* vbo)
//
// PROPÓSITO: Crea un Vertex Buffer
//
void CAVulkanState::createVertexBuffer(size_t vertexSize, const void* vertexData, CAVertexBuffer* vbo)
{
	VkBuffer buffer;
	VkDeviceMemory deviceMemory;

	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = vertexSize;
	bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	if (vkAllocateMemory(device, &allocInfo, nullptr, &deviceMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	vkBindBufferMemory(device, buffer, deviceMemory, 0);

	void* data;
	vkMapMemory(device, deviceMemory, 0, vertexSize, 0, &data);
	memcpy(data, vertexData, vertexSize);
	vkUnmapMemory(device, deviceMemory);

	vbo->buffer = buffer;
	vbo->memory = deviceMemory;
}

//
// FUNCIÓN: CAVulkanState::destroyVertexBuffer(CAVertexBuffer vbo)
//
// PROPÓSITO: Destruye los campos de un Vertex Buffer
//
void CAVulkanState::destroyVertexBuffer(CAVertexBuffer vbo)
{
	vkDestroyBuffer(device, vbo.buffer, nullptr);
	vkFreeMemory(device, vbo.memory, nullptr);
}

//
// FUNCIÓN: CAVulkanState::createIndexBuffer(size_t bufferSize, void * bufferData, CAIndexBuffer* ibo)
//
// PROPÓSITO: Crea un Index Buffer
//
void CAVulkanState::createIndexBuffer(size_t bufferSize, const void* bufferData, CAIndexBuffer* ibo)
{
	VkBuffer buffer;
	VkDeviceMemory deviceMemory;

	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = bufferSize;
	bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	if (vkAllocateMemory(device, &allocInfo, nullptr, &deviceMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	vkBindBufferMemory(device, buffer, deviceMemory, 0);

	void* data;
	vkMapMemory(device, deviceMemory, 0, bufferSize, 0, &data);
	memcpy(data, bufferData, bufferSize);
	vkUnmapMemory(device, deviceMemory);

	ibo->buffer = buffer;
	ibo->memory = deviceMemory;
}

//
// FUNCIÓN: CAVulkanState::destroyIndexBuffer(CAIndexBuffer ibo)
//
// PROPÓSITO: Destruye los campos de un Index Buffer
//
void CAVulkanState::destroyIndexBuffer(CAIndexBuffer ibo)
{
	vkDestroyBuffer(device, ibo.buffer, nullptr);
	vkFreeMemory(device, ibo.memory, nullptr);
}

//
// FUNCIÓN: CAVulkanState::createIndexBuffer(size_t bufferSize, CAUniformBuffer* ubo)
//
// PROPÓSITO: Crea una lista de Uniform Buffers asociados a cada imagen a generar
//
void CAVulkanState::createUniformBuffer(size_t bufferSize, CAUniformBuffer* ubo)
{
	ubo->buffers.resize(imageCount);
	ubo->memories.resize(imageCount);

	for (uint32_t i = 0; i < imageCount; i++)
	{
		VkBuffer buffer;
		VkDeviceMemory deviceMemory;

		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = bufferSize;
		bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create buffer!");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &deviceMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate buffer memory!");
		}

		vkBindBufferMemory(device, buffer, deviceMemory, 0);

		ubo->buffers[i] = buffer;
		ubo->memories[i] = deviceMemory;
	}
}

//
// FUNCIÓN: CAVulkanState::updateUniformBuffer(size_t size, const void* data, CAUniformBuffer ubo)
//
// PROPÓSITO: Actualiza el valor almacenado en un Uniform Buffer
//
void CAVulkanState::updateUniformBuffer(size_t size, const void* data, CAUniformBuffer ubo)
{
	void* mdata;
	vkMapMemory(device, ubo.memories[currentImage], 0, size, 0, &mdata);
	memcpy(mdata, data, size);
	vkUnmapMemory(device, ubo.memories[currentImage]);
}

//
// FUNCIÓN: CAVulkanState::destroyUniformBuffer(CAUniformBuffer ubo)
//
// PROPÓSITO: Destruye los campos de un Uniform Buffer
//
void CAVulkanState::destroyUniformBuffer(CAUniformBuffer ubo)
{
	for (uint32_t i = 0; i < imageCount; i++)
	{
		vkDestroyBuffer(device, ubo.buffers[i], nullptr);
		vkFreeMemory(device, ubo.memories[i], nullptr);
	}
}

//
// FUNCIÓN:CAVulkanState::createDescriptorSets(VkDescriptorPool* descriptorPool, std::vector<VkDescriptorSet> *descriptorSets, std::vector<CAUniformBuffer> buffers,size_t* bufferSizes, size_t sizes)
//
// PROPÓSITO: Crea los conjuntos de descriptores asociados a los buffers
//
void CAVulkanState::createDescriptorSets(VkDescriptorPool* descriptorPool, std::vector<VkDescriptorSet>* descriptorSets, CAUniformBuffer** buffers, size_t* bufferSizes, size_t size)
{
	std::vector<VkDescriptorPoolSize> poolSizes(size);

	for(int i=0; i<size; i++)
	{
		VkDescriptorPoolSize poolSize = {};
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSize.descriptorCount = imageCount;
		poolSizes[i] = poolSize;
	}

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = (uint32_t) poolSizes.size();
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = imageCount;

	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, descriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor pool!");
	}

	std::vector<VkDescriptorSetLayout> layouts(imageCount, descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = *descriptorPool;
	allocInfo.descriptorSetCount = imageCount;
	allocInfo.pSetLayouts = layouts.data();

	(*descriptorSets).resize(imageCount);

	if (vkAllocateDescriptorSets(device, &allocInfo, (*descriptorSets).data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	for (size_t i = 0; i < imageCount; i++)
	{
		std::vector<VkDescriptorBufferInfo> buffersInfo;
		buffersInfo.resize(size);

		for (int j = 0; j < size; j++)
		{
			buffersInfo[j] = {};
			buffersInfo[j].buffer = buffers[j]->buffers[i];
			buffersInfo[j].offset = 0;
			buffersInfo[j].range = bufferSizes[j];
		}

		VkWriteDescriptorSet descriptorWrite = {};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = (*descriptorSets)[i];
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = (uint32_t) buffersInfo.size();
		descriptorWrite.pBufferInfo = buffersInfo.data();

		vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
	}
}

//
// FUNCIÓN:CAVulkanState::createDescriptorSets(VkDescriptorPool descriptorPool, std::vector<VkDescriptorSet> descriptorSets)
//
// PROPÓSITO: Destruye los conjuntos de descriptores
//
void CAVulkanState::destroyDescriptorSets(VkDescriptorPool descriptorPool, std::vector<VkDescriptorSet> descriptorSets)
{
	vkFreeDescriptorSets(device, descriptorPool, (uint32_t) descriptorSets.size(), descriptorSets.data());
	vkDestroyDescriptorPool(device, descriptorPool, nullptr);
}

//
// FUNCIÓN: CAVulkanState::getPipelineLayout()
//
// PROPÓSITO: Obtiene la plantilla de descriptores
//
VkPipelineLayout CAVulkanState::getPipelineLayout()
{
	return this->pipelineLayout;
}

///////////////////////////////////////////////////////////////////////////////////////////
/////                                                                                 /////
/////                     Métodos de inicialización de Vulkan                         /////
/////                                                                                 /////
///////////////////////////////////////////////////////////////////////////////////////////

//
// FUNCIÓN: CAVulkanState::createInstance()
//
// PROPÓSITO: Crea la instancia de Vulkan
//
void CAVulkanState::createInstance()
{
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Computer Animation";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = glfwExtensionCount;
	createInfo.ppEnabledExtensionNames = glfwExtensions;
	createInfo.enabledLayerCount = 0;

	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create instance!");
	}
}

//
// FUNCIÓN: CAVulkanState::createSurface(GLFWwindow* window)
//
// PROPÓSITO: Crea la superficie sobre la que mostrar la representación gráfica
//
void CAVulkanState::createSurface(GLFWwindow* window)
{
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create window surface!");
	}
}

//
// FUNCIÓN: CAVulkanState::pickPhysicalDevice()
//
// PROPÓSITO: Selecciona el dispositivo físico sobre el que generar los gráficos
//
void CAVulkanState::pickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0)
	{
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	for (uint32_t i = 0; i < deviceCount; i++)
	{
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(devices[i], &deviceProperties);

		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && isDeviceSuitable(devices[i]))
		{
			physicalDevice = devices[i];
			break;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE)
	{
		for (uint32_t i = 0; i < deviceCount; i++)
		{
			if (isDeviceSuitable(devices[i]))
			{
				physicalDevice = devices[i];
				break;
			}
		}
	}

	if (physicalDevice == VK_NULL_HANDLE)
	{
		throw std::runtime_error("failed to find a suitable GPU!");
	}

	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
}

//
// FUNCIÓN: CAVulkanState::createLogicalDevice()
//
// PROPÓSITO: Crea el dispositivo lógico sobre el que generar los gráficos 
//            y selecciona la familia de colas sobre el dispositivo
//
void CAVulkanState::createLogicalDevice()
{
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfo;
	uint32_t queueCreateInfoCount;

	if (graphicsQueueFamilyIndex == presentQueueFamilyIndex)
	{
		VkDeviceQueueCreateInfo graphicsQueueCreateInfo = {};
		graphicsQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		graphicsQueueCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
		graphicsQueueCreateInfo.queueCount = 1;
		float graphicsQueuePriority = 1.0f;
		graphicsQueueCreateInfo.pQueuePriorities = &graphicsQueuePriority;

		queueCreateInfoCount = 1;
		queueCreateInfo = { graphicsQueueCreateInfo };
	}
	else
	{
		VkDeviceQueueCreateInfo graphicsQueueCreateInfo = {};
		graphicsQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		graphicsQueueCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
		graphicsQueueCreateInfo.queueCount = 1;
		float graphicsQueuePriority = 1.0f;
		graphicsQueueCreateInfo.pQueuePriorities = &graphicsQueuePriority;

		VkDeviceQueueCreateInfo presentQueueCreateInfo = {};
		presentQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		presentQueueCreateInfo.queueFamilyIndex = presentQueueFamilyIndex;
		presentQueueCreateInfo.queueCount = 1;
		float presentQueuePriority = 1.0f;
		presentQueueCreateInfo.pQueuePriorities = &presentQueuePriority;

		queueCreateInfoCount = 2;
		queueCreateInfo = { graphicsQueueCreateInfo ,presentQueueCreateInfo };
	}

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueCreateInfo.data();
	createInfo.queueCreateInfoCount = queueCreateInfoCount;
	createInfo.enabledExtensionCount = 0;
	createInfo.enabledLayerCount = 0;

	std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	VkPhysicalDeviceFeatures supportedFeatures = {};
	VkPhysicalDeviceFeatures requiredFeatures = {};
	vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);
	requiredFeatures.multiDrawIndirect = supportedFeatures.multiDrawIndirect;
	requiredFeatures.tessellationShader = VK_TRUE;
	requiredFeatures.geometryShader = VK_TRUE;
	createInfo.pEnabledFeatures = &requiredFeatures;

	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create logical device!");
	}

	vkGetDeviceQueue(device, graphicsQueueFamilyIndex, 0, &graphicsQueue);
	vkGetDeviceQueue(device, presentQueueFamilyIndex, 0, &presentQueue);
}

//
// FUNCIÓN: CAVulkanState::createSwapChain()
//
// PROPÓSITO: Crea los buffers de intercambio de imágenes, el vector de imágenes
//            y sus formatos y tamaños
//
void CAVulkanState::createSwapChain()
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
	if (formatCount != 0)
	{
		formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data());
	}

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(formats);
	VkExtent2D extent = chooseSwapExtent(capabilities);

	imageCount = 4;
	if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
	{
		imageCount = capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	if (graphicsQueueFamilyIndex != presentQueueFamilyIndex)
	{
		std::vector<uint32_t> queueFamilyIndices;
		queueFamilyIndices.resize(2);
		queueFamilyIndices.push_back(graphicsQueueFamilyIndex);
		queueFamilyIndices.push_back(presentQueueFamilyIndex);

		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 1;
		createInfo.pQueueFamilyIndices = &graphicsQueueFamilyIndex;
	}

	createInfo.preTransform = capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
	createInfo.clipped = VK_TRUE;

	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;
}

//
// FUNCIÓN: CAVulkanState::createImageViews()
//
// PROPÓSITO: Crea una vista para cada imagen de la cadena de intercambio
//
void CAVulkanState::createImageViews()
{
	swapChainImageViews.resize(imageCount);

	for (size_t i = 0; i < imageCount; i++)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapChainImageFormat;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i])
			!= VK_SUCCESS)
		{
			throw std::runtime_error("failed to create image views!");
		}
	}
}

//
// FUNCIÓN: CAVulkanState::createRenderPass()
//
// PROPÓSITO: Crea los pasos de renderizado
//
void CAVulkanState::createRenderPass()
{
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = findDepthFormat();
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	VkAttachmentDescription attachment[] = { colorAttachment, depthAttachment };

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 2;
	renderPassInfo.pAttachments = attachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create render pass!");
	}
}

//
// FUNCIÓN: CAVulkanState::createPipelineLayout()
//
// PROPÓSITO: Crea el esquema del descriptor set
//
//
// FUNCIÓN: CAVulkanState::createPipelineLayout()
//
// PROPÓSITO: Crea el esquema del descriptor set
//
void CAVulkanState::createPipelineLayout()
{
	VkDescriptorSetLayoutBinding transformLayoutBinding = {};
	transformLayoutBinding.binding = 0;
	transformLayoutBinding.descriptorCount = 1;
	transformLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	transformLayoutBinding.pImmutableSamplers = nullptr;
	transformLayoutBinding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;

	VkDescriptorSetLayoutBinding lightLayoutBinding = {};
	lightLayoutBinding.binding = 1;
	lightLayoutBinding.descriptorCount = 1;
	lightLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	lightLayoutBinding.pImmutableSamplers = nullptr;
	lightLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding materialLayoutBinding = {};
	materialLayoutBinding.binding = 2;
	materialLayoutBinding.descriptorCount = 1;
	materialLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	materialLayoutBinding.pImmutableSamplers = nullptr;
	materialLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding pBindings[] = { transformLayoutBinding, lightLayoutBinding , materialLayoutBinding };

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 3;
	layoutInfo.pBindings = pBindings;

	if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor set layout!");
	}

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 0;

	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline layout!");
	}
}

//
// FUNCIÓN: CAVulkanState::createGraphicsPipeline()
//
// PROPÓSITO: Crea el Pipeline de renderizado
//
void CAVulkanState::createGraphicsPipeline()
{
	VkShaderModule vertShaderModule, fragShaderModule;
	VkPipelineShaderStageCreateInfo vertShaderStageInfo, fragShaderStageInfo;
	VkVertexInputBindingDescription* bindingDescriptions = nullptr;
	VkVertexInputAttributeDescription* attributeDescriptions = nullptr;
	VkPipelineVertexInputStateCreateInfo vertexInputInfo;
	VkPipelineInputAssemblyStateCreateInfo inputAssembly;
	VkViewport viewport;
	VkRect2D scissor;
	VkPipelineViewportStateCreateInfo viewportState;
	VkPipelineRasterizationStateCreateInfo rasterizer;
	VkPipelineMultisampleStateCreateInfo multisampling;
	VkPipelineDepthStencilStateCreateInfo depthStencil;
	VkPipelineColorBlendAttachmentState colorBlendAttachment;
	VkPipelineColorBlendStateCreateInfo colorBlending;

	createVertexShaderStageCreateInfo(&vertShaderModule, &vertShaderStageInfo);
	createFragmentShaderStageCreateInfo(&fragShaderModule, &fragShaderStageInfo);
	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };
	createPipelineVertexInputStateCreateInfo(&vertexInputInfo, bindingDescriptions, attributeDescriptions);
	createPipelineInputAssemblyStateCreateInfo(&inputAssembly);
	createPipelineViewportStateCreateInfo(&viewportState, &viewport, &scissor);
	createPipelineRasterizationStateCreateInfo(&rasterizer);
	createPipelineMultisampleStateCreateInfo(&multisampling);
	createPipelineDepthStencilStateCreateInfo(&depthStencil);
	createPipelineColorBlendStateCreateInfo(&colorBlendAttachment, &colorBlending);

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	vkDestroyShaderModule(device, fragShaderModule, nullptr);
	vkDestroyShaderModule(device, vertShaderModule, nullptr);
}


//
// FUNCIÓN: CAVulkanState::createDepthBuffers()
//
// PROPÓSITO: Crea los buffers de profundidad
//
void CAVulkanState::createDepthBuffers()
{
	depthImages.resize(imageCount);
	depthImageMemories.resize(imageCount);
	depthImageViews.resize(imageCount);
	VkFormat depthFormat = findDepthFormat();

	for (size_t i = 0; i < imageCount; i++)
	{
		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = swapChainExtent.width;
		imageInfo.extent.height = swapChainExtent.height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = depthFormat;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateImage(device, &imageInfo, nullptr, &depthImages[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create image!");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, depthImages[i], &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &depthImageMemories[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate image memory!");
		}

		vkBindImageMemory(device, depthImages[i], depthImageMemories[i], 0);

		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = depthImages[i];
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = depthFormat;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device, &viewInfo, nullptr, &depthImageViews[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create texture image view!");
		}
	}
}

//
// FUNCIÓN: CAVulkanState::createFramebuffers()
//
// PROPÓSITO: Crea un Framebuffer para cada imagen del swapchain
//
void CAVulkanState::createFramebuffers()
{
	swapChainFramebuffers.resize(imageCount);

	for (size_t i = 0; i < imageCount; i++)
	{
		VkImageView attachments[] = { swapChainImageViews[i], depthImageViews[i] };

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 2;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

//
// FUNCIÓN: CAVulkanState::createCommandPool()
//
// PROPÓSITO: Crea el command pool vinculado a la familia de colas para gráficos
//
void CAVulkanState::createCommandPool()
{
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = graphicsQueueFamilyIndex;

	if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create command pool!");
	}
}

//
// FUNCIÓN: CAVulkanState::createCommandBuffers()
//
// PROPÓSITO: Crea los buffers de comandos que se enviarán a la cola gráfica
//            El contenido de los buffers incluye la orden de dibujar.
//
void CAVulkanState::createCommandBuffers()
{
	commandBuffers.resize(imageCount);

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = imageCount;

	if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate command buffers!");
	}
}

//
// FUNCIÓN: CAVulkanState::fillCommandBuffers()
//
// PROPÓSITO: Crea el contenido de los buffers de comandos
//
void CAVulkanState::fillCommandBuffers()
{
	for (size_t i = 0; i < imageCount; i++)
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		VkClearValue clearValues[2];
		clearValues[0].color = { 1.0f, 1.0f, 1.0f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = swapChainFramebuffers[i];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChainExtent;
		renderPassInfo.clearValueCount = 2;
		renderPassInfo.pClearValues = clearValues;

		vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

		model->addCommands(commandBuffers[i], (int)i);

		vkCmdEndRenderPass(commandBuffers[i]);

		if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to record command buffer!");
		}
	}
}

//
// FUNCIÓN: CAVulkanState::createSyncObjects()
//
// PROPÓSITO: Crea los semáforos y los fences para no sobreescribir las imágenes
//
void CAVulkanState::createSyncObjects()
{
	frameCount = imageCount - 1;
	imageAvailableSemaphores.resize(frameCount);
	renderFinishedSemaphores.resize(frameCount);
	inFlightFences.resize(frameCount);
	imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < frameCount; i++)
	{
		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}
}

//
// FUNCIÓN: CAVulkanState::recreateSwapChain()
//
// PROPÓSITO: Reconstruye las estructuras vinculadas a la swapchain con el
//            nuevo tamaño de ventana
//
void CAVulkanState::recreateSwapChain()
{
	int width = 0, height = 0;
	glfwGetFramebufferSize(window, &width, &height);
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}
	vkDeviceWaitIdle(device);

	vkFreeCommandBuffers(device, commandPool, imageCount, commandBuffers.data());
	vkDestroyCommandPool(device, commandPool, nullptr);
	for (uint32_t i = 0; i < imageCount; i++)
	{
		vkDestroyImageView(device, depthImageViews[i], nullptr);
		vkFreeMemory(device, depthImageMemories[i], nullptr);
		vkDestroyImage(device, depthImages[i], nullptr);
		vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
		vkDestroyImageView(device, swapChainImageViews[i], nullptr);
	}
	vkDestroyPipeline(device, graphicsPipeline, nullptr);
	vkDestroyRenderPass(device, renderPass, nullptr);
	vkDestroySwapchainKHR(device, swapChain, nullptr);

	createSwapChain();
	createImageViews();
	createRenderPass();
	createGraphicsPipeline();
	createDepthBuffers();
	createFramebuffers();
	createCommandPool();
	createCommandBuffers();
	fillCommandBuffers();
}

///////////////////////////////////////////////////////////////////////////////////////////
/////                                                                                 /////
/////              Métodos de definición del pipeline de renderizado                  /////
/////                                                                                 /////
///////////////////////////////////////////////////////////////////////////////////////////

//
// FUNCIÓN: CAVulkanState::createVertexShaderStageCreateInfo()
//
// PROPÓSITO: Crea la información sobre el Vertex Shader
//
void CAVulkanState::createVertexShaderStageCreateInfo(VkShaderModule* vertShaderModule, VkPipelineShaderStageCreateInfo* vertShaderStageInfo)
{
	std::vector<char> vertShaderCode = getFileFromResource(IDR_HTML1);

	*vertShaderModule = createShaderModule(vertShaderCode);

	*vertShaderStageInfo = {};
	vertShaderStageInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo->stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo->module = *vertShaderModule;
	vertShaderStageInfo->pName = "main";
}

//
// FUNCIÓN: CAVulkanState::createFragmentShaderStageCreateInfo()
//
// PROPÓSITO: Crea la información sobre el Vertex Shader
//
void CAVulkanState::createFragmentShaderStageCreateInfo(VkShaderModule* fragShaderModule, VkPipelineShaderStageCreateInfo* fragShaderStageInfo)
{
	std::vector<char> fragShaderCode = getFileFromResource(IDR_HTML2);

	*fragShaderModule = createShaderModule(fragShaderCode);

	*fragShaderStageInfo = {};
	fragShaderStageInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo->stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo->module = *fragShaderModule;
	fragShaderStageInfo->pName = "main";
}

//
// FUNCIÓN: CAVulkanState::createPipelineVertexInputStateCreateInfo()
//
// PROPÓSITO: Crea la descripción de los atributos de los vértices
//
void CAVulkanState::createPipelineVertexInputStateCreateInfo(VkPipelineVertexInputStateCreateInfo* vertexInputInfo,
	VkVertexInputBindingDescription* bindingDescriptions,
	VkVertexInputAttributeDescription* attributeDescriptions
)
{
	bindingDescriptions = (VkVertexInputBindingDescription*)malloc(sizeof(VkVertexInputBindingDescription));

	bindingDescriptions[0] = {};
	bindingDescriptions[0].binding = 0;
	bindingDescriptions[0].stride = sizeof(CAVertex);
	bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	attributeDescriptions = (VkVertexInputAttributeDescription*)malloc(2 * sizeof(VkVertexInputAttributeDescription));
	attributeDescriptions[0] = {};
	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(CAVertex, pos);

	attributeDescriptions[1] = {};
	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(CAVertex, norm);

	*vertexInputInfo = {};
	vertexInputInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo->vertexBindingDescriptionCount = 1;
	vertexInputInfo->vertexAttributeDescriptionCount = 2;
	vertexInputInfo->pVertexBindingDescriptions = bindingDescriptions;
	vertexInputInfo->pVertexAttributeDescriptions = attributeDescriptions;
}


//
// FUNCIÓN: CAVulkanState::createPipelineVertexInputStateCreateInfo()
//
// PROPÓSITO: Crea la información para el ensamblado de primitivas
//
void CAVulkanState::createPipelineInputAssemblyStateCreateInfo(VkPipelineInputAssemblyStateCreateInfo* inputAssembly)
{
	*inputAssembly = {};
	inputAssembly->sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly->topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly->primitiveRestartEnable = VK_FALSE;
}

//
// FUNCIÓN: CAVulkanState::createPipelineViewportStateCreateInfo()
//
// PROPÓSITO: Crea la información del viewport
//
void CAVulkanState::createPipelineViewportStateCreateInfo(VkPipelineViewportStateCreateInfo* viewportState, VkViewport* viewport, VkRect2D* scissor)
{
	*viewport = {};
	viewport->x = 0.0f;
	viewport->y = 0.0f;
	viewport->width = (float)swapChainExtent.width;
	viewport->height = (float)swapChainExtent.height;
	viewport->minDepth = 0.0f;
	viewport->maxDepth = 1.0f;

	*scissor = {};
	scissor->offset = { 0, 0 };
	scissor->extent = swapChainExtent;

	*viewportState = {};
	viewportState->sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState->viewportCount = 1;
	viewportState->pViewports = viewport;
	viewportState->scissorCount = 1;
	viewportState->pScissors = scissor;
}

//
// FUNCIÓN: CAVulkanState::createPipelineRasterizationStateCreateInfo()
//
// PROPÓSITO: Crea la información de la etapa de rasterización
//
void CAVulkanState::createPipelineRasterizationStateCreateInfo(VkPipelineRasterizationStateCreateInfo* rasterizer)
{
	*rasterizer = {};
	rasterizer->sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer->depthClampEnable = VK_FALSE;
	rasterizer->rasterizerDiscardEnable = VK_FALSE;
	rasterizer->polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer->lineWidth = 1.0f;
	rasterizer->cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer->frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer->depthBiasEnable = VK_FALSE;
}

//
// FUNCIÓN: CAVulkanState::createPipelineRasterizationStateCreateInfo()
//
// PROPÓSITO: Crea la información de la etapa de sampleado
//
void CAVulkanState::createPipelineMultisampleStateCreateInfo(VkPipelineMultisampleStateCreateInfo* multisampling)
{
	*multisampling = {};
	multisampling->sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling->sampleShadingEnable = VK_FALSE;
	multisampling->rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
}

//
// FUNCIÓN: CAVulkanState::createPipelineRasterizationStateCreateInfo()
//
// PROPÓSITO: Crea la información sobre los tests de profundidad y de plantilla
//
void CAVulkanState::createPipelineDepthStencilStateCreateInfo(VkPipelineDepthStencilStateCreateInfo* depthStencil)
{
	*depthStencil = {};
	depthStencil->sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil->pNext = nullptr;
	depthStencil->flags = 0;
	depthStencil->depthTestEnable = VK_TRUE;
	depthStencil->depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthStencil->depthWriteEnable = VK_TRUE;
	depthStencil->depthBoundsTestEnable = VK_FALSE;
	depthStencil->stencilTestEnable = VK_FALSE;
}

//
// FUNCIÓN: CAVulkanState::createPipelineColorBlendStateCreateInfo()
//
// PROPÓSITO: Crea la información sobre la etapa de mezcla de colores
//
void CAVulkanState::createPipelineColorBlendStateCreateInfo(VkPipelineColorBlendAttachmentState* colorBlendAttachment, VkPipelineColorBlendStateCreateInfo* colorBlending)
{
	*colorBlendAttachment = {};
	colorBlendAttachment->colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment->blendEnable = VK_FALSE;

	*colorBlending = {};
	colorBlending->sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending->logicOpEnable = VK_FALSE;
	colorBlending->logicOp = VK_LOGIC_OP_COPY;
	colorBlending->attachmentCount = 1;
	colorBlending->pAttachments = colorBlendAttachment;
	colorBlending->blendConstants[0] = 0.0f;
	colorBlending->blendConstants[1] = 0.0f;
	colorBlending->blendConstants[2] = 0.0f;
	colorBlending->blendConstants[3] = 0.0f;
}

///////////////////////////////////////////////////////////////////////////////////////////
/////                                                                                 /////
/////                       Métodos de generación de la imagen                        /////
/////                                                                                 /////
///////////////////////////////////////////////////////////////////////////////////////////

//
// FUNCIÓN: CAVulkanState::waitForNextImage()
//
// PROPÓSITO: Espera hasta que la próxima imagen esté lista para ser generada
//
void CAVulkanState::waitForNextImage()
{
	vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
	currentImage = imageIndex;

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreateSwapChain();
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("failed to acquire swap chain image!");
	}
}

//
// FUNCIÓN: CAVulkanState::submitGraphicsCommands()
//
// PROPÓSITO: Envía los comandos gráficos al dispositivo
//
void CAVulkanState::submitGraphicsCommands()
{
	if (imagesInFlight[currentImage] != VK_NULL_HANDLE)
	{
		vkWaitForFences(device, 1, &imagesInFlight[currentImage], VK_TRUE, UINT64_MAX);
	}
	imagesInFlight[currentImage] = inFlightFences[currentFrame];

	VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[currentImage];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	vkResetFences(device, 1, &inFlightFences[currentFrame]);

	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to submit draw command buffer!");
	}
}

//
// FUNCIÓN: CAVulkanState::submitPresentCommands()
//
// PROPÓSITO: Envía los comandos de presentación al dispositivo
//
void CAVulkanState::submitPresentCommands()
{
	VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
	VkSwapchainKHR swapChains[] = { swapChain };

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &currentImage;

	VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized)
	{
		framebufferResized = false;
		recreateSwapChain();
	}
	else if (result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to present swap chain image!");
	}

	currentFrame = (currentFrame + 1) % frameCount;
}

///////////////////////////////////////////////////////////////////////////////////////////
/////                                                                                 /////
/////                              Métodos auxiliares                                 /////
/////                                                                                 /////
///////////////////////////////////////////////////////////////////////////////////////////

//
// FUNCIÓN: CAVulkanState::isDeviceSuitable(VkPhysicalDevice pDevice)
//
// PROPÓSITO: Verifica si un dispositivo físico admite generación de gráficos
//
bool CAVulkanState::isDeviceSuitable(VkPhysicalDevice pDevice)
{
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &queueFamilyCount, nullptr);
	VkQueueFamilyProperties* queueFamilies = (VkQueueFamilyProperties*)malloc(queueFamilyCount * sizeof(VkQueueFamilyProperties));
	vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &queueFamilyCount, queueFamilies);

	bool graphics = false;
	bool present = false;
	for (uint32_t i = 0; i < queueFamilyCount; i++)
	{
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(pDevice, i, surface, &presentSupport);
		if (presentSupport && (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
		{
			graphicsQueueFamilyIndex = i;
			presentQueueFamilyIndex = i;
			graphics = true;
			present = true;
			break;
		}
	}

	if (!(graphics && present))
	{
		for (uint32_t i = 0; i < queueFamilyCount; i++)
		{
			if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				graphicsQueueFamilyIndex = i;
				graphics = true;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(pDevice, i, surface, &presentSupport);
			if (presentSupport)
			{
				presentQueueFamilyIndex = i;
				present = true;
			}
		}
	}

	free(queueFamilies);

	if (graphics && present) return true;

	return false;
}

//
// FUNCIÓN: CAVulkanState::chooseSwapSurfaceFormat()
//
// PROPÓSITO: Escoge el formato de imagen entre los soportados por la superficie
//
VkSurfaceFormatKHR CAVulkanState::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}

	return availableFormats[0];
}

//
// FUNCIÓN: CAVulkanState::chooseSwapExtent()
//
// PROPÓSITO: Escoge el tamaño de las imágenes asegurando que puede ser soportado por
//            la superficie
//
VkExtent2D CAVulkanState::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != UINT32_MAX)
	{
		return capabilities.currentExtent;
	}
	else
	{
		VkExtent2D actualExtent = { (uint32_t)wWidth, (uint32_t)wHeight };

		actualExtent.width = glm::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = glm::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

//
// FUNCIÓN: CAVulkanState::createShaderModule()
//
// PROPÓSITO: Crea un shader a partir de su código en SPIR-V
//
VkShaderModule CAVulkanState::createShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}

//
// FUNCIÓN: CAVulkanState::getFileFromResource(int resource)
//
// PROPÓSITO: Extrae el contenido de un fichero incluido como recurso de la aplicación
//
std::vector<char> CAVulkanState::getFileFromResource(int resource)
{
	HRSRC shaderHandle = FindResource(NULL, MAKEINTRESOURCE(resource), RT_HTML);
	HGLOBAL shaderGlobal = LoadResource(NULL, shaderHandle);
	LPCTSTR shaderPtr = static_cast<LPCTSTR>(LockResource(shaderGlobal));
	DWORD shaderSize = SizeofResource(NULL, shaderHandle);

	std::vector<char> shader(shaderSize);
	memcpy(shader.data(), shaderPtr, shaderSize);
	UnlockResource(shaderGlobal);
	FreeResource(shaderGlobal);
	return shader;
}

//
// FUNCIÓN: CAVulkanState::findDepthFormat()
//
// PROPÓSITO: Busca el formato adecuado para el buffer de profundidad
//
VkFormat CAVulkanState::findDepthFormat()
{
	VkFormat candidates[] = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
	VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
	VkFormatFeatureFlags features = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

	for (VkFormat format : candidates)
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
		{
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
		{
			return format;
		}
	}

	throw std::runtime_error("failed to find supported format!");
}

//
// FUNCIÓN: CAVulkanState::findMemoryType()
//
// PROPÓSITO: Busca el tipo de memoria adecuado para el filtro indicado
//
uint32_t CAVulkanState::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}