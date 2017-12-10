//============================== TCR PC code: ===============================

/*      ____SINGNS:_____
						0 -- dead zone
						1--obstacle
						2--free zone
						3--docking station (not in use to this moment)
						4-- horizon
*/


// 25/9/17
//__________________________libreries:__________________________

#include "stdafx.h"
#include <iostream>
#include <stdlib.h>
#include <tchar.h>
#include "SerialClass.h" // arduino comunication header
#include <string>
#include <sstream>
#include <conio.h>
#include <ctime>
#include<vector>
#include <algorithm>
#include <math.h>
//#include "..\..\RPlidar\sdk\include\rplidar.h"
#include "rplidar.h" //RPLIDAR standard sdk, all-in-one header
#include <signal.h>
//#include "..\..\SDL2\include\SDL.h"
#include <SDL.h> // graphics library
#include <fstream>

// other RPlidar settings:
#undef main

#ifndef _countof
#define _countof(_Array) (int)(sizeof(_Array) / sizeof(_Array[0]))
#endif
#ifdef _WIN32
#include <Windows.h>
#define delay(x)   ::Sleep(x)
#else
#include <unistd.h>
static inline void delay(_word_size_t ms){
	while (ms >= 1000){
		usleep(1000 * 1000);
		ms -= 1000;
	};
	if (ms != 0)
		usleep(ms * 1000);
}
#endif

using namespace rp::standalone::rplidar;
using namespace std;

//___________________________GLOBAL VARIABLES:_______________________________

//from experiment the lidar returns angel in degrees. and distance in cm
//max distance of the lidar by its specs is 300 cm, we will use 200 cm to be safe.

const int danger_radii = 60; //[cm]  //for futer use- using the lidar to see obstacles while driving.
							 // today onely the bumper sensor is implamented while driving and the lidar is used when the robot is stopped.
bool danger = 0;

const int robot_radii = 30; //[cm]  //for safty				//units in CM			//(robot daiameter is 50cm)

//const int wheels_dist = 200;
const int max_dist = 250;			//units in CM.	//max distance radius that the lidar can see. //the lidar max distance specs is 6oo cm, 200 is taken as a safety factor. 

const int microMapSize = 2 * max_dist;			//needs to be even number, units in CM.			//size of the micro map (named robomap) is microMapSize X microMapSize.

												//the robomap a map the robot craeting on each time the lidar works. this map is translated and moved to the room coordinated map named RoomMap. 

const int macroMapSize = 2000;			//needs to be even number			//units in CM (size of the macro map is macroMapSizeXmacroMapSize).
int RoboMap[microMapSize][microMapSize] = { 0 };			// this map shows what the lidar sampeld right now (micro map).
int RoomMap[macroMapSize][macroMapSize] = { 0 };			// whole appartment map (macro map).
int BuffMap[macroMapSize][macroMapSize] = { 0 };			//creat a map whith larger buffer used for navigation.THIS BUFFER IS NOT ADDED TO THE ROOM MAP!. 
															//the purpuse of this map is to avoid hitting obsticles while driving. 
int NavigationMap[macroMapSize][macroMapSize] = { 0 };		// the navigation map, contains the free zone in the RoomMap with the distances from the robot lacation.


const int docking_location[2] = { macroMapSize / 2, macroMapSize / 2 };			//set location of the docking stashion on the world map. 

const double pi = 3.14159;

int location[2] = { 0,0 };				// the current position of the robot from the center of the world map.
double heading = 0;						// the current heading of the robot

int nextpoint[2];						//the next wanted position of the robot.

vector<pair<int,int>> oldpoints ;				 //creats 2D vector for old points  on the path..
vector <double> oldheading;						//creats vector for the headings at the old points.
vector <pair<int,int>> history_looced_at;		//creats 2D vector for places the robot allready chacked.
vector<pair<int, int>> horizonlist;				 //creats 2D vector for horizon points. the robot is using this list to calculate the next point.
vector<pair<int, int>> pathpoints;				 //creats 2D vector for the basic path points.
vector < pair<int, int>> impruved_Vcut;			//creats 2D vector to optimize a section in the path points.
vector<pair<int, int>> impruved_pathpoints;		//creats 2D vector to combine the  for the impruved sictions and creat a "less number of turning optimized path".
vector<double> right_encoder_mesure;	
vector<double> left_encoder_mesure;
//vector<double> accelaration_mesure;			// today, the accelaration sensor is not in use. might be functional in the future. 
vector<clock_t> timer;							// vector for time measuring.

double stopped_dist;							// set a global variable to get the distance the robot made when it stops in the middle of its way.

//interest points are point that the patiant define as important. such as the patiant bed or kitchen. the robot can the points coordonat in an extarnal text file (named "interest_points.txt").
#define NUMBER_OF_INTEREST_POINTS 10 //max number of interest points, can be changed.
int points_of_interest_list[NUMBER_OF_INTEREST_POINTS][2];

bool ENDMAPING = 0;	// a marker to end the Maping process. (the map is finished).

#define WINDOW_WIDTH 600 // the size of the map shoing on the screen. can be changed to ajast to the screen resolution.



//_________________________________________________________________________________________________________________________________
//_________________________________________________________FUNCTIONS:______________________________________________________________

// END_MAPING: this function is finishing the Maping process when it is finish to map the room:
void END_MAPING()
{
	std:: cout << endl<<"			fin!!!!!!		"<<endl;
	ENDMAPING = 1;
	if (horizonlist.size() != 0)
	{
		for (unsigned int i = 0; i < horizonlist.size(); i++)
		{
			RoomMap[horizonlist[i].first][horizonlist[i].second] = 1;
		}
	}
}

// checkRPLIDARHealth: this function is chaking the lidar, taken from the lidar sdk.
bool checkRPLIDARHealth(RPlidarDriver * drv)
{
	u_result     op_result;
	rplidar_response_device_health_t healthinfo;


	op_result = drv->getHealth(healthinfo);
	if (IS_OK(op_result)) { // the macro IS_OK is the preperred way to judge whether the operation is succeed.
		printf("RPLidar health status : %d\n", healthinfo.status);
		if (healthinfo.status == RPLIDAR_STATUS_ERROR) {
			fprintf(stderr, "Error, rplidar internal error detected. Please reboot the device to retry.\n");
			// enable the following code if you want rplidar to be reboot by software
			// drv->reset();
			return false;
		}
		else {
			return true;
		}

	}
	else {
		fprintf(stderr, "Error, cannot retrieve the lidar health code: %x\n", op_result);
		return false;
	}
}

//--------------		polar to cartezian functions:  (theta in rad) -------------------

int D_and_Theta_2_X(double d, double theta)
{
	int x;
	double xdouble;
	xdouble = sin(theta)*d;
	x = (int)(ceil(xdouble - 0.5));
	return x;
}					
int D_and_Theta_2_Y(double d, double theta)
{
	int y;
	double ydouble;
	ydouble = cos(theta)*d;
	y = (int)(ceil(ydouble - 0.5));
	return y;
}				

//--------------		lidar oparation & built RoboMap functions:		-------------- 

//add_buffer: this function is adding mor area to the obsacles on the "robomap" map.
void add_buffer()
{
	int buffer_size = 10;
	for (int i = 0; i < microMapSize; i++)
	{
		for (int j = 0; j < microMapSize; j++)
		{
			if (RoboMap[i][j] == 1 )
			{
				int k0 = i - buffer_size;;
				int kmax = i + buffer_size;
				int z0 = j - buffer_size;
				int zmax = j + buffer_size;

				if (i <= buffer_size)
				{
					k0 = 0;
				}
				if (j <= buffer_size)
				{
					z0 = 0;
				}
				if (i >= microMapSize - buffer_size)
				{
					kmax = microMapSize;
				}
				if (j >= microMapSize - buffer_size)
				{
					zmax = microMapSize;
				}

				for (int k = k0; k < kmax; k++)
				{
					for (int z = z0; z < zmax; z++)
					{
						RoboMap[k][z] = 5;
					}
				}
			}
		}
	}
	for (int i = 0; i < microMapSize; i++)
	{
		for (int j = 0; j < microMapSize; j++)
		{
			if (RoboMap[i][j] == 5)
			{
				RoboMap[i][j] = 1;
			}
		}
	}
}

//fill_obstacles: this function is taking the obsacles distance from the robot and angle that the lidar is sending, transform tham to XY robomap coordinat, and fill the robomap with those obstacles.  
void fill_obstacles(rplidar_response_measurement_node_t * nodes, size_t count)
{
	RoboMap[microMapSize][microMapSize] = { 0 }; // initialazing map
	for (int pos = 0; pos < (int)count; ++pos)
	{

		double theta = ((nodes[pos].angle_q6_checkbit >> RPLIDAR_RESP_MEASUREMENT_ANGLE_SHIFT) / 64.0f);
		theta = ceil(theta - 0.5);
		double d = (nodes[pos].distance_q2 / 4.0f) / 10.0f;
		double theta_rad = theta*3.14159 / 180.0;
		int x = D_and_Theta_2_X(d, theta_rad);
		int y = D_and_Theta_2_Y(d, theta_rad);
				
		//printf("theta: %03.2f, d: %08.2f, x: %f, y: %f \n", theta, d, x, y);
		if (sqrt(pow((double)x, 2) + pow((double)y, 2)) > max_dist || sqrt(pow((double)x, 2) + pow((double)y, 2)) < robot_radii)
		{
			x = 0;
			y = 0;
		}
		
		if (90 < theta && theta < 270)
		{
			x = 0;
			y = 0;
		}
		//printf("int: \n x_int: %d, y_int: %d\n ", x_int, y_int);
		
		int x_map = x + max_dist;
		int y_map = y + max_dist;
		
		if (sqrt(pow((double)x, 2) + pow((double)y, 2)) > robot_radii)
		{
		// printf("map: \n x_map: %d, y_map: %d \n", x_map, y_map);
			RoboMap[x_map][y_map] = 1;		
		}

	}
}

