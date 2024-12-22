#include "CAFigure.h"
#include "CAVertex.h"
#include "CATransform.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

//
// FUNCIÓN: CAFigure::initialize(CAVulkanState* vulkan)
//
// PROPÓSITO: Crea el Vertex Buffer
//
void CAFigure::initialize(CAVulkanState* vulkan)
{
	location = glm::mat4(1.0f);

	size_t vertexSize = sizeof(CAVertex) * vertices.size();
	vulkan->createVertexBuffer(vertexSize, vertices.data(), &vbo);

	size_t indexSize = sizeof(indices[0]) * indices.size();
	vulkan->createIndexBuffer(indexSize, indices.data(), &ibo);

	size_t transformBufferSize = sizeof(CATransform);
	vulkan->createUniformBuffer(transformBufferSize, &transformBuffer);

	size_t lightBufferSize = sizeof(CALight);
	vulkan->createUniformBuffer(lightBufferSize, &lightBuffer);

	size_t materialBufferSize = sizeof(CAMaterial);
	vulkan->createUniformBuffer(materialBufferSize, &materialBuffer);


	CAUniformBuffer* buffers[] = { &transformBuffer, &lightBuffer, &materialBuffer };
	size_t sizes[] = { transformBufferSize, lightBufferSize, materialBufferSize };

	vulkan->createDescriptorSets(&descriptorPool, &descriptorSets, buffers, sizes, 3);
}

//
// FUNCIÓN: CAFigure::finalize(VkDevice device)
//
// PROPÓSITO: Libera los buffers de la figura
//
void CAFigure::finalize(CAVulkanState* vulkan)
{
	vulkan->destroyVertexBuffer(vbo);
	vulkan->destroyIndexBuffer(ibo);
	vulkan->destroyUniformBuffer(transformBuffer);
	vulkan->destroyUniformBuffer(lightBuffer);
	vulkan->destroyUniformBuffer(materialBuffer);
	vulkan->destroyDescriptorSets(descriptorPool, descriptorSets);
}

//
// FUNCIÓN: CAFigure::addCommands(CAVulkanState* vulkan, VkCommandBuffer commandBuffer, int index)
//
// PROPÓSITO: Añade los comandos de renderizado al command buffer
//
void CAFigure::addCommands(CAVulkanState* vulkan, VkCommandBuffer commandBuffer, int index)
{
	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vbo.buffer, &offset);
	vkCmdBindIndexBuffer(commandBuffer, ibo.buffer, 0, VK_INDEX_TYPE_UINT16);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan->getPipelineLayout(), 0, 1, &descriptorSets[index], 0, nullptr);
	vkCmdDrawIndexed(commandBuffer, (uint32_t)indices.size(), 1, 0, 0, 0);
}

//
// FUNCIÓN: CAFigure::updateUniformBuffers(CAVulkanState* vulkan, glm::mat4 view, glm::mat4 projection)
//
// PROPÓSITO: Actualiza las variables uniformes sobre una imagen del swapchain
//
void CAFigure::updateUniformBuffers(CAVulkanState* vulkan, glm::mat4 view, glm::mat4 projection)
{
	CATransform transform;
	transform.MVP = projection * view * location;
	transform.ModelViewMatrix = view * location;
	transform.ViewMatrix = view;

	vulkan->updateUniformBuffer(sizeof(CATransform), &transform, transformBuffer);
	vulkan->updateUniformBuffer(sizeof(CALight), &light, lightBuffer);
	vulkan->updateUniformBuffer(sizeof(CAMaterial), &material, materialBuffer);
}

//
// FUNCIÓN: CAFigure::setLight(CALight l)
//
// PROPÓSITO: Asigna las propiedades de la luz que incide en la figura
//
void CAFigure::setLight(CALight l)
{
	this->light = l;
}

//
// FUNCIÓN: CAFigure::setMaterial(CAMaterial m)
//
// PROPÓSITO: Asigna las propiedades del material del que está formada la figura
//
void CAFigure::setMaterial(CAMaterial m)
{
	this->material = m;
}

//
// FUNCIÓN: CAFigure::resetLocation()
//
// PROPÓSITO: Resetea la matriz de localización (Model).
//
void CAFigure::resetLocation()
{
	location = glm::mat4(1.0f);
}

//
// FUNCIÓN: CAFigure::setLocation()
//
// PROPÓSITO: Resetea la matriz de localización (Model).
//
void CAFigure::setLocation(glm::mat4 m)
{
	location = glm::mat4(m);
}


//
// FUNCIÓN: CAFigure::translate(glm::vec3 t)
//
// PROPÓSITO: Añade un desplazamiento la matriz de localización (Model).
//
void CAFigure::translate(glm::vec3 t)
{
	location = glm::translate(location, t);
}

//
// FUNCIÓN: CAFigure::rotate(float angle, glm::vec3 axis)
//
// PROPÓSITO: Añade una rotación la matriz de localización (Model).
//
void CAFigure::rotate(float angle, glm::vec3 axis)
{
	location = glm::rotate(location, glm::radians(angle), axis);
}
