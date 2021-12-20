#include <iostream>
#include <vector>
#include <queue>
#include <list>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include "../header/shader.h"
#include "../header/camera.h"

#define WINDOW_WIDTH 600
#define	WINDOW_HEIGHT 800


//Callback functions for viewport, mouse, and keyboard
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void keyboard_input_callback(GLFWwindow* window, int key, int scancode, int action, int mod);
void mouse_input_callback(GLFWwindow* window, double x_pos, double y_pos);
void process_input(GLFWwindow* window);

//Triangle creation
void create_equalaterial_triangle(std::vector<glm::vec3>* buffer, glm::vec3 position, double side_len); //creates a single equalaterial triangle
void recursive_equalaterial(std::vector<glm::vec3>* buffer, int iteration); //breaks down a equalaterial triangle down into smaller equalaterial triangle
void rotate_triangle(std::vector<glm::vec3>* buffer); //rotates triangle to create octahedron
void sphere_normalization(std::vector<glm::vec3>* buffer, double radius = 1, glm::vec3 origin = glm::vec3(0.0f ,0.0f,0.0f)); //adjust vertices of octahedron to become a sphere

//FPS variables
float delta_time = 0.0f;
float last_frame = 0.0f;

//Camera movement variables
camera main_camera(glm::vec3(0.5f, 0.0f, 0.5f), glm::vec3(0.0f, 0.0f, -1.0f));
bool first_mouse = true;
double last_x = WINDOW_WIDTH / 2.0f;
double last_y = WINDOW_HEIGHT / 2.0f;

shader_program* program;

//Calculated vertices structure for recursive triangle breakdown
struct calculated_vertex {
	glm::vec3 left;
	glm::vec3 right;
};
calculated_vertex* create_calculated_pair(glm::vec3 left, glm::vec3 right);

//Debugging functions
void print_vector3(glm::vec3& vector);

