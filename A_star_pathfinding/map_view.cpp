#include "map_view.h"
#include "global.h"

void MapView::init(Map* map)
{
	m_map = map;
	int w = map->getWidth();
	int h = map->getHeight();
	
	for (int y = 0; y < h; y++)
	{
		vector<WorldObject> row;

		for (int x = 0; x < w; x++)
		{
			WorldObject obj = WorldObject();
			obj.setModel(global.modelMgr->get(ModelEnum::centeredQuad));

			glm::vec2 pos = map->getCellCenter(glm::ivec2(x, y));

			obj.setPosition(glm::vec3(pos.x, pos.y, 0));

			obj.setScale(0.8);
			row.push_back(obj);
		}

		gridCells.push_back(row);
	}

	initGridLines();
}




void MapView::initGridLines()
{
	//	gridLines
	std::vector<VertexData> vertices;
	std::vector<unsigned int> indices;
	VertexData v;

	for (int x = 0; x <= m_map->getWidth(); x++)
	{
		glm::vec3 pos0 = glm::vec3(x, 0, 0);
		glm::vec3 pos1 = glm::vec3(x, m_map->getHeight(), 0);

		ModelManager::buildLinesBetweenTwoPoints2D(pos0, pos1, 0.02, vertices, indices);
	}

	for (int y = 0; y <= m_map->getHeight(); y++)
	{
		glm::vec3 pos0 = glm::vec3(0, y, 0);
		glm::vec3 pos1 = glm::vec3(m_map->getWidth(), y, 0);
		ModelManager::buildLinesBetweenTwoPoints2D(pos0, pos1, 0.02, vertices, indices);
	}
	Mesh m(vertices, indices);

	gridLineModels = new Model();
	gridLineModels->addMesh(m);

	gridLines.setModel(gridLineModels);


}

WorldObject& MapView::getWorldObject(int x, int y)
{
	return gridCells[y][x];
}

void MapView::reset()
{
	gridCells.clear();
}

void MapView::render(Pipeline& p)
{
	p_renderer = &global.rendererMgr->r_fullColor;

	p_renderer->enableShader();
	for (int y = 0; y < m_map->getHeight(); y++)
	{
		for (int x = 0; x < m_map->getWidth(); x++)
		{
			glm::vec3 color;
			switch (m_map->getCell(x,y))
			{
				case Map::Cell::Empty:
					color = COLOR_WHITE;
					break;
				case Map::Cell::Wall:
					color = COLOR_GRAY;
					break;
				case Map::Cell::Water:
					color = COLOR_BLUE;
					break;
				case Map::Cell::Grass:
					color = COLOR_GREEN;
					break;
				default:
					break;
			}

			if (m_map->getGridCell(x, y).renderAsPath)
			{
				color = COLOR_ORANGE;
			}
				
			p_renderer->setData(R_FULL_COLOR::u_color, color);
			gridCells[y][x].renderGroup(p, p_renderer);
		}

	}

	p_renderer->setData(R_FULL_COLOR::u_color, COLOR_BLACK);
	gridLines.renderGroup(p, p_renderer);


	if (circleGameObject.canRender())
	{
		p_renderer->setData(R_FULL_COLOR::u_color, COLOR_RED);
		circleGameObject.renderGroup(p, p_renderer);
	}

	if (cirlceCenterGameObject.canRender())
	{
		p_renderer->setData(R_FULL_COLOR::u_color, COLOR_RED);
		cirlceCenterGameObject.renderGroup(p, p_renderer);
	}


	p_renderer->disableShader();
}

void MapView::createMeshForGridCellsHighlight(vector<glm::ivec2> traversal)
{
	//	gridLines
	std::vector<VertexData> vertices;
	std::vector<unsigned int> indices;
	VertexData v;

	for (int i = 0; i < traversal.size(); i++)
	{
		glm::ivec2 gridCoord = traversal[i];

		glm::vec2 pos0 = m_map->getCellMinCorner(gridCoord);
		glm::vec2 pos2 = m_map->getCellMaxCorner(gridCoord);
		
		glm::vec2 pos1 = glm::vec2(pos2.x, pos0.y);
		glm::vec2 pos3 = glm::vec2(pos0.x, pos2.y);


		ModelManager::buildLinesBetweenTwoPoints2D(pos0, pos1, 0.05, vertices, indices);
		ModelManager::buildLinesBetweenTwoPoints2D(pos1, pos2, 0.05, vertices, indices);
		ModelManager::buildLinesBetweenTwoPoints2D(pos2, pos3, 0.05, vertices, indices);
		ModelManager::buildLinesBetweenTwoPoints2D(pos3, pos0, 0.05, vertices, indices);
	}

	Mesh m(vertices, indices);


}