//fill_freezone: this function is taking the Robomap that is allredy fiiled with obstacles on it, and filling the free zone on the area between the robot to the obstacles (the area that the robot can see and go to.)
void fill_freezone()
{
	for (double theta = 0; theta < 90; theta = theta + 0.01)
	{
		double theta_rad = theta*pi / 180.0;
		double d = 0;
		int stopper = 0;

		while (d < 200 && stopper == 0)
		{
			int i = D_and_Theta_2_X(d, theta_rad) + max_dist;
			int j = D_and_Theta_2_Y(d, theta_rad) + max_dist;
			for (int k = i - 4; k < i + 4; k++)
			{
				for (int z = j - 4; z < j + 4; z++)
				{
					if (RoboMap[k][z] == 1)
					{
						stopper = 1;
					}
				}
				if (stopper != 1)
				{
					RoboMap[i][j] = 2;
				}
			}
			d = d + 1;
		}
	}

	for (double theta = 270; theta < 360; theta = theta + 0.01)
	{
		double theta_rad = theta*pi / 180.0;
		double d = 0;
		int stopper = 0;

		while (d < 200 && stopper == 0)
		{
			int i = D_and_Theta_2_X(d, theta_rad) + max_dist;
			int j = D_and_Theta_2_Y(d, theta_rad) + max_dist;
			for (int k = i - 4; k < i + 4; k++)
			{
				for (int z = j - 4; z < j + 4; z++)
				{
					if (RoboMap[k][z] == 1)
					{
						stopper = 1;
					}
				}
				if (stopper != 1)
				{
					RoboMap[i][j] = 2;
				}
			}
			d = d + 1;
		}

	}
}

void fill_more_buffer()
{
	int buffer_size = 4;
	for (int i = 0; i < microMapSize; i++)
	{
		for (int j = 0; j < microMapSize; j++)
		{
			if (RoboMap[i][j] == 1)
			{
				int k0 = i - buffer_size;;
				int kmax = i + buffer_size;
				int z0 = j - buffer_size;
				int zmax = j + buffer_size;

				if (i <= buffer_size)
				{
					k0 = 0;
				}
				if (j <= buffer_size)
				{
					z0 = 0;
				}
				if (i >= microMapSize - buffer_size)
				{
					kmax = microMapSize;
				}
				if (j >= microMapSize - buffer_size)
				{
					zmax = microMapSize;
				}

				for (int k = k0; k < kmax; k++)
				{
					for (int z = z0; z < zmax; z++)
					{
						RoboMap[k][z] = 5;
					}
				}
			}
			if (RoboMap[i][j] == 2)
			{
				int k0 = i - buffer_size;;
				int kmax = i + buffer_size;
				int z0 = j - buffer_size;
				int zmax = j + buffer_size;

				if (i <= buffer_size)
				{
					k0 = 0;
				}
				if (j <= buffer_size)
				{
					z0 = 0;
				}
				if (i >= microMapSize - buffer_size)
				{
					kmax = microMapSize;
				}
				if (j >= microMapSize - buffer_size)
				{
					zmax = microMapSize;
				}
			
				for (int k = k0; k < kmax; k++)
				{
					for (int z = z0; z < zmax; z++)
					{
						RoboMap[k][z] = 6;
					}
				}

			}
		}
	}
	for (int i = 0; i < microMapSize; i++)
	{
		for (int j = 0; j < microMapSize; j++)
		{
			if (RoboMap[i][j] == 5)
			{
				RoboMap[i][j] = 1;
			}
			if (RoboMap[i][j] == 6)
			{
				RoboMap[i][j] = 2;
			}
		}
	}
}

// PrintRobotMap: this function is creating an image of the robomap on a new window. to finish this funcion you must exit the window. //not in use today.
int PrintRobotMap(void) {
	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_Window* window = SDL_CreateWindow("SDL App", 5, 40, WINDOW_WIDTH, WINDOW_WIDTH, SDL_WINDOW_RESIZABLE);

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

	SDL_RenderSetLogicalSize(renderer, microMapSize, microMapSize);

	SDL_Event event;
	
	SDL_SetRenderDrawColor(renderer, 0, 225, 0, 225);
	SDL_RenderClear(renderer);
	for (int i = 0; i < microMapSize; i++)
	{
		for (int j = 0; j < microMapSize; j++)
		{
			if (RoboMap[i][j] == 0 ) // print dead zone
			{
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
				SDL_RenderDrawPoint(renderer, microMapSize - i, microMapSize - j);
			}

			if (RoboMap[i][j] == 1) //print obstacles
			{
				SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
				SDL_RenderDrawPoint(renderer, microMapSize - i, microMapSize - j);
			}

			if (RoboMap[i][j] == 2) //print free zone
			{
				SDL_SetRenderDrawColor(renderer, 225, 225, 225, 255);
				SDL_RenderDrawPoint(renderer, microMapSize - i, microMapSize - j);
			}

			if (RoboMap[i][j] == 3) // print docking
			{
				SDL_SetRenderDrawColor(renderer, 0, 225, 0, 255);
				SDL_RenderDrawPoint(renderer, microMapSize - i, microMapSize - j);
			}


		}
	}

	for (int i = -2; i <= 2; i++)
	{
		for (int j = -2; j <= 2; j++)
		{
			SDL_SetRenderDrawColor(renderer, 0, 0, 225, 255);
			SDL_RenderDrawPoint(renderer, i+max_dist, j+max_dist);
		}
	}
	
	SDL_RenderPresent(renderer);

	while (1) {
		if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
			break;
	}
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return EXIT_SUCCESS;
}

//RoboMap_Built: Combines "fill_obstacles","add_buffer","fill_freezone","fill_more_buffer" to one function.
void RoboMap_Built(rplidar_response_measurement_node_t * nodes, size_t count)

{
	for (int i = 0; i < microMapSize; i++)
	{
		for (int j = 0; j < microMapSize; j++)
		{
			RoboMap[i][j] = 0;					//initialize Robomap
		}
	}
	fill_obstacles(nodes, count);
	add_buffer();
	fill_freezone();
	fill_more_buffer();
}

//look: this function checks the area close to the robot to find ani obsacles. can be neccacery for using the lidar ehile driving.   
void look(rplidar_response_measurement_node_t * nodes, size_t count)    // looks for dangerus obstacles while driving.
{
	
	for (int pos = 0; pos < (int)count; ++pos)
	{

		double theta = ((nodes[pos].angle_q6_checkbit >> RPLIDAR_RESP_MEASUREMENT_ANGLE_SHIFT) / 64.0f);
		double d = (nodes[pos].distance_q2 / 4.0f) / 10.0f;
		if ((theta < 30 || theta>350) && d < danger_radii && d>robot_radii)
		{
			danger = 1;
		}
	}
}

//capture_and_displa (taken from the lidar sdk with some changes): 1. taking the informaition from the lidar. 2. built the robomap or look for close obstacle.
u_result capture_and_display(RPlidarDriver * drv, int mode)
{
	// mode 1: built robomap		mode 2: look for close obstacle while driving
	u_result ans;

	rplidar_response_measurement_node_t nodes[360 * 2];
	size_t   count = _countof(nodes);

	printf("waiting for data...\n");

	// fetech extactly one 0-360 degrees' scan
	ans = drv->grabScanData(nodes, count);
	if (IS_OK(ans) || ans == RESULT_OPERATION_TIMEOUT)
	{
		drv->ascendScanData(nodes, count);
		if (mode==1) RoboMap_Built(nodes, count);
		if (mode == 2) look(nodes, count);
	}
	else {
		printf("error code: %x\n", ans);
	}

	return ans;
}

//start_lidar_and_gett_RoboMap  (taken from the lidar sdk with some changes): 1. initialize the lidar. 2. start capture_and_display in the desired mode.
void start_lidar_and_gett_RoboMap(RPlidarDriver * drv,int mode)
{// mode 1: built robomap		mode 2: look for close obstacle while driving
	drv->startMotor();

	do {
		drv->startMotor();

		// take only one 360 deg scan and display the result as a map
		////////////////////////////////////////////////////////////////////////////////
		if (IS_FAIL(drv->startScan( /* true */))) // you can force rplidar to perform scan operation regardless whether the motor is rotating
		{
			fprintf(stderr, "Error, cannot start the scan operation.\n");
			break;
		}

		if (IS_FAIL(capture_and_display(drv,mode))) 
		{
			fprintf(stderr, "Error, cannot grab scan data.\n");
			break;

		}

	} while (0);

}



//--------------		built RoomMap functions		-----------------

//after the robot create the robomap, we want to add this map to the roommap. to do so, wwe need to rotate and translate the robomap to the roomap coordinates.
//after the roommap is updated the robomap is initialized for another use in the next point.

void RotateRobotMap(double theta)
{
	int temp_RoboMap[microMapSize][microMapSize];
	for (int i = 0; i < microMapSize; i++)
	{
		for (int j = 0; j < microMapSize; j++)
		{
			temp_RoboMap[i][j] = RoboMap[i][j];
			RoboMap[i][j] = 0;
		}
	}
	for (int i = 0; i < microMapSize; i++)
	{
		for (int j = 0; j < microMapSize; j++)
		{
			double flo_irotat = cos(theta)*(i - max_dist) - sin(theta)*(j - max_dist);
			double flo_jrotat = sin(theta)*(i - max_dist) + cos(theta)*(j - max_dist);
			int irotat = (int)(ceil(flo_irotat - 0.5))+max_dist;
			int jrotat = (int)(ceil(flo_jrotat - 0.5))+max_dist;
			if (irotat >= 0 && irotat < microMapSize && jrotat >= 0 && jrotat < microMapSize)
			{
				RoboMap[irotat][jrotat] = temp_RoboMap[i][j];
			}
		}
	}
}

void add_result_to_whole_map(int x_rob, int y_rob, double dirction_rob)  //dirction_rob are in radians
{

	RotateRobotMap(dirction_rob);
	for (int i = 0; i < microMapSize; i++)
	{
		for (int j = 0; j < microMapSize; j++)
		{
			if (RoboMap[i][j] == 1 || RoboMap[i][j] == 2)
			{
				if (RoomMap[i + macroMapSize / 2 + x_rob - microMapSize / 2][j + macroMapSize / 2 + y_rob - microMapSize / 2] != 1)
				{
					RoomMap[i + macroMapSize / 2 + x_rob - microMapSize / 2][j + macroMapSize / 2 + y_rob - microMapSize / 2] = RoboMap[i][j];
				}
			}
		}
	}
}