int main() {
	glfwInit();
	std::cout << "OpenGL version supported: " << glfwGetVersionString() << std::endl;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Sphere", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create window" << std::endl;
		glfwTerminate();
		return 1;
	}

	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to load GLAD" << std::endl;
		glfwTerminate();
		return 1;
	}
	
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, keyboard_input_callback);
	glfwSetCursorPosCallback(window, mouse_input_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//setup shaders
	const char* shader_paths[2] = { "src/shader/basic_vertex.glsl", "src/shader/basic_fragment.glsl" };
	program = new shader_program(shader_paths[0], shader_paths[1]);

	//setup equalaterial triangle
	GLuint VAO, VBO;
	GLuint vertex_position, vertex_normal, vertex_color;
	glm::vec3 u, v, normal;
	std::vector<glm::vec3> sphere_position_buffer;
	std::vector<glm::vec4> sphere_color_buffer;
	std::vector<glm::vec3> sphere_normal_buffer;
	double vertex_size = sizeof(glm::vec3) * 2 + sizeof(glm::vec4);
	int buffer_size;

	create_equalaterial_triangle(&sphere_position_buffer, glm::vec3(0.0f, 0.0f, 0.0f), 2);
	u = sphere_position_buffer[1] - sphere_position_buffer[0];
	v = sphere_position_buffer[2] - sphere_position_buffer[0];
	normal = glm::cross(u, v);
	recursive_equalaterial(&sphere_position_buffer, 5);
	rotate_triangle(&sphere_position_buffer);
	sphere_normalization(&sphere_position_buffer, 10);
	for (int i = 0; i < sphere_position_buffer.size(); i++) {
		if (i % 3 == 0)
			sphere_color_buffer.push_back(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
		else if (i % 3 == 1)
			sphere_color_buffer.push_back(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
		else
			sphere_color_buffer.push_back(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
		sphere_normal_buffer.push_back(normal);
	}
	buffer_size = sphere_position_buffer.size();

	//generate equalaterial triangle
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	vertex_position = glGetAttribLocation(program->ID, "vPosition");
	vertex_normal = glGetAttribLocation(program->ID, "vNormal");
	vertex_color = glGetAttribLocation(program->ID, "vColor");
	glBufferData(GL_ARRAY_BUFFER, buffer_size * vertex_size, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, buffer_size * sizeof(glm::vec3), (void*)sphere_position_buffer.data());
	glBufferSubData(GL_ARRAY_BUFFER, buffer_size * sizeof(glm::vec3), buffer_size * sizeof(glm::vec4), (void*)sphere_color_buffer.data());
	glBufferSubData(GL_ARRAY_BUFFER, buffer_size * (sizeof(glm::vec3) + sizeof(glm::vec4)), buffer_size * sizeof(glm::vec3), (void*)sphere_normal_buffer.data());

	glVertexAttribPointer(vertex_position, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
	glVertexAttribPointer(vertex_color, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)(buffer_size * sizeof(glm::vec3)));
	glVertexAttribPointer(vertex_normal, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)(buffer_size * (sizeof(glm::vec3) + sizeof(glm::vec4))));

	glEnableVertexAttribArray(vertex_position);
	glEnableVertexAttribArray(vertex_color);
	glEnableVertexAttribArray(vertex_normal);

	float current_frame;
	glm::mat4 model(1.0f);
	glm::mat4 projection = glm::perspective(glm::radians(main_camera.zoom), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f);
	int shininess = 32;
	float ambient_str = 0.1f, specular_str = 0.5f;
	glClearColor(0.529f, 0.807f, 0.92f, 1.0f);
	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		//Update frames
		current_frame = glfwGetTime();
		delta_time = current_frame - last_frame;
		last_frame = current_frame;
		process_input(window);

		//Set uniforms
		program->set_mat4("model", model);
		program->set_mat4("view", main_camera.get_view_matrix());
		program->set_mat4("projection", projection);
		program->set_vec3("light_color", glm::vec3(1.0f, 1.0f, 1.0f));
		program->set_vec3("light_pos", main_camera.position);
		program->set_vec3("viewer_pos", main_camera.position);
		program->set_int("shininess", shininess);
		program->set_float("ambient_stren", ambient_str);
		program->set_float("specular_stren", specular_str);

		glUseProgram(program->ID);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, buffer_size);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}
void keyboard_input_callback(GLFWwindow* window, int key, int scancode, int action, int mod) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}
void mouse_input_callback(GLFWwindow* window, double x_pos, double y_pos) {
	if (first_mouse) {
		last_x = x_pos;
		last_y = y_pos;
		first_mouse = false;
	}

	double x_diff = x_pos - last_x;
double y_diff = last_y - y_pos;

last_x = x_pos;
last_y = y_pos;

main_camera.process_mouse(x_diff, y_diff);
}
void process_input(GLFWwindow* window) {

	//WASD movment
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		main_camera.process_keyboard(FORWARD, delta_time);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		main_camera.process_keyboard(LEFT, delta_time);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		main_camera.process_keyboard(BACKWARD, delta_time);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		main_camera.process_keyboard(RIGHT, delta_time);

}

//Create an equalaterial triangle with no z-value
void create_equalaterial_triangle(std::vector<glm::vec3>* buffer, glm::vec3 position, double side_len) {
	double base_half = side_len / 2;
	double height_half = side_len * glm::cos(glm::pi<double>() / 6) / 2;

	buffer->push_back(glm::vec3(base_half, -height_half, 0.0f));
	buffer->push_back(glm::vec3(0, height_half, 0.0f));
	buffer->push_back(glm::vec3(-base_half, -height_half, 0.0f));

}

