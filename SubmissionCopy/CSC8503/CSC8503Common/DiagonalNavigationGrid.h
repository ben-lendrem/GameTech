#pragma once
#include "NavigationMap.h"
#include <string>

/*
This class is functionally very similar to the NavigationGrid class, with a few differences:
1. Heuristic is calculated with Chebyshev distance instead of Manhatten distance
2. The inputted file has costs that aren't just 1 or a wall
*/


namespace NCL {
	namespace CSC8503 {
		
		struct DiagonalNode {
			DiagonalNode* parent;

			DiagonalNode* connected[8];
			int costs[8];

			Vector3 position;

			float f;
			float g;

			int type;

			DiagonalNode() {
				for (int i = 0; i < 8; ++i) {
					connected[i] = nullptr;
					costs[i] = 0;
				}
				f = 0;
				g = 0;
				type = 0;
				parent = nullptr;
			}
			~DiagonalNode() {}
		};
		typedef std::vector<DiagonalNode*> DNodeVector;

		class DiagonalNavigationGrid : public NavigationMap {
		public:
			DiagonalNavigationGrid();
			DiagonalNavigationGrid(const std::string& filename);
			~DiagonalNavigationGrid();

			bool FindPath(const Vector3& from, const Vector3& to, NavigationPath& outPath) override;

		protected:
			bool NodeInList(DiagonalNode* n, DNodeVector& list) const;
			DiagonalNode* RemoveBestNode(DNodeVector& list) const;
			float Heuristic(DiagonalNode* hNode, DiagonalNode* endNode) const;
			int nodeSize;
			int gridWidth;
			int gridHeight;

			DiagonalNode* allNodes;
		};
	}
}