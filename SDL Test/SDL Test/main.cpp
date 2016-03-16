// Based loosly on the first triangle OpenGL tutorial
// http://www.opengl.org/wiki/Tutorial:_OpenGL_3.1_The_First_Triangle_%28C%2B%2B/Win%29
// This program will render two triangles
// Most of the OpenGL code for dealing with buffer objects, etc has been moved to a 
// utility library, to make creation and display of mesh objects as simple as possible

// Windows specific: Uncomment the following line to open a console window for debug output
#if _DEBUG
#pragma comment(linker, "/subsystem:\"console\" /entry:\"WinMainCRTStartup\"")
#endif

#include "rt3d.h"
#include <glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stack>

#define DEG_TO_RADIAN 0.017453293

using namespace std;

std::stack<glm::mat4> mvStack;

GLuint shaderProgram;
GLuint shaderProgram2;
glm::mat4 MVP;

GLuint cubeVertCount = 8;
GLfloat cubeVerts[] = { -0.25, -0.25f, -0.25f,
-0.25, 0.25f, -0.25f,
0.25f, 0.25f, -0.25f,
0.25f, -0.25f, -0.25f,
-0.25f, -0.25f, 0.25f,
-0.25f, 0.25f, 0.25f,
0.25f, 0.25f, 0.25f,
0.25f, -0.25f, 0.25f };
GLfloat cubeColours[] = { 0.0f, 0.0f, 0.0f,
0.0f, 1.0f, 0.0f,
1.0f, 1.0f, 0.0f,
1.0f, 0.0f, 0.0f,
0.0f, 0.0f, 1.0f,
0.0f, 1.0f, 1.0f,
1.0f, 1.0f, 1.0f,
1.0f, 0.0f, 1.0f };

GLuint cubeIndexCount = 36;
GLuint cubeIndices[] = { 0,1,2, 0,2,3, // back  
1,0,5, 0,4,5, // left					
6,3,2, 3,6,7, // right
1,5,6, 1,6,2, // top
0,3,4, 3,7,4, // bottom
6,5,4, 7,6,4 }; // front


GLuint meshObjects[1];


GLfloat dx = 0.0f;
GLfloat dy = 0.0f;
GLfloat rot = 0.0f;

GLfloat sx = 1.0f;
GLfloat sy = 1.0f;
GLfloat sz = 1.0f;

GLfloat red = 1.0f;
GLfloat green = 1.0f;
GLfloat blue = 1.0f;

rt3d::lightStruct light0 = {
	{ 0.2f, 0.2f, 0.2f, 1.0f }, // ambient
	{ 0.7f, 0.7f, 0.7f, 1.0f }, // diffuse
	{ 0.8f, 0.8f, 0.8f, 1.0f }, // specular
	{ 0.0f, 0.0f, 1.0f, 1.0f }  // position
};

rt3d::materialStruct material0 = {
	{ 0.4f, 0.2f, 0.2f, 1.0f }, // ambient
	{ 0.8f, 0.5f, 0.5f, 1.0f }, // diffuse
	{ 1.0f, 0.8f, 0.8f, 1.0f }, // specular
	2.0f  // shininess
};


// Set up rendering context
SDL_Window * setupRC(SDL_GLContext &context) {
	SDL_Window * window;
	if (SDL_Init(SDL_INIT_VIDEO) < 0) // Initialize video
		rt3d::exitFatalError("Unable to initialize SDL");

	// Request an OpenGL 3.0 context.
	// Not able to use SDL to choose profile (yet), should default to core profile on 3.2 or later
	// If you request a context not supported by your drivers, no OpenGL context will be created

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);  // double buffering on
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4); // Turn on x4 multisampling anti-aliasing (MSAA)

													   // Create 800x600 window
	window = SDL_CreateWindow("SDL/GLM/OpenGL Demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (!window) // Check window was created OK
		rt3d::exitFatalError("Unable to create window");

	context = SDL_GL_CreateContext(window); // Create opengl context and attach to window
	SDL_GL_SetSwapInterval(1); // set swap buffers to sync with monitor's vertical refresh rate
	return window;
}

