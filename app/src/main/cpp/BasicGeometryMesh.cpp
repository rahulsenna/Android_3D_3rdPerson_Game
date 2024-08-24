#include "BasicGeometryMesh.h"
#include "global.h"

GLMeshData::GLMeshData()
{
	meshVAO = meshVBO_pos = meshVBO_uv = meshIBO = 0;

	numVertices = numPrimitives = 0;

	primitiveType = GL_TRIANGLES;
}

GLMeshData::~GLMeshData()
{
	clear();
}

void GLMeshData::clear()
{
	if (meshVBO_pos)
	{
		glDeleteBuffers(1, &meshVBO_pos);
		meshVBO_pos = 0;
	}

	if (meshVBO_uv)
	{
		glDeleteBuffers(1, &meshVBO_uv);
		meshVBO_uv = 0;
	}

	if (meshIBO)
	{
		glDeleteBuffers(1, &meshIBO);
		meshIBO = 0;
	}

	if (meshVAO)
	{
		glDeleteVertexArrays(1, &meshVAO);
		meshVAO = 0;
	}
}

void GLMeshData::createPlane(float base, float size, float uvScale)
{
	numPrimitives = 2;

	posData = {
		-size,base,-size,
		 size,base,-size,
		 size,base, size,
		-size,base, size,
	};

	uvData = {
		0.0f*uvScale, 0.0f*uvScale,
		1.0f*uvScale, 0.0f*uvScale,
		1.0f*uvScale, 1.0f*uvScale,
		0.0f*uvScale, 1.0f*uvScale,
	};

	indexData = {
		0, 1, 2,
		2, 3, 0
	};

	createGLObjects();
}

