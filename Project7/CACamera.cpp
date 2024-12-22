#include "CACamera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//
// FUNCIÓN: CACamera::CACamera()
//
// PROPÓSITO: Construye una cámara
//
// COMENTARIOS: 
//     La posición inicial es (0,0,0).
//     La orientación incial es el sistema de coordenadas del modelo
//     El tamaño del paso inicial es 0.1
//     El támaño del giro inicial es 1.0 grados
//
CACamera::CACamera()
{
	Pos = glm::vec3(0.0f, 0.0f, 0.0f);
	Dir = glm::vec3(0.0f, 0.0f, 1.0f);
	Up = glm::vec3(0.0f, 1.0f, 0.0f);
	Right = glm::vec3(1.0f, 0.0f, 0.0f);

	moveStep = 0.1f;
	turnStep = 1.0f;
	cosAngle = (float)cos(glm::radians(turnStep));
	sinAngle = (float)sin(glm::radians(turnStep));
}

//
// FUNCIÓN: CACamera::getViewMatrix()
//
// PROPÓSITO: Obtiene la matriz View para situar la cámara.
//
glm::mat4 CACamera::getViewMatrix()
{
	return glm::lookAt(Pos, Pos - Dir, Up);
}

//
// FUNCIÓN: CACamera::setPosition(glm::vec3 pos)
//
// PROPÓSITO: Asigna la posición de la cámara con respecto al sistema de coordenadas del modelo.
//
void CACamera::setPosition(glm::vec3 pos)
{
	Pos = glm::vec3(pos);
}

//
// FUNCIÓN: CACamera::setDirection(glm::vec3 dir, glm::vec3 up)
//
// PROPÓSITO: Asigna la orientación de la cámara.
//
void CACamera::setDirection(glm::vec3 dir, glm::vec3 up)
{
	Dir = glm::vec3(dir);
	Up = glm::vec3(up);
	Right = glm::cross(Up, Dir);
}

//
// FUNCIÓN: CACamera::setMoveStep(float step)
//
// PROPÓSITO: Asigna el avance en cada paso.
//
void CACamera::setMoveStep(float step)
{
	moveStep = step;
}

//
// FUNCIÓN: CACamera::setTurnStep(float step)
//
// PROPÓSITO: Asigna el ángulo de giro en cada paso.
//
void CACamera::setTurnStep(float step)
{
	turnStep = step;
	cosAngle = (float)cos(glm::radians(turnStep));
	sinAngle = (float)sin(glm::radians(turnStep));
}

//
// FUNCIÓN: CACamera::getPosition()
//
// PROPÓSITO: Obtiene la posición de la cámara.
//
glm::vec3 CACamera::getPosition()
{
	return Pos;
}

//
// FUNCIÓN: CACamera::getDirection()
//
// PROPÓSITO: Obtiene la orientación de la cámara (eje Z).
//
glm::vec3 CACamera::getDirection()
{
	return Dir;
}

//
// FUNCIÓN: CACamera::getUpDirection()
//
// PROPÓSITO: Obtiene la orientación cenital de la cámara (eje Y).
//
glm::vec3 CACamera::getUpDirection()
{
	return Up;
}

//
// FUNCIÓN: CACamera::getMoveStep()
//
// PROPÓSITO: Obtiene el avance en cada paso.
//
float CACamera::getMoveStep()
{
	return moveStep;
}

//
// FUNCIÓN: CACamera::getTurnStep()
//
// PROPÓSITO: Obtiene el ángulo de giro en cada paso.
//
float CACamera::getTurnStep()
{
	return turnStep;
}

//
// FUNCIÓN: CACamera::moveFront()
//
// PROPÓSITO: Mueve el observador un paso (moveStep) en la dirección -Dir 
//
void CACamera::moveFront()
{
	Pos -= moveStep * Dir;
}

//
// FUNCIÓN: CACamera::moveBack()
//
// PROPÓSITO: Mueve el observador un paso (moveStep) hacia atrás en la dirección Dir 
//
void CACamera::moveBack()
{
	Pos += moveStep * Dir;
}

