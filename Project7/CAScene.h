#pragma once

#include "CAVulkanState.h"
#include "CAFigure.h"
#include "CABalljoint.h"
#include "CASkeleton.h"
#include "Animation.h"

class CAScene {
public:
	CAScene(CAVulkanState* vulkan);
	~CAScene();
	void finalize(CAVulkanState* vulkan);
	void addCommands(CAVulkanState* vulkan, VkCommandBuffer commandBuffer, int index);
	void update(CAVulkanState* vulkan, glm::mat4 view, glm::mat4 projection);
	Animation* getAnimation();
	void setDuration(float d);
	void setMovement(float m);
	void setIncremento(float i);
	

private:
	float duration = 0.0f;
	float movement = 0.0f;
	float incremento = 0.02f;
	CAFigure* ground;
	CASkeleton* esqueleto;
	Animation* animacion;
};

