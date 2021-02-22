#ifndef MAIN_H_
#define MAIN_H_

class WorldObject;

#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include <string>
#include <sstream>
#include <unordered_set>
#include "SDL.h"
#include "SDL_image.h"

#include "define.h"
#include "utility.h"
#include "map.h"
#include "map.h"
#include "map_view.h"

#include <queue>

#include <GL/glew.h>

#include "utility.h"
#include "shader.h"

#include "label.h"
#include "gui_manager.h"
#include <chrono>

#include "pipeline.h"

#include <ft2build.h>

#include "renderer_manager.h"
#include "renderer.h"
#include "renderer_constants.h"
#include "circle.h"

#include "quad_model.h"
#include "xyz_axis_model.h"
#include "world_object.h"
#include "raycaster.h"
#include "model_manager.h"

#define FRAME_VALUES 10
#include <list>

#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>


#include <vector>

using namespace std;

const int TIME_PROFILER_BUFFER = 10;
const int FPS_PROFILER_BUFFER = 20;

enum class CostFunctionToUse
{
	BFS, 
	Dijkstra,
	Herustic,
	A_star
};

enum DebugDisplayMode
{
	None,
	Coord,
	Cost,
	PathCost,
};

class PathFinding
{
	public:
		Renderer*					p_renderer;
		
		/// GUI
		long long m_runningTime;
		Uint32 m_nextGameTick = 0;
		MouseState m_mouseState;

		Pipeline m_pipeline;

		float m_fps;
		float m_iterRefreshRate;
		float m_curIter;
		unsigned int m_frameCount;
		unsigned int m_frameTicks[FRAME_VALUES];
		unsigned int m_frameTicksIndex;
		unsigned int m_prevFrameTick;

		bool isRunning;

	//	vector<WorldObject> players;
	//	vector<WorldObject> food;

		WorldObject player;
		WorldObject dstObject;

		glm::ivec2 dstCoord;

		// models
		Model*          p_model;
		bool containedFlag;

		WorldObject     o_worldAxis;
		
		int pathIter = 0;
		int pathIndex = 0;
		vector<glm::ivec2> traversePath;
		
		Map map;
		MapView mapView;
	
		GUIManager m_gui;
		bool loadData;
		int frameNum;
		float m_zoom;
		float m_range;
	public:
		void ResetPath();
		void ChangeDstination();

		long long m_currentTimeMillis;

		int timeProfilerIndex;
		long long timeProfiler[TIME_PROFILER_BUFFER];

		int fpsProfilerIndex;
		int fpsProfiler[FPS_PROFILER_BUFFER];

		void DebugClear();
		void DebugShowCoords();
		void DebugShowCosts();
		void DebugShowAccumulatedCosts();
		DebugDisplayMode debugDisplayMode;
		CostFunctionToUse costFunctionMode;

		void ExecutePathFinding(glm::ivec2 src, glm::ivec2 dst, vector<glm::ivec2>& path);
		void BFSPathFinding(glm::ivec2 src, glm::ivec2 dst, vector<glm::ivec2>& path);
		void DijkstraPathFinding(glm::ivec2 src, glm::ivec2 dst, vector<glm::ivec2>& path);
		void HeursiticSearchPathFinding(glm::ivec2 src, glm::ivec2 dst, vector<glm::ivec2>& path);

		unordered_map<int, int> nodeCostsSoFar;
		

		/// init functions
		void init();

		void initGUI();

		WorldObject* constructLine(glm::vec2 p0, glm::vec2 p1, float width) const;

		int endWithError(char* msg, int error = 0);
 
		void StartWalking();
		void start();
		void update();

		glm::vec3 screenToUISpace(glm::vec2 screenPoint);
		glm::vec3 screenToWorldPoint(glm::vec2 screenPoint);
		glm::vec3 worldToScreen(glm::vec3 pos);

		int getAverageFPS();

		void Eat(WorldObject* eater, WorldObject* food);
		void SimTick(long long dt);
		void clientFrame(long long dt);
		void render();
		void onMouseBtnUp();
		void onMouseBtnHold();

		WorldObject constructPoint(glm::vec2 p, float width) const;

		void GetTimeProfilerAverages();

		void renderGUI();
		void updateCamera();
		glm::vec2 m_cameraCenter;
		float m_cameraZoom = 50;
		long long getCurrentTimeMillis();
};

#endif