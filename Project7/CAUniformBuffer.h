#pragma once

#include <vulkan/vulkan.h>
#include <vector>

typedef struct
{
	std::vector<VkBuffer> buffers;
	std::vector<VkDeviceMemory> memories;
} CAUniformBuffer;

