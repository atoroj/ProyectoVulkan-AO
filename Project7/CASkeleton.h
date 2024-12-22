#pragma once

#include "CABalljoint.h"

class CASkeleton {
protected:
	std::string name;
	glm::mat4 location;

	glm::vec3 offset;
	glm::vec3 dir;
	glm::vec3 up;
	glm::vec3 right;
	
	std::vector<CABalljoint*> articulaciones;
	CALight light;
	CAMaterial material;

public:
	CASkeleton(CAVulkanState* vulkan, std::string name, glm::vec3 offset, glm::vec3 up, glm::vec3 dir);
	~CASkeleton();
	void initialize(CAVulkanState* vulkan);
	void finalize(CAVulkanState* vulkan);
	void addCommands(CAVulkanState* vulkan, VkCommandBuffer commandBuffer, int index);
	void updateUniformBuffers(CAVulkanState* vulkan, glm::mat4 view, glm::mat4 projection);
	void resetLocation();
	void setLocation(glm::mat4 m);
	void translate(glm::vec3 t);
	void rotate(float angle, glm::vec3 axis);
	void setLight(CALight l);
	void setMaterial(CAMaterial m);
	std::vector<CABalljoint*> getHijas();

private:
	CAUniformBuffer transformBuffer;
	CAUniformBuffer lightBuffer;
	CAUniformBuffer materialBuffer;
	VkDescriptorPool descriptorPool;
	std::vector<VkDescriptorSet> descriptorSets;
};