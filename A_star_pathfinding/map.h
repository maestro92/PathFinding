#pragma once

#include "utility_math.h"
#include <vector>
#include <functional>
#include "utility_json.h"
#include "utility_debug.h"
#include <unordered_set>
#include <iostream>
#include <fstream>
using namespace std;


class Map
{


	public: 	
		enum class Cell
		{
			Empty,
			Wall,	
			Water,
			Grass,
			NUM_TYPES
		};

		struct GridCell
		{
			Cell type;
			int cost;
			bool renderAsPath;
		};

		Map();
		void SetCell_Internal(int x, int y, Map::Cell type);
		void init(int w, int h);

		void debug();
		
		int getWidth();
		int getHeight();

		Cell getCell(glm::vec2 coord);
		Cell getCell(int x, int y);
		void setCell(int x, int y, Map::Cell gem);
		void setCell(glm::ivec2 gc, Map::Cell gem);
		Map::GridCell getGridCell(int x, int y);

		void RenderPath(vector<glm::ivec2> path);
		void clearPath();

		glm::ivec2 worldPos2GridCoord(glm::vec2 pos);
		glm::vec2 gridCoord2WorldPos(glm::ivec2 gridCoord);
		glm::vec2 getCellMinCorner(glm::ivec2 gridCoord);
		glm::vec2 getCellMaxCorner(glm::ivec2 gridCoord);
		glm::vec2 getCellCenter(glm::ivec2 gridCoord);

		float getCellSize();
		bool IsTraversable(glm::ivec2 coord);
		bool IsValidRange(glm::vec2 coord);
		bool IsValidCoord(glm::ivec2 coord);

		int CoordToId(glm::ivec2 coord)
		{
			return coord.y * getWidth() + coord.x;
		}

		glm::ivec2 IdToCoord(int id)
		{
			return glm::ivec2(id % getWidth(), id / getWidth());
		}




	private:
		float m_cellSize;
		vector<vector<GridCell>> gridmap;
};