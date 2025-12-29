#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Camera.h"

const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aColor;\n"
"out vec3 vColor;\n"
"uniform mat4 uProjection;\n"
"uniform mat4 uView;\n"
"uniform mat4 uModel;\n"
"void main()\n"
"{\n"
"	gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);\n"
"	vColor = aColor;\n"
"}\0";

const char* fragmentShaderSource = "#version 330 core\n"
"in vec3 vColor;\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"	FragColor = vec4(vColor, 1.0f);\n"
"}\n\0";

bool checkShaderCompilation(GLuint shader, const char* name);
bool checkProgramLink(GLuint program);
void cameraMovement(GLFWwindow *window, float &camX, float &camY, float &camZ, float &camSpeed);

int main() {
	// wireframe mode toggle
	bool wireframe = false;
	bool wPressedLastFrame = false;

	// camera movement
	Camera camera({ 0, 0, 3 });
	
	float lastFrame = 0.0f;

	/* ---------- INITIALIZE WINDOW, SHADERS ---------- */
	// initialize opengl, add hints and create window
	if (!glfwInit()) {
		return -1;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow *window = glfwCreateWindow(800, 800, "Renderer", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}

	// add window to current context
	glfwMakeContextCurrent(window);

	// using glad to load opengl functions
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	// creating a starting and ending position on the screen for opengl
	glViewport(0, 0, 800, 800);
	glEnable(GL_DEPTH_TEST);

	// make vertex and fragment shaders
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	if (!checkShaderCompilation(vertexShader, "VERTEX")) {
		glDeleteShader(vertexShader);
		return -1;
	}
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	if (!checkShaderCompilation(fragmentShader, "FRAGMENT")) {
		glDeleteShader(fragmentShader);
		return -1;
	}

	// combine them into one program
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	if (!checkProgramLink(shaderProgram)) {
		return -1;
	}
	GLint modelLoc = glGetUniformLocation(shaderProgram, "uModel");
	GLint viewLoc = glGetUniformLocation(shaderProgram, "uView");
	GLint projectionLoc = glGetUniformLocation(shaderProgram, "uProjection");
	// now shaderProgram contains the combination of vertexShader
	// and fragmentShader. so, vertexShader and fragmentShader themselves
	// are now useless and can be deleted
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);


	/* ---------- INITIALIZE VERTICES, BUFFERS ---------- */
	// creating vertex data
	float fov = 45.0f * (3.1415926f / 180.0f);
	float aspect = 800.0f / 800.0f;
	float near = 0.1f;
	float far = 100.0f;

	float f = 1.0f / tan(fov / 2.0f);

	GLfloat projection[16] = {
		f / aspect, 0,  0,							    0,
		0,			f,	0,							    0,
		0,			0, (far + near) / (near - far),	   -1,
		0,			0, (2 * far * near) / (near - far), 0
	};



	/*
	GLfloat vertices[] = {
		// position			   // color
		-0.5f, -0.5f,  0.0f,   1, 0, 0,
		 0.5f, -0.5f,  0.0f,   0, 1, 0,
		 0.0f,  0.5f, -0.5f,   0, 0, 1
	};

	GLuint indices[] = {
		0, 1, 2
	};
	*/

	GLfloat vertices[] = {
		// position            // color
		-0.5f, -0.5f, -0.5f,   1, 0, 0,  // 0
		 0.5f, -0.5f, -0.5f,   0, 1, 0,  // 1
		 0.5f,  0.5f, -0.5f,   0, 0, 1,  // 2
		-0.5f,  0.5f, -0.5f,   1, 1, 0,  // 3

		-0.5f, -0.5f,  0.5f,   1, 0, 1,  // 4
		 0.5f, -0.5f,  0.5f,   0, 1, 1,  // 5
		 0.5f,  0.5f,  0.5f,   1, 1, 1,  // 6
		-0.5f,  0.5f,  0.5f,   0.3,0.3,0.3 // 7
	};

	GLuint indices[] = {
		// back face
		0, 1, 2,
		2, 3, 0,

		// front face
		4, 5, 6,
		6, 7, 4,

		// left face
		0, 3, 7,
		7, 4, 0,

		// right face
		1, 5, 6,
		6, 2, 1,

		// bottom face
		0, 1, 5,
		5, 4, 0,

		// top face
		3, 2, 6,
		6, 7, 3
	};


	GLuint VAO, VBO, EBO;
	
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &EBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	/* ---------- MAINLOOP ---------- */
	// mainloop
	while (!glfwWindowShouldClose(window)) {
		// rendering here
		glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(shaderProgram);

		float currentFrame = glfwGetTime();
		float deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		camera.processKeyboard(window, deltaTime);

		float view[16];
		camera.getViewMatrix(view);

		float t = glfwGetTime();
		float c = cos(t);
		float s = sin(t);
		GLfloat model[16] = { // using a matrix model allows u to store more data and shit basically. fun.
			c, -s, 0, 0,
			s, c, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1,
		};

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, model);
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view);
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, projection);
		glBindVertexArray(VAO);

		bool wPressed = glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS;
		if (wPressed && !wPressedLastFrame) {
			wireframe = !wireframe;
			glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
		}
		wPressedLastFrame = wPressed;

		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

		// swap front and back buffer
		glfwSwapBuffers(window);

		// poll events :: IMPORTANT (unresponsive otherwise)
		glfwPollEvents();
	}

	// close window and exit program
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteProgram(shaderProgram);
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

bool checkShaderCompilation(GLuint shader, const char* name) {
	int success;
	char infoLog[512];

	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER_COMPILATION_FAILED (" << name << ")\n" << infoLog << std::endl;
		return false;
	}

	return true;
}

bool checkProgramLink(GLuint program) {
	int success;
	char infoLog[512];

	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(program, 512, NULL, infoLog);
		std::cout << "ERROR::PROGRAM LINKING FAILED\n" << infoLog << std::endl;
		return false;
	}

	return true;
}