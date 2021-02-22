#include "main.h"

#include <stdio.h>
#include <string.h>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <cmath>

#include "global.h"

#define RENDER_DEBUG_FLAG 0



float CAMERA_POS_DELTA = 1;
float CAMERA_ZOOM_DELTA = 1;

#define PI 3.14159265

// the server simluates the game in descirete time steps called ticks



// http://stackoverflow.com/questions/4845410/error-lnk2019-unresolved-external-symbol-main-referenced-in-function-tmainc
#undef main


using namespace std;

//display surface
SDL_Surface* pDisplaySurface = NULL;
//event structure
SDL_Event event;


const float GAME_SPEED = 1.0f;
const float _FIXED_UPDATE_TIME_s = 0.01667f;
const float FIXED_UPDATE_TIME_s = _FIXED_UPDATE_TIME_s / GAME_SPEED;
const float FIXED_UPDATE_TIME_ms = FIXED_UPDATE_TIME_s * 1000;

const float MOUSE_DIST_THRESHOLD = 0.05;



const int INVALID_OBJECT = 0x7FFFFFFF;

// link
// http://lodev.org/cgtutor/raycasting.html



void PathFinding::init()
{
	frameNum = 0;

	global.modelMgr = new ModelManager();
	global.modelMgr->init();

	// renderMgr has to init after the lightMgr
	global.rendererMgr = new RendererManager();
	global.rendererMgr->init(utl::SCREEN_WIDTH, utl::SCREEN_HEIGHT);

	srand(0);

	player.setPosition(0.5, 0.5, 0);
	player.radius = 0.5;
	player.setScale(glm::vec3(player.radius));
	player.setModel(global.modelMgr->get(ModelEnum::unitCircle));

	// somewhere invisible
	dstObject.setPosition(-10, -10, 0);
	dstObject.setScale(glm::vec3(0.2));
	dstObject.setModel(global.modelMgr->get(ModelEnum::unitCircle));

	isRunning = true;


	containedFlag = false;

	timeProfilerIndex = 0;
	fpsProfilerIndex = 0;

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// initGUI depends on the m_defaultPlayerID, so initNetworkLobby needs to run first
	initGUI();

	for (int i = 0; i < FPS_PROFILER_BUFFER; i++)
	{
		fpsProfiler[i] = 0;
	}


	//Initialize clear color
	glClearColor(0.0f, 0.5f, 0.0f, 1.0f);

	m_pipeline.setMatrixMode(PROJECTION_MATRIX);
	m_pipeline.loadIdentity();
	//	m_pipeline.perspective(90, utl::SCREEN_WIDTH / utl::SCREEN_HEIGHT, utl::Z_NEAR, utl::Z_FAR);

	m_cameraZoom = 12;
	m_cameraCenter = glm::vec2(10, 10);

	updateCamera();

	ModelManager::enableVertexAttribArrays();

	glCullFace(GL_BACK);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	SDL_WM_SetCaption("MatchThree", NULL);


	loadData = false;
	bool runningTests = false;

	float scale = 100.0;
	o_worldAxis.setScale(scale);
	o_worldAxis.setModel(global.modelMgr->get(ModelEnum::xyzAxis));


	map.init(20, 20);
	mapView.init(&map);
	DebugShowCoords();


}

GLuint tempTexture;

void PathFinding::DebugClear()
{
	m_gui.removeDebugLabels();
}

void PathFinding::DebugShowCoords()
{
	m_gui.removeDebugLabels();
	float size = 11;

	for (int y = 0; y < map.getHeight(); y++)
	{
		for (int x = 0; x < map.getWidth(); x++)
		{
			{
				glm::vec3 pos = mapView.getWorldObject(x, y).m_position;
				glm::vec3 screenPos = worldToScreen(glm::vec3(pos.x, pos.y, 0));
				glm::vec3 labelPos = screenToUISpace(glm::vec2(screenPos.x, screenPos.y));

				string s = utl::intToStr(x) + " " + utl::intToStr(y);

				Label coordLabel(s, labelPos.x + 15, labelPos.y, 0, 0, COLOR_WHITE);
				coordLabel.setFont(size, COLOR_BLUE);
				m_gui.addDebugLabel(coordLabel);
			}
			
		}
	}
}