void GLMeshData::createBox(float w, float h, float l)
{
	numPrimitives = 6*2;

	w *= 0.5f;
	h *= 0.5f;
	l *= 0.5f;

	// bottom face
	posData.insert(posData.end(), { w, -h,  l}); uvData.insert(uvData.end(), {0.0f, 0.0f});
	posData.insert(posData.end(), {-w, -h,  l}); uvData.insert(uvData.end(), {1.0f, 0.0f});
	posData.insert(posData.end(), {-w, -h, -l}); uvData.insert(uvData.end(), {1.0f, 1.0f});
	posData.insert(posData.end(), { w, -h, -l}); uvData.insert(uvData.end(), {0.0f, 1.0f});

	// top face
	posData.insert(posData.end(), {-w,  h,  l}); uvData.insert(uvData.end(), {0.0f, 0.0f});
	posData.insert(posData.end(), { w,  h,  l}); uvData.insert(uvData.end(), {1.0f, 0.0f});
	posData.insert(posData.end(), { w,  h, -l}); uvData.insert(uvData.end(), {1.0f, 1.0f});
	posData.insert(posData.end(), {-w,  h, -l}); uvData.insert(uvData.end(), {0.0f, 1.0f});

	// left face
	posData.insert(posData.end(), {-w, -h, -l}); uvData.insert(uvData.end(), {0.0f, 0.0f});
	posData.insert(posData.end(), {-w, -h,  l}); uvData.insert(uvData.end(), {1.0f, 0.0f});
	posData.insert(posData.end(), {-w,  h,  l}); uvData.insert(uvData.end(), {1.0f, 1.0f});
	posData.insert(posData.end(), {-w,  h, -l}); uvData.insert(uvData.end(), {0.0f, 1.0f});

	// right face
	posData.insert(posData.end(), { w, -h,  l}); uvData.insert(uvData.end(), {0.0f, 0.0f});
	posData.insert(posData.end(), { w, -h, -l}); uvData.insert(uvData.end(), {1.0f, 0.0f});
	posData.insert(posData.end(), { w,  h, -l}); uvData.insert(uvData.end(), {1.0f, 1.0f});
	posData.insert(posData.end(), { w,  h,  l}); uvData.insert(uvData.end(), {0.0f, 1.0f});

	// front face
	posData.insert(posData.end(), {-w, -h,  l}); uvData.insert(uvData.end(), {0.0f, 0.0f});
	posData.insert(posData.end(), { w, -h,  l}); uvData.insert(uvData.end(), {1.0f, 0.0f});
	posData.insert(posData.end(), { w,  h,  l}); uvData.insert(uvData.end(), {1.0f, 1.0f});
	posData.insert(posData.end(), {-w,  h,  l}); uvData.insert(uvData.end(), {0.0f, 1.0f});

	// back face
	posData.insert(posData.end(), { w, -h, -l}); uvData.insert(uvData.end(), {0.0f, 0.0f});
	posData.insert(posData.end(), {-w, -h, -l}); uvData.insert(uvData.end(), {1.0f, 0.0f});
	posData.insert(posData.end(), {-w,  h, -l}); uvData.insert(uvData.end(), {1.0f, 1.0f});
	posData.insert(posData.end(), { w,  h, -l}); uvData.insert(uvData.end(), {0.0f, 1.0f});

	std::vector< unsigned int > tri00;
	std::vector< unsigned int > tri01;
	tri00.push_back(0); tri00.push_back(1); tri00.push_back(2);
	tri01.push_back(0); tri01.push_back(2); tri01.push_back(3);

	std::vector< unsigned int > tri02;
	std::vector< unsigned int > tri03;
	tri02.push_back(4); tri02.push_back(5); tri02.push_back(6);
	tri03.push_back(4); tri03.push_back(6); tri03.push_back(7);

	std::vector< unsigned int > tri04;
	std::vector< unsigned int > tri05;
	tri04.push_back(8); tri04.push_back(9); tri04.push_back(10);
	tri05.push_back(8); tri05.push_back(10); tri05.push_back(11);

	std::vector< unsigned int > tri06;
	std::vector< unsigned int > tri07;
	tri06.push_back(12); tri06.push_back(13); tri06.push_back(14);
	tri07.push_back(12); tri07.push_back(14); tri07.push_back(15);

	std::vector< unsigned int > tri08;
	std::vector< unsigned int > tri09;
	tri08.push_back(16); tri08.push_back(17); tri08.push_back(18);
	tri09.push_back(16); tri09.push_back(18); tri09.push_back(19);

	std::vector< unsigned int > tri10;
	std::vector< unsigned int > tri11;
	tri10.push_back(20); tri10.push_back(21); tri10.push_back(22);
	tri11.push_back(20); tri11.push_back(22); tri11.push_back(23);

	indexData.insert(indexData.end(), tri00.begin(), tri00.end());
	indexData.insert(indexData.end(), tri01.begin(), tri01.end());
	indexData.insert(indexData.end(), tri02.begin(), tri02.end());
	indexData.insert(indexData.end(), tri03.begin(), tri03.end());
	indexData.insert(indexData.end(), tri04.begin(), tri04.end());
	indexData.insert(indexData.end(), tri05.begin(), tri05.end());
	indexData.insert(indexData.end(), tri06.begin(), tri06.end());
	indexData.insert(indexData.end(), tri07.begin(), tri07.end());
	indexData.insert(indexData.end(), tri08.begin(), tri08.end());
	indexData.insert(indexData.end(), tri09.begin(), tri09.end());
	indexData.insert(indexData.end(), tri10.begin(), tri10.end());
	indexData.insert(indexData.end(), tri11.begin(), tri11.end());

	createGLObjects();
}

void GLMeshData::createSphere(float rad, uint32_t hSegs, uint32_t vSegs)
{
	numPrimitives = hSegs * vSegs * 2;

	float dphi = (float)(2.0*M_PI) / (float)(hSegs);
	float dtheta = (float)(M_PI) / (float)(vSegs);

	for (uint32_t v = 0; v <= vSegs; ++v)
	{
		float theta = v * dtheta;

		for (uint32_t h = 0; h <= hSegs; ++h)
		{
			float phi = h * dphi;

			float x = std::sin(theta) * std::cos(phi);
			float y = std::cos(theta);
			float z = std::sin(theta) * std::sin(phi);

			posData.insert(posData.end(), { rad * x, rad * y, rad * z });
			uvData.insert(uvData.end(), { 1.0f - (float)h / hSegs, (float)v / vSegs });
		}
	}

	for (uint32_t v = 0; v < vSegs; v++)
	{
		for (uint32_t h = 0; h < hSegs; h++)
		{
			uint32_t topRight = v * (hSegs + 1) + h;
			uint32_t topLeft = v * (hSegs + 1) + h + 1;
			uint32_t lowerRight = (v + 1) * (hSegs + 1) + h;
			uint32_t lowerLeft = (v + 1) * (hSegs + 1) + h + 1;

			std::vector< unsigned int > tri0;
			std::vector< unsigned int > tri1;

			tri0.push_back(lowerLeft);
			tri0.push_back(lowerRight);
			tri0.push_back(topRight);

			tri1.push_back(lowerLeft);
			tri1.push_back(topRight);
			tri1.push_back(topLeft);

			indexData.insert(indexData.end(), tri0.begin(), tri0.end());
			indexData.insert(indexData.end(), tri1.begin(), tri1.end());
		}
	}

	createGLObjects();
}

