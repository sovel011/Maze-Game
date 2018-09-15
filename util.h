#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;


class object
{
	public:
		char type; //use later for textures??
		float* data;
		int lines;
		int x;
		int y;
};

class Character
{
	public:
		bool hasA;
		bool openedA;
		bool hasB;
		bool openedB;
		bool hasC;
		bool openedC;
		bool hasD;
		bool openedD;
		bool hasE;
		bool openedE;
		bool powerUp;
		Character();
};

class MapData
{
	public:
		float* modelData;
		int numLines;
		MapData(const char* file);
		float startX;
		float startY;
		char** grid;
		int mapHeight;
		int mapWidth;
		int numObjects;
		float timeLimit;
		object* objects;
		
};

MapData::MapData(const char* file)
{
	startX = 0;
	startY = 0;
	ifstream mapFile;
	mapFile.open(file);
	int width = 0, height = 0;
	mapFile >> width;
	mapFile >> height;
	mapFile >> timeLimit;
	mapWidth = width;
	mapHeight = height;
	char map[width][height]; //do I need this?
	
	grid = (char**)malloc(width*sizeof(char*));
	for (int i = 0; i < height; i++)
	{
		grid[i] = (char*)malloc(height);
	}
	
	float vertices[] = {
	  //   X          Y        Z      U         V        Nx     Ny     Nz
		height*2.5, -width*2.5, -1.f,    0.0f,     0.0f,    0.0f,  0.0f,  1.0f,
		height*2.5,  width*2.5, -1.f,    0.0f,   width*5,  0.0f,  0.0f,  1.0f,
	   -height*2.5,  width*2.5, -1.f, height*5, width*5,  0.0f,  0.0f,  1.0f,
		 
		height*2.5, -width*2.5, -1.f,    0.0f,     0.0f,    0.0f,  0.0f,  1.0f,
	   -height*2.5,  width*2.5, -1.f, height*5, width*5,  0.0f,  0.0f,  1.0f,
	   -height*2.5, -width*2.5, -1.f, height*5,   0.0f,    0.0f,  0.0f,  1.0f
	};
	
	int totalLines = 48;
	float* floorData = new float[48]; //temporary buffer for floor
	
	object* objects = new object[width*height];
	int objectsIndex = 0;
	
	object floor;
	floor.type = 'F', floor.lines = 48;
	
	object wallObj;
	wallObj.type = 'W';
	
	object goal;
	goal.type = 'G';
	
	object keyObj;
	
	object doorObj;
	
	object powerObj;
	powerObj.type = 'P';
	
	for (int i = 0; i < 48; i++)
	{
		floorData[i] = vertices[i];
	}
	floor.data = floorData;
	objects[0] = floor;
	objectsIndex++;
	
	for (int x = height-1; x >= 0; x--)
	{
		for (int y = 0; y < width; y++)
		{
			mapFile >> map[x][y];
			if (map[x][y] == 'W') //WALL
			{
				int wallLines = 0;
				float* wallData = new float[288]; //temporary buffer for walls
				ifstream wall;
				wall.open("models/cube.txt");
				wall >> wallLines;
				wallObj.lines = wallLines;
				totalLines += wallLines;
				for (int i = 0; i < wallLines; i++)
				{
					float current = 0.0;
					wall >> current;
					if (i % 8 == 0)
					{
						current += .5;
						current *= 5;
						current += ((-height/2.0)*5);
						current += 5*x;
						current *= -1; //
					}
					if (i % 8 == 1)
					{
						current += .5;
						current *= 5;
						current += ((-width/2.0)*5);
						current += 5*y;
					}
					if (i % 8 == 2 && current < 0)
					{
						current = -1;
					}
					if (i % 8 == 3)
					{
						current *= 2.5;
					}
					if (i % 8 == 4)
					{
						current *= 2;
					}
					
					wallData[i] = current;
				}
				wallObj.data = wallData;
				objects[objectsIndex] = wallObj;
				objectsIndex++;
			}
			if (map[x][y] == 'G') //GOAL
			{
				int goalLines = 0;
				ifstream teapot;
				teapot.open("models/teapot.txt");
				teapot >> goalLines;
				float* goalData = new float[goalLines];
				goal.lines = goalLines;
				totalLines += goalLines;
				for (int i = 0; i < goalLines; i++)
				{
					float current = 0.0;
					teapot >> current;
					if (i % 8 == 0)
					{
						current -= .5;
						current += x*5 + 2.5;
						current -= (height*5)/2;
						current *= -1;
					}
					if (i % 8 == 1)
					{
						current -= .5;
						current += y*5 + 2.5;
						current -= (width*5)/2;
					}
					if (i % 8 == 2)
					{
						current -= .3;
					}
					goalData[i] = current;
				}
				goal.data = goalData;
				objects[objectsIndex] = goal;
				objectsIndex++;
			}
			if (map[x][y] == 'a' || map[x][y] == 'b' || map[x][y] == 'c' || map[x][y] == 'd' || map[x][y] == 'e') //KEY
			{
				int keyLines = 0;
				keyObj.type =  map[x][y];
				ifstream key;
				key.open("models/sphere.txt");
				key >> keyLines;
				float* keyData = new float[keyLines];
				keyObj.lines = keyLines;
				totalLines += keyLines;
				for (int i = 0; i < keyLines; i++)
				{
					float current = 0.0;
					key >> current;
					if (i % 8 == 0)
					{
						current *= .3;
						current -= .5;
						current += x*5 + 2.5;
						current -= (height*5)/2;
						current *= -1;
					}
					if (i % 8 == 1)
					{
						current *= .3;
						current -= .5;
						current += y*5 + 2.5;
						current -= (width*5)/2;
					}
					if (i % 8 == 2)
					{
						current *= .3;
						current -= .5;
					}
					keyData[i] = current;
				}
				keyObj.data = keyData;
				keyObj.x = x, keyObj.y = y;
				objects[objectsIndex] = keyObj;
				objectsIndex++;
			}
			if (map[x][y] == 'S') //START
			{
				startX = -1*((x*5 + 2.5) - (height*5)/2.0);
				startY = (y*5 + 2.5) - (height*5)/2.0;
			}
			if (map[x][y] == 'P')
			{
				int powerLines = 0;
				ifstream powerUp;
				powerUp.open("models/sphere.txt");
				powerUp >> powerLines;
				float* powerData = new float[powerLines];
				powerObj.lines = powerLines;
				totalLines += powerLines;
				for (int i = 0; i < powerLines; i++)
				{
					float current = 0.0;
					powerUp >> current;
					if (i % 8 == 0)
					{
						current *= .3;
						current -= .5;
						current += x*5 + 2.5;
						current -= (height*5)/2;
						current *= -1;
					}
					if (i % 8 == 1)
					{
						current *= .3;
						current -= .5;
						current += y*5 + 2.5;
						current -= (width*5)/2;
					}
					if (i % 8 == 2)
					{
						current *= .3;
						current -= .5;
					}
					powerData[i] = current;
				}
				powerObj.data = powerData;
				objects[objectsIndex] = powerObj;
				objectsIndex++;
			}
			if (map[x][y] == 'A' || map[x][y] == 'B' || map[x][y] == 'C' || map[x][y] == 'D' || map[x][y] == 'E') //DOOR
			{
				int doorLines = 0;
				doorObj.type =  map[x][y];
				float* doorData = new float[240]; //temporary buffer for doors
				ifstream door;
				door.open("models/door.txt");
				door >> doorLines;
				doorObj.lines = doorLines;
				totalLines += doorLines;
				for (int i = 0; i < doorLines; i++)
				{
					float current = 0.0;
					door >> current;
					if (i % 8 == 0)
					{
						current *= 5.01;
						current += 2.5;
						current += ((-height/2.0)*5);
						current += 5*x;
						current *= -1; //
					}
					if (i % 8 == 1)
					{
						current *= 5.01;
						current += 2.5;
						current += ((-width/2.0)*5);
						current += 5*y;
					}
					if (i % 8 == 2 && current < 0)
					{
						current = -1;
					}
					
					doorData[i] = current;
				}
				doorObj.data = doorData;
				objects[objectsIndex] = doorObj;
				objectsIndex++;
			}
			
		}
	}
	float* allModels = new float[totalLines];
	
	int sum = 0;
	for (int i = 0; i < objectsIndex; i++)
		{
			if (i == 0)
			{
				copy(objects[0].data, objects[0].data+48, allModels);
				sum = 48;
			}
			else
			{
				copy(objects[i].data, objects[i].data+objects[i].lines, allModels+sum); 
				sum += objects[i].lines;
			}
		}
	


	
	numLines = totalLines;
	modelData = allModels;
	this->objects = objects;
	numObjects = objectsIndex; 
	
	for (int i = height-1; i >= 0; i--)
	{
		for (int j = 0; j < width; j++)
		{
			grid[i][j] = map[i][j];
		}
	}
}