void PathFinding::DebugShowCosts()
{
	m_gui.removeDebugLabels();
	float size = 11;

	for (int y = 0; y < map.getHeight(); y++)
	{
		for (int x = 0; x < map.getWidth(); x++)
		{
			{
				glm::vec3 pos = mapView.getWorldObject(x, y).m_position;
				glm::vec3 screenPos = worldToScreen(glm::vec3(pos.x, pos.y, 0));
				glm::vec3 labelPos = screenToUISpace(glm::vec2(screenPos.x, screenPos.y));

				int cost = map.getGridCell(x, y).cost;
				string s = utl::intToStr(cost);

				Label coordLabel(s, labelPos.x + 10, labelPos.y, 0, 0, COLOR_WHITE);
				coordLabel.setFont(size, COLOR_BLUE);
				m_gui.addDebugLabel(coordLabel);
			}

		}
	}
}

void PathFinding::DebugShowAccumulatedCosts()
{
	m_gui.removeDebugLabels();
	float size = 11;

	for (auto it = nodeCostsSoFar.begin(); it != nodeCostsSoFar.end(); it++)
	{
		int id = it->first;
		glm::ivec2 coord = map.IdToCoord(id);

		glm::vec3 pos = mapView.getWorldObject(coord.x, coord.y).m_position;
		glm::vec3 screenPos = worldToScreen(glm::vec3(pos.x, pos.y, 0));
		glm::vec3 labelPos = screenToUISpace(glm::vec2(screenPos.x, screenPos.y));

		string s = utl::intToStr(it->second);

		Label coordLabel(s, labelPos.x+20, labelPos.y, 0, 0, COLOR_WHITE);
		coordLabel.setFont(size, COLOR_BLUE);
		m_gui.addDebugLabel(coordLabel);
	}
}


void PathFinding::Eat(WorldObject* eater, WorldObject* food)
{
	// Eat and Grow
}

/*
The way I see this problem

*/
void PathFinding::SimTick(long long dt)
{
	// get next dstination
	if (pathIndex < traversePath.size())
	{
		pathIter++;
		if (pathIter % 2 == 0)
		{
			int index = traversePath.size() - 1 - pathIndex;
			glm::ivec2 nextDst = traversePath[index];

			glm::vec2 pos = map.gridCoord2WorldPos(nextDst);
			player.setPosition(pos.x, pos.y, 0);
			pathIndex++;
		}
	}
	else
	{
		pathIter = 1000000;
		pathIndex = 1000000;
	}

}


void PathFinding::clientFrame(long long dt)
{
	SimTick(dt);
	render();
}

void PathFinding::GetTimeProfilerAverages()
{
	long long total = 0;
	for (int i = 0; i < TIME_PROFILER_BUFFER; i++)
	{
		total += timeProfiler[i];
	}
	cout << "average is " << total / TIME_PROFILER_BUFFER << endl;
}


void PathFinding::start()
{
	cout << "Start" << endl;

	long long dt = 0;
	long long oldTime = utl::getCurrentTime_ms();
	long long newTime = 0;

	Uint32 time0 = 0;
	Uint32 time1 = 0;

	while (isRunning)
	{
		time0 = SDL_GetTicks();

		newTime = utl::getCurrentTime_ms();

		dt = newTime - oldTime;
		update();

		clientFrame(dt);


		oldTime = newTime;

		time1 = SDL_GetTicks();


		// cout << fpsProfilerIndex << endl;
		if (fpsProfilerIndex == FPS_PROFILER_BUFFER)
		{
			fpsProfilerIndex = 0;
		}
		fpsProfiler[fpsProfilerIndex] = (int)(time1 - time0);
		++fpsProfilerIndex;

		int fps = getAverageFPS();
		// cout << fps << endl;

		/*
		++fpsProfilerIndex;
		if (fpsProfilerIndex > 1000)
		{
			fpsProfilerIndex = 0;
		}
		*/

		// fpsProfilerIndex = 1;
		m_gui.setFPS(fps);
	}
}


int PathFinding::getAverageFPS()
{
	float averageFrameTime = 0;
	for (int i = 0; i < FPS_PROFILER_BUFFER; i++)
	{
		averageFrameTime += fpsProfiler[i];
	}

	if (averageFrameTime == 0)
	{
		return 0;
	}
	else
	{
		averageFrameTime = averageFrameTime / FPS_PROFILER_BUFFER;

		int fps = 1000 / averageFrameTime;

		//	cout << averageFrameTime << " " << fps << endl;
		return fps;
	}
}