void init(void) {
	// For this simple example we'll be using the most basic of shader programs
	shaderProgram = rt3d::initShaders("gouraud.vert", "simple.frag");
	shaderProgram2 = rt3d::initShaders("phong.vert", "phong.frag");

	glEnable(GL_DEPTH_TEST);

	rt3d::setLight(shaderProgram, light0);
	rt3d::setMaterial(shaderProgram, material0);


	// Going to create our mesh objects here
	meshObjects[0] = rt3d::createMesh(cubeVertCount, cubeVerts, nullptr, cubeVerts,
		nullptr, cubeIndexCount, cubeIndices);


}

void update() {
	const Uint8 *keys = SDL_GetKeyboardState(NULL);

	/*if (keys[SDL_SCANCODE_R]) {
		dx = dy = 0;
		rot = 0;
		sx = sy = sz = 1;
	}*/
	//postion
	if (keys[SDL_SCANCODE_W]) dy += 0.1;
	if (keys[SDL_SCANCODE_S]) dy -= 0.1;

	if (keys[SDL_SCANCODE_D]) dx += 0.1;
	if (keys[SDL_SCANCODE_A]) dx -= 0.1;

	//scale
	if (keys[SDL_SCANCODE_UP]) sy += 0.1;
	if (keys[SDL_SCANCODE_DOWN]) sy -= 0.1;

	if (keys[SDL_SCANCODE_LEFT]) sx += 0.1;
	if (keys[SDL_SCANCODE_RIGHT]) sx -= 0.1;

	//rotation
	if (keys[SDL_SCANCODE_T]) rot += 0.2;
	if (keys[SDL_SCANCODE_Y]) rot -= 0.2;


	if (keys[SDL_SCANCODE_R]) red += 0.05;
	if (red > 1.0f)
		red = 0.0f;

	if (keys[SDL_SCANCODE_G]) green += 0.05;
	if (green > 1.0f)
		green = 0.0f;

	if (keys[SDL_SCANCODE_B]) blue += 0.05;
	if (blue > 1.0f)
		blue = 0.0f;
}

void draw(SDL_Window * window) {
	// clear the screen
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shaderProgram2);

	rt3d::setLight(shaderProgram, light0);
	rt3d::setMaterial(shaderProgram, material0);
	// set up projection
	glm::mat4 projection(1.0);
	projection = glm::perspective(float(60.0f*DEG_TO_RADIAN), 800.0f / 600.0f, 1.0f, 50.0f);
	rt3d::setUniformMatrix4fv(shaderProgram, "projection", glm::value_ptr(projection));

