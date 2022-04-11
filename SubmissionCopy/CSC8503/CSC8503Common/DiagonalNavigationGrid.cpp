#include "DiagonalNavigationGrid.h"
#include "../../Common/Assets.h"

#include <fstream>

using namespace NCL;
using namespace CSC8503;

const char WALL_NODE = 'X';
const char FLOOR_NODE = '0';
const char TAR_NODE = '1';
const char ICE_NODE = '2';

const int TOP = 0;
const int TOP_RIGHT = 1;
const int RIGHT = 2;
const int BOTTOM_RIGHT = 3;
const int BOTTOM = 4;
const int BOTTOM_LEFT = 5;
const int LEFT = 6;
const int TOP_LEFT = 7;

DiagonalNavigationGrid::DiagonalNavigationGrid() {
	nodeSize = 0;
	gridWidth = 0;
	gridHeight = 0;
	allNodes = nullptr;
}

DiagonalNavigationGrid::DiagonalNavigationGrid(const std::string& filename)
	: DiagonalNavigationGrid() {
	std::ifstream infile(Assets::DATADIR + filename);

	infile >> nodeSize;
	infile >> gridWidth;
	infile >> gridHeight;

	allNodes = new DiagonalNode[gridWidth * gridHeight];

	for (int x = 0; x < gridHeight; ++x) {
		for (int z = 0; z < gridWidth; ++z) {
			DiagonalNode& n = allNodes[(gridWidth * x) + z];
			char type = 0;
			infile >> type;
			n.type = type;
			/*see comment 1 below*/
			int xPosMult = gridHeight - x;
			int zPosMult = z - (gridWidth/2); //over 2 because centred on z = 0
			n.position = Vector3(((xPosMult * nodeSize) - 5), 0, ((zPosMult * nodeSize) + 5));
		}
	}

	for (int x = 0; x < gridHeight; ++x) {
		for (int z = 0; z < gridWidth; ++z) {
			DiagonalNode&n = allNodes[(gridWidth * x) + z];

			if (x > 0) {
				n.connected[TOP] = &allNodes[(gridWidth * (x - 1)) + z];
				if (z > 0) {
					n.connected[TOP_LEFT] = &allNodes[(gridWidth * (x - 1)) + (z - 1)];
				}
				if (z < gridWidth - 1) {
					n.connected[TOP_RIGHT] = &allNodes[(gridWidth * (x - 1)) + (z + 1)];
				}
			}
			if (z > 0) {
				n.connected[LEFT] = &allNodes[(gridWidth * x) + (z - 1)];
			}
			if (z < gridWidth - 1) {
				n.connected[RIGHT] = &allNodes[(gridWidth * x) + (z + 1)];
			}
			if (x < gridHeight - 1) {
				n.connected[BOTTOM] = &allNodes[(gridWidth * (x + 1)) + z];
				if (z > 0) {
					n.connected[BOTTOM_LEFT] = &allNodes[(gridWidth * (x + 1)) + (z - 1)];
				}
				if (z < gridWidth - 1) {
					n.connected[BOTTOM_RIGHT] = &allNodes[(gridWidth * (x + 1)) + (z + 1)];
				}
			}

			for (int i = 0; i < 8; ++i) {
				if (n.connected[i]) {
					switch (n.connected[i]->type) {
					case WALL_NODE: n.connected[i] = nullptr; break;
					case FLOOR_NODE: n.costs[i] = 2; break;
					case TAR_NODE: n.costs[i] = 3; break;
					case ICE_NODE: n.costs[i] = 1; break;
					}
				}
			}
		}
	}
}

DiagonalNavigationGrid::~DiagonalNavigationGrid() {
	delete[] allNodes;
}