void PathFinding::updateCamera()
{
	m_pipeline.ortho(m_cameraCenter.x - m_cameraZoom,
		m_cameraCenter.x + m_cameraZoom,
		m_cameraCenter.y - m_cameraZoom,
		m_cameraCenter.y + m_cameraZoom, utl::Z_NEAR, utl::Z_FAR);

	//	debugDrawing(curDrawing);
}


glm::vec3 PathFinding::screenToWorldPoint(glm::vec2 screenPoint)
{
	glm::vec4 viewPort = glm::vec4(0, 0, utl::SCREEN_WIDTH, utl::SCREEN_HEIGHT);
	glm::vec3 temp = glm::vec3(screenPoint.x, screenPoint.y, 0);

	glm::vec3 worldPoint = glm::unProject(temp, (m_pipeline.getModelViewMatrix()), m_pipeline.getProjectionMatrix(), viewPort);
	return worldPoint;
}

glm::vec3 PathFinding::screenToUISpace(glm::vec2 screenPoint)
{
	glm::vec4 viewPort = glm::vec4(0, 0, utl::SCREEN_WIDTH, utl::SCREEN_HEIGHT);
	glm::vec3 temp = glm::vec3(screenPoint.x, screenPoint.y, 0);

	glm::vec3 worldPoint = glm::unProject(temp, (m_gui.getPipeline().getModelViewMatrix()), m_gui.getPipeline().getProjectionMatrix(), viewPort);

	return worldPoint;
}

glm::vec3 PathFinding::worldToScreen(glm::vec3 pos)
{
	glm::vec4 viewPort = glm::vec4(0, 0, utl::SCREEN_WIDTH, utl::SCREEN_HEIGHT);
	//	glm::vec3 screenPos = glm::project(pos, glm::inverse(m_pipeline.getModelViewMatrix()), m_pipeline.getProjectionMatrix(), viewPort);
	glm::vec3 screenPos = glm::project(pos, m_pipeline.getModelViewMatrix(), m_pipeline.getProjectionMatrix(), viewPort);
	return screenPos;
}

void PathFinding::ResetPath()
{
	// hack method
	pathIter = 1000000;
	pathIndex = 1000000;
}

void PathFinding::ChangeDstination()
{
	ResetPath();
	cout << "change dstination" << endl;
	int tmpx, tmpy;
	SDL_GetMouseState(&tmpx, &tmpy);
	tmpy = utl::SCREEN_HEIGHT - tmpy;

	glm::vec2 screenPoint = glm::vec2(tmpx, tmpy);
	glm::vec3 worldPoint = screenToWorldPoint(screenPoint);
	glm::vec2 dst = glm::vec2(worldPoint.x, worldPoint.y);

	glm::ivec2 dstGc = map.worldPos2GridCoord(dst);


	if (!map.IsTraversable(dstGc))
	{
		return;
	}
	else if (dstGc == dstCoord)
	{
		return;
	}
	else
	{
		glm::vec2 src = glm::vec2(player.getPosition().x, player.getPosition().y);

		glm::ivec2 srcGc = map.worldPos2GridCoord(src);
		dstObject.setPosition(dst.x, dst.y, 0);

		vector<glm::ivec2> path;
		ExecutePathFinding(srcGc, dstGc, path);

		traversePath = path;
		map.clearPath();
		map.RenderPath(path);

		if (debugDisplayMode == DebugDisplayMode::PathCost)
		{
			DebugShowAccumulatedCosts();
		}
	}

}

void PathFinding::StartWalking()
{
	pathIter = 0;
	pathIndex = 0;
}