void fill_Room_buffer()
{
	int buffer_size = 2;
	for (int i = 0; i < macroMapSize; i++)
	{
		for (int j = 0; j < macroMapSize; j++)
		{
			if (RoomMap[i][j] == 1)
			{
				int k0 = i - buffer_size;;
				int kmax = i + buffer_size;
				int z0 = j - buffer_size;
				int zmax = j + buffer_size;

				if (i <= buffer_size)
				{
					k0 = 0;
				}
				if (j <= buffer_size)
				{
					z0 = 0;
				}
				if (i >= macroMapSize - buffer_size)
				{
					kmax = macroMapSize;
				}
				if (j >= macroMapSize - buffer_size)
				{
					zmax = macroMapSize;
				}

				for (int k = k0; k < kmax; k++)
				{
					for (int z = z0; z < zmax; z++)
					{
						RoomMap[k][z] = 5;
					}
				}
			}
			if (RoomMap[i][j] == 2)
			{
				int k0 = i - buffer_size;;
				int kmax = i + buffer_size;
				int z0 = j - buffer_size;
				int zmax = j + buffer_size;

				if (i <= buffer_size)
				{
					k0 = 0;
				}
				if (j <= buffer_size)
				{
					z0 = 0;
				}
				if (i >= macroMapSize - buffer_size)
				{
					kmax = macroMapSize;
				}
				if (j >= macroMapSize - buffer_size)
				{
					zmax = macroMapSize;
				}

				for (int k = k0; k < kmax; k++)
				{
					for (int z = z0; z < zmax; z++)
					{
						RoomMap[k][z] = 6;
					}
				}

			}
		}
	}
	for (int i = 0; i < macroMapSize; i++)
	{
		for (int j = 0; j < macroMapSize; j++)
		{
			if (RoomMap[i][j] == 5)
			{
				RoomMap[i][j] = 1;
			}
			if (RoomMap[i][j] == 6)
			{
				RoomMap[i][j] = 2;
			}
		}
	}
}

//the horizon: the points on the map witch borders free zone on one side, and unnone zone on the other side.  
void fill_horizon()
{
	for (int i = 0; i < macroMapSize; i++)
	{
		for (int j = 0; j < macroMapSize; j++)
		{
			if (RoomMap[i][j] == 2 && (RoomMap[i + 1][j] == 0 || RoomMap[i + 1][j + 1] == 0 || RoomMap[i][j + 1] == 0 || RoomMap[i - 1][j + 1] == 0 || RoomMap[i - 1][j] == 0 || RoomMap[i - 1][j - 1] == 0 || RoomMap[i][j - 1] == 0 || RoomMap[i + 1][j - 1] == 0))
			{
				RoomMap[i][j] = 4;
			}

		}
	}
}

int PrintRoomMap(void) 
{
	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_Window* window = SDL_CreateWindow("SDL App", 5, 40, WINDOW_WIDTH, WINDOW_WIDTH, SDL_WINDOW_RESIZABLE);

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

	SDL_RenderSetLogicalSize(renderer, macroMapSize, macroMapSize);

	SDL_Event event;

	SDL_SetRenderDrawColor(renderer, 0, 225, 0, 225);
	SDL_RenderClear(renderer);

	for (int i = 0; i < macroMapSize; i++)
	{
		for (int j = 0; j < macroMapSize; j++)
		{
			if (RoomMap[i][j] == 0) // print dead zone
			{
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
				SDL_RenderDrawPoint(renderer,  i, macroMapSize - j);
			}

			if (RoomMap[i][j] == 1) //print obstacles
			{
				SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
				SDL_RenderDrawPoint(renderer,  i, macroMapSize - j);
			}

			if (RoomMap[i][j] == 2) //print free zone
			{
				SDL_SetRenderDrawColor(renderer, 140, 120, 255, 255);
				SDL_RenderDrawPoint(renderer,  i, macroMapSize - j);
			}
		
			if (RoomMap[i][j] == 3) // print docking
			{
				SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
				SDL_RenderDrawPoint(renderer,  i, macroMapSize - j);
			}
			if (RoomMap[i][j] == 4) // print docking
			{
				SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
				SDL_RenderDrawPoint(renderer,  i, macroMapSize - j);
			}

		}

	}

	for (int i = -2; i <= 2; i++)   // print robot position in silver
	{
		for (int j = -2; j <= 2; j++)
		{
			SDL_SetRenderDrawColor(renderer, 169, 169, 169, 255);
			SDL_RenderDrawPoint(renderer, (i + location[0]+macroMapSize/2), macroMapSize - (j + location[1] + macroMapSize/2));
		}
	}
	
	for (int i = -2; i <= 2; i++)
	{
		for (int j = -2; j <= 2; j++)
		{
			SDL_SetRenderDrawColor(renderer, 0, 0, 225, 255);
			SDL_RenderDrawPoint(renderer,  (i + nextpoint[0]), macroMapSize - (j + nextpoint[1]));
		}
	}


	if (pathpoints.size() != 0)   //print the basic path
	{
		for (unsigned int i = 0; i < pathpoints.size(); i++)
		{
			SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);
			SDL_RenderDrawPoint(renderer, pathpoints[i].first, macroMapSize - (pathpoints[i].second));
			SDL_RenderDrawPoint(renderer, pathpoints[i].first+1, macroMapSize - (pathpoints[i].second));
			SDL_RenderDrawPoint(renderer, pathpoints[i].first-1, macroMapSize - (pathpoints[i].second));
			SDL_RenderDrawPoint(renderer, pathpoints[i].first, macroMapSize - (pathpoints[i].second+1));
			SDL_RenderDrawPoint(renderer, pathpoints[i].first+1, macroMapSize - (pathpoints[i].second + 1));
			SDL_RenderDrawPoint(renderer, pathpoints[i].first-1, macroMapSize - (pathpoints[i].second + 1));
			SDL_RenderDrawPoint(renderer, pathpoints[i].first, macroMapSize - (pathpoints[i].second - 1));
			SDL_RenderDrawPoint(renderer, pathpoints[i].first+1, macroMapSize - (pathpoints[i].second - 1));
			SDL_RenderDrawPoint(renderer, pathpoints[i].first-1, macroMapSize - (pathpoints[i].second - 1));
		}
	}

	if (impruved_pathpoints.size() != 0)   //print the impruved path
	{
		for (unsigned int i = 0; i < impruved_pathpoints.size(); i++)
		{
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			SDL_RenderDrawPoint(renderer, impruved_pathpoints[i].first, macroMapSize - (impruved_pathpoints[i].second));
			SDL_RenderDrawPoint(renderer, impruved_pathpoints[i].first+1, macroMapSize - (impruved_pathpoints[i].second));
			SDL_RenderDrawPoint(renderer, impruved_pathpoints[i].first-1, macroMapSize - (impruved_pathpoints[i].second));
			SDL_RenderDrawPoint(renderer, impruved_pathpoints[i].first, macroMapSize - (impruved_pathpoints[i].second+1));
			SDL_RenderDrawPoint(renderer, impruved_pathpoints[i].first+1, macroMapSize - (impruved_pathpoints[i].second + 1));
			SDL_RenderDrawPoint(renderer, impruved_pathpoints[i].first-1, macroMapSize - (impruved_pathpoints[i].second + 1));
			SDL_RenderDrawPoint(renderer, impruved_pathpoints[i].first, macroMapSize - (impruved_pathpoints[i].second - 1));
			SDL_RenderDrawPoint(renderer, impruved_pathpoints[i].first+1, macroMapSize - (impruved_pathpoints[i].second - 1));
			SDL_RenderDrawPoint(renderer, impruved_pathpoints[i].first-1, macroMapSize - (impruved_pathpoints[i].second - 1));
		}
	}

	SDL_RenderPresent(renderer);

	while (1) {
		if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
			break;
	}
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return EXIT_SUCCESS;
}




//--------------		find next point functions:		-------------


//getthorizonlist: this function fills the global horizon list and return its length.
int getthorizonlist()  
{
	int index = 0;
	for (int i = 0; i < macroMapSize; i++)
	{
		for (int j = 0; j < macroMapSize; j++)
		{
			if (RoomMap[i][j] == 4)
			{
				index++;
			}

		}
	}
	horizonlist.resize(index);
	index = 0;
	for (int i = 0; i < macroMapSize; i++)
	{
		for (int j = 0; j < macroMapSize; j++)
		{
			if (RoomMap[i][j] == 4)
			{
				horizonlist[index].first = i;
				horizonlist[index].second = j;
				index++;
			}
		}
	}
	return index;
}

// distancefromrobot and order are functions that are used to help ordering the horizon list from the farthest point to the nearest.
double distancefromrobot(pair<int, int> A)
{
	double d;
	double xdouble = (double)(location[0] - A.first);
	double ydouble = (double)(location[1] - A.second);
	d = sqrt(pow(xdouble, 2) + pow(ydouble, 2));
	return d;
}								

bool order(pair<int, int> A, pair<int, int> B)
{
	double dA = distancefromrobot(A);
	double dB = distancefromrobot(B);
	return dA < dB;
}

//ok_to_go: takes point coordinats and robot radi, and checks if the robot can path thru that point.
bool ok_to_go(int x, int y, int robot_rad)
{	
	bool OK = 1;
	for (int i = x-robot_rad; i <x+ robot_rad; i++)
	{
		for (int j = y-robot_rad; j <y+ robot_rad; j++)
		{
			if (i > macroMapSize || j > macroMapSize)      // its not OK to go where the robot gets out of the map capacity.
			{
				OK = 0;
			}
			else
			{
				if (RoomMap[i][j] != 2)						// its not OK to go where the robot gets in to an obstacle.
				{
					OK = 0;
				}
			}

		}
	}
	return OK;
}

//robot_can_go: takes point coordinats, checks if the robot can path thru that point (using oktogo), if it can the function is fixing the next point.
bool robot_can_go(pair <int, int> point)
{
	bool success = 0;
	int xpoint = point.first;
	int ypoint = point.second;
	int serch_aera=40;   //40 cm
	int robot_rad = 30; // R=20[cm] + 10[cm]tollerance
	
	for (int i = xpoint - serch_aera; i < xpoint + serch_aera; i++)
	{
		for (int j = ypoint - serch_aera; j < ypoint + serch_aera; j++)
		{
			if (ok_to_go( i,  j, robot_rad))
			{
				pair<int, int> oldpoint;
				oldpoint.first = nextpoint[0];
				oldpoint.second = nextpoint[1];
				oldpoints.push_back(oldpoint);
				nextpoint[0] =  i;
				nextpoint[1] = j;
				success = 1;
				break;
			}
		}
	}
	return success;
}