bool DiagonalNavigationGrid::FindPath(const Vector3& from,
	const Vector3& to, NavigationPath& outPath) {
	int fromX = (int)((gridHeight - 1) - (from.x / nodeSize));
	int fromZ = (int)(((from.z - 5)/nodeSize) + (gridWidth/2));

	int toX = (int)((gridHeight - 1) - (to.x / nodeSize));
	int toZ = (int)(((to.z - 5) / nodeSize) + (gridWidth / 2));

	if (fromX < 0 || fromX > gridHeight - 1 ||
		fromZ < 0 || fromZ > gridWidth - 1) {
		return false;
	}

	if (toX < 0 || toX > gridHeight - 1 ||
		toZ < 0 || toZ > gridWidth - 1) {
		return false;
	}

	DiagonalNode* startNode = &allNodes[(gridWidth * fromX) + fromZ];
	DiagonalNode* endNode = &allNodes[(gridWidth * toX) + toZ];
	DNodeVector openList;
	DNodeVector closedList;
	openList.emplace_back(startNode);

	startNode->f = 0;
	startNode->g = 0;
	startNode->parent = nullptr;
	DiagonalNode* currentBestNode = nullptr;

	while (!openList.empty()) {
		currentBestNode = RemoveBestNode(openList);

		if (currentBestNode == endNode) {
			DiagonalNode* node = endNode;
			while (node != nullptr) {
				outPath.PushWaypoint(node->position);
				node = node->parent;
			}
			return true;
		}
		else {
			for (int i = 0; i < 8; ++i) {
				DiagonalNode* neighbour = currentBestNode->connected[i];
				if (!neighbour) {
					continue;
				}
				bool inClosed = NodeInList(neighbour, closedList);
				if (inClosed) {
					continue;
				}
				float h = Heuristic(neighbour, endNode);
				float g = currentBestNode->g + currentBestNode->costs[i];
				float f = h + g;

				bool inOpen = NodeInList(neighbour, openList);
				if (!inOpen) {
					openList.emplace_back(neighbour);
				}
				if (!inOpen || f < neighbour->f) {
					neighbour->parent = currentBestNode;
					neighbour->f = f;
					neighbour->g = g;
				}
			}
			closedList.emplace_back(currentBestNode);
		}
	}
	return false;
}

bool DiagonalNavigationGrid::NodeInList(DiagonalNode* n, DNodeVector& list) const {
	DNodeVector::iterator i = std::find(list.begin(), list.end(), n);
	return i == list.end() ? false : true;
}

DiagonalNode* DiagonalNavigationGrid::RemoveBestNode(DNodeVector& list) const {
	DNodeVector::iterator bestI = list.begin();

	DiagonalNode* bestNode = *list.begin();
	for (auto i = list.begin(); i != list.end(); ++i) {
		if ((*i)->f < bestNode->f) {
			bestNode = (*i);
			bestI = i;
		}
	}
	list.erase(bestI);

	return bestNode;
}

float DiagonalNavigationGrid::Heuristic(DiagonalNode* hNode, DiagonalNode* endNode) const {
	/*
	http://theory.stanford.edu/~amitp/GameProgramming/Heuristics.html provides
	a heuristic for diagonal movement
	Since D and D2 are equivalent (Chebyshev distance), just declaring variable D to sub
	for both
	*/
	float dx = abs(endNode->position.x - hNode->position.x);
	float dz = abs(endNode->position.z - hNode->position.z);
	float D = 2.0f;
	return D * (dx + dz) + (D - (2 * D)) * fminf(dx, dz);
}

/* comment 1

iteration starts at x = 0, z = 0, to access node [0,0] as top left node in the structure.
This node needs position of (805, 0, -75) to be accurate to layout within
"courseworkGrid.txt".
xPosMult and zPosMult created to transform the values of the iterator variables into the 
valid number to multiply by.
-5 on the x-coordinate and +5 on the z-coordinate are so the node position is located within
the centre of a 10x10 unit area within the 3D gameworld.
e.g. with node [0 , 0] for x-z position of (805, -75):

(810, -80)	----------------- (810, -70)
			|               |
			|  (805, -75)   |
			|       *       |
			|               |
			|               |
(800, -80)	----------------- (800, -70)


*/