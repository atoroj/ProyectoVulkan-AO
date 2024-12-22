#pragma once

#include <glm/glm.hpp>

typedef struct
{
	alignas(16) glm::mat4 MVP;
	alignas(16) glm::mat4 ViewMatrix;
	alignas(16) glm::mat4 ModelViewMatrix;
} CATransform;
