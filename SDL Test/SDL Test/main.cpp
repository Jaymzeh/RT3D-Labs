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
//glm::mat4 MVP;

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
GLuint textures[6];

GLfloat cubeTexCoords[] = { 0.0f, 0.0f,
0.0f, 1.0f,
1.0f, 1.0f,
1.0f, 0.0f,
1.0f, 1.0f,
1.0f, 0.0f,
0.0f, 0.0f,
0.0f, 1.0f };



GLfloat dx = 0.0f;
GLfloat dy = 0.0f;
GLfloat rot = 0.0f;

GLfloat scale = 3.0f;

GLfloat red = 1.0f;
GLfloat green = 1.0f;
GLfloat blue = 1.0f;

rt3d::lightStruct light0 = {
	{ 0.2f, 0.2f, 0.2f, 1.0f }, // ambient
	{ 0.7f, 0.7f, 0.7f, 1.0f }, // diffuse
	{ 0.8f, 0.8f, 0.8f, 1.0f }, // specular
	{ 0.0f, 0.0f, 1.0f, 1.0f }  // position
};

rt3d::materialStruct whiteMat = {
	{ 1.0f, 1.0f, 1.0f, 1.0f }, // ambient
	{ 1.0f, 1.0f, 1.0f, 1.0f }, // diffuse
	{ 1.0f, 1.0f, 1.0f, 1.0f }, // specular
	2.0f  // shininess
};

rt3d::materialStruct redMat = {
	{ 1.0f, 0.33f, 0.33f, 0.3f }, // ambient
	{ 1.0f, 0.33f, 0.33f, 0.3f }, // diffuse
	{ 1.0f, 0.33f, 0.33f, 0.3f }, // specular
	2.0f  // shininess
};

rt3d::materialStruct greenMat = {
	{ 0.33f, 1.0f, 0.33f, 0.3f }, // ambient
	{ 0.33f, 1.0f, 0.33f, 0.3f }, // diffuse
	{ 0.33f, 1.0f, 0.33f, 0.3f }, // specular
	2.0f  // shininess
};

rt3d::materialStruct blueMat = {
	{ 0.33f, 0.33f, 1.0f, 1.0f }, // ambient
	{ 0.33f, 0.33f, 1.0f, 1.0f }, // diffuse
	{ 0.33f, 0.33f, 1.0f, 1.0f }, // specular
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

	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8); // 8 bit alpha buffering

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

