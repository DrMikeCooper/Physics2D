#pragma once
#include <vector>
#include <tinyobjloader-master\tiny_obj_loader.h>
#include <glm/glm/glm.hpp>

class Camera;

class Model
{
public:

	struct GLInfo
	{
		unsigned int m_VAO;
		unsigned int m_VBO;
		unsigned int m_IBO;
		unsigned int m_index_count;
	};

	bool LoadObjFile(const char* name);
	void CreateBuffers();
	void Draw(Camera* camera, GLuint _programID, glm::mat4& _transform);

	static GLuint CompileShaders(const char* vsSource, const char* fsSource);
	static bool LoadShaderFromFile(const char* filePath, std::string& code);

	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::vector<GLInfo> m_gl_info;
};
