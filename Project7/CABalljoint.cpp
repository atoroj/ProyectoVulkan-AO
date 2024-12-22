#include "CABalljoint.h"
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

//
// FUNCIÓN: CABalljoint::CABalljoint()
//
// PROPÓSITO: Construye una articulación.
//
CABalljoint::CABalljoint(std::string name, float l)
{
	length = l;
	parentMatrix = glm::mat4(1.0f);
	this->name = name;
	location = glm::vec3(0.0f, 0.0f, 0.0f);
	dir = glm::vec3(0.0f, 0.0f, 1.0f);
	up = glm::vec3(0.0f, 1.0f, 0.0f);
	right = glm::vec3(1.0f, 0.0f, 0.0f);

	angles[0] = 0.0f;
	angles[1] = 0.0f;
	angles[2] = 0.0f;

	limit[0][0] = -180.0f; limit[1][0] = 180.0f;
	limit[0][1] = -180.0f; limit[1][1] = 180.0f;
	limit[0][2] = -180.0f; limit[1][2] = 180.0f;
}

//
// FUNCIÓN: CABalljoint::~CABalljoint()
//
// PROPÓSITO: Destruye una articulación.
//
CABalljoint::~CABalljoint()
{
	delete joint;
	delete bone;
	hijas.clear();
}


void CABalljoint::anadirHijo(CABalljoint* c) {
	hijas.push_back(c);
	ComputeMatrix();
}

void CABalljoint::setLimitX(GLfloat min, GLfloat max) {
	limit[0][0] = min;
	limit[1][0] = max;
}

void CABalljoint::setLimitY(GLfloat min, GLfloat max) {
	limit[0][1] = min;
	limit[1][1] = max;
}

void CABalljoint::setLimitZ(GLfloat min, GLfloat max) {
	limit[0][2] = min;
	limit[1][2] = max;
}

//
// FUNCIÓN: CABalljoint::ComputeMatrix()
//
// PROPÓSITO: Crea la matriz de transformación a partir de la posición, la orientación y la pose.
//
void CABalljoint::ComputeMatrix()
{
	// Formato glm::mat4[column][row]
	glm::mat4 jointm;
	jointm[0][0] = right.x;
	jointm[0][1] = right.y;
	jointm[0][2] = right.z;
	jointm[0][3] = 0.0;

	jointm[1][0] = up.x;
	jointm[1][1] = up.y;
	jointm[1][2] = up.z;
	jointm[1][3] = 0.0;

	jointm[2][0] = dir.x;
	jointm[2][1] = dir.y;
	jointm[2][2] = dir.z;
	jointm[2][3] = 0.0;

	jointm[3][0] = location.x;
	jointm[3][1] = location.y;
	jointm[3][2] = location.z;
	jointm[3][3] = 1.0;

	float cx = (float)cos(glm::radians(angles[0]));
	float sx = (float)sin(glm::radians(angles[0]));
	float cy = (float)cos(glm::radians(angles[1]));
	float sy = (float)sin(glm::radians(angles[1]));
	float cz = (float)cos(glm::radians(angles[2]));
	float sz = (float)sin(glm::radians(angles[2]));

	glm::mat4 posem;
	posem[0][0] = cz * cy;
	posem[1][0] = -sz * cx + cz * sy*sx;
	posem[2][0] = sz * sx + cz * sy*cx;
	posem[3][0] = 0;

	posem[0][1] = sz * cy;
	posem[1][1] = cz * cx + sz * sy*sx;
	posem[2][1] = -cz * sx + sz * sy*cx;
	posem[3][1] = 0;

	posem[0][2] = -sy;
	posem[1][2] = cy * sx;
	posem[2][2] = cy * cx;
	posem[3][2] = 0;

	posem[0][3] = 0;
	posem[1][3] = 0;
	posem[2][3] = 0;
	posem[3][3] = 1;

	glm::mat4 matrix = parentMatrix * jointm * posem;

	joint->setLocation(matrix);
	glm::mat4 mm = glm::translate(matrix, glm::vec3(0.0f, 0.0f, length / 2));
	bone->setLocation(mm);

	glm::mat4 mmHijas = glm::translate(matrix, glm::vec3(0.0f, 0.0f, length));
	for (int i = 0; i < hijas.size(); i++) {
		hijas[i]->setParentLocation(mmHijas);
	}
}