void PathFinding::update()
{
	int mx, my;
	SDL_GetMouseState(&mx, &my);

	// need this for GUI
	m_mouseState.m_pos = glm::vec2(mx, utl::SCREEN_HEIGHT - my);


	SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym)
			{
			case SDLK_ESCAPE:
				isRunning = false;
				break;

			case SDLK_1:
				costFunctionMode = CostFunctionToUse::BFS;
				m_gui.setCostFunction("BFS");
				break;

			case SDLK_2:
				costFunctionMode = CostFunctionToUse::Dijkstra;
				m_gui.setCostFunction("Dijkstra");
				break;

			case SDLK_3:
				costFunctionMode = CostFunctionToUse::Herustic;
				m_gui.setCostFunction("Herustic");
				break;

			case SDLK_4:
				costFunctionMode = CostFunctionToUse::A_star;
				m_gui.setCostFunction("A*");
				break;

			case SDLK_5:
			case SDLK_6:

			case SDLK_7:
				DebugShowCoords();
				debugDisplayMode = DebugDisplayMode::Coord;
				break;
			case SDLK_8:
				DebugShowCosts();
				debugDisplayMode = DebugDisplayMode::Cost;
				break;
			case SDLK_9:
				DebugShowAccumulatedCosts();
				debugDisplayMode = DebugDisplayMode::PathCost;
				break;
			case SDLK_0:
				DebugClear();
				debugDisplayMode = DebugDisplayMode::None;
				break;


			case SDLK_m:
				StartWalking();
				break;

			default:
				break;
			}
			break;

		case SDL_MOUSEBUTTONDOWN:
			switch (event.button.button)
			{
			case SDL_BUTTON_LEFT:
				break;

			case SDL_BUTTON_RIGHT:
				ChangeDstination();
				break;

			case SDL_BUTTON_WHEELUP:
				// m_cameraZoom -= CAMERA_ZOOM_DELTA;
				updateCamera();
				break;
			case SDL_BUTTON_WHEELDOWN:
				// m_cameraZoom += CAMERA_ZOOM_DELTA;
				updateCamera();
				break;
			}
			break;

		case SDL_MOUSEBUTTONUP:
			switch (event.button.button)
			{
			case SDL_BUTTON_LEFT:
				onMouseBtnUp();
				break;
			}
			break;
		}
	}

}




struct Entry
{
	glm::ivec2 cur;
	glm::ivec2 from;	// remembering the path
};

vector<glm::ivec2> directions = { glm::ivec2(1, 0), glm::ivec2(-1, 0), glm::ivec2(0, 1), glm::ivec2(0, -1) };

// https://www.redblobgames.com/pathfinding/a-star/introduction.html


struct Node
{
	glm::ivec2 cur;
	glm::ivec2 from;	// remembering the path
	int cost;
};

class HeursiticSearchNodeComparison
{
public:
	// moderling the < operator. Determine who goes in front of the array.
	// min heap, min is gonna be at the end of the array
	// so whoever is larger goes in the front
	bool operator() (const Node& lhs, const Node& rhs) const
	{
		return lhs.cost > rhs.cost;
	}
};

int HeuristicCost(glm::ivec2 coord, glm::ivec2 dst)
{
	return std::abs(coord.x - dst.x) + std::abs(coord.y - dst.y);
}

void PathFinding::HeursiticSearchPathFinding(glm::ivec2 src, glm::ivec2 dst, vector<glm::ivec2>& path)
{
	nodeCostsSoFar.clear();

	// min heap to return the node that has the least cost
	priority_queue<Node, vector<Node>, HeursiticSearchNodeComparison> q;

	unordered_map<int, Node> visited;
	// if you are not in the nodeCostsSoFar table, that means you have infinity cost
	// cost so far traveling to that node

	q.push({ src, glm::ivec2(0,0), 0 });
	int srcNodeId = map.CoordToId(src);

	Node dstEntry;
	while (!q.empty())
	{
		Node curNode = q.top();
		q.pop();

		int coordId = map.CoordToId(curNode.cur);

		// mark the current node as visited
		visited[coordId] = curNode;
		if (curNode.cur == dst)
		{
			dstEntry = curNode;
			break;
		}

		glm::ivec2 from = curNode.cur;
		for (int i = 0; i < directions.size(); i++)
		{
			glm::ivec2 neighbor = from + directions[i];
			int neighborId = map.CoordToId(neighbor);

			// if not valid skip
			if (!map.IsValidCoord(neighbor)) { continue; }

			if (visited.find(neighborId) != visited.end()) { continue; }

			// if you never appeared innodeCostsSoFar, that means you have infinite costs
			// so we add you.
			int heuristicCost = HeuristicCost(neighbor, dst);
			q.push({ neighbor, from, heuristicCost });
			nodeCostsSoFar[neighborId] = heuristicCost;
		}
	}

	// find path
	glm::ivec2 cur = dst;
	while (cur != src)
	{
		path.push_back(cur);

		int id = map.CoordToId(cur);
		Node entry = visited[id];
		cur = entry.from;
	}
}







class DijkstraNodeComparison
{
public:
	// moderling the < operator. Determine who goes in front of the array.
	// min heap, min is gonna be at the end of the array
	// so whoever is larger goes in the front
	bool operator() (const Node& lhs, const Node& rhs) const
	{
		return lhs.cost > rhs.cost;
	}
};