void GLMeshData::createCone(float radius, float height, uint32_t segments)
{
    numPrimitives = segments * 2;

    // Create the base circle
    for (uint32_t i = 0; i <= segments; ++i)
    {
        float angle = 2.0f * M_PI * i / segments;
        float x = radius * std::cos(angle);
        float z = radius * std::sin(angle);
        
        posData.insert(posData.end(), {x, 0, z});
        uvData.insert(uvData.end(), {(float)i / segments, 0});
    }

    // Add the apex
    posData.insert(posData.end(), {0, height, 0});
    uvData.insert(uvData.end(), {0.5f, 1.0f});

    // Create the base triangles
    for (uint32_t i = 0; i < segments; ++i)
    {
        indexData.insert(indexData.end(), {0, i + 1, i + 2});
    }

    // Create the side triangles
    uint32_t apexIndex = segments + 1;
    for (uint32_t i = 1; i <= segments; ++i)
    {
        indexData.insert(indexData.end(), {i, apexIndex, i + 1});
    }

    createGLObjects();
}

void GLMeshData::createCylinder(float radius, float height, uint32_t segments)
{
    numPrimitives = segments * 4;

    // Create the bottom circle
    for (uint32_t i = 0; i <= segments; ++i)
    {
        float angle = 2.0f * M_PI * i / segments;
        float x = radius * std::cos(angle);
        float z = radius * std::sin(angle);
        
        posData.insert(posData.end(), {x, 0, z});
        uvData.insert(uvData.end(), {(float)i / segments, 0});
    }

    // Create the top circle
    for (uint32_t i = 0; i <= segments; ++i)
    {
        float angle = 2.0f * M_PI * i / segments;
        float x = radius * std::cos(angle);
        float z = radius * std::sin(angle);
        
        posData.insert(posData.end(), {x, height, z});
        uvData.insert(uvData.end(), {(float)i / segments, 1});
    }

    // Create the bottom triangles
    for (uint32_t i = 0; i < segments; ++i)
    {
        indexData.insert(indexData.end(), {0, i + 1, i + 2});
    }

    // Create the top triangles
    uint32_t topStart = segments + 1;
    for (uint32_t i = 0; i < segments; ++i)
    {
        indexData.insert(indexData.end(), {topStart, topStart + i + 1, topStart + i + 2});
    }

    // Create the side triangles
    for (uint32_t i = 0; i < segments; ++i)
    {
        uint32_t bottomLeft = i + 1;
        uint32_t bottomRight = i + 2;
        uint32_t topLeft = topStart + i + 1;
        uint32_t topRight = topStart + i + 2;

        indexData.insert(indexData.end(), {bottomLeft, topLeft, bottomRight});
        indexData.insert(indexData.end(), {bottomRight, topLeft, topRight});
    }
    createGLObjects();
}


void GLMeshData::createTrapezoid(float baseWidth, float topWidth, float height, float depth)
{
    numPrimitives = 12;  // 6 faces, 2 triangles each

    float baseHalfWidth = baseWidth / 2.0f;
    float topHalfWidth = topWidth / 2.0f;
    float halfDepth = depth / 2.0f;

    // Bottom face
    posData.insert(posData.end(), {-baseHalfWidth, 0, -halfDepth});
    posData.insert(posData.end(), { baseHalfWidth, 0, -halfDepth});
    posData.insert(posData.end(), { baseHalfWidth, 0,  halfDepth});
    posData.insert(posData.end(), {-baseHalfWidth, 0,  halfDepth});

    // Top face
    posData.insert(posData.end(), {-topHalfWidth, height, -halfDepth});
    posData.insert(posData.end(), { topHalfWidth, height, -halfDepth});
    posData.insert(posData.end(), { topHalfWidth, height,  halfDepth});
    posData.insert(posData.end(), {-topHalfWidth, height,  halfDepth});

    // UV coordinates (simplified)
    for (int i = 0; i < 8; ++i)
    {
        uvData.insert(uvData.end(), {(i % 2) * 1.0f, (i / 2) % 2 * 1.0f});
    }

    // Indices for all faces
    std::vector<uint32_t> faceIndices = {
        0, 1, 2, 2, 3, 0,  // Bottom
        4, 5, 6, 6, 7, 4,  // Top
        0, 4, 7, 7, 3, 0,  // Left
        1, 5, 6, 6, 2, 1,  // Right
        0, 1, 5, 5, 4, 0,  // Front
        3, 2, 6, 6, 7, 3   // Back
    };

    indexData.insert(indexData.end(), faceIndices.begin(), faceIndices.end());

    createGLObjects();
}

