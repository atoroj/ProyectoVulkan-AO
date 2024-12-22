#pragma once

#include "CAVulkanState.h"
#include "CAScene.h"
#include "CACamera.h"


class CAModel
{
private:
	CAVulkanState* vulkan;
	glm::mat4 projection;
	float aspect;
	float time = 0.0f;
	CAScene* scene;
	CACamera* camera;

public:
	CAModel(CAVulkanState* vulkan);
	~CAModel();

	void addCommands(VkCommandBuffer commandBuffer, int index);
	void update();
	void key_pressed(int key);
	void mouse_button(int button, int action);
	void mouse_move(double xpos, double ypos);
	void aspect_ratio(double aspect);
};


