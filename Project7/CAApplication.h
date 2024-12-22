#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "CAModel.h"
#include "CAVulkanState.h"

const int WIDTH = 800;
const int HEIGHT = 600;

class CAApplication
{
public:
	void run();

private:
	GLFWwindow* window;
	CAModel* model;
	CAVulkanState* vulkan;

	// Métodos principales
	GLFWwindow* initWindow();
	void mainLoop();
	void cleanup();

	// Respuesta a eventos
	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
};


