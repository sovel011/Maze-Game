#include <GL/glew.h>   //Include order can matter here
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2_mixer/SDL_mixer.h>

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include "util.h"

using namespace std;

bool saveOutput = false;
float timePast = 0;
float timeLimit = 0;

bool DEBUG_ON = false;
GLuint InitShader(const char* vShaderFileName, const char* fShaderFileName);
bool fullscreen = false;
void Win2PPM(int width, int height);
bool hitWall(float x, float y, float rx, float ry, MapData data, char dir);
bool win = false;
bool jump = false;
float jumpHeight = 0;
float angle2 = 0;
float speed = 1;
float pUpTime = 0;
int variable = 1;
Character character;

Character::Character()
{
	hasA = false, hasB = false, hasC = false, hasD = false, hasE = false;
	openedA = false, openedB = false, openedC = false, openedD = false, openedE = false;
}

int main(int argc, char *argv[]){
	
	
    while(variable){
	if (argc != 1)
	{
		printf("ERROR Usage: %s\n", argv[0]);
		return -1;
	}
    
    win = false;
    printf("%d\n", variable);
    char* map;
    if (variable == 1)
    {
        map = "map.txt";
    }
    else if (variable == 2)
    {
        map = "map1.txt";
    }
    else
    {
        variable = 0;
        break;
    }
	float angle = 1.57, rotateX = 1, rotateY = 0;
	character = Character();
	
    SDL_Init(SDL_INIT_VIDEO);  //Initialize Graphics (for OpenGL)
    
    //Ask SDL to get a recent version of OpenGL (3.2 or greater)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	
    Mix_Music* theme;
    Mix_Chunk* jump_sound;
    Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096);
    theme = Mix_LoadMUS("theme.mp3");
    Mix_PlayMusic(theme, -1);
	
	//Create a window (offsetx, offsety, width, height, flags)
	SDL_Window* window = SDL_CreateWindow("My OpenGL Program", 100, 100, 800, 600, SDL_WINDOW_OPENGL);
	
	//The above window cannot be resized which makes some code slightly easier.
	//Below show how to make a full screen window or allow resizing
	//SDL_Window* window = SDL_CreateWindow("My OpenGL Program", 0, 0, 800, 600, SDL_WINDOW_FULLSCREEN|SDL_WINDOW_OPENGL);
	//SDL_Window* window = SDL_CreateWindow("My OpenGL Program", 100, 100, 800, 600, SDL_WINDOW_RESIZABLE|SDL_WINDOW_OPENGL);
	//SDL_Window* window = SDL_CreateWindow("My OpenGL Program",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,0,0,SDL_WINDOW_FULLSCREEN_DESKTOP|SDL_WINDOW_OPENGL); //Boarderless window "fake" full screen
    
	
	//Create a context to draw in
	SDL_GLContext context = SDL_GL_CreateContext(window);
	
	
	//GLEW loads new OpenGL functions
	glewExperimental = GL_TRUE; //Use the new way of testing which methods are supported
	glewInit();
	
	//Build a Vertex Array Object. This stores the VBO and attribute mappings in one object
	GLuint vao;
	glGenVertexArrays(1, &vao); //Create a VAO
	glBindVertexArray(vao); //Bind the above created VAO to the current context
	
	
	MapData data = MapData(map);
	int numLines = data.numLines;
	float* modelData = data.modelData;
	int numTris = numLines/8;
	
	float x = data.startX, y = data.startY;
	timeLimit = data.timeLimit;
	
	//WALL TEXTURE
	SDL_Surface* surface = SDL_LoadBMP("Textures/wall.bmp");
	if (surface==NULL){ //If it failed, print the error
        printf("Error: \"%s\"\n",SDL_GetError()); return 1;
    }
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    
    //Load the texture into memory
    glActiveTexture(GL_TEXTURE0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w,surface->h, 0, GL_BGR,GL_UNSIGNED_BYTE,surface->pixels);
    
    //What to do outside 0-1 range
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glGenerateMipmap(GL_TEXTURE_2D);
    
    SDL_FreeSurface(surface);
    
    //FLOOR TEXTURE
    SDL_Surface* surface2 = SDL_LoadBMP("Textures/stone.bmp");
	if (surface2==NULL){ //If it failed, print the error
		printf("Error: \"%s\"\n",SDL_GetError()); return 1;
	}
	GLuint floorTex;
	glGenTextures(1, &floorTex);
	glBindTexture(GL_TEXTURE_2D, floorTex);
	
	//Load the texture into memory
	glActiveTexture(GL_TEXTURE0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface2->w,surface2->h, 0, GL_BGR,GL_UNSIGNED_BYTE,surface2->pixels);

	glGenerateMipmap(GL_TEXTURE_2D);
	SDL_FreeSurface(surface2);
	
	//TEAPOT TEXTURE
	SDL_Surface* surface3 = SDL_LoadBMP("Textures/goldtex.bmp");
	if (surface3==NULL){ //If it failed, print the error
		printf("Error: \"%s\"\n",SDL_GetError()); return 1;
	}
	GLuint teapotTex;
	glGenTextures(1, &teapotTex);
	glBindTexture(GL_TEXTURE_2D, teapotTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	//Load the texture into memory
	glActiveTexture(GL_TEXTURE0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface3->w,surface3->h, 0, GL_BGR,GL_UNSIGNED_BYTE,surface3->pixels);

	glGenerateMipmap(GL_TEXTURE_2D);
	SDL_FreeSurface(surface3);
	
	//DOOR A TEXTURE
	SDL_Surface* surface4 = SDL_LoadBMP("Textures/doorA.bmp");
	if (surface4==NULL){ //If it failed, print the error
		printf("Error: \"%s\"\n",SDL_GetError()); return 1;
	}
	GLuint doorTexA;
	glGenTextures(1, &doorTexA);
	glBindTexture(GL_TEXTURE_2D, doorTexA);
	
	//Load the texture into memory
	glActiveTexture(GL_TEXTURE0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface4->w,surface4->h, 0, GL_BGR,GL_UNSIGNED_BYTE,surface4->pixels);

	glGenerateMipmap(GL_TEXTURE_2D);
	SDL_FreeSurface(surface4);
	
	//DOOR B TEXTURE
	SDL_Surface* surface5 = SDL_LoadBMP("Textures/doorB.bmp");
	if (surface5==NULL){ //If it failed, print the error
		printf("Error: \"%s\"\n",SDL_GetError()); return 1;
	}
	GLuint doorTexB;
	glGenTextures(1, &doorTexB);
	glBindTexture(GL_TEXTURE_2D, doorTexB);
	
	//Load the texture into memory
	glActiveTexture(GL_TEXTURE0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface5->w,surface5->h, 0, GL_BGR,GL_UNSIGNED_BYTE,surface5->pixels);

	glGenerateMipmap(GL_TEXTURE_2D);
	SDL_FreeSurface(surface5);
	
	//DOOR C TEXTURE
	SDL_Surface* surface6 = SDL_LoadBMP("Textures/doorC.bmp");
	if (surface6==NULL){ //If it failed, print the error
		printf("Error: \"%s\"\n",SDL_GetError()); return 1;
	}
	GLuint doorTexC;
	glGenTextures(1, &doorTexC);
	glBindTexture(GL_TEXTURE_2D, doorTexC);
	
	//Load the texture into memory
	glActiveTexture(GL_TEXTURE0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface6->w,surface6->h, 0, GL_BGR,GL_UNSIGNED_BYTE,surface6->pixels);

	glGenerateMipmap(GL_TEXTURE_2D);
	SDL_FreeSurface(surface6);
	
	//DOOR D TEXTURE
	SDL_Surface* surface7 = SDL_LoadBMP("Textures/doorD.bmp");
	if (surface7==NULL){ //If it failed, print the error
		printf("Error: \"%s\"\n",SDL_GetError()); return 1;
	}
	GLuint doorTexD;
	glGenTextures(1, &doorTexD);
	glBindTexture(GL_TEXTURE_2D, doorTexD);
	
	//Load the texture into memory
	glActiveTexture(GL_TEXTURE0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface7->w,surface7->h, 0, GL_BGR,GL_UNSIGNED_BYTE,surface7->pixels);

	glGenerateMipmap(GL_TEXTURE_2D);
	SDL_FreeSurface(surface7);
	
	//DOOR E TEXTURE
	SDL_Surface* surface8 = SDL_LoadBMP("Textures/doorE.bmp");
	if (surface4==NULL){ //If it failed, print the error
		printf("Error: \"%s\"\n",SDL_GetError()); return 1;
	}
	GLuint doorTexE;
	glGenTextures(1, &doorTexE);
	glBindTexture(GL_TEXTURE_2D, doorTexE);
	
	//Load the texture into memory
	glActiveTexture(GL_TEXTURE0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface8->w,surface8->h, 0, GL_BGR,GL_UNSIGNED_BYTE,surface8->pixels);

	glGenerateMipmap(GL_TEXTURE_2D);
	SDL_FreeSurface(surface8);
	
	//KEY A TEXTURE
	SDL_Surface* surface9 = SDL_LoadBMP("Textures/red.bmp");
	if (surface9==NULL){ //If it failed, print the error
		printf("Error: \"%s\"\n",SDL_GetError()); return 1;
	}
	GLuint keyTexA;
	glGenTextures(1, &keyTexA);
	glBindTexture(GL_TEXTURE_2D, keyTexA);
	
	//Load the texture into memory
	glActiveTexture(GL_TEXTURE0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface9->w,surface9->h, 0, GL_BGR,GL_UNSIGNED_BYTE,surface9->pixels);

	glGenerateMipmap(GL_TEXTURE_2D);
	SDL_FreeSurface(surface9);
	
	//KEY B TEXTURE
	SDL_Surface* surface10 = SDL_LoadBMP("Textures/blue.bmp");
	if (surface10==NULL){ //If it failed, print the error
		printf("Error: \"%s\"\n",SDL_GetError()); return 1;
	}
	GLuint keyTexB;
	glGenTextures(1, &keyTexB);
	glBindTexture(GL_TEXTURE_2D, keyTexB);
	
	//Load the texture into memory
	glActiveTexture(GL_TEXTURE0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface10->w,surface10->h, 0, GL_BGR,GL_UNSIGNED_BYTE,surface10->pixels);

	glGenerateMipmap(GL_TEXTURE_2D);
	SDL_FreeSurface(surface10);
	
	//KEY C TEXTURE
	SDL_Surface* surface11 = SDL_LoadBMP("Textures/green.bmp");
	if (surface11==NULL){ //If it failed, print the error
		printf("Error: \"%s\"\n",SDL_GetError()); return 1;
	}
	GLuint keyTexC;
	glGenTextures(1, &keyTexC);
	glBindTexture(GL_TEXTURE_2D, keyTexC);
	
	//Load the texture into memory
	glActiveTexture(GL_TEXTURE0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface11->w,surface11->h, 0, GL_BGR,GL_UNSIGNED_BYTE,surface11->pixels);

	glGenerateMipmap(GL_TEXTURE_2D);
	SDL_FreeSurface(surface11);
	
	//KEY D TEXTURE
	SDL_Surface* surface12 = SDL_LoadBMP("Textures/purple.bmp");
	if (surface12==NULL){ //If it failed, print the error
		printf("Error: \"%s\"\n",SDL_GetError()); return 1;
	}
	GLuint keyTexD;
	glGenTextures(1, &keyTexD);
	glBindTexture(GL_TEXTURE_2D, keyTexD);
	
	//Load the texture into memory
	glActiveTexture(GL_TEXTURE0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface12->w,surface12->h, 0, GL_BGR,GL_UNSIGNED_BYTE,surface12->pixels);

	glGenerateMipmap(GL_TEXTURE_2D);
	SDL_FreeSurface(surface12);
	
	//KEY E TEXTURE
	SDL_Surface* surface13 = SDL_LoadBMP("Textures/yellow.bmp");
	if (surface13==NULL){ //If it failed, print the error
		printf("Error: \"%s\"\n",SDL_GetError()); return 1;
	}
	GLuint keyTexE;
	glGenTextures(1, &keyTexE);
	glBindTexture(GL_TEXTURE_2D, keyTexE);
	
	//Load the texture into memory
	glActiveTexture(GL_TEXTURE0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface13->w,surface13->h, 0, GL_BGR,GL_UNSIGNED_BYTE,surface13->pixels);

	glGenerateMipmap(GL_TEXTURE_2D);
	SDL_FreeSurface(surface13);
	
	//KEY E TEXTURE
	SDL_Surface* surface14 = SDL_LoadBMP("Textures/powerUp.bmp");
	if (surface14==NULL){ //If it failed, print the error
		printf("Error: \"%s\"\n",SDL_GetError()); return 1;
	}
	GLuint powerUpTex;
	glGenTextures(1, &powerUpTex);
	glBindTexture(GL_TEXTURE_2D, powerUpTex);
	
	//Load the texture into memory
	glActiveTexture(GL_TEXTURE0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface14->w,surface14->h, 0, GL_BGR,GL_UNSIGNED_BYTE,surface14->pixels);

	glGenerateMipmap(GL_TEXTURE_2D);
	SDL_FreeSurface(surface14);
	
	
	
	//Allocate memory on the graphics card to store geometry (vertex buffer object)
	GLuint vbo[1];
	glGenBuffers(1, vbo);  //Create 1 buffer called vbo
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); //Set the vbo as the active array buffer (Only one buffer can be active at a time)
	//GL_STATIC_DRAW means we won't change the geometry, GL_DYNAMIC_DRAW = geometry changes infrequently
	//GL_STREAM_DRAW = geom. changes frequently.  This effects which types of GPU memory is used
	
	int shaderProgram = InitShader("vertexTex.glsl", "fragmentTex.glsl");
	glUseProgram(shaderProgram); //Set the active shader (only one can be used at a time)
	
	
	 
	glEnable(GL_DEPTH_TEST);  
	
	//Event Loop (Loop forever processing each event as fast as possible)
	SDL_Event windowEvent;
	while (true){
	  if (SDL_PollEvent(&windowEvent)){
		if (windowEvent.type == SDL_QUIT)
        {
            variable = 0; //Exit event loop
            break;
        }
		//List of keycodes: https://wiki.libsdl.org/SDL_Keycode - You can catch many special keys
		//Scancode referes to a keyboard position, keycode referes to the letter (e.g., EU keyboards)
        if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_ESCAPE)
        {
		  variable = 0; //Exit event loop
          break;
        }
		if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_DOWN) 
		{
			if (!hitWall(x,y,rotateX,rotateY, data, 'D'))
			{
				x += rotateX*speed;
				y += -rotateY*speed;
			}
		}
		if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_UP)
		{
			if (!hitWall(x,y,rotateX,rotateY, data, 'U'))
			{
				x -= rotateX*speed;
				y -= -rotateY*speed;
			}
		}
		if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_LEFT)
		{
			angle += .15;
			rotateX = sin(angle);
			rotateY = cos(angle);
		}
		if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_RIGHT)
		{
			angle -= .15;
			rotateX = sin(angle);
			rotateY = cos(angle);
		}
		if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_SPACE)
		{
			jump = true;
		}
		if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_f) //If "f" is pressed
		  fullscreen = !fullscreen;
		  SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0); //Toggle fullscreen 
	  }
      
      
      
      if (!saveOutput) timePast = SDL_GetTicks()/1000.f; 
      if (saveOutput) timePast += .07; //Fix framerate at 14 FPS
      
      float timeLeft = timeLimit - timePast;
      if (timeLeft <= 0.0)
      {
          variable = 0;
    	  printf("------------------------\n");
    	  printf("| YOU RAN OUT OF TIME! |\n");
    	  printf("------------------------\n");
    	  break;
      }
      
      if (timePast - pUpTime > 7)
      {
    	  character.powerUp = false;
    	  speed = 1;
      }
      
      if (jump)
      {
    	  angle2 += .2;
    	  jumpHeight = 1.5*sin(angle2);
    	  if (jumpHeight < .15)
    	  {
    		  jumpHeight = 0;
    		  angle2 = 0;
    		  jump = false;
    	  }
      }
      
     glUseProgram(shaderProgram); 
     
     glBufferData(GL_ARRAY_BUFFER, numTris*8*sizeof(float), modelData, GL_STATIC_DRAW);
     
     //Tell OpenGL how to set fragment shader input 
	GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), 0);
	  //Attribute, vals/attrib., type, normalized?, stride, offset
	  //Binds to VBO current GL_ARRAY_BUFFER 
	glEnableVertexAttribArray(posAttrib);
	
	//GLint colAttrib = glGetAttribLocation(shaderProgram, "inColor");
	//glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
	//glEnableVertexAttribArray(colAttrib);
	
	GLint normAttrib = glGetAttribLocation(shaderProgram, "inNormal");
	glVertexAttribPointer(normAttrib, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(5*sizeof(float)));
	glEnableVertexAttribArray(normAttrib);
	
	GLint texAttrib = glGetAttribLocation(shaderProgram, "inTexcoord");
	glEnableVertexAttribArray(texAttrib);
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE,
                       8*sizeof(float), (void*)(3*sizeof(float)));

      
      
      
      GLint uniModel = glGetUniformLocation(shaderProgram, "model");
      GLint uniView = glGetUniformLocation(shaderProgram, "view");
      GLint uniProj = glGetUniformLocation(shaderProgram, "proj");

        
        // Clear the screen to default color
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
                 
        
        glm::mat4 view = glm::lookAt(
		glm::vec3(x, y, jumpHeight),  //Cam Position
		glm::vec3(x-rotateX, y+rotateY, jumpHeight),  //Look at point
		glm::vec3(0.0f, 0.0f, 1.0f)); //Up
      
        glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));
      
      
        glm::mat4 proj = glm::perspective(3.14f/4, 800.0f / 600.0f, 0.5f, 500.0f); //FOV, aspect, near, far
        glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));
      
      	glm::mat4 model;
        glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
      
        int numObjects = data.numObjects;
        int index = 0;
        for(int i = 0; i < numObjects; i++)
        {
        	object current = data.objects[i];
        	if (current.type == 'F')
        	{
        		glBindTexture(GL_TEXTURE_2D, floorTex);
        	}
        	if (current.type == 'G')
        	{
        		model = glm::mat4();
        		model = glm::translate(model,glm::vec3(0,0,sin(timePast)/4));
        		glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
        		glBindTexture(GL_TEXTURE_2D, teapotTex);
        	}
        	if (current.type == 'W')
        	{
        		model = glm::mat4();
        		glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
        		glBindTexture(GL_TEXTURE_2D, tex);
        	}
        	if (current.type >= 65 && current.type <= 69)
        	{
        		model = glm::mat4();
        		glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
        		if (current.type == 'A')
        			glBindTexture(GL_TEXTURE_2D, doorTexA);
        		if (current.type == 'B')
        			glBindTexture(GL_TEXTURE_2D, doorTexB);
        		if (current.type == 'C')
        		    glBindTexture(GL_TEXTURE_2D, doorTexC);
        		if (current.type == 'D')
        		    glBindTexture(GL_TEXTURE_2D, doorTexD);
        		if (current.type == 'E')
        		    glBindTexture(GL_TEXTURE_2D, doorTexE);
        	}
        	if (current.type >= 97 && current.type <= 101)
			{
				model = glm::mat4();
				float keyX = current.x, keyY = current.y;
				int gridHeight = data.mapHeight, gridWidth = data.mapWidth;
				if (current.type == 'a')
				{
					if (character.openedA)
						model = glm::scale(model, glm::vec3(0.0f)); 
					else if (character.hasA)
					{
						keyX = keyX*5 + 2.5;
						keyX -= (gridHeight*5)/2.0;
						keyY = keyY*5 + 2.5;
						keyY -= ((gridWidth)*5)/2.0;
						model = glm::translate(model, glm::vec3(keyX, -keyY, .04));
						model = glm::translate(model, glm::vec3(x-rotateX, y+rotateY, 0));
						glBindTexture(GL_TEXTURE_2D, keyTexA);
					}
					else 
					{
						model = glm::translate(model,glm::vec3(0,0,sin(timePast)/4));
						glBindTexture(GL_TEXTURE_2D, keyTexA);
					}
				}
				if (current.type == 'b')
				{
					if (character.openedB)
						model = glm::scale(model, glm::vec3(0.0f)); 
					else if (character.hasB)
					{
						keyX = keyX*5 + 2.5;
						keyX -= (gridHeight*5)/2.0;
						keyY = keyY*5 + 2.5;
						keyY -= ((gridWidth)*5)/2.0;
						model = glm::translate(model, glm::vec3(keyX, -keyY, .03));
						model = glm::translate(model, glm::vec3(-rotateX, rotateY, 0));
						model = glm::translate(model, glm::vec3(x, y, 0));
						glBindTexture(GL_TEXTURE_2D, keyTexB);
					}
					else
					{
						model = glm::translate(model,glm::vec3(0,0,sin(timePast)/4));
						glBindTexture(GL_TEXTURE_2D, keyTexB);
					}
				}
				if (current.type == 'c')
				{
					if (character.openedC)
						model = glm::scale(model, glm::vec3(0.0f)); 
					else if (character.hasC)
					{
						keyX = keyX*5 + 2.5;
						keyX -= (gridHeight*5)/2.0;
						keyY = keyY*5 + 2.5;
						keyY -= ((gridWidth)*5)/2.0;
						model = glm::translate(model, glm::vec3(keyX, -keyY, .02));
						model = glm::translate(model, glm::vec3(-rotateX, rotateY, 0));
						model = glm::translate(model, glm::vec3(x, y, 0));
						glBindTexture(GL_TEXTURE_2D, keyTexC);
					}
					else 
					{
						model = glm::translate(model,glm::vec3(0,0,sin(timePast)/4));
						glBindTexture(GL_TEXTURE_2D, keyTexC);
					}
				}
				if (current.type == 'd')
				{
					if (character.openedD)
						model = glm::scale(model, glm::vec3(0.0f)); 
					else if (character.hasD)
					{
						keyX = keyX*5 + 2.5;
						keyX -= (gridHeight*5)/2.0;
						keyY = keyY*5 + 2.5;
						keyY -= ((gridWidth)*5)/2.0;
						model = glm::translate(model, glm::vec3(keyX, -keyY, .01));
						model = glm::translate(model, glm::vec3(-rotateX, rotateY, 0));
						model = glm::translate(model, glm::vec3(x, y, 0));
						glBindTexture(GL_TEXTURE_2D, keyTexD);
					}
					else
					{
						model = glm::translate(model,glm::vec3(0,0,sin(timePast)/4));
						glBindTexture(GL_TEXTURE_2D, keyTexD);
					}
				}
				if (current.type == 'e')
				{
					if (character.openedE)
						model = glm::scale(model, glm::vec3(0.0f)); 
					else if (character.hasE)
					{
						keyX = keyX*5 + 2.5;
						keyX -= (gridHeight*5)/2.0;
						keyY = keyY*5 + 2.5;
						keyY -= ((gridWidth)*5)/2.0;
						model = glm::translate(model, glm::vec3(keyX, -keyY, 0));
						model = glm::translate(model, glm::vec3(-rotateX, rotateY, 0));
						model = glm::translate(model, glm::vec3(x, y, 0));
						glBindTexture(GL_TEXTURE_2D, keyTexE);
					}
					else
					{
						model = glm::translate(model,glm::vec3(0,0,sin(timePast)/4));
						glBindTexture(GL_TEXTURE_2D, keyTexE);
					}
				}
			glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
			}
        	if (current.type == 'P')
			{
				model = glm::mat4();
				if (character.powerUp)
				{
					model = glm::scale(model, glm::vec3(0.0f));
				}
				else
				{
					model = glm::translate(model,glm::vec3(0,0,sin(timePast)/4));
					glBindTexture(GL_TEXTURE_2D, powerUpTex);
				}
				glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
			}
        	glDrawArrays(GL_TRIANGLES, index, current.lines/8);
        	index += current.lines/8;
        }
      

        
      
           
      if (saveOutput) Win2PPM(800,600);
      

      SDL_GL_SwapWindow(window); //Double buffering
      if (win)
      {
          variable++;
    	  printf("========================================================================\n");
    	  printf("CONGRATULATIONS, YOU'VE FOUND THE GOLDEN TEAPOT!\n");
    	  printf("========================================================================\n");
    	  break;
      }
	}
	
    Mix_FreeMusic(theme);
    Mix_CloseAudio();
	glDeleteProgram(shaderProgram);
    glDeleteBuffers(1, vbo);
    glDeleteVertexArrays(1, &vao);

	//Clean Up
	SDL_GL_DeleteContext(context);
	SDL_Quit();
    }
	return 0;
    
}

