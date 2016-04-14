#ifndef MESH
#define MESH

#include "rt3d.h"


class Mesh {
public:

	Mesh() {}

protected:

	GLfloat verts[] = { -0.25, -0.25f, -0.25f,
		-0.25, 0.25f, -0.25f,
		0.25f, 0.25f, -0.25f,
		0.25f, -0.25f, -0.25f,
		-0.25f, -0.25f, 0.25f,
		-0.25f, 0.25f, 0.25f,
		0.25f, 0.25f, 0.25f,
		0.25f, -0.25f, 0.25f };

	rt3d::materialStruct material;
	GLuint texture;

};



#endif