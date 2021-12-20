#ifndef __SHADER_H__
#define	__SHADER_H__

#include<stdio.h>
#include<stdlib.h>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>

const char* read_file(const char* file_path);

class shader_program {
public:
	GLuint ID;
	
	shader_program();
	shader_program(const char* vertex_path, const char* fragment_path);


	void use();
	
	/*----- set primitive data -----*/
	void set_bool(const char* name, GLboolean& value);
	void set_int(const char* name, GLint& value);
	void set_float(const char* name, GLfloat& value);
	void set_double(const char* name, GLdouble &value);

	/*----- set vectors -----*/
	void set_vec2(const char* name, const glm::vec2 &value);
	void set_vec3(const char* name, const glm::vec3 &value);
	void set_vec4(const char* name, const glm::vec4 &value);

	/*----- set matrices ----*/
	void set_mat2(const char* name, const glm::mat2 &value);
	void set_mat3(const char* name, const glm::mat3 &value);
	void set_mat4(const char* name, const glm::mat4 &value);

private:
	GLuint* compile_shader(const char** shader_paths);
};

#endif // ! __SHADER_H__