// Create a NULL-terminated string by reading the provided file
static char* readShaderSource(const char* shaderFile)
{
	FILE *fp;
	long length;
	char *buffer;

	// open the file containing the text of the shader code
	fp = fopen(shaderFile, "r");

	// check for errors in opening the file
	if (fp == NULL) {
		printf("can't open shader source file %s\n", shaderFile);
		return NULL;
	}

	// determine the file size
	fseek(fp, 0, SEEK_END); // move position indicator to the end of the file;
	length = ftell(fp);  // return the value of the current position

	// allocate a buffer with the indicated number of bytes, plus one
	buffer = new char[length + 1];

	// read the appropriate number of bytes from the file
	fseek(fp, 0, SEEK_SET);  // move position indicator to the start of the file
	fread(buffer, 1, length, fp); // read all of the bytes

	// append a NULL character to indicate the end of the string
	buffer[length] = '\0';

	// close the file
	fclose(fp);

	// return the string
	return buffer;
}

// Create a GLSL program object from and fragment shader files
GLuint InitShader(const char* vShaderFileName, const char* fShaderFileName)
{
	GLuint vertex_shader, fragment_shader;
	GLchar *vs_text, *fs_text;
	GLuint program;

	// check GLSL version
	//printf("GLSL version: %s\n\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	// Create shader handlers
	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

	// Read source code from shader files
	vs_text = readShaderSource(vShaderFileName);
	fs_text = readShaderSource(fShaderFileName);

	// error check
	if (vs_text == NULL) {
		printf("Failed to read from vertex shader file %s\n", vShaderFileName);
		exit(1);
	} else if (DEBUG_ON) {
		printf("Vertex Shader:\n=====================\n");
		printf("%s\n", vs_text);
		printf("=====================\n\n");
	}
	if (fs_text == NULL) {
		printf("Failed to read from fragent shader file %s\n", fShaderFileName);
		exit(1);
	} else if (DEBUG_ON) {
		printf("\nFragment Shader:\n=====================\n");
		printf("%s\n", fs_text);
		printf("=====================\n\n");
	}

	// Load Vertex Shader
	const char *vv = vs_text;
	glShaderSource(vertex_shader, 1, &vv, NULL);  //Read source
	glCompileShader(vertex_shader); // Compile shaders
	
	// Check for errors
	GLint  compiled;
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		printf("Vertex shader failed to compile:\n");
		if (DEBUG_ON) {
			GLint logMaxSize, logLength;
			glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &logMaxSize);
			printf("printing error message of %d bytes\n", logMaxSize);
			char* logMsg = new char[logMaxSize];
			glGetShaderInfoLog(vertex_shader, logMaxSize, &logLength, logMsg);
			printf("%d bytes retrieved\n", logLength);
			printf("error message: %s\n", logMsg);
			delete[] logMsg;
		}
		exit(1);
	}
	
	// Load Fragment Shader
	const char *ff = fs_text;
	glShaderSource(fragment_shader, 1, &ff, NULL);
	glCompileShader(fragment_shader);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compiled);
	
	//Check for Errors
	if (!compiled) {
		printf("Fragment shader failed to compile\n");
		if (DEBUG_ON) {
			GLint logMaxSize, logLength;
			glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &logMaxSize);
			printf("printing error message of %d bytes\n", logMaxSize);
			char* logMsg = new char[logMaxSize];
			glGetShaderInfoLog(fragment_shader, logMaxSize, &logLength, logMsg);
			printf("%d bytes retrieved\n", logLength);
			printf("error message: %s\n", logMsg);
			delete[] logMsg;
		}
		exit(1);
	}

	// Create the program
	program = glCreateProgram();

	// Attach shaders to program
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);

	// Link and set program to use
	glLinkProgram(program);
	glUseProgram(program);

	return program;
}