void GLMeshData::createQuad()
{
    float quadVertices[] = {
        // positions   // texture coords
         0.0f,  1.0f,  0.0f, 1.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
         0.0f,  0.0f,  0.0f, 0.0f,
         1.0f,  0.0f,  1.0f, 0.0f
    };

    glGenVertexArrays(1, &meshVAO);
    glGenBuffers(1, &meshVBO_pos);
    glBindVertexArray(meshVAO);
    glBindBuffer(GL_ARRAY_BUFFER, meshVBO_pos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

const GLsizei MAX_INSTANCES = 1000; // Or whatever maximum number of instances you expect
void GLMeshData::createGLObjects()
{
	// create vertex array
	glGenVertexArrays(1, &meshVAO);
	glBindVertexArray(meshVAO);
	CHECK_GL;

	// create vertex buffer objects for pos, uv
	glGenBuffers(1, &meshVBO_pos);
	glBindBuffer(GL_ARRAY_BUFFER, meshVBO_pos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * posData.size(), posData.data(), GL_STATIC_DRAW);
	CHECK_GL;

	glGenBuffers(1, &meshVBO_uv);
	glBindBuffer(GL_ARRAY_BUFFER, meshVBO_uv);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * uvData.size(), uvData.data(), GL_STATIC_DRAW);
	CHECK_GL;

	// create index buffer object
	glGenBuffers(1, &meshIBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshIBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indexData.size(), indexData.data(), GL_STATIC_DRAW);
	CHECK_GL;

	// Create instance buffer
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    // We'll fill this buffer later, so just allocate it for now
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * MAX_INSTANCES, nullptr, GL_DYNAMIC_DRAW);
    CHECK_GL;

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	CHECK_GL;


	GLuint loc_pos = 0;
	GLuint loc_uv = 1;
    GLuint loc_instance_matrix = 2; // New location for instance matrix


	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshIBO);

	glBindBuffer(GL_ARRAY_BUFFER, meshVBO_pos);
	glVertexAttribPointer(loc_pos, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glBindBuffer(GL_ARRAY_BUFFER, meshVBO_uv);
	glVertexAttribPointer(loc_uv, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	CHECK_GL;

	// Set up instance matrix attribute
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    for (int i = 0; i < 4; ++i)
    {
        glEnableVertexAttribArray(loc_instance_matrix + i);
        glVertexAttribPointer(loc_instance_matrix + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4),
                              (void*)(sizeof(glm::vec4) * i));
        glVertexAttribDivisor(loc_instance_matrix + i, 1);
    }
    CHECK_GL;

	glEnableVertexAttribArray(loc_pos);
	glEnableVertexAttribArray(loc_uv);
	CHECK_GL;

	glBindVertexArray(0);

	glDisableVertexAttribArray(loc_pos);
	glDisableVertexAttribArray(loc_uv);
	for (int i = 0; i < 4; ++i)
    {
        glDisableVertexAttribArray(loc_instance_matrix + i);
    }


	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	CHECK_GL;
}

void GLMeshData::render()
{
	glBindVertexArray(meshVAO);
	CHECK_GL;

	glDrawElements(primitiveType, 3 * numPrimitives, GL_UNSIGNED_INT, (void*)0);
	CHECK_GL;

	glBindVertexArray(0);
	CHECK_GL;
}

void GLMeshData::renderInstanced(const std::vector<glm::mat4>& modelMatrices)
{
    // Update instance buffer
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::mat4) * modelMatrices.size(), modelMatrices.data());

    // Bind VAO and draw
    glBindVertexArray(meshVAO);
    glDrawElementsInstanced(GL_TRIANGLES, indexData.size(), GL_UNSIGNED_INT, 0, modelMatrices.size());
    glBindVertexArray(0);
}

void GLMeshData::renderQuad()
{
	glBindVertexArray(meshVAO);
	CHECK_GL;

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	CHECK_GL;

	glBindVertexArray(0);
	CHECK_GL;
}