//findnextpoint: 1. sort the horizonlist from the farthest point to the nearest. 2. go on the sorted list, and chack if thare is any point that the robot can go to. 
//3. fix the first point on the list that the robot can go to as the next point. 
bool findnextpoint()
{
	bool found = 0;
	int i = 0;
	int n = getthorizonlist();
	sort(horizonlist.begin(), horizonlist.end(), order);

	//for (int j = 0; j < n;j++) cout << horizonlist[j].first << "  "<< horizonlist[j].second << endl;	
	
	while (i<n)
	{
		if (robot_can_go(horizonlist[i]) == 1)			// the next point is declird inside robot_can_go function.
		{
			found = 1;
			cout << "the robot next point is the " << i << " point in the horizon distans list" << endl;	
			break;
		}
		i++;
	}
	if (found==0) END_MAPING();
	return found;
}



//--------------		path funcions:		---------------------


// Navdistans: find the distans of all the points on the map from the robot. the algorithem for this function is slow, might be a better one. 
bool Navdistans()     
{
	bool done = 0;
	time_t time0 = clock()/CLOCKS_PER_SEC;
	while (done != 1)
	{
		done = 1;
		time_t time1 = clock() / CLOCKS_PER_SEC;
		for (int i = 0; i < macroMapSize; i++)
		{
			for (int j = 0; j < macroMapSize; j++)
				if (NavigationMap[i][j] == 1)
				{
					done = 0;
					int index = 0;
					int value = 1;
					int a = NavigationMap[i + 1][j];
					int b = NavigationMap[i - 1][j];
					int c = NavigationMap[i][j + 1];
					int d = NavigationMap[i][j - 1];

					if (a != 0 && a != 1)      // a is not an obtacle or a dead zone.
					{
						if (index == 0)
						{
							NavigationMap[i][j] = a + 1;
						}
						if (index != 0)
						{
							NavigationMap[i][j] = min(NavigationMap[i][j], a + 1);
						}
						index++;
					}
					if (b != 0 && b != 1)	// b is not an obtacle or a dead zone.
					{

						if (index == 0)
						{
							NavigationMap[i][j] = b + 1;
						}
						if (index != 0)
						{
							NavigationMap[i][j] = min(NavigationMap[i][j], b + 1);
						}
						index++;
					}
					if (c != 0 && c != 1)	// c is not an obtacle or a dead zone.
					{

						if (index == 0)
						{
							NavigationMap[i][j] = c + 1;
						}
						if (index != 0)
						{
							NavigationMap[i][j] = min(NavigationMap[i][j], c + 1);
						}
						index++;
					}
					if (d != 0 && d != 1)	// d is not an obtacle or a dead zone.
					{

						if (index == 0)
						{
							NavigationMap[i][j] = d + 1;
						}
						if (index != 0)
						{
							NavigationMap[i][j] = min(NavigationMap[i][j], d + 1);
						}
						index++;

					}
				}
		}
		if (time1 - time0 > 5)
			break;		
	}

		return done;
}

//	 max_map():  find the largest point in navigationmap .
int max_map() 
{
	
	vector<int> temp_vec;
	vector<int> maxinaraw;

	for (int i = 0; i < macroMapSize; i++)
	{
		for (int j = 0; j < macroMapSize; j++)
		{
			temp_vec.push_back(NavigationMap[i][j]);
		}
		maxinaraw.push_back(*max_element(temp_vec.begin(), temp_vec.end()));
		temp_vec.clear();
	}
	int ans = *max_element(maxinaraw.begin(), maxinaraw.end());
	return ans;


}

int PrintNavMap(void)
{

	SDL_Window* window = SDL_CreateWindow("SDL App", 5, 40, WINDOW_WIDTH, WINDOW_WIDTH, SDL_WINDOW_RESIZABLE);

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

	SDL_RenderSetLogicalSize(renderer, macroMapSize, macroMapSize);

	SDL_Event event;

	SDL_SetRenderDrawColor(renderer, 0, 225, 0, 225);
	SDL_RenderClear(renderer);

	int maxDataValue = max_map();
	printf("\n maxDataValue %d\n", maxDataValue);
	for (int i = 0; i < macroMapSize; i++)
	{
		for (int j = 0; j < macroMapSize; j++)
		{
			if (NavigationMap[i][j] != 0)
			{
				double value = ((double)NavigationMap[i][j] / (maxDataValue));   // normalize to a number between 0 and 1.
				int R, G, B;
				B = (int)(max(0, 255 *( 1 - value)));
				G = (int)(max(0, 255 *( -1 + value)));
				R = 255 - B - G;
				SDL_SetRenderDrawColor(renderer, R, G, B, 255);
				SDL_RenderDrawPoint(renderer, i, macroMapSize - j);
			}
		}


	}

	for (int i = -2; i <= 2; i++)   // print robot position in silver
	{
		for (int j = -2; j <= 2; j++)
		{
			SDL_SetRenderDrawColor(renderer, 169, 169, 169, 255);
			SDL_RenderDrawPoint(renderer, (i + location[0] + macroMapSize / 2), macroMapSize - (j + location[1] + macroMapSize / 2));
		}
	}

	/*for (int i = -2; i <= 2; i++)
	{
		for (int j = -2; j <= 2; j++)
		{
			SDL_SetRenderDrawColor(renderer, 0, 0, 225, 255);
			SDL_RenderDrawPoint(renderer, macroMapSize - (i + nextpoint[0]), macroMapSize - (j + nextpoint[1]));
		}
	}*/

	SDL_RenderPresent(renderer);

	while (1) {
		if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
			break;
	}
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return EXIT_SUCCESS;

}

//fill_BuffMap: add a temporary map that is the roomMap with larger buffer.  
void fill_BuffMap() //creat a list of points ajaisent to the obstacles that the robot can go thru tham. THIS BUFFER IS NOT ADDED TO THE ROOM MAP!.
{
	int buffer_size = 15;
	for (int i = 0; i < macroMapSize; i++)
	{
		for (int j = 0; j < macroMapSize; j++)
		{
			if (RoomMap[i][j] == 1)
			{
				int k0 = i - buffer_size;;
				int kmax = i + buffer_size;
				int z0 = j - buffer_size;
				int zmax = j + buffer_size;

				if (i <= buffer_size)
				{
					k0 = 0;
				}
				if (j <= buffer_size)
				{
					z0 = 0;
				}
				if (i >= macroMapSize - buffer_size)
				{
					kmax = macroMapSize;
				}
				if (j >= macroMapSize - buffer_size)
				{
					zmax = macroMapSize;
				}

				for (int k = k0; k < kmax; k++)
				{
					for (int z = z0; z < zmax; z++)
					{
						BuffMap[k][z] = 1;
					}
				}
			}
		}
	}
}
void clear_BuffMap()
{
	for (int i=0;i<macroMapSize;i++)
		for(int j=0;j<macroMapSize;j++)
		{
			BuffMap[i][j] = 0;
		}
}

//  fill_navigationMap: combining "fill_BuffMap","max_map","Navdistans" and "clear_BuffMap" to built a base for heatmap("NavMap").
bool fill_navigationMap()
{
	fill_BuffMap();
// 0. initialize Navigation map
	for (int i = 0; i < macroMapSize; i++){
		for (int j = 0; j < macroMapSize; j++){
			NavigationMap[i][j] = 0;
		}
	}
	
//  1. fill freezone with 1 
	for (int i = 0; i < macroMapSize; i++){
		for (int j = 0; j < macroMapSize; j++){
			if (RoomMap[i][j] == 2 && BuffMap[i][j]!=1)
			{
				NavigationMap[i][j] = 1;
			}
		}
	}
//  2. fill robot position with 2
	NavigationMap[location[0] + macroMapSize / 2][location[1] + macroMapSize / 2] = 2;

// 3. fill the distanse from the robot location

	if (Navdistans())
	{
		for (int i = 0; i < macroMapSize; i++)
		{
			for (int j = 0; j < macroMapSize; j++)
			{
				if (NavigationMap[i][j] == 1)
					printf("error  ");
			}

		}
		clear_BuffMap();
	}
	else { clear_BuffMap(); return 0; }
	return 1;
}

