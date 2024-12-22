#pragma once

#include <vulkan/vulkan.h>

typedef struct
{
	VkBuffer buffer;
	VkDeviceMemory memory;
} CAIndexBuffer;
