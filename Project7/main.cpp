#include "CAApplication.h"
#include <iostream>
#include <stdexcept>

//
// PROYECTO: Project7
//
// DESCRIPCI�N: A�ade una articulaci�n de tipo balljoint
//              Define una escena con una cruz y dos articulaciones
//
int main()
{
	CAApplication app;

	try
	{
		app.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}