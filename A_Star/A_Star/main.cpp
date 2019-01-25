#include <iostream>
#include <vector>
#include <algorithm>
#include <SDL.h>

constexpr unsigned int node_block_size = 20;
constexpr unsigned int map_size_x = 800 / node_block_size;
constexpr unsigned int map_size_y = 600 / node_block_size;

struct Mouse
{
	int X, Y;
	bool Pressed;
	int Button;
} g_Mouse;

enum class Node_Type : char
{
	NODE_EMPTY = 0,
	NODE_WALL,
	NODE_START,
	NODE_END,
	NODE_NEIGHBOUR,
	NODE_VISITED,
	NODE_FINAL_PATH,
};

struct Node
{
	Node_Type Type = Node_Type::NODE_EMPTY;
	int x = 0;
	int y = 0;	
	int GCost = 0;	// distance from starting node
	int HCost = 0;	// distance from end node
	int FCost = 0;	// GCost + HCost
	Node* CameFrom = nullptr;

	void Draw(SDL_Renderer* renderer) const
	{
		SDL_Rect rect = { x * node_block_size, y * node_block_size, node_block_size, node_block_size };
		switch (Type)
		{
			case Node_Type::NODE_EMPTY:
				SDL_SetRenderDrawColor(renderer, 0x0, 0x00, 0xFF, 0xFF);
				SDL_RenderDrawRect(renderer, &rect);
				break;
			case Node_Type::NODE_WALL:
				SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);
				SDL_RenderFillRect(renderer, &rect);
				break;
			case Node_Type::NODE_START:
				SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
				SDL_RenderFillRect(renderer, &rect);
				break;
			case Node_Type::NODE_END:
				SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0x00, 0xFF);
				SDL_RenderFillRect(renderer, &rect);
				break;
			case Node_Type::NODE_NEIGHBOUR:
				SDL_SetRenderDrawColor(renderer, 0x0, 0x00, 0xFF, 0xFF);
				SDL_RenderFillRect(renderer, &rect);
				break;
			case Node_Type::NODE_VISITED:
				SDL_SetRenderDrawColor(renderer, 238, 244, 66, 0xFF);
				SDL_RenderFillRect(renderer, &rect);
				break;
			case Node_Type::NODE_FINAL_PATH:
				SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0xFF);
				SDL_RenderFillRect(renderer, &rect);
				break;
			default:
				break;
		}
	}
};

void DrawMap(const Node map[][map_size_y], SDL_Renderer* renderer)
{
	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
	SDL_RenderClear(renderer);

	for (int i = 0; i < map_size_x; ++i)
		for (int j = 0; j < map_size_y; ++j)
			map[i][j].Draw(renderer);

	SDL_RenderPresent(renderer);
}

std::vector<Node*> GetNeighbours(Node map[map_size_x][map_size_y], Node* node)
{
	std::vector<Node*> neighbours;
	int start_pos_x = node->x > 0 ? node->x - 1 : 0;
	int end_pos_x = node->x + 1 >= map_size_x ? node->x : node->x + 1;
	int start_pos_y = node->y > 0 ? node->y - 1 : 0;
	int end_pos_y = node->y + 1 >= map_size_y ? node->y : node->y + 1;

	for (int i = start_pos_x; i <= end_pos_x; ++i)
		for (int j = start_pos_y; j <= end_pos_y; ++j)
			if (i != node->x || j != node->y)
				neighbours.push_back(&map[i][j]);

	return neighbours;
}


