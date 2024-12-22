#include "CACamera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//
// FUNCI�N: CACamera::CACamera()
//
// PROP�SITO: Construye una c�mara
//
// COMENTARIOS: 
//     La posici�n inicial es (0,0,0).
//     La orientaci�n incial es el sistema de coordenadas del modelo
//     El tama�o del paso inicial es 0.1
//     El t�ma�o del giro inicial es 1.0 grados
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
// FUNCI�N: CACamera::getViewMatrix()
//
// PROP�SITO: Obtiene la matriz View para situar la c�mara.
//
glm::mat4 CACamera::getViewMatrix()
{
	return glm::lookAt(Pos, Pos - Dir, Up);
}

//
// FUNCI�N: CACamera::setPosition(glm::vec3 pos)
//
// PROP�SITO: Asigna la posici�n de la c�mara con respecto al sistema de coordenadas del modelo.
//
void CACamera::setPosition(glm::vec3 pos)
{
	Pos = glm::vec3(pos);
}

//
// FUNCI�N: CACamera::setDirection(glm::vec3 dir, glm::vec3 up)
//
// PROP�SITO: Asigna la orientaci�n de la c�mara.
//
void CACamera::setDirection(glm::vec3 dir, glm::vec3 up)
{
	Dir = glm::vec3(dir);
	Up = glm::vec3(up);
	Right = glm::cross(Up, Dir);
}

//
// FUNCI�N: CACamera::setMoveStep(float step)
//
// PROP�SITO: Asigna el avance en cada paso.
//
void CACamera::setMoveStep(float step)
{
	moveStep = step;
}

//
// FUNCI�N: CACamera::setTurnStep(float step)
//
// PROP�SITO: Asigna el �ngulo de giro en cada paso.
//
void CACamera::setTurnStep(float step)
{
	turnStep = step;
	cosAngle = (float)cos(glm::radians(turnStep));
	sinAngle = (float)sin(glm::radians(turnStep));
}

//
// FUNCI�N: CACamera::getPosition()
//
// PROP�SITO: Obtiene la posici�n de la c�mara.
//
glm::vec3 CACamera::getPosition()
{
	return Pos;
}

//
// FUNCI�N: CACamera::getDirection()
//
// PROP�SITO: Obtiene la orientaci�n de la c�mara (eje Z).
//
glm::vec3 CACamera::getDirection()
{
	return Dir;
}

//
// FUNCI�N: CACamera::getUpDirection()
//
// PROP�SITO: Obtiene la orientaci�n cenital de la c�mara (eje Y).
//
glm::vec3 CACamera::getUpDirection()
{
	return Up;
}

//
// FUNCI�N: CACamera::getMoveStep()
//
// PROP�SITO: Obtiene el avance en cada paso.
//
float CACamera::getMoveStep()
{
	return moveStep;
}

//
// FUNCI�N: CACamera::getTurnStep()
//
// PROP�SITO: Obtiene el �ngulo de giro en cada paso.
//
float CACamera::getTurnStep()
{
	return turnStep;
}

//
// FUNCI�N: CACamera::moveFront()
//
// PROP�SITO: Mueve el observador un paso (moveStep) en la direcci�n -Dir 
//
void CACamera::moveFront()
{
	Pos -= moveStep * Dir;
}

//
// FUNCI�N: CACamera::moveBack()
//
// PROP�SITO: Mueve el observador un paso (moveStep) hacia atr�s en la direcci�n Dir 
//
void CACamera::moveBack()
{
	Pos += moveStep * Dir;
}

//
// FUNCI�N: CACamera::moveLeft()
//
// PROP�SITO: Mueve el observador un paso (moveStep) hacia la izquierda. 
//
void CACamera::moveLeft()
{
	Pos -= moveStep * Right;
}

//
// FUNCI�N: CACamera::moveRight()
//
// PROP�SITO: Mueve el observador un paso (moveStep) hacia la derecha. 
//
void CACamera::moveRight()
{
	Pos += moveStep * Right;
}

//
// FUNCI�N: CACamera::moveUp()
//
// PROP�SITO: Mueve el observador un paso (moveStep) hacia arriba. 
//
void CACamera::moveUp()
{
	Pos += moveStep * Up;
}

//
// FUNCI�N: CACamera::moveDown()
//
// PROP�SITO: Mueve el observador un paso (moveStep) hacia abajo. 
//
void CACamera::moveDown()
{
	Pos -= moveStep * Up;
}

//
// FUNCI�N: CACamera::turnRight()
//
// PROP�SITO: Rota el observador un paso (angleStep) hacia su derecha.
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
// FUNCI�N: CACamera::turnLeft()
//
// PROP�SITO: Rota el observador un paso (angleStep) hacia su izquierda.
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
// FUNCI�N: CACamera::turnUp()
//
// PROP�SITO: Rota el observador un paso (angleStep) hacia arriba.
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
// FUNCI�N: CACamera::turnDown()
//
// PROP�SITO: Rota el observador un paso (angleStep) hacia abajo.
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
// FUNCI�N: CACamera::turnCW()
//
// PROP�SITO: Rota el observador un paso (angleStep) en sentido del reloj.
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
// FUNCI�N: CACamera::turnCCW()
//
// PROP�SITO: Rota el observador un paso (angleStep) en sentido contrario al reloj.
//
void CACamera::turnCCW()
{
	Up.x = cosAngle * Up.x - sinAngle * Right.x;
	Up.y = cosAngle * Up.y - sinAngle * Right.y;
	Up.z = cosAngle * Up.z - sinAngle * Right.z;

	// Right = Up x Dir
	Right = glm::cross(Up, Dir);
}

