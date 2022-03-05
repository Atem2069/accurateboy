#pragma once

#include<iostream>
#include<string>
#include<Windows.h>
#include<glad/glad.h>
#include<GLFW/glfw3.h>

#include"Logger.h"
#include"guiRenderer.h"
#include"common/vec3.h"
#include"common/vec2.h"

struct Vertex
{
	vec3 position;
	vec2 uv;
};

class Display
{
public:
	Display(int width, int height);
	~Display();

	bool shouldClose();
	void draw();

	void upload(void* newData, bool isNew);

	bool getInitialized();
	bool getKeyPressed(int key);

private:
	GLFWwindow* m_window;

	GLuint m_VBO=0, m_VAO=0, m_program=0;
	GLuint m_texHandle = 0;
	GLuint m_prevFrameTexHandle = 0;
	bool m_initialized = false;

	uint32_t lastFrame[160 * 144];

};