void Win2PPM(int width, int height){
	char outdir[10] = "out/"; //Must be defined!
	int i,j;
	FILE* fptr;
    static int counter = 0;
    char fname[32];
    unsigned char *image;
    
    /* Allocate our buffer for the image */
    image = (unsigned char *)malloc(3*width*height*sizeof(char));
    if (image == NULL) {
      fprintf(stderr,"ERROR: Failed to allocate memory for image\n");
    }
    
    /* Open the file */
    sprintf(fname,"%simage_%04d.ppm",outdir,counter);
    if ((fptr = fopen(fname,"w")) == NULL) {
      fprintf(stderr,"ERROR: Failed to open file for window capture\n");
    }
    
    /* Copy the image into our buffer */
    glReadBuffer(GL_BACK);
    glReadPixels(0,0,width,height,GL_RGB,GL_UNSIGNED_BYTE,image);
    
    /* Write the PPM file */
    fprintf(fptr,"P6\n%d %d\n255\n",width,height);
    for (j=height-1;j>=0;j--) {
      for (i=0;i<width;i++) {
         fputc(image[3*j*width+3*i+0],fptr);
         fputc(image[3*j*width+3*i+1],fptr);
         fputc(image[3*j*width+3*i+2],fptr);
      }
    }
    
    free(image);
    fclose(fptr);
    counter++;
}