//Breakdown equalaterial triangle into polyogns shaped as equalateral trianlges
void recursive_equalaterial(std::vector<glm::vec3>* buffer, int iteration) {
	if (iteration == 0)
		return;
	int total_vertices = pow(4, iteration) * 3; //number of polygons * number of vertices per polygon
	int rows = pow(2, iteration);
	int current_sub_triangle = 0; //holds current polygon being created
	int dequeue_index = 0, delta = 3, assigned = 0;
	bool is_flipped = false, use_left = false, first_triple = false; //flages for alternating polygons
	glm::vec3 displacment = buffer->at(1) - buffer->at(0);
	double side = sqrt(displacment.x * displacment.x + displacment.y * displacment.y + displacment.z * displacment.z); //side length of equalaterial triangle
	double sub_side_len = side / rows; //length of each polygon 
	double sub_base_len = sub_side_len * glm::cos(glm::pi<float>() / 3); //base of octahedron face
	double sub_height_len = sub_side_len * glm::cos(glm::pi<float>() / 6); //height octahedron face
	double dihedral_angle = glm::acos(-1 / 3); //base angles of triangle on xz-plane
	double beta_angle = 180.0f - dihedral_angle; //beta angle for law of sines
	double sub_depth_len = glm::sin(glm::radians(dihedral_angle / 2)) / glm::sin(glm::radians(beta_angle)) * sub_side_len; //Using Law of Sines to determine value of sub_depth_len

	//Setup for queue
	glm::vec3 top_vertex = buffer->at(1);
	std::queue<calculated_vertex*, std::list<calculated_vertex*>> calculated_vertices;
	calculated_vertex* init_pair = new calculated_vertex;
	init_pair->right = top_vertex;
	calculated_vertices.push(init_pair);
	buffer->clear();

	/* Diagram of triangle position data
	[vertex + 1]   [vertex + 4] [vertex + 3]
		  / \			\        /
		 /   \			 \      /
		/     \			  \    /
	   /       \		   \  /
	  /_________\           \/				.....
[vertex + 2]  [vertex]	[vertex +  5]
	*/
	//adds positional data for polygon into buffer as shown above; uses verteices from previous row to reduce number of calculations
	for (int vertex = 0; vertex < total_vertices; vertex += 3) {
		calculated_vertex* sub_vertex;
		//if polygon is standing upright
		if (!is_flipped) {
			if (use_left) { //uses bottom left vertex value from polygon above
				buffer->push_back(glm::vec3(calculated_vertices.front()->left.x + sub_base_len, calculated_vertices.front()->left.y - sub_height_len, calculated_vertices.front()->left.z - sub_depth_len)); //bottom left vertex
				buffer->push_back(calculated_vertices.front()->left); //top vertex
				buffer->push_back(glm::vec3(calculated_vertices.front()->left.x - sub_base_len, calculated_vertices.front()->left.y - sub_height_len, calculated_vertices.front()->left.z - sub_depth_len)); //bottom right vertex
				use_left = false;
			}
			else { //uses bottom right vertex value from polygon above
				buffer->push_back(glm::vec3(calculated_vertices.front()->right.x + sub_base_len, calculated_vertices.front()->right.y - sub_height_len, calculated_vertices.front()->right.z - sub_depth_len)); //bottom left vertex
				buffer->push_back(calculated_vertices.front()->right); //top vertex
				buffer->push_back(glm::vec3(calculated_vertices.front()->right.x - sub_base_len, calculated_vertices.front()->right.y - sub_height_len, calculated_vertices.front()->right.z - sub_depth_len)); //bottom right vertex
				use_left = true;
			}
			sub_vertex = create_calculated_pair(buffer->at(static_cast<unsigned __int64>(vertex) + 2), buffer->at(vertex)); //create vertex pair generated 
			calculated_vertices.push(sub_vertex); //push vertex pair onto queue
			is_flipped = true;
		}
		//if polyogn is upside down
		else { //uses both left and right bottom vertex values from polygon above
			buffer->push_back(calculated_vertices.front()->right); //top right vertex
			buffer->push_back(calculated_vertices.front()->left); //top left vertex
			buffer->push_back(glm::vec3(calculated_vertices.front()->left.x + sub_base_len, calculated_vertices.front()->left.y - sub_height_len, calculated_vertices.front()->left.z - sub_depth_len)); //bottom vertex
			is_flipped = false;
		}
		assigned++; //update number of polygons generated by vertex in front of queue

		//Dequeue vertex if current polygon being generated is last one of the row
		if (current_sub_triangle == dequeue_index) { 
			free(calculated_vertices.front());
			calculated_vertices.pop();
			dequeue_index += delta;
			delta += 2;
			assigned = 0;
			is_flipped = false;
			first_triple = false;
		}

		//Dequeues inital vertex after first 3 polyons generated
		else if (assigned == 3 && !first_triple) {
			free(calculated_vertices.front());
			calculated_vertices.pop();
			assigned = 0;
			if (use_left)
				use_left = false;
			else
				use_left = true;
			first_triple = true;
		}
		//Dequeue vertex if vertex is used to generate 2 polygons
		else if(first_triple && assigned == 2){
			free(calculated_vertices.front());
			calculated_vertices.pop();
			assigned = 0;
			if (use_left)
				use_left = false;
			else
				use_left = true;
		}
		current_sub_triangle++;
	}

}

