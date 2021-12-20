#include "../header/shader.h"

shader_program::shader_program() { ID = -1; }
shader_program::shader_program(const char* vertex_path, const char* fragment_path) {
	GLuint* shader_IDs;
	GLint linked;
	const char* shader_paths[2] = { vertex_path, fragment_path };

	shader_IDs = compile_shader(shader_paths);
	
	ID = glCreateProgram();
	if (!ID) {
		fprintf(stderr, "Failed to create shader program using IDs: %d, %d\n", shader_IDs[0], shader_IDs[1]);
		exit(EXIT_FAILURE);
	}
	glAttachShader(ID, shader_IDs[0]);
	glAttachShader(ID, shader_IDs[1]);
	
	glLinkProgram(ID);
	glGetProgramiv(ID, GL_LINK_STATUS, &linked);
	if (!linked) {
		GLint log_size;
		char* log_msg;
		fprintf(stderr, "Shader program failed to link; ID: %d\n", ID);
		glGetProgramiv(ID, GL_INFO_LOG_LENGTH, &log_size);
		log_msg = new char[log_size];
		glGetProgramInfoLog(ID, log_size, NULL, log_msg);
		perror(log_msg);
		delete[] log_msg;
		system("pause");
		exit(EXIT_FAILURE);
	}

	glDeleteShader(shader_IDs[0]);
	glDeleteShader(shader_IDs[1]);
	free(shader_IDs);
}	
void shader_program::use() {
	glUseProgram(ID);
}
GLuint* shader_program::compile_shader(const char** shader_paths) {
	GLint compiled;
	GLuint* shader_IDs = new GLuint[2]; //[0]: vertex id; [1]: fragment id
	const char* shader_source[2];  //[0]: vertex source; [1]: fragment source

	printf("Attempting to read: %s\n", shader_paths[0]);
	shader_source[0] = read_file(shader_paths[0]);
	printf("Attempting to read: %s\n", shader_paths[1]);
	shader_source[1] = read_file(shader_paths[1]);

	shader_IDs[0] = glCreateShader(GL_VERTEX_SHADER);
	shader_IDs[1] = glCreateShader(GL_FRAGMENT_SHADER);

	for (int i = 0; i < 2; i++) {
		glShaderSource(shader_IDs[i], 1, (const GLchar**)&shader_source[i], NULL);
		glCompileShader(shader_IDs[i]);
		glGetShaderiv(shader_IDs[i], GL_COMPILE_STATUS, &compiled);

		if (!compiled) {
			GLint log_size;
			char* log_msg;
			fprintf(stderr, "%s failed to compile\n", shader_paths[i]);
			glGetShaderiv(shader_IDs[i], GL_INFO_LOG_LENGTH, &log_size);
			log_msg = new char[log_size];
			glGetShaderInfoLog(shader_IDs[i], log_size, NULL, log_msg);
			fprintf(stderr, "\nReason:\n%s\n", log_msg);
			delete[] log_msg;
			system("pause");
			exit(EXIT_FAILURE);
		}
		printf("Successfully compiled %s\nID: %d\n", shader_paths[i], i);
	}

	return shader_IDs;
}

/*----- Set uniform variables -----*/
void shader_program::set_bool(const char* name, GLboolean& value) { glUniform1i(glGetUniformLocation(ID,name), value); };
void shader_program::set_int(const char* name, GLint& value) { glUniform1i(glGetUniformLocation(ID,name), value); };
void shader_program::set_float(const char* name, GLfloat& value) { glUniform1f(glGetUniformLocation(ID,name), value); };
void shader_program::set_double(const char* name, GLdouble &value) { glUniform1d(glGetUniformLocation(ID,name), value); };

/*----- set uniform vectors -----*/
void shader_program::set_vec2(const char* name, const glm::vec2 &value) { glUniform2fv(glGetUniformLocation(ID, name), 1, &value[0]); };
void shader_program::set_vec3(const char* name, const glm::vec3 &value) { glUniform3fv(glGetUniformLocation(ID,name), 1, &value[0]); };
void shader_program::set_vec4(const char* name, const glm::vec4 &value) { glUniform4fv(glGetUniformLocation(ID,name), 1, &value[0]); };

/*----- set uniform matrices ----*/
void shader_program::set_mat2(const char* name, const glm::mat2 &value) { glUniformMatrix2fv(glGetUniformLocation(ID,name), 1, GL_FALSE, &value[0][0]); };
void shader_program::set_mat3(const char* name, const glm::mat3 &value) { glUniformMatrix3fv(glGetUniformLocation(ID,name), 1, GL_FALSE, &value[0][0]); };
void shader_program::set_mat4(const char* name, const glm::mat4 &value) { glUniformMatrix4fv(glGetUniformLocation(ID,name), 1, GL_FALSE, &value[0][0]); };

const char* read_file(const char* file_path) {
	FILE* fp;
	long length;
	char* buffer;

	errno_t err = fopen_s(&fp, file_path, "r");
	if (fp == NULL) {
		fprintf(stderr, "Failed to read file: %s\n", file_path);
		system("pause");
		exit(EXIT_FAILURE);
	}
	printf("Successfully read %s\n", file_path);
	fseek(fp, 0L, SEEK_END);
	length = ftell(fp);

	fseek(fp, 0L, SEEK_SET);
	buffer = new char[static_cast<_int64>(length) + 1];
	fread(buffer, sizeof(char), static_cast<_int64>(length) + 1, fp);
	buffer[length] = '\0';
	fclose(fp);

	for (int i = 0; i < length; i++) {
		if (!(buffer[i] >= 0 && buffer[i] <= 128))
			buffer[i] = ' ';
	}
	return buffer;
}
