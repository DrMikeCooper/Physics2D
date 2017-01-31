#include <gl_core_4_4.h>
#include <iostream>
#include <fstream>

#include "Model.h"
#include "Camera.h"

bool Model::LoadObjFile(const char* name)
{
	std::string err;
	if (!tinyobj::LoadObj(shapes, materials, err, name))
	{
		printf(err.c_str());
		return false;
	}
	CreateBuffers();
}

void Model::CreateBuffers()
{
	m_gl_info.resize(shapes.size());
	for (unsigned int mesh_index = 0; mesh_index < shapes.size(); ++mesh_index)
	{
		glGenVertexArrays(1, &m_gl_info[mesh_index].m_VAO); // get a handle for the master handle
		glGenBuffers(1, &m_gl_info[mesh_index].m_VBO); // get a handle for the Vertex buffer
		glGenBuffers(1, &m_gl_info[mesh_index].m_IBO); // get a handle for the index buffer

		glBindVertexArray(m_gl_info[mesh_index].m_VAO); // following operations will happen to the master handle

														// create a big array that holds our positions, normals and UV coords
		unsigned int float_count = shapes[mesh_index].mesh.positions.size();
		float_count += shapes[mesh_index].mesh.normals.size();
		float_count += shapes[mesh_index].mesh.texcoords.size();
		std::vector<float> vertex_data;
		vertex_data.reserve(float_count);
		vertex_data.insert(vertex_data.end(), shapes[mesh_index].mesh.positions.begin(), shapes[mesh_index].mesh.positions.end());
		vertex_data.insert(vertex_data.end(), shapes[mesh_index].mesh.normals.begin(), shapes[mesh_index].mesh.normals.end());
		vertex_data.insert(vertex_data.end(), shapes[mesh_index].mesh.texcoords.begin(), shapes[mesh_index].mesh.texcoords.end());

		m_gl_info[mesh_index].m_index_count = shapes[mesh_index].mesh.indices.size();

		// feed the big composite array into a OGL array accessible by the VBO handle
		glBindBuffer(GL_ARRAY_BUFFER, m_gl_info[mesh_index].m_VBO);
		glBufferData(GL_ARRAY_BUFFER, vertex_data.size() * sizeof(float), vertex_data.data(), GL_STATIC_DRAW);

		// feed the indices array into an OGL array accessible by the IBO handle
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_gl_info[mesh_index].m_IBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, shapes[mesh_index].mesh.indices.size() * sizeof(unsigned int), shapes[mesh_index].mesh.indices.data(), GL_STATIC_DRAW);

		// describe how the big VBO buffer is laid out - note correspondence to the big array we've laid out above.
		// 1) just declare the 3 arrays
		glEnableVertexAttribArray(0); //position
		glEnableVertexAttribArray(1); //uv coordinates 
		glEnableVertexAttribArray(2); //normal data
									  // 2) specify the offset in the VBO for each array
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(float)*(shapes[mesh_index].mesh.positions.size() + shapes[mesh_index].mesh.normals.size())));
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_TRUE, 0, (void*)(sizeof(float)*shapes[mesh_index].mesh.positions.size()));

		glBindVertexArray(0); //break the existing binding set up abocve to the master handle.
		glBindBuffer(GL_ARRAY_BUFFER, 0); //break bindings to buffers set up above.
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		//note that the big array we passed into the VBO handle now destructs. We've gopt a copy stored safely in the OGL system, we don't need it.
	}
}


void Model::Draw(Camera* camera, GLuint _programID, glm::mat4& _transform)
{
	glUseProgram(_programID);

	// draw the model
	glm::mat4 mvp = camera->getProjection() * camera->getView() * _transform;
	GLuint mvpID = glGetUniformLocation(_programID, "MVP");
	GLuint viewMatrixID = glGetUniformLocation(_programID, "V");
	GLuint modelMatrixID = glGetUniformLocation(_programID, "M");

	glUniformMatrix4fv(mvpID, 1, GL_FALSE, (float*)&mvp);
	glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, (float*)&_transform);
	mat4 view = camera->getView();
	glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, (float*)&view);

	for (unsigned int i = 0; i < m_gl_info.size(); ++i)
	{
		glBindVertexArray(m_gl_info[i].m_VAO);
		glDrawElements(GL_TRIANGLES, m_gl_info[i].m_index_count, GL_UNSIGNED_INT, 0);
	}

}

bool Model::LoadShaderFromFile(const char* filePath, std::string& code)
{
	code = "";

	// Read the Vertex Shader code from the file
	std::ifstream VertexShaderStream(filePath, std::ios::in);
	if (VertexShaderStream.is_open()) {
		std::string line;
		while (getline(VertexShaderStream, line))
			code += line + "\n";
		VertexShaderStream.close();
		return true;
	}
	else {
		printf("Cannot open shader file %s.\n", filePath);
		return false;
	}
}

GLuint Model::CompileShaders(const char* vsFile, const char* fsFile)
{
	std::string vsCode, fsCode;
	if (!(Model::LoadShaderFromFile(vsFile, vsCode) && Model::LoadShaderFromFile(fsFile, fsCode)))
		printf("failed to load shaders");
	const char* vsSource = vsCode.c_str();
	const char* fsSource = fsCode.c_str();

	GLuint programID;
	int success = GL_FALSE;
	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(vertexShader, 1, (const char**)&vsSource, 0);
	glCompileShader(vertexShader);
	glShaderSource(fragmentShader, 1, (const char**)&fsSource, 0);
	glCompileShader(fragmentShader);
	programID = glCreateProgram();
	glAttachShader(programID, vertexShader);
	glAttachShader(programID, fragmentShader);
	glLinkProgram(programID);
	glGetProgramiv(programID, GL_LINK_STATUS, &success);
	if (success == GL_FALSE) {
		int infoLogLength = 0;
		glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);
		char* infoLog = new char[infoLogLength];
		glGetProgramInfoLog(programID, infoLogLength, 0, infoLog);
		printf("Error: Failed to link shader program!\n");
		printf("%s\n", infoLog);
		delete[] infoLog;
	}
	glDeleteShader(fragmentShader);
	glDeleteShader(vertexShader);

	return programID;
}