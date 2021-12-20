#ifndef __CAMERA_H__
#define __CAMERA_H__

#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>


enum camera_movement {
	FORWARD,
	BACKWARD,
	RIGHT,
	LEFT
};

const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

/*----- CAMERA CLASS -----*/
class camera{
public:
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 right;
	glm::vec3 up;
	glm::vec3 world_up;

	double yaw;
	double pitch;

	float zoom = 45.0f;
	float speed;
	float mouse_sensivity;

	camera(glm::vec3 camera_position, glm::vec3 camera_front, glm::vec3 world_up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);

	void process_keyboard(camera_movement movement, float delta_time);
	void process_mouse(double x_offset, double y_offest, bool constrain_pitch = true);
	glm::mat4 get_view_matrix();

private:
	void update_camera_vectors();	
};


#endif // !__CAMERA_H__