// https://www.youtube.com/watch?v=pVfj6mxhdMw&ab_channel=ComputerScience
void PathFinding::DijkstraPathFinding(glm::ivec2 src, glm::ivec2 dst, vector<glm::ivec2>& path)
{
	nodeCostsSoFar.clear();

	// min heap to return the node that has the least cost
	priority_queue<Node, vector<Node>, DijkstraNodeComparison> q;
	
	unordered_map<int, Node> visited;
	// if you are not in the nodeCostsSoFar table, that means you have infinity cost
	// cost so far traveling to that node
	
	q.push({ src, glm::ivec2(0,0), 0 });
	int srcNodeId = map.CoordToId(src);
	nodeCostsSoFar[srcNodeId] = 0;

	Node dstEntry;
	while (!q.empty())
	{
		Node curNode = q.top();
		q.pop();

		int coordId = map.CoordToId(curNode.cur);

		// mark the current node as visited
		visited[coordId] = curNode;
		if (curNode.cur == dst)
		{
			dstEntry = curNode;
			break;
		}

		glm::ivec2 from = curNode.cur;
		for (int i = 0; i < directions.size(); i++)
		{
			glm::ivec2 neighbor = from + directions[i];
			int neighborId = map.CoordToId(neighbor);

			// if not valid skip
			if (!map.IsValidCoord(neighbor)) { continue;	}
			
			if (visited.find(neighborId) != visited.end()) { continue;	}

			int nodeNewCost = nodeCostsSoFar[coordId] + map.getGridCell(neighbor.x, neighbor.y).cost;
			// if you never appeared innodeCostsSoFar, that means you have infinite costs
			// so we add you.
			if (nodeCostsSoFar.find(neighborId) == nodeCostsSoFar.end() || 
				nodeNewCost < nodeCostsSoFar[neighborId])
			{
				q.push({ neighbor, from, nodeNewCost });
				nodeCostsSoFar[neighborId] = nodeNewCost;
			}
		}
	}

	// find path
	glm::ivec2 cur = dst;
	while (cur != src)
	{
		path.push_back(cur);

		int id = map.CoordToId(cur);
		Node entry = visited[id];
		cur = entry.from;
	}
}


void PathFinding::BFSPathFinding(glm::ivec2 src, glm::ivec2 dst, vector<glm::ivec2>& path)
{
	queue<Entry> q;
	q.push({ src, glm::ivec2(0,0) });
	unordered_map<int, Entry> visited;

	Entry dstEntry;
	while (!q.empty())
	{
		Entry curNode = q.front();
		q.pop();

		if (!map.IsValidCoord(curNode.cur))
		{
			continue;
		}

		int coordId = map.CoordToId(curNode.cur);
		if (visited.find(coordId) != visited.end())
		{
			continue;
		}

		if (map.getCell(curNode.cur.x, curNode.cur.y) == Map::Cell::Wall)
		{
			continue;
		}


		visited[coordId] = curNode;

		if (curNode.cur == dst)
		{
			dstEntry = curNode;
			break;
		}

		glm::ivec2 from = curNode.cur;
		for (int i = 0; i < directions.size(); i++)
		{
			q.push({ from + directions[i], from });
		}
	}

	// find path
	glm::ivec2 cur = dst;
	while (cur != src)
	{
		path.push_back(cur);

		int id = map.CoordToId(cur);
		Entry entry = visited[id];
		cur = entry.from;
	}
}

void PathFinding::ExecutePathFinding(glm::ivec2 src, glm::ivec2 dst, vector<glm::ivec2>& path)
{
	// BFSPathFinding(src, dst, path);
	// DijkstraPathFinding(src, dst, path);
	// HeursiticSearchPathFinding(src, dst, path);
}

void PathFinding::onMouseBtnUp()
{

}


WorldObject PathFinding::constructPoint(glm::vec2 p, float width) const
{
	WorldObject obj = WorldObject();
	obj.setModel(global.modelMgr->get(ModelEnum::centeredQuad));
	obj.setPosition(glm::vec3(p.x, p.y, 0));

	obj.setScale(width);

	return obj;
}

