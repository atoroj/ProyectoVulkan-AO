#pragma once

#include <glm/glm.hpp>

typedef struct
{
	alignas(16) glm::vec3 Ldir;	// Light direction
	alignas(16) glm::vec3 La;   // Ambient intensity
	alignas(16) glm::vec3 Ld;   // Difusse intensity
	alignas(16) glm::vec3 Ls;   // Specular intensity
} CALight;
