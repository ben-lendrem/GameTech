#pragma once
#include "GameObject.h"
#include "BehaviourHeaders.h"
#include "DiagonalNavigationGrid.h"
#include "CollisionDetection.h"
#include "GameWorld.h"
/*
Opponent must:
	@At init:
		- Find a path through a navigation grid that represents the level it is put in
		- have a render and physics object attached to it (done in courseworkgame)
		- set a name (done through calling gameObject constructor)
		- set a worldID (done in courseworkgame)
		- 

	@On Update:
		- Decrease scanTimer by dt
		- if scanTimer < 0:
			: reset scanTimer
			: raycast to try and detect 
	@Have access to:

	@Be provided through the constructor:
		- A pointer to the navigation grid it will use to find a path through

	@Store:
		- scanTimer (float)
*/

namespace NCL {
	namespace CSC8503 {
		class Opponent : public GameObject {
		public:
			Opponent(GameObject* p = nullptr,
				DiagonalNavigationGrid* g = nullptr, GameWorld* w = nullptr,
				Vector3 startPos = Vector3(0, 5, -10));
			~Opponent();
			void Update(float dt) override;
			void SetPlayer(GameObject* p) { Player = p; }
			void SetNavGrid(DiagonalNavigationGrid* g) { grid = g; }
			void SetMoveForce(float forceIn) { moveForce = forceIn; }
		protected:
			float distanceToPlayer() {
				Vector3 toPlayer = Player->GetTransform().GetPosition()
					- transform.GetPosition();
				return toPlayer.Length();
			}
			bool hasFinished() {
				Vector3 pos = transform.GetPosition();
				return (pos.x >= 670 &&
					pos.y >= -1 &&
					pos.z >= -50 && pos.z <= 50);
			}

			RayCollision* scanForBonuses();
			void MoveTowardsWaypoint(float dtInAgain);
			Vector3 VectorToCurrentWaypoint();
			Vector3 VectorToNextWaypoint();
			void InitPath(Vector3 startPosition);
			void HandleMovement(float dtIn);
			void AdvanceWaypoints();

			float moveForce = 3500.0f;
			float scanTimer;
			bool canMove;
			vector<Vector3> waypoints;
			GameObject* Player;
			Vector3 currentWaypoint;
			Vector3 lastWaypoint;
			DiagonalNavigationGrid* grid;
			GameWorld* world;
		};
	}
}