void FindAStarPath(Node map[map_size_x][map_size_y], Node* startNode, Node* endNode, SDL_Renderer* renderer)
{
	std::vector<Node*> openList;
	std::vector<Node*> closedList;
	openList.push_back(startNode);

	while (true)
	{
		DrawMap(map, renderer);

		std::sort(openList.begin(), openList.end(), [](Node* a, Node* b) {
			return a->FCost < b->FCost;
		});

		Node* currentNode = *(openList.begin());
		openList.erase(openList.begin());

		if (currentNode != startNode && currentNode != endNode)
			currentNode->Type = Node_Type::NODE_VISITED;

		closedList.push_back(currentNode);
		if (currentNode == endNode)
		{
			while (currentNode)
			{
				if (currentNode != startNode && currentNode != endNode)
					currentNode->Type = Node_Type::NODE_FINAL_PATH;
				currentNode = currentNode->CameFrom;
			}
			return;
		}

		const std::vector<Node*>& neighbours = GetNeighbours(map, currentNode);
		for (auto it = neighbours.begin(); it != neighbours.end(); ++it)
		{
			Node* currentNeighbour = *it;
			if (currentNeighbour->Type == Node_Type::NODE_WALL ||
				std::find(closedList.begin(), closedList.end(), currentNeighbour) != closedList.end())
				continue;

			// use Manhatten distance calculation
			int currentGCost = std::abs(startNode->x - currentNeighbour->x) +
				std::abs(startNode->y - currentNeighbour->y);
			int currentHCost = std::abs(endNode->x - currentNeighbour->x) +
				std::abs(endNode->y - currentNeighbour->y);
			int currentFCost = currentGCost + currentHCost;

			bool inList = std::find(openList.begin(), openList.end(), currentNeighbour) != openList.end();
			if (currentFCost < currentNeighbour->FCost || !inList)
			{
				currentNeighbour->GCost = currentGCost;
				currentNeighbour->HCost = currentHCost;
				currentNeighbour->FCost = currentFCost;
				currentNeighbour->CameFrom = currentNode;
				currentNeighbour->Type = currentNeighbour != endNode ? Node_Type::NODE_NEIGHBOUR : endNode->Type;
				if (!inList)
					openList.push_back(currentNeighbour);
			}
		}
	}
}

int main(int argc, char* argv[])
{
	Node map[map_size_x][map_size_y];
	for (int i = 0; i < map_size_x; ++i)
	{
		for (int j = 0; j < map_size_y; ++j)
		{
			map[i][j].x = i;
			map[i][j].y = j;
			map[i][j].Type = Node_Type::NODE_EMPTY;
		}
	}

	for (int i = 0; i < map_size_x / 2; ++i)
		map[map_size_x / 4 + i][map_size_y / 2].Type = Node_Type::NODE_WALL;

	Node* start = &map[map_size_x / 2][0];
	start->Type = Node_Type::NODE_START;
	Node* end = &map[map_size_x / 2][map_size_y / 2 + 1];
	end->Type = Node_Type::NODE_END;

	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Window* window = SDL_CreateWindow("A* Pathfinding Demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, 0);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	bool run = true;
	while (run)
	{
		int mouse_x = 0, mouse_y = 0;
		SDL_GetMouseState(&mouse_x, &mouse_y);
		g_Mouse.X = mouse_x;
		g_Mouse.Y = mouse_y;
		int index_x = mouse_x / node_block_size;
		int index_y = mouse_y / node_block_size;
		if (g_Mouse.Pressed)
		{
			switch (g_Mouse.Button)
			{
				case SDL_BUTTON_LEFT:
					map[index_x][index_y].Type = Node_Type::NODE_WALL;
					break;
				case SDL_BUTTON_RIGHT:
					map[index_x][index_y].Type = Node_Type::NODE_EMPTY;
					break;
				default:
					break;
			}
		}
		DrawMap(map, renderer);

		SDL_Event sdl_event;
		while (SDL_PollEvent(&sdl_event) > 0)
		{
			switch (sdl_event.type)
			{
				case SDL_QUIT:
					run = false;
					break;
				case SDL_MOUSEBUTTONDOWN:
					g_Mouse.Pressed = true;
					g_Mouse.Button = sdl_event.button.button;
					break;
				case SDL_MOUSEBUTTONUP:
					g_Mouse.Pressed = false;
					g_Mouse.Button = 0;
					break;
				case SDL_KEYUP:
					switch (sdl_event.key.keysym.sym)
					{
						case SDLK_RETURN:
							FindAStarPath(map, start, end, renderer);
							break;
						case SDLK_ESCAPE:
							for (int i = 0; i < map_size_x; ++i)
								for (int j = 0; j < map_size_y; ++j)
									if (map[i][j].Type != Node_Type::NODE_END && map[i][j].Type != Node_Type::NODE_START)
										map[i][j].Type = Node_Type::NODE_EMPTY;
							break;
						default:
							break;
					}
					break;
				case SDL_WINDOWEVENT:
					switch (sdl_event.window.event)
					{
						case SDL_WINDOWEVENT_CLOSE:
							run = false;
							break;
						default:
							break;
					}
					break;
				default:
					break;
			}		
		}
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}