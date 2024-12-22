#include "CASphere.h"

CASphere::CASphere(GLint p, GLint m, GLfloat r)
{
	int numFaces = 2 * m * (p - 1);         // Number of faces
	int numVertices = (m + 1) * (p + 1);  // Number of vertices

	vertices.resize(numVertices);
	indices.resize(numFaces * 3);

	int verticesIndex = 0;
	int indicesIndex = 0;

	/* northern polar cap*/
	for (int j = 0; j <= m; j++)
	{
		vertices[verticesIndex] = { { 0.0f, 0.0f, r }, {0.0f, 0.0f, 1.0f} };
		verticesIndex++;
	}

	for (int i = 1; i < p; i++)
	{
		for (int j = 0; j <= m; j++)
		{
			float pCos = (float)cos(glm::radians(180.0f * i / p));
			float pSin = (float)sin(glm::radians(180.0f * i / p));
			float mCos = (float)cos(glm::radians(360.0f * j / m));
			float mSin = (float)sin(glm::radians(360.0f * j / m));

			vertices[verticesIndex] = { {pSin * mCos * r, pSin * mSin * r, pCos * r}, {pSin * mCos, pSin * mSin, pCos} };
			verticesIndex++;
		}
	}

	/* southern polar cap*/
	for (int j = 0; j <= m; j++)
	{
		vertices[verticesIndex] = { { 0.0f, 0.0f, -r}, {0.0f, 0.0f, -1.0f} };
		verticesIndex++;
	}

	/* northern polar cap*/
	for (int j = 0; j < m; j++)
	{
		indices[indicesIndex] = j;
		indices[indicesIndex + 1] = m + j + 1;
		indices[indicesIndex + 2] = m + j + 2;
		indicesIndex += 3;
	}
	for (int i = 1; i < p - 1; i++)
	{
		for (int j = 0; j < m; j++)
		{
			indices[indicesIndex] = i * (m + 1) + j;
			indices[indicesIndex + 1] = (i + 1) * (m + 1) + j;
			indices[indicesIndex + 2] = i * (m + 1) + j + 1;
			indices[indicesIndex + 3] = (i + 1) * (m + 1) + j;
			indices[indicesIndex + 4] = (i + 1) * (m + 1) + j + 1;
			indices[indicesIndex + 5] = i * (m + 1) + j + 1;
			indicesIndex += 6;
		}
	}
	for (int j = 0; j < m; j++)
	{
		indices[indicesIndex] = (p - 1) * (m + 1) + j;
		indices[indicesIndex + 1] = p * (m + 1) + j;
		indices[indicesIndex + 2] = (p - 1) * (m + 1) + j + 1;
		indicesIndex += 3;
	}
}