bool hitWall(float x, float y, float rx, float ry, MapData data, char dir)
{
	float newX, newY;
	float actualX, actualY;
	if (dir == 'U')
	{
		actualX = x - rx*1;
		actualY = y - -ry*1;
		newX = x - rx*2.25;
		newY = y - -ry*2.25;
	}
	else
	{
		actualX = x + rx*1;
		actualY = y + -ry*1;
		newX = x + rx*2.25;
		newY = y + -ry*2.25;
	}
	int height = data.mapHeight;
	int width = data.mapWidth;
	int gridV = (-newX + ((height*5)/2.0))/5;
	int gridH = (newY + ((width*5)/2.0))/5;
	char type = data.grid[gridV][gridH];
	if (type == 'W')
	{
		return true;
	}
	else if (type == 'G')
	{
		float centerX = -(gridV*5 + 2.5 - (height*5)/2.0); 
		float centerY = gridH*5 + 2.5 - (width*5)/2.0; 
		if (abs(actualX - centerX) < 1.5 && abs(actualY - centerY) < 1.5)
		{
			win = true;
		}
	}
	else if (type >= 65 && type <= 69) // DOOR
	{
		if (!((type == 'A' && character.hasA) || (type == 'B' && character.hasB) || (type == 'C' && character.hasC)
				|| (type == 'D' && character.hasD) || (type == 'E' && character.hasE)))
		{
			return true;
		}
		if (type == 'A')
			character.openedA = true;
		if (type == 'B')
			character.openedB = true;
		if (type == 'C')
			character.openedC = true;
		if (type == 'D')
			character.openedD = true;
		if (type == 'E')
			character.openedE = true;
		float centerX = -(gridV*5 + 2.5 - (height*5)/2.0); 
		float centerY = gridH*5 + 2.5 - (width*5)/2.0; 
		if (abs(actualX - centerX) > .5 && abs(actualY - centerY) > .5)
		{
			if (!(abs(actualX - centerX) < 1.5 && abs(actualY - centerY) < 1.5))
			{
				return true;
			}
		}
	}
	else if (type >= 97 && type <= 101) // KEY
	{
		float centerX = -(gridV*5 + 2.5 - (height*5)/2.0); 
		float centerY = gridH*5 + 2.5 - (width*5)/2.0; 
		if (abs(actualX - centerX) < 1.5 && abs(actualY - centerY) < 1.5)
		{
			if (type == 'a')
				character.hasA = true;
			if (type == 'b')
				character.hasB = true;
			if (type == 'c')
				character.hasC = true;
			if (type == 'd')
				character.hasD = true;
			if (type == 'e')
				character.hasE = true;
		}
	}
	else if (type == 'P')
	{
		float centerX = -(gridV*5 + 2.5 - (height*5)/2.0); 
		float centerY = gridH*5 + 2.5 - (width*5)/2.0; 
		if (abs(actualX - centerX) < 1.5 && abs(actualY - centerY) < 1.5)
		{
			character.powerUp = true;
			speed = 2;
			pUpTime = timePast;
		}
	}
	return false;
}



