#pragma once

#include <cmath>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <GLES3/gl3.h>


#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct GLMeshData
{
	GLMeshData();
	~GLMeshData();

	void createBox(float w, float h, float l);
	void createPlane(float base, float size, float uvScale = 1.0f);
	void createSphere(float rad, uint32_t hSegs, uint32_t vSegs);
	void createCone(float radius, float height, uint32_t segments);
    void createCylinder(float radius, float height, uint32_t segments);
    void createTrapezoid(float baseWidth, float topWidth, float height, float depth);

	void renderInstanced(const std::vector<glm::mat4>& modelMatrices);
	void render();
	void clear();


	void createGLObjects();

	GLenum primitiveType;
	unsigned int numVertices;
	unsigned int numPrimitives;

	GLuint meshVAO;
	GLuint meshIBO;
	GLuint meshVBO_pos;
	GLuint meshVBO_uv;
	GLuint instanceVBO;
	
	std::vector<GLuint> indexData;
	std::vector<GLfloat> posData;
	std::vector<GLfloat> uvData;
};