#pragma region cubeArray

	material0 = {
		{ 0.4f, 0.2f, 0.2f, 1.0f }, // ambient
		{ 0.8f, 0.5f, 0.5f, 1.0f }, // diffuse
		{ 1.0f, 0.8f, 0.8f, 1.0f }, // specular
		2.0f  // shininess
	};
	rt3d::setMaterial(shaderProgram, material0);
	glm::mat4 modelview(1.0);
	mvStack.push(modelview); // push modelview to stack
	mvStack.top() = glm::translate(mvStack.top(), glm::vec3(-2.0f, 1.0f, -4.0f));
	mvStack.top() = glm::rotate(mvStack.top(), float(45*DEG_TO_RADIAN), glm::vec3(0.0f, 1.0f, 0.0f));
	MVP = projection * mvStack.top();
	rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
	rt3d::drawIndexedMesh(meshObjects[0], cubeIndexCount, GL_TRIANGLES);
	mvStack.pop();

	material0 = {
		{ 0.2f, 0.4f, 0.2f, 1.0f }, // ambient
		{ 0.5f, 0.8f, 0.5f, 1.0f }, // diffuse
		{ 0.8f, 1.0f, 0.8f, 1.0f }, // specular
		2.0f  // shininess
	};
	rt3d::setMaterial(shaderProgram, material0);
	mvStack.push(modelview); // push modelview to stack
	mvStack.top() = glm::translate(mvStack.top(), glm::vec3(0.0f, 1.0f, -4.0f));
	mvStack.top() = glm::rotate(mvStack.top(), float(45*DEG_TO_RADIAN), glm::vec3(0.0f, 1.0f, 0.0f));
	MVP = projection * mvStack.top();
	rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
	rt3d::drawIndexedMesh(meshObjects[0], cubeIndexCount, GL_TRIANGLES);
	mvStack.pop();

	material0 = {
		{ 0.2f, 0.2f, 0.4f, 1.0f }, // ambient
		{ 0.5f, 0.5f, 0.8f, 1.0f }, // diffuse
		{ 0.8f, 0.8f, 1.0f, 1.0f }, // specular
		2.0f  // shininess
	};
	rt3d::setMaterial(shaderProgram, material0);

	mvStack.push(modelview); // push modelview to stack
	mvStack.top() = glm::translate(mvStack.top(), glm::vec3(2.0f, 1.0f, -4.0f));
	mvStack.top() = glm::rotate(mvStack.top(), float(45*DEG_TO_RADIAN), glm::vec3(0.0f, 1.0f, 0.0f));
	MVP = projection * mvStack.top();
	rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
	rt3d::drawIndexedMesh(meshObjects[0], cubeIndexCount, GL_TRIANGLES);
	mvStack.pop();

	//bottom row
	material0 = {
		{ 0.4f, 0.4f, 0.2f, 1.0f }, // ambient
		{ 0.8f, 0.8f, 0.5f, 1.0f }, // diffuse
		{ 1.0f, 0.8f, 0.8f, 1.0f }, // specular
		2.0f  // shininess
	};
	rt3d::setMaterial(shaderProgram, material0);
	mvStack.push(modelview); // push modelview to stack
	mvStack.top() = glm::translate(mvStack.top(), glm::vec3(-2.0f, -1.0f, -4.0f));
	mvStack.top() = glm::rotate(mvStack.top(), float(45*DEG_TO_RADIAN), glm::vec3(0.0f, 1.0f, 0.0f));
	MVP = projection * mvStack.top();
	rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
	rt3d::drawIndexedMesh(meshObjects[0], cubeIndexCount, GL_TRIANGLES);
	mvStack.pop();

	material0 = {
		{ 0.2f, 0.4f, 0.4f, 1.0f }, // ambient
		{ 0.5f, 0.8f, 0.8f, 1.0f }, // diffuse
		{ 0.8f, 1.0f, 1.0f, 1.0f }, // specular
		2.0f  // shininess
	};
	rt3d::setMaterial(shaderProgram, material0);
	mvStack.push(modelview); // push modelview to stack
	mvStack.top() = glm::translate(mvStack.top(), glm::vec3(0.0f, -1.0f, -4.0f));
	mvStack.top() = glm::rotate(mvStack.top(), float(45*DEG_TO_RADIAN), glm::vec3(0.0f, 1.0f, 0.0f));
	MVP = projection * mvStack.top();
	rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
	rt3d::drawIndexedMesh(meshObjects[0], cubeIndexCount, GL_TRIANGLES);
	mvStack.pop();

	material0 = {
		{ 0.4f, 0.2f, 0.4f, 1.0f }, // ambient
		{ 1.0f, 0.5f, 0.8f, 1.0f }, // diffuse
		{ 1.0f, 0.8f, 1.0f, 1.0f }, // specular
		2.0f  // shininess
	};
	rt3d::setMaterial(shaderProgram, material0);
	mvStack.push(modelview); // push modelview to stack
	mvStack.top() = glm::translate(mvStack.top(), glm::vec3(2.0f, -1.0f, -4.0f));
	mvStack.top() = glm::rotate(mvStack.top(), float(45*DEG_TO_RADIAN), glm::vec3(0.5f, 1.0f, 0.5f));
	MVP = projection * mvStack.top();
	rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
	rt3d::drawIndexedMesh(meshObjects[0], cubeIndexCount, GL_TRIANGLES);
	mvStack.pop();

#pragma endregion cubeArray

