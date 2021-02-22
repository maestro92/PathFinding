#include "map.h"


Map::Map()
{
	m_cellSize = 1;
}

void Map::SetCell_Internal(int x, int y, Map::Cell type)
{
	switch (type)
	{
	case Map::Cell::Empty:
		gridmap[y][x] = { type, 1 };
		break;
	case Map::Cell::Wall:
		gridmap[y][x] = { type, 99999};
		break;
	case Map::Cell::Water:
		gridmap[y][x] = { type, 10 };
		break;
	case Map::Cell::Grass:
		gridmap[y][x] = { type, 5 };
		break;

	}
}

void Map::init(int w, int h)
{
	for (int y = 0; y < h; y++)
	{
		vector<GridCell> row(w);
		gridmap.push_back(row);
	}
	
	srand(time(NULL));
	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			SetCell_Internal(x, y, Map::Cell::Empty);
		}
	}

	glm::vec2 min = glm::vec2(2, 2);
	glm::vec2 max = glm::vec2(4, 8);

	for (int y = min.y; y <= max.y; y++)
	{
		for (int x = min.x; x <= max.x; x++)
		{
			SetCell_Internal(x, y, Map::Cell::Wall);
		}
	}

	min = glm::vec2(6, 6);
	max = glm::vec2(13, 14);
	for (int y = min.y; y <= max.y; y++)
	{
		for (int x = min.x; x <= max.x; x++)
		{
			SetCell_Internal(x, y, Map::Cell::Grass);
		}
	}
}

bool Map::IsValidRange(glm::vec2 coord)
{
	if (coord.x < 0)	return false;
	if (coord.y < 0)	return false;
	if (coord.x >= getWidth())	return false;
	if (coord.y >= getHeight())	return false;
	return true;
}

bool Map::IsValidCoord(glm::ivec2 coord)
{
	if (coord.x < 0)	return false;
	if (coord.y < 0)	return false;
	if (coord.x >= getWidth())	return false;
	if (coord.y >= getHeight())	return false;
	return true;
}

int Map::getWidth()
{
	return gridmap[0].size();
}

int Map::getHeight()
{
	return gridmap.size();
}

Map::Cell Map::getCell(int x, int y)
{
	return gridmap[y][x].type;
}

Map::Cell Map::getCell(glm::vec2 coord)
{
	return gridmap[coord.y][coord.x].type;
}

Map::GridCell Map::getGridCell(int x, int y)
{
	return gridmap[y][x];
}

void Map::debug()
{
	for (int y = 0; y < getHeight(); y++)
	{
		string s = "";
		for (int x = 0; x < getWidth(); x++)
		{
			s += utl::intToStr((int)(gridmap[y][x].type));
		}
		cout << s << endl;
	}
	cout << endl;
}

void Map::RenderPath(vector<glm::ivec2> path)
{
	for (int i = 0; i < path.size(); i++)
	{
		gridmap[path[i].y][path[i].x].renderAsPath = true;
	}
}


void Map::clearPath()
{
	for (int y = 0; y < getHeight(); y++)
	{
		for (int x = 0; x < getWidth(); x++)
		{
			if (gridmap[y][x].renderAsPath)
			{
				gridmap[y][x].renderAsPath = false;
			}
		}
	}
}

float Map::getCellSize()
{
	return m_cellSize;
}

bool Map::IsTraversable(glm::ivec2 coord)
{
	return IsValidCoord(coord) && getCell(coord.x, coord.y) != Map::Cell::Wall;
}

// http://www.cplusplus.com/forum/beginner/60827/
// c++ truncates, aka rounds down
glm::ivec2 Map::worldPos2GridCoord(glm::vec2 pos)
{
	return glm::ivec2(static_cast<int>(pos.x), static_cast<int>(pos.y));
}

glm::vec2 Map::gridCoord2WorldPos(glm::ivec2 gridCoord)
{
	return getCellCenter(gridCoord);
}

glm::vec2 Map::getCellMinCorner(glm::ivec2 gridCoord)
{
	return glm::vec2(gridCoord.x, gridCoord.y);
}

glm::vec2 Map::getCellMaxCorner(glm::ivec2 gridCoord)
{
	glm::vec2 pos = glm::vec2(gridCoord.x, gridCoord.y);
	pos.x += m_cellSize;
	pos.y += m_cellSize;
	return pos;
}

glm::vec2 Map::getCellCenter(glm::ivec2 gridCoord)
{
	glm::vec2 pos = glm::vec2((int)gridCoord.x, (int)gridCoord.y);
	pos.x += m_cellSize / 2;
	pos.y += m_cellSize / 2;
	return pos;
}

void Map::setCell(int x, int y, Map::Cell gem)
{
	if (IsValidRange(glm::vec2(x, y)))
	{
		SetCell_Internal(x, y, gem);
	}
}

void Map::setCell(glm::ivec2 gc, Map::Cell gem)
{
	if (IsValidRange(glm::vec2(gc.x, gc.y)))
	{
		SetCell_Internal(gc.x, gc.y, gem);
	}
}



