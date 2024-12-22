#pragma once

#include "CASphere.h"
#include "CACylinder.h"
#include <glm\glm.hpp>
#include <string>

//
// CLASE: Balljoint
//
// DESCRIPCIÓN: Representa una articulación con 3 grados de libertad
// 
class CABalljoint {
private:
	std::string name;
	GLfloat length;
	glm::vec3 location;
	glm::vec3 dir;
	glm::vec3 up;
	glm::vec3 right;
	glm::mat4 matrizpadre;
	GLfloat angles[3];
	CASphere* joint;
	CACylinder* bone;


	std::vector<CABalljoint*> hijas;
	glm::mat2x3 limit;
	void ComputeMatrix();
	glm::mat4 parentMatrix;
	
public:
	CABalljoint(std::string name, float length);
	~CABalljoint();
	void initialize(CAVulkanState* vulkan);
	void finalize(CAVulkanState* vulkan);
	void addCommands(CAVulkanState* vulkan, VkCommandBuffer commandBuffer, int index);
	void updateUniformBuffers(CAVulkanState* vulkan, glm::mat4 view, glm::mat4 projection);
	void setMatrix(glm::mat4 matrix);
	void setLight(CALight l);
	void setLocation(glm::vec3 loc);
	void setOrientation(glm::vec3 nDir, glm::vec3 nUp);
	void setPose(float xrot, float yrot, float zrot);
	void anadirHijo(CABalljoint* c);
	void setLimitX(GLfloat min, GLfloat max);
	void setLimitY(GLfloat min, GLfloat max);
	void setLimitZ(GLfloat min, GLfloat max);
	void setParentLocation(glm::mat4 l);
	std::string getName();
	std::vector<CABalljoint*> getHijas();
};