// path: building the path to the next point: 1.gett next point coordinats. 2. go to the smallest point on "NavMap" bordering the next point. 3. add the point to the path vector. 4. return untill you are in the robot location.
void path(int xtarg, int ytarg)
{
	pair<int, int> nextpointonpath = { xtarg,ytarg };
	pathpoints.push_back(nextpointonpath);
	int step = 0;

	do
	{
		pair<int, int> current_point = nextpointonpath;
		step = NavigationMap[nextpointonpath.first][nextpointonpath.second];
		int a = NavigationMap[nextpointonpath.first - 1][nextpointonpath.second - 1];
		int b = NavigationMap[nextpointonpath.first - 1][nextpointonpath.second];
		int c = NavigationMap[nextpointonpath.first - 1][nextpointonpath.second + 1];
		int d = NavigationMap[nextpointonpath.first][nextpointonpath.second - 1];
		int e = NavigationMap[nextpointonpath.first][nextpointonpath.second + 1];
		int f = NavigationMap[nextpointonpath.first + 1][nextpointonpath.second - 1];
		int g = NavigationMap[nextpointonpath.first + 1][nextpointonpath.second];
		int h = NavigationMap[nextpointonpath.first + 1][nextpointonpath.second + 1];

		int index = 0;
		if (a < step && a != 0)
		{
			if (index == 0)
			{
				nextpointonpath.first = current_point.first - 1;
				nextpointonpath.second = current_point.second - 1;
			}
			if (index != 0 && a < NavigationMap[nextpointonpath.first][nextpointonpath.second])
			{
				nextpointonpath.first = current_point.first - 1;
				nextpointonpath.second = current_point.second - 1;
			}
		}
		if (b < step && b != 0)
		{
			if (index == 0)
			{
				nextpointonpath.first = current_point.first - 1;
				nextpointonpath.second = current_point.second;
			}
			if (index != 0 && b < NavigationMap[nextpointonpath.first][nextpointonpath.second])
			{
				nextpointonpath.first = current_point.first - 1;
				nextpointonpath.second = current_point.second;
			}
		}
		if (c < step && c != 0)
		{
			if (index == 0)
			{
				nextpointonpath.first = current_point.first - 1;
				nextpointonpath.second = current_point.second + 1;
			}
			if (index != 0 && c < NavigationMap[nextpointonpath.first][nextpointonpath.second])
			{
				nextpointonpath.first = current_point.first - 1;
				nextpointonpath.second = current_point.second + 1;
			}
		}
		if (d < step && d != 0)
		{
			if (index == 0)
			{
				nextpointonpath.first = current_point.first;
				nextpointonpath.second = current_point.second - 1;
			}
			if (index != 0 && d < NavigationMap[nextpointonpath.first][nextpointonpath.second])
			{
				nextpointonpath.first = current_point.first;
				nextpointonpath.second = current_point.second - 1;
			}
		}
		if (e < step && e != 0)
		{
			if (index == 0)
			{
				nextpointonpath.first = current_point.first;
				nextpointonpath.second = current_point.second + 1;
			}
			if (index != 0 && e < NavigationMap[nextpointonpath.first][nextpointonpath.second])
			{
				nextpointonpath.first = current_point.first;
				nextpointonpath.second = current_point.second + 1;
			}
		}
		if (f < step && f != 0)
		{
			if (index == 0)
			{
				nextpointonpath.first = current_point.first + 1;
				nextpointonpath.second = current_point.second - 1;
			}
			if (index != 0 && f < NavigationMap[nextpointonpath.first][nextpointonpath.second])
			{
				nextpointonpath.first = current_point.first + 1;
				nextpointonpath.second = current_point.second - 1;
			}
		}
		if (g < step && g != 0)
		{
			if (index == 0)
			{
				nextpointonpath.first = current_point.first + 1;
				nextpointonpath.second = current_point.second;
			}
			if (index != 0 && g < NavigationMap[nextpointonpath.first][nextpointonpath.second])
			{
				nextpointonpath.first = current_point.first + 1;
				nextpointonpath.second = current_point.second;
			}
		}
		if (h < step && h != 0)
		{
			if (index == 0)
			{
				nextpointonpath.first = current_point.first + 1;
				nextpointonpath.second = current_point.second + 1;
			}
			if (index != 0 && h < NavigationMap[nextpointonpath.first][nextpointonpath.second])
			{
				nextpointonpath.first = current_point.first + 1;
				nextpointonpath.second = current_point.second + 1;
			}
		}
		pathpoints.push_back(nextpointonpath);

	} while (step > 2);
	std::reverse(pathpoints.begin(), pathpoints.end());
}
		
	
	/*
	bool path(int xtarg, int ytarg, bool recursiondirection)
	int step = NavigationMap[xtarg][ytarg];
	if (step == 2)    // end of recorsion
	{
		recursiondirection = 1;
		return recursiondirection;
	}
	int a = NavigationMap[xtarg - 1][ytarg - 1];
	int b = NavigationMap[xtarg - 1][ytarg];
	int c = NavigationMap[xtarg - 1][ytarg + 1];
	int d = NavigationMap[xtarg][ytarg - 1];
	int e = NavigationMap[xtarg][ytarg + 1];
	int f = NavigationMap[xtarg + 1][ytarg - 1];
	int g = NavigationMap[xtarg + 1][ytarg];
	int h = NavigationMap[xtarg + 1][ytarg + 1];

	pair<int, int> nextpointonpath;

	int index = 0;
	if (a < step && a != 0)
	{
		if (index == 0)
		{
			nextpointonpath.first = xtarg - 1;
			nextpointonpath.second = ytarg - 1;
		}
		if (index != 0 && a < NavigationMap[nextpointonpath.first][nextpointonpath.second])
		{
			nextpointonpath.first = xtarg - 1;
			nextpointonpath.second = ytarg - 1;
		}
	}
	if (b < step && b != 0)
	{
		if (index == 0)
		{
			nextpointonpath.first = xtarg - 1;
			nextpointonpath.second = ytarg;
		}
		if (index != 0 && b < NavigationMap[nextpointonpath.first][nextpointonpath.second])
		{
			nextpointonpath.first = xtarg - 1;
			nextpointonpath.second = ytarg;
		}
	}
	if (c < step && c != 0)
	{
		if (index == 0)
		{
			nextpointonpath.first = xtarg - 1;
			nextpointonpath.second = ytarg + 1;
		}
		if (index != 0 && c < NavigationMap[nextpointonpath.first][nextpointonpath.second])
		{
			nextpointonpath.first = xtarg - 1;
			nextpointonpath.second = ytarg + 1;
		}
	}
	if (d < step && d != 0)
	{
		if (index == 0)
		{
			nextpointonpath.first = xtarg;
			nextpointonpath.second = ytarg - 1;
		}
		if (index != 0 && d < NavigationMap[nextpointonpath.first][nextpointonpath.second])
		{
			nextpointonpath.first = xtarg;
			nextpointonpath.second = ytarg - 1;
		}
	}
	if (e < step && e != 0)
	{
		if (index == 0)
		{
			nextpointonpath.first = xtarg;
			nextpointonpath.second = ytarg + 1;
		}
		if (index != 0 && e < NavigationMap[nextpointonpath.first][nextpointonpath.second])
		{
			nextpointonpath.first = xtarg;
			nextpointonpath.second = ytarg + 1;
		}
	}
	if (f < step && f != 0)
	{
		if (index == 0)
		{
			nextpointonpath.first = xtarg + 1;
			nextpointonpath.second = ytarg - 1;
		}
		if (index != 0 && f < NavigationMap[nextpointonpath.first][nextpointonpath.second])
		{
			nextpointonpath.first = xtarg + 1;
			nextpointonpath.second = ytarg - 1;
		}
	}
	if (g < step && g != 0)
	{
		if (index == 0)
		{
			nextpointonpath.first = xtarg + 1;
			nextpointonpath.second = ytarg;
		}
		if (index != 0 && g < NavigationMap[nextpointonpath.first][nextpointonpath.second])
		{
			nextpointonpath.first = xtarg + 1;
			nextpointonpath.second = ytarg;
		}
	}
	if (h < step && h != 0)
	{
		if (index == 0)
		{
			nextpointonpath.first = xtarg + 1;
			nextpointonpath.second = ytarg + 1;
		}
		if (index != 0 && h < NavigationMap[nextpointonpath.first][nextpointonpath.second])
		{
			nextpointonpath.first = xtarg + 1;
			nextpointonpath.second = ytarg + 1;
		}
	}
	recursiondirection = path(nextpointonpath.first, nextpointonpath.second, recursiondirection);
	pathpoints.push_back(nextpointonpath);
	return recursiondirection;
}*/

// ------ impruve path functions: -------


pair <double, double> XY_2_dist_theta(pair<int, int > XY)
{
	pair <double, double> dist_theta;
	double xdouble = (float)(XY.first);
	double ydouble = (float)(XY.second);

	dist_theta.second = atan2(xdouble, ydouble) * 180 / pi; // angle in degrees

	dist_theta.first = sqrt(pow(xdouble, 2) + pow(ydouble, 2));

	return dist_theta;
}

/*pair<int, int> find_the_closest_point_on_basic_path(pair<int, int> i_j)
{
	pair<int, int> closestpoint = pathpoints.front();
	int dist = (int) (pow( (double)(pathpoints.front().first - i_j.first), 2) + pow( (double) (pathpoints.front().second - i_j.second), 2));
	for (unsigned int ind = 1; ind < pathpoints.size(); ind++)
	{
		int d= (int) pow( (double) (pathpoints[ind].first - i_j.first), 2) + pow( (double) (pathpoints[ind].second - i_j.second), 2);
		if (d < dist)
		{
			closestpoint = pathpoints[ind];
			dist = d;
		}
	}
	return closestpoint;
}
*/
bool chacke_2_points_path (pair <int,int> pointA, pair <int, int> pointB)
{
	//check for obsticals between the points.
	//in the case when an obsticle is found- returne 0. else - return 1.
	pair <int, int> dxdy = { pointB.first - pointA.first,pointB.second - pointA.second };
	pair<double, double> dist_theta = XY_2_dist_theta(dxdy);
	
	double theta_rad = dist_theta.second*pi / 180.0;
	double d = 0;
	bool stopper = 0;
	int i = pointA.first;
	int j = pointA.second;
	while (d < dist_theta.first && stopper == 0)
	{
		i = D_and_Theta_2_X(d, theta_rad)+pointA.first;
		j = D_and_Theta_2_Y(d, theta_rad)+pointA.second;
			//BuffMap
		if (NavigationMap[i][j] == 1)
		{
			stopper = 1;
		}

		d = d + 1;
	}
 	if (stopper==1)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}
vector<pair <int,int>> cut_vector (vector<pair<int,int>> in_vec,int steps,int chosen_partition)
{
	vector<pair <int, int>> out_vec;
	int S = in_vec.size();
	int cuts = S / steps;
	for (int i = 0; i < cuts; i++)
	{
		if (i == chosen_partition)
		{
			for (int k = 0; k < 100; k++)
			{		out_vec.push_back(in_vec[100 * i + k]);
			}
		}
	}
	return (out_vec);
}
vector < pair<int, int>> optimize_cut(vector < pair<int, int>> vec_cut)
{
	vector <int> Vec_index_on_basic_path;
	std::vector<pair<int, int>>::iterator it;
	std::vector<int>::iterator it2;
	unsigned int cuts = 0;
	impruved_Vcut;
	impruved_Vcut.push_back(vec_cut.front());
	Vec_index_on_basic_path.push_back(0);
	impruved_Vcut.push_back(vec_cut.back());
	Vec_index_on_basic_path.push_back(100);
	bool done = 0;
	while (done == 0)
	{
		unsigned int i = 1;
		while (i < impruved_Vcut.size())
		{
			if (chacke_2_points_path(impruved_Vcut[i - 1], impruved_Vcut[i]) == 1)	i++;
			else
			{	
				unsigned int devidewith = pow(2, (double)(cuts));
				unsigned int index_on_basic_path = Vec_index_on_basic_path[i - 1];
				cuts++;
				devidewith = pow(2, (double) (cuts));
				unsigned int newpoint_index = index_on_basic_path + 100 /(devidewith);
				pair<int, int> newpoint = pathpoints[newpoint_index];
				it = impruved_Vcut.begin();
				it = it + i;
				impruved_Vcut.insert(it, newpoint);
				it2 = Vec_index_on_basic_path.begin();
				it2 = it2 + i;
				Vec_index_on_basic_path.insert(it2, newpoint_index);
				i = 1;												// if "chacke_2_points_path" find an obstacle it add a new point to the path, their for the loop need to be initialized.
			}
		}
		done = 1;
	}
	return(impruved_Vcut);
}
void optimize_path()
{
	//fill_BuffMap();
	int steps4cut = 100;
	int num_of_cuts = pathpoints.size() / steps4cut;
	impruved_pathpoints.push_back(pathpoints[0]);
	for (int i = 0; i < num_of_cuts; i++)
	{
		vector<pair <int, int>> vec_cut = cut_vector(pathpoints, steps4cut, i);
		optimize_cut(vec_cut);
		for (int k = 1; k < impruved_Vcut.size(); k++)
		{
			impruved_pathpoints.push_back(impruved_Vcut[k]);
		}
		impruved_Vcut.clear();
		impruved_pathpoints[0] = { 1000,1000 };
	}
	

}
//		old optimize_path
/*
{
	fill_BuffMap();
	impruved_pathpoints.push_back(pathpoints.front());
	impruved_pathpoints.push_back(pathpoints.back());
	bool done = 0;
	while (done == 0)
	{
		unsigned int i = 1;
		while( i < impruved_pathpoints.size() )
		{
			if (chacke_2_points_path(impruved_pathpoints[i - 1], impruved_pathpoints[i], i) == 1)	i++;
			// if "chacke_2_points_path" find an obstacle it add a new point to the path, their for the loop need to be initialized.
			else 
			{
				i = 1;
			}
		}
		done = 1;
	}
}
*/