//
// FUNCIÓN: CACamera::moveLeft()
//
// PROPÓSITO: Mueve el observador un paso (moveStep) hacia la izquierda. 
//
void CACamera::moveLeft()
{
	Pos -= moveStep * Right;
}

//
// FUNCIÓN: CACamera::moveRight()
//
// PROPÓSITO: Mueve el observador un paso (moveStep) hacia la derecha. 
//
void CACamera::moveRight()
{
	Pos += moveStep * Right;
}

//
// FUNCIÓN: CACamera::moveUp()
//
// PROPÓSITO: Mueve el observador un paso (moveStep) hacia arriba. 
//
void CACamera::moveUp()
{
	Pos += moveStep * Up;
}

//
// FUNCIÓN: CACamera::moveDown()
//
// PROPÓSITO: Mueve el observador un paso (moveStep) hacia abajo. 
//
void CACamera::moveDown()
{
	Pos -= moveStep * Up;
}

//
// FUNCIÓN: CACamera::turnRight()
//
// PROPÓSITO: Rota el observador un paso (angleStep) hacia su derecha.
//
void CACamera::turnRight()
{
	Dir.x = cosAngle * Dir.x - sinAngle * Right.x;
	Dir.y = cosAngle * Dir.y - sinAngle * Right.y;
	Dir.z = cosAngle * Dir.z - sinAngle * Right.z;

	// Right = Up x Dir
	Right = glm::cross(Up, Dir);
}

//
// FUNCIÓN: CACamera::turnLeft()
//
// PROPÓSITO: Rota el observador un paso (angleStep) hacia su izquierda.
//
void CACamera::turnLeft()
{
	Dir.x = cosAngle * Dir.x + sinAngle * Right.x;
	Dir.y = cosAngle * Dir.y + sinAngle * Right.y;
	Dir.z = cosAngle * Dir.z + sinAngle * Right.z;

	// Right = Up x Dir
	Right = glm::cross(Up, Dir);
}

//
// FUNCIÓN: CACamera::turnUp()
//
// PROPÓSITO: Rota el observador un paso (angleStep) hacia arriba.
//
void CACamera::turnUp()
{
	Dir.x = cosAngle * Dir.x - sinAngle * Up.x;
	Dir.y = cosAngle * Dir.y - sinAngle * Up.y;
	Dir.z = cosAngle * Dir.z - sinAngle * Up.z;

	// Up = Dir x Right
	Up = glm::cross(Dir, Right);
}

//
// FUNCIÓN: CACamera::turnDown()
//
// PROPÓSITO: Rota el observador un paso (angleStep) hacia abajo.
//
void CACamera::turnDown()
{
	Dir.x = cosAngle * Dir.x + sinAngle * Up.x;
	Dir.y = cosAngle * Dir.y + sinAngle * Up.y;
	Dir.z = cosAngle * Dir.z + sinAngle * Up.z;

	// Up = Dir x Right
	Up = glm::cross(Dir, Right);
}

//
// FUNCIÓN: CACamera::turnCW()
//
// PROPÓSITO: Rota el observador un paso (angleStep) en sentido del reloj.
//
void CACamera::turnCW()
{
	Up.x = cosAngle * Up.x + sinAngle * Right.x;
	Up.y = cosAngle * Up.y + sinAngle * Right.y;
	Up.z = cosAngle * Up.z + sinAngle * Right.z;

	// Right = Up x Dir
	Right = glm::cross(Up, Dir);
}

//
// FUNCIÓN: CACamera::turnCCW()
//
// PROPÓSITO: Rota el observador un paso (angleStep) en sentido contrario al reloj.
//
void CACamera::turnCCW()
{
	Up.x = cosAngle * Up.x - sinAngle * Right.x;
	Up.y = cosAngle * Up.y - sinAngle * Right.y;
	Up.z = cosAngle * Up.z - sinAngle * Right.z;

	// Right = Up x Dir
	Right = glm::cross(Up, Dir);
}