//
// FUNCIÓN: CABalljoint::createBuffers(CAVulkanState* vulkan)
//
// PROPÓSITO: Inicializa las piezas de la figura
//
void CABalljoint::initialize(CAVulkanState* vulkan)
{
	CAMaterial jointMat = {};
	jointMat.Ka = glm::vec3(1.0f, 0.0f, 0.0f);
	jointMat.Kd = glm::vec3(1.0f, 0.0f, 0.0f);
	jointMat.Ks = glm::vec3(0.8f, 0.8f, 0.8f);
	jointMat.Shininess = 16.0f;

	joint = new CASphere(10, 20, 0.1f);
	joint->initialize(vulkan);
	joint->setMaterial(jointMat);

	CAMaterial boneMat = {};
	boneMat.Ka = glm::vec3(0.0f, 0.0f, 0.8f);
	boneMat.Kd = glm::vec3(0.0f, 0.0f, 0.8f);
	boneMat.Ks = glm::vec3(0.8f, 0.8f, 0.8f);
	boneMat.Shininess = 16.0f;

	bone = new CACylinder(2, 10, 0.05f, length / 2);
	bone->initialize(vulkan);
	bone->setMaterial(boneMat);
	ComputeMatrix();
}

//
// FUNCIÓN: CABalljoint::updateUniformBuffers(CAVulkanState* vulkan, glm::mat4 view, glm::mat4 projection)
//
// PROPÓSITO: Actualiza las variables uniformes
//
void CABalljoint::updateUniformBuffers(CAVulkanState* vulkan, glm::mat4 view, glm::mat4 projection)
{
	joint->updateUniformBuffers(vulkan, view, projection);
	bone->updateUniformBuffers(vulkan, view, projection);

	for (int i = 0; i < hijas.size(); i++){
		hijas[i]->updateUniformBuffers(vulkan, view, projection);
	}
}

//
// FUNCIÓN: CABalljoint::finalize(CAVulkanState* vulkan)
//
// PROPÓSITO: Destruye los buffers de las piezas
//
void CABalljoint::finalize(CAVulkanState* vulkan)
{
	joint->finalize(vulkan);
	bone->finalize(vulkan);
	for (int i = 0; i < hijas.size(); i++) {
		hijas[i]->finalize(vulkan);
	}
}

//
// FUNCIÓN: CABalljoint::setLocation(glm::vec3 loc)
//
// PROPÓSITO: Asigna la posición de la articulación
//
void CABalljoint::setLocation(glm::vec3 loc)
{
	location = loc;
	ComputeMatrix();
}

//
// FUNCIÓN: CABalljoint::setOrientation(glm::vec3 nDir, glm::vec3 nUp)
//
// PROPÓSITO: Asigna la orientación de la articulación (con pose 0,0,0).
//
void CABalljoint::setOrientation(glm::vec3 nDir, glm::vec3 nUp)
{
	dir = nDir;
	up = nUp;
	right = glm::cross(up, dir);
	ComputeMatrix();
}

//
// FUNCIÓN: CABalljoint::setPose()
//
// PROPÓSITO: Asigna la rotación de la articulación
//
void CABalljoint::setPose(float xrot, float yrot, float zrot)
{
	if (xrot < limit[0][0]) {
		angles[0] = limit[0][0];
	}else if(xrot > limit[1][0]){
		angles[0] = limit[1][0];
	}else {
		angles[0] = xrot;
	}

	if (yrot < limit[0][1]) {
		angles[1] = limit[0][1];
	}
	else if (yrot > limit[1][1]) {
		angles[1] = limit[1][1];
	}else {
		angles[1] = yrot;
	}

	if (zrot < limit[0][2]) {
		angles[2] = limit[0][2];
	}
	else if (zrot > limit[1][2]) {
		angles[2] = limit[1][2];
	}
	else {
		angles[2] = zrot;
	}

	ComputeMatrix();
}

//
// FUNCIÓN: CABalljoint::addCommands(CAVulkanState* vulkan, VkCommandBuffer commandBuffer, int index)
//
// PROPÓSITO: Añade los comandos de dibujo
//
void CABalljoint::addCommands(CAVulkanState* vulkan, VkCommandBuffer commandBuffer, int index)
{
	joint->addCommands(vulkan, commandBuffer, index);
	bone->addCommands(vulkan, commandBuffer, index);
	for (int i = 0; i < hijas.size(); i++){
		hijas[i]->addCommands(vulkan, commandBuffer, index);
	}

}

//
// FUNCIÓN: CABalljoint::setLight(CALight l)
//
// PROPÓSITO: Asigna las propiedades de la luz
//
void CABalljoint::setLight(CALight l)
{
	joint->setLight(l);
	bone->setLight(l);
	for (int i = 0; i < hijas.size(); i++) {
		hijas[i]->setLight(l);
	}

}

void CABalljoint::setParentLocation(glm::mat4 loc) {
	parentMatrix = loc;
	ComputeMatrix();
}

std::string CABalljoint::getName()
{
	return this->name;
}

std::vector<CABalljoint*> CABalljoint::getHijas()
{
	return this->hijas;
}