//-------------		control functions:		---------------------

//experiment functions for the connection:

void read_arduino_check(Serial* SP)
{
	if (SP->IsConnected())
		std::cout << "We're connected";
	const int dataLength = 255;
	char incomingData[dataLength] = "";			// don't forget to pre-allocate memory
	int readResult = 0;
	char ledOn [] = "ON\n";
	char ledOff[] = "OFF\n";
	SP->WriteData(ledOn, dataLength);
	Sleep(1000);
	SP->WriteData(ledOff, dataLength);
	Sleep(1000);
}   

void read_arduino_check2(Serial* SP)
{
	
	while (1)
	{
		if (SP->IsConnected())
		std::cout << "We're connected";
		char out[255] = "";
		const int dataLength = 255;
		char incomingData[dataLength] = "";			// don't forget to pre-allocate memory

		int readResult = 0;

		char ledOn[] = "ON\n";
		char ledOff[] = "OFF\n";


		std::cout << "\npleas write somthing:" << endl;

		fgets(out, 255, stdin);
 		if (out[0] == '0')
		{
			break;
		}
		SP->WriteData(out, dataLength);
		readResult = SP->ReadData(incomingData, dataLength);
		std::cout << "Bytes read: (0 means no data available)" << readResult << endl;
		std::cout << incomingData;
	}

}

//end of experiment functions for the connection:

void send_mess(Serial* SP, int mode, double u = 0)
{
	//      mode 1: turning.	  mode 2: forward.			mode 3: Right motor.		mode 4: left motor		mode 5: get sensors data.

	if (u > 255) u = 255;
	SP->flush();
	char out[20];
	if (mode == 1) sprintf_s(out, "T %f \n", u);
	if (mode == 2) sprintf_s(out, "F %f \n", u);
	if (mode == 3) sprintf_s(out, "R %f \n", u);
	if (mode == 4) sprintf_s(out, "L %f \n", u);
	if (mode == 5) sprintf_s(out, "SENS \n");
	SP->flush();

	SP->WriteData(out, sizeof(out));

}

void read_sensors(Serial* SP)
{
	double REvalue;
	double LEvalue;
	//double Accelarationvalue;

	if (SP->IsConnected())
		std::cout << "We're connected";

	int readResult = 0;

	int indx = 0;
	char endMarker = '\n';

	int m = 0;
	int num_of_sensors = 4;                  // need to change by demand.
	while (m < num_of_sensors)
	{
		string incomingstring;
		char incomingchar[1] = "";			// don't forget to pre-allocate memory
		m = 0;
		//SP->WriteData(out, dataLength);
		readResult = 0;

		//while (readResult == 0)
		//{
		send_mess(SP, 5);
		delay(100);
		int i = 0;
		//char old_char = 0;
		while (incomingchar[0] != '\n')
		{
			i++;
			readResult = SP->ReadData(incomingchar, 1);
			if (i > 1000)
			{
				send_mess(SP, 5);
				i = 0;
			}

			//if (old_char != incomingchar[0])
			//{
			incomingstring.push_back(incomingchar[0]);
			//	old_char = incomingchar[0];
			//}
		}

		std::cout << "in: " << incomingstring << endl;

		istringstream tempstring(incomingstring);

		do
		{
			string title;
			tempstring >> title;
			if (title == " ")
			{
			}
			if (title == "EL" || title == " EL")			// recive left encoder
			{
				tempstring >> LEvalue;
				std::cout << "\nLE: " << LEvalue << endl;
				//delay(500);
				m = m + 1;
			}
			if (title == "ER")			// recive right encoder
			{
				tempstring >> REvalue;
				std::cout << "RE: " << REvalue << endl;
				//delay(500);
				m = m + 1;
			}
			if (title == "H")				// recive compass engle
			{
				tempstring >> heading;
				std::cout << "H: " << heading << endl;
				//delay(500);
				m = m + 1;
			}
			if (title == "D")				// recive compass engle
			{
				tempstring >> stopped_dist;
				std::cout << "stopped dist: " << stopped_dist << endl;
				//delay(500);
				m = m + 1;
			}
			//if (title == "AC")				// recive accelaration
			//{
			//tempstring >> Accelarationvalue;
			//std::cout << "AC: " << Accelarationvalue<<endl;
			//delay(500);
			//m = m + 1;
			//}
		} while (tempstring);
	}
	if (m == num_of_sensors)
	{
		//std::cin >> Compassvalue;
		//std::cin >> Accelarationvalue;
		right_encoder_mesure.push_back(REvalue);
		left_encoder_mesure.push_back(LEvalue);
		//compass_mesure.push_back(heading);
		//accelaration_mesure.push_back(Accelarationvalue);

		timer.push_back((long)clock() / CLOCKS_PER_SEC);
		cout << "time: " << timer.back() << endl;

	}
	else
	{
		std::cout << "cant read sensors, trying again..";
		read_sensors(SP);
	}
}

int Control_Low_level(RPlidarDriver * drv, Serial* SP, double ref, int mode)
{
	//      mode 1: turning.	  mode 2: forward.			mode 3: Right motor.		mode 4: left motor.					 default modeis Right motor.

	int readResult = 0;
	int finish_condition = 0;
	//int ret;
	char incomingchar[2] = "";
	SP->flush();
	delay(1000);
	send_mess(SP, mode, ref);
	clock_t Tinitial = clock() / CLOCKS_PER_SEC;
	int time_max;
	if (mode == 1) time_max = 10; //sec
	if (mode == 2) time_max = 30; //sec
	
	while (finish_condition == 0)			//wait to finih the turning, read lidar, stop if neccesery
	{
		
		clock_t Tnow = clock() / CLOCKS_PER_SEC;
		if (Tnow - Tinitial > time_max)
		{
			cout << "out of time"<<endl;
			read_sensors(SP);
			if (mode == 1)
			{
				finish_condition = Control_Low_level(drv, SP, ref, 1);
			}
			if (mode == 2)
			{
				double error = ref - stopped_dist;
				finish_condition = Control_Low_level(drv, SP, error, 2);
			}

		}
		string incomingstring;

		char incomingdata[10] = "";
		
		readResult = SP->ReadData(incomingdata, 10);
		int i = 0;
		while (incomingdata[i] != '\r' && i<10)
		{
			incomingstring.push_back(incomingdata[i]);
			i++;
		}
		istringstream tempstring(incomingstring);
		string title;
		tempstring >> title;
		if (title == "D")									//D==Done
		{
			std::cout << "DONE" << endl;
			finish_condition = 1;
		}
		if (title == "S")
		{
			std::cout << "emergency stop" << endl;
			finish_condition = 2;
		}

		/*if (title == "P1")
		{
			std::cout << "lidar saw a close obstacle, turn stopped"<<endl;
			finish_condition = 2;
		}
		if (title == "P2")
		{
			std::cout << "stopped before finish, Emergancy botton was pushed";
			finish_condition = 2;
		}
		if (title == "P3")
		{
			std::cout << "stopped before finish, Emergancy Bumper was pushed";
			finish_condition = 2;
		}
		if (title == "P4")
		{
			std::cout << "stopped before finish, cliff sensor saw a cliff";
			finish_condition = 2;
		}*/
		
		//read lidar from -30 to 30
		//start_lidar_and_gett_RoboMap(drv, 2);
		//if (danger != 0)
		//{
		//	char out[2] = "S";
		//	std::cout << "S";
		//	SP->WriteData(out, 2);
		//}

		//SP->flush();
	}
	return finish_condition;
}

/*void final_position_and_orientation ()
// uses the diferntial drive robot forward kinematics to determine the real position of the robot (odeometry) 
{
	vector<double> delta_right_encoder_mesure;
	vector<double> delta_left_encoder_mesure;
	vector<double> W;
	vector<double> R;
	vector<double> X;
	vector<double> Y;
	vector<double> theta;
	vector <pair<double, double>> ICC;
	pair <int, int> start_pozition = oldpoints.back();

	X.push_back(start_pozition.first);
	Y.push_back(start_pozition.second);
	theta.push_back(oldheading.back());
	double delt = 0.1;
	for (unsigned int i = 0; i < min(right_encoder_mesure.size() - 1, left_encoder_mesure.size() - 1); i++)
	{
		delta_right_encoder_mesure.push_back(right_encoder_mesure[i + 1] - right_encoder_mesure[i]);
		delta_left_encoder_mesure.push_back(left_encoder_mesure[i + 1] - left_encoder_mesure[i]);
		W.push_back((delta_right_encoder_mesure[i] - delta_left_encoder_mesure[i]) / (wheels_dist));
		R.push_back((wheels_dist / 2)*(delta_right_encoder_mesure[i] + delta_left_encoder_mesure[i]) / (delta_right_encoder_mesure[i] - delta_left_encoder_mesure[i]));
		ICC.push_back({ X[i] - R[i] * sin(theta[i]), Y[i] + R[i] * cos(theta[i]) });
		X.push_back(cos(W[i] * delt)*(X[i] - ICC[i].first) - sin(W[i] * delt)*(Y[i] - ICC[i].second) + ICC[i].first);
		Y.push_back(sin(W[i] * delt)*(X[i] - ICC[i].first) + cos(W[i] * delt)*(Y[i] - ICC[i].second) + ICC[i].second);
		theta.push_back(theta[i] + W[i] * delt);
	}
	// the current position of the robot from the center of the world map.
	location[0] = (int)X[X.size()];
	location[1] = (int)Y[Y.size()];
	heading = theta[theta.size()];
}*/

