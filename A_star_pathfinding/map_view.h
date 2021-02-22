#ifndef MAP_VIEW_H_
#define MAP_VIEW_H_

#include "utility_sdl.h"
#include "map.h"
#include "world_object.h"
#include "model_manager.h"
#include "renderer.h"
#include "renderer_manager.h"
#include "circle.h"


class MapView
{
	public:
		void init(Map* map);
		void initGridLines();
		void render(Pipeline& p);
		void reset();
		void createMeshForGridCellsHighlight(vector<glm::ivec2> traversal);
		WorldObject& getWorldObject(int x, int y);


	private:

		Map * m_map;

		Renderer*					p_renderer;
		vector< vector<WorldObject> > gridCells;
		
		Model* gridLineModels;			
		WorldObject gridLines;

		Model* circleModel;
		WorldObject circleGameObject;

		WorldObject cirlceCenterGameObject;
};


#endif