//Create octahedron using equalateral triangle
void rotate_triangle(std::vector<glm::vec3>* buffer) {
	int new_buffer_size = buffer->size() * 3; //3 additional triangles to be added
	float height = buffer->at(1).y * 2; //doubled because buffer holds half of the full height
	glm::mat4 rotation_y, rotation_z, translate_y; //matrices used for transformation of initial triangle
	glm::vec4 bottom_half; //holds vertex for bottom portion of octahedron
	rotation_y = glm::rotate(rotation_y, glm::pi<float>() / 2, glm::vec3(0.0f, 1.0f, 0.0f)); //rotation around y-axis
	rotation_z = glm::rotate(rotation_z, glm::pi<float>(), glm::vec3(0.0f, 0.0f, 1.0f)); //rotation around z-axis; for proper orientation 
	translate_y = glm::translate(translate_y, glm::vec3(0.0f, -height, 0.0f)); //translate z-rotation to bottom for correct placment

	//add vertices for top half of octahedron
	for (int i = 0; i < new_buffer_size; i++) {
		buffer->push_back(glm::vec4(buffer->at(i), 1.0f) * rotation_y);
	}

	//add vertices for bottom half of octahedron
	new_buffer_size = buffer->size();
	for (int i = 0; i < new_buffer_size; i++) {
		bottom_half = glm::vec4(buffer->at(i), 1.0f) * rotation_z;
		bottom_half.y -= height;
		buffer->push_back(bottom_half);
	}
}

//Normalizes vertices of octahedron to become a sphere
void sphere_normalization(std::vector<glm::vec3>* buffer, double radius, glm::vec3 origin) {
	int buffer_size = buffer->size();
	float dx, dy, dz;
	glm::vec3 sphere_vertex;
	for(int i = 0; i < buffer_size; i++){
		//displacement components from origin to vertex on octahedron
		dx = buffer->at(i).x - origin.x;
		dy = buffer->at(i).y - origin.y;
		dz = buffer->at(i).z - origin.z;
		
		//convert displacement to be relative to sphere surface
		dx = dx * radius / glm::distance(origin, buffer->at(i));
		dy = dy * radius / glm::distance(origin, buffer->at(i));
		dz = dz * radius / glm::distance(origin, buffer->at(i));
		sphere_vertex = glm::vec3(dx, dy, dz);
		buffer->at(i) = sphere_vertex;
	}
}


//Create calculated pair vertices; struct calculated_vertex
calculated_vertex* create_calculated_pair(glm::vec3 left, glm::vec3 right) {
	calculated_vertex* pair = new calculated_vertex;
	pair->left = left;
	pair->right = right;
	return pair;
}

//Debugging functions
void print_vector3(glm::vec3& vector) {
	std::cout << vector.x << " " << vector.y << " " << vector.z << " " << std::endl;
}