#pragma once

#include <glm/glm.hpp>

typedef struct 
{
	alignas(16) glm::vec3 Ka;        // Ambient reflectivity
	alignas(16) glm::vec3 Kd;        // Difusse reflectivity
	alignas(16) glm::vec3 Ks;        // Specular reflectivity
	alignas(4) float Shininess;     // Specular shininess factor
} CAMaterial;