bool Control_High_level(RPlidarDriver * drv, Serial* SP)
//takes the points in impruved_pathpoints, uses "control_turn()","control_farward()" and "final_position_and_orientation_chek()" to take the robot to next point.
{
	bool done = 0;
	vector <pair<double, double>> dist_theta;

	int delx = (impruved_pathpoints[1].first) - (impruved_pathpoints[0].first);
	int dely = impruved_pathpoints[1].second - impruved_pathpoints[0].second;
	pair<double, double> step_dist_theta = XY_2_dist_theta({ delx, dely }); // the distance and engle to the next step on the path
	dist_theta.push_back(step_dist_theta);  // the distance and engle to the next turn on the path

	int k = 0;
	for (int i = 1; i < impruved_pathpoints.size() - 1; i++)
	{
		delx = (impruved_pathpoints[i + 1].first ) - (impruved_pathpoints[i].first);
		dely = impruved_pathpoints[i + 1].second - impruved_pathpoints[i].second;
		step_dist_theta = XY_2_dist_theta({ delx, dely }); // the distance and engle to the next step on the path
		if (step_dist_theta.second == dist_theta[k].second)
		{ 
			dist_theta[k].first += step_dist_theta.first;
		}
		else
		{
			dist_theta.push_back(step_dist_theta);
			k++;
		}
	}
	
	for (int i = 0; i < dist_theta.size(); i++)
	{
		done = 1;
		int finish_condition = 0;
		do
			// turn the robot to the path diraction:
		{	 
			finish_condition = Control_Low_level(drv, SP, dist_theta[i].second, 1);
		// (posiball addition: get the real angle and fix)
		}	
		while (finish_condition != 1);
		
		
		finish_condition=Control_Low_level(drv, SP, dist_theta[i].first, 2);		// go forward.

		if (finish_condition==2)                                  
		// when the robot is stoped befor it finish the path. it will get the robot position, and return to main that the path wasn't complited.
		{
			//get_location_when_stoped:
			int x = location[0]; int y = location[1];
			for (int k = 0; k < i; k++)
			{
				int dX = D_and_Theta_2_X(dist_theta[k].first, dist_theta[k].second*pi / 180);
				int dY= D_and_Theta_2_Y(dist_theta[k].first, dist_theta[k].second*pi / 180);
				x += dX;
				y += dY;
			}
			location[0] = x;
			location[1] = y;
			read_sensors(SP);
			int stoped_dx = D_and_Theta_2_X(stopped_dist, dist_theta[i].second*pi/180);
			int stoped_dy = D_and_Theta_2_Y(stopped_dist, dist_theta[i].second*pi/180);
			location[0] += stoped_dx;
			location[1] += stoped_dy;
			done = 0; 
			i = dist_theta.size();
		}
			}

	if (done == 1)
	{
		location[0] = impruved_pathpoints.back().first - macroMapSize / 2;
		location[1] = impruved_pathpoints.back().second - macroMapSize / 2;
		pathpoints.clear();
		impruved_pathpoints.clear();
		return done;
	}
	if (done == 0)
	{
		pathpoints.clear();
		impruved_pathpoints.clear();
		return done;
	}
}



//__________Maping functions:________

void add_a_new_obstacle()
{
	int bufferx = 15;
	if (abs(location[0]+macroMapSize/2) > macroMapSize - 5)
		bufferx = macroMapSize - abs(location[0]);
	
	int buffery = 15;
	if (abs(location[1] + macroMapSize / 2) > macroMapSize - 5)
		buffery = macroMapSize - abs(location[1]);

	for (int i = 0; i < bufferx; i++)
	{
		for (int j = 0; j < buffery; j++)
		{
			RoomMap[location[0] + macroMapSize / 2 + i][location[1] + macroMapSize / 2 + j] = 1;
		}
	}
}
void savemap()
{
	ofstream wr;
	wr.open ("Map.txt", ios_base::trunc);
	
	//char charmap[macroMapSize][macroMapSize];
	
	for (int i = 0; i < macroMapSize; i++)
	{
		for (int j = 0; j < macroMapSize; j++)
		{
			wr << RoomMap[i][j];
			wr << " ";
		}
		wr << endl;
	}
	wr.close();
}
void loadmap()
{
	ifstream read;
	read.open("Map.txt", ios_base::in);
	for (int i = 0; i < macroMapSize; ++i)
	{	
		for (int j = 0; j < macroMapSize; ++j) 
		{ 
			read>>RoomMap[i][j];
		}
	}
	
	read.close();
}
void creatmap(RPlidarDriver * drv, Serial* SP)
{
	int fine = 0;
	while (fine != 1)
	{
		// initialization, set location ass zerro
		location[0] = 0;
		location[1] = 0;

		do {
			// make a scan with the lidar, rotate 180 deg and scan again, go to next point.
			bool tryagain = 1;
			Control_Low_level(drv, SP, 0, 1);
			read_sensors(SP);
			start_lidar_and_gett_RoboMap(drv, 1);
			PrintRobotMap();
			add_result_to_whole_map(location[0], location[1], heading*pi / 180);
			fill_Room_buffer();
			fill_horizon();
			//PrintRoomMap();

			double turn180 = 0;
			Control_Low_level(drv, SP, 180, 1);
			read_sensors(SP);

			/*if (heading >= 180) turn180 = heading - 180;
			if (heading < 180) turn180 = heading + 180;
			cout << turn180;
			delay(500);
			Control_Low_level(drv, SP, turn180, 1);

			delay(500);
			bool check = 0;
			while (check == 0)
			{
			read_sensors(SP);
			check = 1;
			if (abs(heading - turn180)>1.5 && abs(heading - turn180)<30)
			{
			check = 0;
			cout << "		!!!fixing!!!		"<<endl;
			cout << "disierd angle: "<< turn180 <<"		actual angle: "<< heading << endl<<endl;
			Control_Low_level(drv, SP, turn180, 1);
			}
			if (abs(heading - turn180)>30)
			{
			check = 0;
			cout << "		!!!fixing!!!		" << endl;
			cout << "disierd angle: " << turn180 << "		actual angle: " << heading << endl << endl;
			Control_Low_level(drv, SP, 0, 1);
			Control_Low_level(drv, SP, (int)turn180, 1);
			}
			}*/
			start_lidar_and_gett_RoboMap(drv, 1);
			add_result_to_whole_map(location[0], location[1], heading*pi / 180);
			fill_Room_buffer();
			fill_horizon();
			//PrintRoomMap();

			//choos and go to the closest point on the horizon that the robot can go to:
			int f = findnextpoint();
			std::printf("x: %d y: %d f:%d", nextpoint[0], nextpoint[1], f);
			if (fill_navigationMap() == 1)
			{
				tryagain = 0;
				std::cout << "finish";
			}

			//PrintNavMap();
			std::cout << "size" << pathpoints.size() << '\n';
			path(nextpoint[0], nextpoint[1]);
			std::cout << "size" << pathpoints.size() << '\n';
			//PrintRoomMap();
			optimize_path();
			PrintRoomMap();
			history_looced_at.push_back({ location[0], location[1] });
			bool OK = 0;
			while (OK == 0)
			{
				OK=Control_High_level(drv, SP);
				if (OK == 0)
					// if the robot is stoped: 1. add a new abstacle. 2. go back. 3. calculate a new path to the next point. 4. go to the next point.
				{
					add_a_new_obstacle();
					Control_Low_level(drv, SP, 0, 2);			//resat the controller .
					read_sensors(SP);
					Control_Low_level(drv, SP, -30, 2);			//go back 30 cm. 
					int dX= D_and_Theta_2_X(-30, heading*pi/180);// update the position. 
					int dY= D_and_Theta_2_X(-30, heading*pi/180);
					location[0] += dX;
					location[1] += dY;
					fill_navigationMap();
					//PrintNavMap();
					path(nextpoint[0], nextpoint[1]);
					//PrintRoomMap();
					optimize_path();
					//PrintRoomMap();
				}
			}
		}

		while (ENDMAPING == 0);
		PrintRoomMap();

		cout << "does the map looks fine?" << endl << "1- Yes" << endl << "2- No" << endl << endl;
		cin >> fine;
		if (fine == 1)
		{	savemap();	
		}
		if (fine == 2)
		{
			int d = 0;
			while (d != 1)
			{
				cout << "Pleas put the robot in the charger. press 1 when done." << endl;
				cin >> d;
				if (d != 0 && d != 1)
				{
					cout << "You pressed undifined order! Pleas press 1 when the robot is in the charger!" << endl;
				}
			}
		}
	}

		/**/
}

//________operating functions:____________