WorldObject* PathFinding::constructLine(glm::vec2 p0, glm::vec2 p1, float width) const
{
	WorldObject* obj = new WorldObject();
	obj->setModel(global.modelMgr->get(ModelEnum::centeredQuad));

	glm::vec2 diffVector = p1 - p0;
	glm::vec2 centerPoint = p0 + glm::vec2(diffVector.x / 2.0, diffVector.y / 2.0);

	obj->setPosition(glm::vec3(centerPoint.x, centerPoint.y, 0));

	float angle = atan2(diffVector.y, diffVector.x) * 180 / PI;

	float length = glm::distance(p0, p1);

	glm::vec3 scale(length, width, 1);

	obj->setRotation(glm::rotate(angle, 0.0f, 0.0f, 1.0f));

	obj->setScale(scale);

	return obj;
}


/*
fixing the first and end point,

combine points that can do a line fit
*/
void PathFinding::render()
{
	// *******************************************************
	// ************* Rendering *******************************
	// *******************************************************

	m_pipeline.setMatrixMode(MODEL_MATRIX);
	glBindFramebuffer(GL_FRAMEBUFFER, RENDER_TO_SCREEN);
	glViewport(0, 0, utl::SCREEN_WIDTH, utl::SCREEN_HEIGHT);


	m_pipeline.setMatrixMode(VIEW_MATRIX);
	m_pipeline.loadIdentity();

	m_pipeline.translate(0.0f, 0.0f, 5.0f);

	m_pipeline.setMatrixMode(MODEL_MATRIX);
	//	m_pipeline.loadIdentity();

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);


	glCullFace(GL_BACK);
	glClear(GL_DEPTH_BUFFER_BIT);


	if (frameNum == 0)
	{
		map.debug();
	}

	mapView.render(m_pipeline);

	p_renderer = &global.rendererMgr->r_fullColor;
	p_renderer->enableShader();

	p_renderer->setData(R_FULL_COLOR::u_color, COLOR_BLACK);
	player.renderGroup(m_pipeline, p_renderer);

	p_renderer->setData(R_FULL_COLOR::u_color, COLOR_RED);
	dstObject.renderGroup(m_pipeline, p_renderer);
	p_renderer->disableShader();


	// Rendering wireframes
	p_renderer = &global.rendererMgr->r_fullVertexColor;
	p_renderer->enableShader();
	o_worldAxis.renderGroup(m_pipeline, p_renderer);
	p_renderer->disableShader();



	long long timeNowMillis = getCurrentTimeMillis();

	int deltaTimeMillis = (unsigned int)(timeNowMillis - m_currentTimeMillis);
	m_currentTimeMillis = timeNowMillis;



	int fps = getAverageFPS();
	m_gui.setFPS(fps);
	m_gui.initGUIRenderingSetup();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_BLEND);


	m_gui.updateAndRender(m_mouseState);

	SDL_GL_SwapBuffers();
	frameNum++;
}



long long PathFinding::getCurrentTimeMillis()
{
#ifdef WIN32
	return GetTickCount();
#else
	timeval t;
	gettimeofday(&t, NULL);

	long long ret = t.tv_sec * 1000 + t.tv_usec / 1000;
	return ret;
#endif
}



#define MAX_CLIENTS 10
#define SERVER_PORT 60000


int main(int argc, char* argv[])
{
	utl::debug("Game Starting");
	utl::initSDL(utl::SCREEN_WIDTH, utl::SCREEN_HEIGHT, pDisplaySurface);
	utl::initGLEW();

	PathFinding Martin;


	Martin.init();
	Martin.start();

	utl::exitSDL(pDisplaySurface);
	//normal termination

	while (1)
	{

	}

	cout << "Terminating normally." << endl;
	return EXIT_SUCCESS;
}


int PathFinding::endWithError(char* msg, int error)
{
	//Display error message in console
	cout << msg << "\n";
	system("PAUSE");
	return error;
}


// http://kcat.strangesoft.net/mpstream.c



void PathFinding::initGUI()
{
	// run this before m_gui.init, so the textEngine is initialized
	// need to comeback and re-organize the gui the minimize dependencies
	Control::init("", 25, utl::SCREEN_WIDTH, utl::SCREEN_HEIGHT);
	m_gui.init(utl::SCREEN_WIDTH, utl::SCREEN_HEIGHT);
}


void PathFinding::renderGUI()
{

	m_gui.initGUIRenderingSetup();
	/// http://sdl.beuc.net/sdl.wiki/SDL_Average_FPS_Measurement
	//	unsigned int getTicks = SDL_GetTicks();

	//	static string final_str = "(" + utl::floatToStr(m_mouseState.m_pos.x) + ", " + utl::floatToStr(m_mouseState.m_pos.y) + ")";
	m_gui.updateAndRender(m_mouseState);

	// healthbar and text


}




