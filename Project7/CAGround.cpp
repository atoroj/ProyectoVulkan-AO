#include "CAGround.h"

CAGround::CAGround(float l1, float l2)
{
	vertices = {
		{{ l1, 0.0f, l2 }, {0.0f, 1.0f, 0.0f}},
		{{ l1, 0.0f, -l2 }, {0.0f, 1.0f, 0.0f} },
		{{ -l1, 0.0f, -l2 }, {0.0f, 1.0f, 0.0f}},
		{{ -l1, 0.0f, l2 }, {0.0f, 1.0f, 0.0f}}
	};

	indices = {
		0, 1, 2,
		0, 2, 3
	};
}