bool show_next_pos_on_map(int x, int y)	
{

	SDL_Window* window = SDL_CreateWindow("SDL App", 5, 40, WINDOW_WIDTH, WINDOW_WIDTH, SDL_WINDOW_RESIZABLE);

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

	SDL_RenderSetLogicalSize(renderer, macroMapSize, macroMapSize);

	SDL_Event event;

	SDL_SetRenderDrawColor(renderer, 0, 225, 0, 225);
	SDL_RenderClear(renderer);

	for (int i = 0; i < macroMapSize; i++)
	{
		for (int j = 0; j < macroMapSize; j++)
		{
			if (RoomMap[i][j] == 0) // print dead zone
			{
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
				SDL_RenderDrawPoint(renderer, i, macroMapSize - j);
			}

			if (RoomMap[i][j] == 1) //print obstacles
			{
				SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
				SDL_RenderDrawPoint(renderer, i, macroMapSize - j);
			}

			if (RoomMap[i][j] == 2) //print free zone
			{
				SDL_SetRenderDrawColor(renderer, 140, 120, 255, 255);
				SDL_RenderDrawPoint(renderer, i, macroMapSize - j);
			}

			if (RoomMap[i][j] == 3) // print docking
			{
				SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
				SDL_RenderDrawPoint(renderer, i, macroMapSize - j);
			}
			if (RoomMap[i][j] == 4) // print docking
			{
				SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
				SDL_RenderDrawPoint(renderer, i, macroMapSize - j);
			}

		}

	}

	for (int i = -2; i <= 2; i++)   // print robot position in silver
	{
		for (int j = -2; j <= 2; j++)
		{
			SDL_SetRenderDrawColor(renderer, 169, 169, 169, 255);
			SDL_RenderDrawPoint(renderer, (i + location[0] + macroMapSize / 2), macroMapSize - (j + location[1] + macroMapSize / 2));
		}
	}


	for (int i = -2; i <= 2; i++)   // //print the next position in purple
	{
		for (int j = -2; j <= 2; j++)
		{
			SDL_SetRenderDrawColor(renderer, 221, 160, 221, 255);
			SDL_RenderDrawPoint(renderer, (i + x+ macroMapSize/2), macroMapSize - (j + y+ macroMapSize / 2));
		}
	}


	SDL_RenderPresent(renderer);

	while (1) {
		if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
			break;
	}
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return EXIT_SUCCESS;
}
void delete_the_obstacle(int x, int y)
{
	int bufferx = 15;
	if (abs(x + macroMapSize / 2) > macroMapSize - 5)
		bufferx = macroMapSize - abs(x);

	int buffery = 15;
	if (abs(y + macroMapSize / 2) > macroMapSize - 5)
		buffery = macroMapSize - abs(y);

	for (int i = 0; i < bufferx; i++)
	{
		for (int j = 0; j < buffery; j++)
		{
			RoomMap[x + macroMapSize / 2 + i][y + macroMapSize / 2 + j] = 2;
		}
	}
}
void drive2(int x, int y, RPlidarDriver * drv, Serial* SP)
{
	fill_navigationMap();
	//PrintNavMap();
	path(x+macroMapSize/2, y+macroMapSize/2);
	std::cout << "size" << pathpoints.size() << '\n';
	PrintRoomMap();
	optimize_path();
	//PrintRoomMap();

	vector <pair<int, int>> temp_obs;
	bool OK;
	do
	{
		OK=Control_High_level(drv, SP);
		if (OK == 0)
			// if the robot is stoped: 1. add a temporary abstacle. 2. go back. 3. calculate a new path to the next point. 4. go to the next point.
		{
			//int posx = location[0];
			//int posy = location[1];
			temp_obs.push_back({ location[0],location[1] });
			add_a_new_obstacle();
			Control_Low_level(drv, SP, 0, 2);			//resat the controller .
			read_sensors(SP);
			Control_Low_level(drv, SP, -30, 2);			//go back 30 cm. 
			int dX = D_and_Theta_2_X(-30, heading*pi / 180);// update the position. 
			int dY = D_and_Theta_2_Y(-30, heading*pi / 180);
			location[0] += dX;
			location[1] += dY;
			read_sensors(SP);
			fill_navigationMap();
			PrintNavMap();
			path(x + macroMapSize / 2, y + macroMapSize / 2);
			PrintRoomMap();
			optimize_path();
		}
	} while (OK == 0);
	
	for (int i = 0; i < temp_obs.size(); i++)
	{
		delete_the_obstacle(temp_obs[i].first, temp_obs[i].second);
	}
	
}

//________Points of interes functions:___________

void save_points_to_memory()
{
	ofstream wr;
	wr.open("interest_points.txt", ios_base::trunc);

	for (int i = 0; i < NUMBER_OF_INTEREST_POINTS; i++)
	{
		wr << "i: " << i + 1 << " ";
		wr << "X: " << points_of_interest_list[i][1] << " ";
		wr << "Y: " << points_of_interest_list[i][2] << " ";
		wr << endl;
	}
	wr.close();
}
void change_interess_point_and_save(int point_num)
{
	points_of_interest_list[point_num - 1][1] = location[1];
	points_of_interest_list[point_num - 1][2] = location[2];
	save_points_to_memory();
}
void load_points_from_memory()
{
	bool xdone = 0;
	bool ydone = 0;
	int i;
	ifstream read;
	read.open("interest_points.txt", ios_base::in);
	do
	{
			string title;
			read >> title;
			if (title == " ")
			{
			}
			if (title == "i:")
			{
				read >> i;
			}
			if (title == "X:")			// recive right encoder
			{
				read >> points_of_interest_list[i-1][0];
				if (i == 10) xdone = 1;
			}
			if (title == "Y:")				// recive compass engle
			{
				read >> points_of_interest_list[i - 1][1];
				if (i == 10) ydone = 1;
			}
	}
	while (xdone==0||ydone==0);
	read.close();
}



//__________________________________	MAIN:	_____________________________________

int main(int argc, const char * argv[])
{	
	
	//oldpoints.push_back({ 0, 0 });
	//oldheading.push_back(0);
	//horizonlist.push_back({ 0, 0 });			
	//pathpoints.push_back({ 0, 0 });
	//right_encoder_mesure.push_back(0);
	//left_encoder_mesure.push_back(0);
	//compass_mesure.push_back(0);
	//accelaration_mesure.push_back(0);
	//timer.push_back(0);

	 //initialize lidar conection:
							const char * opt_com_path = NULL;
							_u32         opt_com_baudrate = 115200;
							u_result     op_result;

							std::printf("LIDAR COMUNICATION.\n");
							if (!opt_com_path) {
						#ifdef _WIN32
								// use default com port
								opt_com_path = "\\\\.\\com4";
						#else
								opt_com_path = "/dev/ttyUSB0";
						#endif
							}
							// create the driver instance
							RPlidarDriver * drv = RPlidarDriver::CreateDriver(RPlidarDriver::DRIVER_TYPE_SERIALPORT);

							if (!drv) {
								fprintf(stderr, "insufficent memory, exit\n");
								exit(-2);
							}


							// make connection...
							if (IS_FAIL(drv->connect(opt_com_path, opt_com_baudrate))) {
								fprintf(stderr, "Error, cannot bind to the specified serial port %s.\n"
									, opt_com_path);
								goto on_finished;
							}

							rplidar_response_device_info_t devinfo;

							// retrieving the device info
							////////////////////////////////////////

							op_result = drv->getDeviceInfo(devinfo);

							if (IS_FAIL(op_result)) {
								fprintf(stderr, "Error, cannot get device info.\n");
								goto on_finished;
							}

							// print out the device serial number, firmware and hardware version number..
							std::cout<<"RPLIDAR S/N: ";
							for (int pos = 0; pos < 16; ++pos) {
								std::cout<<"%02X"<< devinfo.serialnum[pos];
							}
							
							
							std :: printf("\n"
								"Firmware Ver: %d.%02d\n"
								"Hardware Rev: %d\n"
								, devinfo.firmware_version >> 8
								, devinfo.firmware_version & 0xFF
								, (int)devinfo.hardware_version);



							// check health...
							if (!checkRPLIDARHealth(drv)) {
								goto on_finished;
							}

//initialize arduino communication:
							std::cout<<"\nARDUINO COMUNICATION."<<endl;
							Serial* SP = new Serial("\\\\.\\COM3");    // adjust as needed
							

								
//work:						
							load_points_from_memory();
							int what2do = 0;
							while (what2do != 1 && what2do != 2)
							{
								cout << "welcome! :)"<<endl<<"What do you want to do?" << endl << "1-	creat a new map" << endl << "2- use old map from memory"<<endl<<endl;
								cin >> what2do;
								if (what2do != 1 && what2do != 2) cout << "error: you enterd undifined order!!" << endl;
							}
							
							if (what2do == 1)			creatmap(drv,SP);
							if (what2do == 2)
							{
								loadmap();
								PrintRoomMap();
								int YN = 0;
								while (YN == 0)
								{
									cout << "does the map maches the room? " << endl << "1-	YES " << endl << "2- NO" << endl << endl;
									cin >> YN;
									if (YN != 1 && YN != 2)
									{
										cout << "error: you enterd undifined order!!" << endl;
										YN = 0;
									}
									if (YN == 1)
									{
										cout << "  :)		:)	    	:)  " << endl;
									}
									if (YN == 2)
									{
										int W2d = 0;
										while (W2d == 0)
										{
											cout << "What to do? " << endl << "1-	built a new map " << endl << "2- quit" << endl << endl;
											cin >> W2d;
											if (W2d != 1 && W2d != 2)
											{
												cout << "error: you enterd undifined order!!" << endl;
												W2d = 0;
											}
											if (W2d == 1) creatmap(drv, SP);
											if (W2d == 2) 
											{
												cout << "good bye"<<endl;
												goto on_finished;
											}
										}
									}
								}
							}
							
							what2do = 0;
							while (what2do !=1)
							{
								cout << "What do you want to do next?" << endl << "1-	QUIT " << endl << "2- Go to another position." << endl<<"3- save robot position"  << endl;
								cin >> what2do;
								
								if (what2do != 1 && what2do != 2 && what2do != 3)
									cout << "error: you enterd undifined order!!" << endl;
								
								if (what2do == 1)
								{
									int YN = 0;
									while (YN == 0)
									{
										cout << "ARE YOU SHURE YOU WANT TO QUIT? " << endl << "1-	YES " << endl << "2- NO" << endl << endl;
										cin >> YN;
										if (YN != 1 && YN != 2)
										{
											cout << "error: you enterd undifined order!!" << endl;
											YN = 0;
										}
										if (YN == 1)
										{
											cout << "GOOD BYE!" << endl;
										}
										if (YN == 2)
										{
											what2do = 0;
										}
									}
								}
								if (what2do == 2)
								{
									int x, y;
									cout << "Pleas enter the coordinat of the next location: " << endl;
									cout << "x:";
									cin >> x;
									cout << "y:";
									cin >> y;
									show_next_pos_on_map(x,y);
									int YN = 0;
									while (YN == 0)
									{
										cout << "is this is the right position?" << endl << "1-	YES " << endl << "2- NO" << endl << endl;
										cin >> YN;
										if (YN != 1 && YN != 2)
										{
											cout << "error: you enterd undifined order!!" << endl;
											YN = 0;
										}
										if (YN == 1)
										{
											drive2(x, y,drv, SP);
										}
										if (YN == 2)
										{
											cout << "Pleas enter the corect coordinat of the next location: " << endl;
											cout << "x:";
											cin >> x;
											cout << "y:";
											cin >> y;
											show_next_pos_on_map(x,y);
											YN = 0;
										}
									}
								}
								if (what2do == 3)
								{
									int pointnum;
									cout << "which old point you want to switch?" <<endl<< "(choos a number between 1 to 10)"<< endl;
									cin >> pointnum;
									change_interess_point_and_save(pointnum);
								}
							}

									on_finished:
	drv->stop();
	drv->stopMotor();
	RPlidarDriver::DisposeDriver(drv);
	delay(1000);
	



	return 0;
	/**/
	}