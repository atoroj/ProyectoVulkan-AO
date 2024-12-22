#pragma once

#include <glm/glm.hpp>

class CACamera {
public:
	CACamera();
	glm::mat4 getViewMatrix();

	void setPosition(glm::vec3 pos);
	void setDirection(glm::vec3 dir, glm::vec3 up);
	void setMoveStep(float step);
	void setTurnStep(float step);

	glm::vec3 getPosition();
	glm::vec3 getDirection();
	glm::vec3 getUpDirection();
	float getMoveStep();
	float getTurnStep();

	void moveFront();
	void moveBack();
	void moveRight();
	void moveLeft();
	void moveUp();
	void moveDown();

	void turnRight();
	void turnLeft();
	void turnUp();
	void turnDown();
	void turnCW();
	void turnCCW();

private:
	glm::vec3 Pos;
	glm::vec3 Dir;
	glm::vec3 Up;
	glm::vec3 Right;

	float moveStep;
	float turnStep;
	float cosAngle;
	float sinAngle;
};

