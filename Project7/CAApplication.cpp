#include "CAApplication.h"
#include <windows.h>
#include <iostream>
#include <vector>
#include <glm/common.hpp>

//
// FUNCIÓN: CAApplication::run()
//
// PROPÓSITO: Ejecuta la aplicación
//
void CAApplication::run()
{
	this->window = initWindow();
	this->vulkan = new CAVulkanState(window);
	this->model = new CAModel(vulkan);
	this->vulkan->setModel(model);
	mainLoop();
	cleanup();
}

//
// FUNCIÓN: CAApplication::initWindow()
//
// PROPÓSITO: Inicializa la ventana
//
GLFWwindow* CAApplication::initWindow()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* w = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	glfwSetWindowUserPointer(w, this);
	glfwSetFramebufferSizeCallback(w, framebufferResizeCallback);
	glfwSetKeyCallback(w, keyCallback);
	glfwSetCursorPosCallback(w, cursorPositionCallback);
	glfwSetMouseButtonCallback(w, mouseButtonCallback);
	return w;
}

//
// FUNCIÓN: CAApplication::mainLoop()
//
// PROPÓSITO: Bucle principal que procesa los eventos de la aplicación
//
void CAApplication::mainLoop()
{
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		vulkan->draw();
	}
}

//
// FUNCIÓN: CAApplication::cleanup()
//
// PROPÓSITO: Libera los recursos y finaliza la aplicación
//
void CAApplication::cleanup()
{
	delete model;
	delete vulkan;
	glfwDestroyWindow(window);
	glfwTerminate();
}

//
// FUNCIÓN: CAApplication::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
//
// PROPÓSITO: Respuesta a un evento de teclado sobre la aplicación
//
void CAApplication::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	CAApplication* app = (CAApplication*)glfwGetWindowUserPointer(window);
	if (action == GLFW_PRESS || action == GLFW_REPEAT) app->model->key_pressed(key);
}

//
// FUNCIÓN: CAApplication::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
//
// PROPÓSITO: Respuesta a un evento de ratón sobre la aplicación
//
void CAApplication::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	CAApplication* app = (CAApplication*)glfwGetWindowUserPointer(window);
	if (action == GLFW_PRESS || action == GLFW_REPEAT) app->model->mouse_button(button, action);
}

//
// FUNCIÓN: CAApplication::cursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
//
// PROPÓSITO: Respuesta a un evento de movimiento del cursor sobre la aplicación
//
void CAApplication::cursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
	CAApplication* app = (CAApplication*)glfwGetWindowUserPointer(window);
	app->model->mouse_move(xpos, ypos);
}

//
// FUNCIÓN: CAApplication::framebufferResizeCallback(GLFWwindow* window, int width, int height)
//
// PROPÓSITO: Respuesta a un evento de redimensionamiento de la ventana de la aplicación
//
void CAApplication::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	CAApplication* app = (CAApplication*)glfwGetWindowUserPointer(window);
	app->vulkan->windowResized(width, height);
	if (height != 0)
	{
		double aspect = (double)width / (double)height;
		app->model->aspect_ratio(aspect);
	}
}