GLuint loadBitmap(char *fname) {
	GLuint texID;
	glGenTextures(1, &texID); // generate texture ID

							  // load file - using core SDL library
	SDL_Surface *tmpSurface;
	tmpSurface = SDL_LoadBMP(fname);
	if (!tmpSurface) {
		std::cout << "Error loading bitmap" << std::endl;
	}

	// bind texture and set parameters
	glBindTexture(GL_TEXTURE_2D, texID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	SDL_PixelFormat *format = tmpSurface->format;
	GLuint externalFormat, internalFormat;
	if (format->Amask) {
		internalFormat = GL_RGBA;
		externalFormat = (format->Rmask < format->Bmask) ? GL_RGBA : GL_BGRA;
	}
	else {
		internalFormat = GL_RGB;
		externalFormat = (format->Rmask < format->Bmask) ? GL_RGB : GL_BGR;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, tmpSurface->w, tmpSurface->h, 0,
		externalFormat, GL_UNSIGNED_BYTE, tmpSurface->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);
	SDL_FreeSurface(tmpSurface); // texture loaded, free the temporary buffer
	return texID;	// return value of texture ID
}


void init(void) {
	// For this simple example we'll be using the most basic of shader programs
	shaderProgram = rt3d::initShaders("phong-tex.vert", "phong-tex.frag");
	shaderProgram2 = rt3d::initShaders("phong.vert", "phong.frag");

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	rt3d::setLight(shaderProgram, light0);
	rt3d::setMaterial(shaderProgram, whiteMat);

	textures[0] = loadBitmap("fabric.bmp");
	textures[1] = loadBitmap("studdedMetal.bmp");
	textures[2] = loadBitmap("paper.bmp");
	textures[3] = loadBitmap("brick.bmp");
	textures[4] = loadBitmap("hexGrid.bmp");
	textures[5] = loadBitmap("neon.bmp");

	meshObjects[0] = rt3d::createMesh(cubeVertCount, cubeVerts, nullptr, cubeVerts, cubeTexCoords, cubeIndexCount, cubeIndices);
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
	if (keys[SDL_SCANCODE_UP]) scale += 0.1;
	if (keys[SDL_SCANCODE_DOWN]) scale -= 0.1;

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

	glUseProgram(shaderProgram);
	

	rt3d::setLight(shaderProgram, light0);
	rt3d::setMaterial(shaderProgram, whiteMat);
	// set up projection
	glm::mat4 projection(1.0);
	glm::mat4 modelview(1.0);
	projection = glm::perspective(float(60.0f*DEG_TO_RADIAN), 800.0f / 600.0f, 1.0f, 50.0f);
	rt3d::setUniformMatrix4fv(shaderProgram, "projection", glm::value_ptr(projection));

#pragma region cubeArray

	//render solid cubes
	glDepthMask(GL_TRUE);

	rt3d::setMaterial(shaderProgram, blueMat);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	mvStack.push(modelview); // push modelview to stack
	mvStack.top() = glm::translate(mvStack.top(), glm::vec3(-2.0f, 1.0f, -12.0f));
	mvStack.top() = glm::rotate(mvStack.top(), float(45 * DEG_TO_RADIAN), glm::vec3(0.0f, 1.0f, 0.0f));
	mvStack.top() = glm::scale(mvStack.top(), glm::vec3(scale, scale, scale));
	rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
	rt3d::drawIndexedMesh(meshObjects[0], cubeIndexCount, GL_TRIANGLES);
	mvStack.pop();

	rt3d::setMaterial(shaderProgram, whiteMat);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	mvStack.push(modelview); // push modelview to stack
	mvStack.top() = glm::translate(mvStack.top(), glm::vec3(2.0f, -1.0f, -12.0f));
	mvStack.top() = glm::rotate(mvStack.top(), float(45 * DEG_TO_RADIAN), glm::vec3(0.0f, 1.0f, 0.0f));
	mvStack.top() = glm::scale(mvStack.top(), glm::vec3(scale, scale, scale));
	rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
	rt3d::drawIndexedMesh(meshObjects[0], cubeIndexCount, GL_TRIANGLES);
	mvStack.pop();

	//render transparent cubes
	glDepthMask(GL_FALSE);

	rt3d::setMaterial(shaderProgram, redMat);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	
	mvStack.push(modelview); // push modelview to stack
	mvStack.top() = glm::translate(mvStack.top(), glm::vec3(-2.0f, 1.0f, -4.0f));
	mvStack.top() = glm::rotate(mvStack.top(), float(45*DEG_TO_RADIAN), glm::vec3(0.0f, 1.0f, 0.0f));
	mvStack.top() = glm::scale(mvStack.top(), glm::vec3(scale, scale, scale));
	rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
	rt3d::drawIndexedMesh(meshObjects[0], cubeIndexCount, GL_TRIANGLES);
	mvStack.pop();

	rt3d::setMaterial(shaderProgram, greenMat);
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	mvStack.push(modelview); // push modelview to stack
	mvStack.top() = glm::translate(mvStack.top(), glm::vec3(-2.0f, 1.0f, -8.0f));
	mvStack.top() = glm::rotate(mvStack.top(), float(45*DEG_TO_RADIAN), glm::vec3(0.0f, 1.0f, 0.0f));
	mvStack.top() = glm::scale(mvStack.top(), glm::vec3(scale, scale, scale));
	rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
	rt3d::drawIndexedMesh(meshObjects[0], cubeIndexCount, GL_TRIANGLES);
	mvStack.pop();

	rt3d::setMaterial(shaderProgram, redMat);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	mvStack.push(modelview); // push modelview to stack
	mvStack.top() = glm::translate(mvStack.top(), glm::vec3(2.0f, -1.0f, -4.0f));
	mvStack.top() = glm::rotate(mvStack.top(), float(45 * DEG_TO_RADIAN), glm::vec3(0.0f, 1.0f, 0.0f));
	mvStack.top() = glm::scale(mvStack.top(), glm::vec3(scale, scale, scale));
	rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
	rt3d::drawIndexedMesh(meshObjects[0], cubeIndexCount, GL_TRIANGLES);
	mvStack.pop();

	rt3d::setMaterial(shaderProgram, greenMat);
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	mvStack.push(modelview); // push modelview to stack
	mvStack.top() = glm::translate(mvStack.top(), glm::vec3(2.0f, -1.0f, -8.0f));
	mvStack.top() = glm::rotate(mvStack.top(), float(45 * DEG_TO_RADIAN), glm::vec3(0.0f, 1.0f, 0.0f));
	mvStack.top() = glm::scale(mvStack.top(), glm::vec3(scale, scale, scale));
	rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
	rt3d::drawIndexedMesh(meshObjects[0], cubeIndexCount, GL_TRIANGLES);
	mvStack.pop();


	

#pragma endregion cubeArray
	glDepthMask(GL_TRUE);
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