#pragma region galaxy

	//// render the sun
	//glm::mat4 modelview(1.0);
	//mvStack.push(modelview); // push modelview to stack
	//mvStack.top() = glm::translate(mvStack.top(), glm::vec3(0.0f, 0.0f, -4.0f));
	//mvStack.top() = glm::rotate(mvStack.top(), float((rot +=0.1f)*DEG_TO_RADIAN), glm::vec3(0.0f, 1.0f, 0.0f));
	//MVP = projection * mvStack.top();
	//rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
	//rt3d::drawIndexedMesh(meshObjects[0], cubeIndexCount, GL_TRIANGLES);


	//// planet1
	//mvStack.push(mvStack.top());// push modelview to stack
	//mvStack.top() = glm::translate(mvStack.top(), glm::vec3(-2.0f, 0.0f, 0.0f));
	//mvStack.top() = glm::rotate(mvStack.top(), float((rot + 0.25f)*DEG_TO_RADIAN), glm::vec3(0.0f, 1.0f, 0.0f));
	//mvStack.top() = glm::scale(mvStack.top(), glm::vec3(0.5f, 0.5f, 0.5f));
	//MVP = projection * mvStack.top();
	//rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
	//rt3d::drawIndexedMesh(meshObjects[0], cubeIndexCount, GL_TRIANGLES);

	//// moon
	//mvStack.push(mvStack.top());// push modelview to stack
	//mvStack.top() = glm::translate(mvStack.top(), glm::vec3(1.0f, 0.0f, 0.0f));
	//mvStack.top() = glm::rotate(mvStack.top(), float((rot + 0.25f)*DEG_TO_RADIAN), glm::vec3(0.0f, 1.0f, 0.0f));
	//mvStack.top() = glm::scale(mvStack.top(), glm::vec3(0.5f, 0.5f, 0.5f));
	//MVP = projection * mvStack.top();
	//rt3d::setUniformMatrix4fv(shaderProgram2, "modelview", glm::value_ptr(mvStack.top()));
	//rt3d::drawIndexedMesh(meshObjects[0], cubeIndexCount, GL_TRIANGLES);

	//mvStack.pop();//planet
	//mvStack.pop();//sun

	//glUseProgram(shaderProgram2);

	//rt3d::setLight(shaderProgram, light0);
	//rt3d::setMaterial(shaderProgram, material0);
	//projection = glm::perspective(float(60.0f*DEG_TO_RADIAN), 800.0f / 600.0f, 1.0f, 50.0f);
	//rt3d::setUniformMatrix4fv(shaderProgram, "projection", glm::value_ptr(projection));

	//			  
	//// planet2
	//mvStack.push(mvStack.top());// push modelview to stack
	//mvStack.top() = glm::translate(mvStack.top(), glm::vec3(2.0f, 0.0f, 0.0f));
	//mvStack.top() = glm::rotate(mvStack.top(), float((rot + 0.25f)*DEG_TO_RADIAN), glm::vec3(0.0f, 1.0f, 0.0f));
	//mvStack.top() = glm::scale(mvStack.top(), glm::vec3(0.25f, 0.25f, 0.25f));
	//MVP = projection * mvStack.top();
	//rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
	//rt3d::drawIndexedMesh(meshObjects[0], cubeIndexCount, GL_TRIANGLES);
	//
	//// moon1
	//mvStack.push(mvStack.top());// push modelview to stack
	//mvStack.top() = glm::translate(mvStack.top(), glm::vec3(-1.0f, 0.0f, 0.0f));
	//mvStack.top() = glm::rotate(mvStack.top(), float((rot + 0.25f)*DEG_TO_RADIAN), glm::vec3(0.0f, 1.0f, 0.0f));
	//mvStack.top() = glm::scale(mvStack.top(), glm::vec3(0.5f, 0.5f, 0.5f));
	//MVP = projection * mvStack.top();
	//rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
	//rt3d::drawIndexedMesh(meshObjects[0], cubeIndexCount, GL_TRIANGLES);

	//mvStack.pop();//planet

	//// moon2
	//mvStack.push(mvStack.top());// push modelview to stack
	//mvStack.top() = glm::translate(mvStack.top(), glm::vec3(1.0f, 0.0f, 0.0f));
	//mvStack.top() = glm::rotate(mvStack.top(), float((rot + 0.25f)*DEG_TO_RADIAN), glm::vec3(0.0f, 1.0f, 0.0f));
	//mvStack.top() = glm::scale(mvStack.top(), glm::vec3(0.25f, 0.25f, 0.25f));
	//MVP = projection * mvStack.top();
	//rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
	//rt3d::drawIndexedMesh(meshObjects[0], cubeIndexCount, GL_TRIANGLES);

	//mvStack.pop();//planet
	//mvStack.pop();//sun

	//// planet3
	//mvStack.push(mvStack.top());// push modelview to stack
	//mvStack.top() = glm::translate(mvStack.top(), glm::vec3(0.0f, 0.0f, 2.0f));
	//mvStack.top() = glm::rotate(mvStack.top(), float((rot + 0.25f)*DEG_TO_RADIAN), glm::vec3(0.0f, 1.0f, 0.0f));
	//mvStack.top() = glm::scale(mvStack.top(), glm::vec3(0.66f, 0.66f, 0.66f));
	//MVP = projection * mvStack.top();
	//rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
	//rt3d::drawIndexedMesh(meshObjects[0], cubeIndexCount, GL_TRIANGLES);

	//// moon1
	//mvStack.push(mvStack.top());// push modelview to stack
	//mvStack.top() = glm::translate(mvStack.top(), glm::vec3(-1.0f, 0.0f, 0.0f));
	//mvStack.top() = glm::rotate(mvStack.top(), float((rot + 0.25f)*DEG_TO_RADIAN), glm::vec3(0.0f, 1.0f, 0.0f));
	//mvStack.top() = glm::scale(mvStack.top(), glm::vec3(0.5f, 0.5f, 0.5f));
	//MVP = projection * mvStack.top();
	//rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
	//rt3d::drawIndexedMesh(meshObjects[0], cubeIndexCount, GL_TRIANGLES);

	//mvStack.pop();//planet

	//// moon2
	//mvStack.push(mvStack.top());// push modelview to stack
	//mvStack.top() = glm::translate(mvStack.top(), glm::vec3(1.0f, 0.0f, 0.0f));
	//mvStack.top() = glm::rotate(mvStack.top(), float((rot + 0.25f)*DEG_TO_RADIAN), glm::vec3(0.0f, 1.0f, 0.0f));
	//mvStack.top() = glm::scale(mvStack.top(), glm::vec3(0.25f, 0.25f, 0.25f));
	//MVP = projection * mvStack.top();
	//rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
	//rt3d::drawIndexedMesh(meshObjects[0], cubeIndexCount, GL_TRIANGLES);

	//mvStack.pop();//planet

	//// moon2
	//mvStack.push(mvStack.top());// push modelview to stack
	//mvStack.top() = glm::translate(mvStack.top(), glm::vec3(0.0f, 0.0f, 1.0f));
	//mvStack.top() = glm::rotate(mvStack.top(), float((rot + 0.25f)*DEG_TO_RADIAN), glm::vec3(0.0f, 1.0f, 0.0f));
	//mvStack.top() = glm::scale(mvStack.top(), glm::vec3(0.1f, 0.1f, 0.1f));
	//MVP = projection * mvStack.top();
	//rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
	//rt3d::drawIndexedMesh(meshObjects[0], cubeIndexCount, GL_TRIANGLES);

	//mvStack.pop();//planet
	//mvStack.pop();//sun

#pragma endregion galaxy

	SDL_GL_SwapWindow(window); // swap buffers
}


// Program entry point - SDL manages the actual WinMain entry point for us
int main(int argc, char *argv[]) {
	SDL_Window * hWindow; // window handle
	SDL_GLContext glContext; // OpenGL context handle
	hWindow = setupRC(glContext); // Create window and render context 

								  // Required on Windows *only* init GLEW to access OpenGL beyond 1.1
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err) { // glewInit failed, something is seriously wrong
		std::cout << "glewInit failed, aborting." << endl;
		exit(1);
	}
	cout << glGetString(GL_VERSION) << endl;

	init();

	bool running = true; // set running to true
	SDL_Event sdlEvent;  // variable to detect SDL events
	while (running) {	// the event loop
		while (SDL_PollEvent(&sdlEvent)) {
			if (sdlEvent.type == SDL_QUIT)
				running = false;
		}
		update();
		draw(hWindow); // call the draw function
	}

	SDL_GL_DeleteContext(glContext);
	SDL_DestroyWindow(hWindow);
	SDL_Quit();
	return 0;
}