/**
 * REQUIRED FEATURES:
 * ~~~Generating Walls
 * ~~~Generate Doors (different colors)
 * ~~~key starting points
 * ~~~start at starting point
 * ~~~end game when you touch teapot
 * ~~~Keys (must be rendered in front of player)
 * ~~~Keyboard Movement
 * ~~~Collision Detection
 * ~~~Floors and Ceilings
 * ~~~Lighting
 * 
 * ADDITIONAL FEATURES:
 * ~~~Texture Map (5)
 * Loading OBJ files (5) 
 * Make own key model (5) 
 * ~~~Items that alter characters abilities (5) 
 * ~~~Video (5) 
 * ~~~Jumping (5) 
 * Point Lights (5) 
 * Moving SpotLight (5)
 * ~~~Art Submission (5) 
 * 
 * WHATS LEFT:
 * 
 * ~~~GENERATING KEYS
 * ~~~DOOR OPENS ONLY WHEN YOU HAVE KEY
 * ~~~KEY MOVES WITH YOU (ROTATE WITH YOU)
 * ~~~BETTER KEY HIT DETECTION
 * ~~~KEY DISAPPEARES AFTER DOOR IS OPENED
 * KEY MODEL
 * ~~~MULTIPLE TEXTURES
 * ~~~JUMPING
 * POINT LIGHTS
 * AUDIO?
 * ~~~TIME LIMIT!! (level specific time in map txt file)
 * MOVING SPIKED OBJECT? (if you touch them you die) (<_>)
 
 */



