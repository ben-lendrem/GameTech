#pragma once
#include "GameTechRenderer.h"
#include "../CSC8503Common/PhysicsSystem.h"
#include "../CSC8503Common/DiagonalNavigationGrid.h"
#include "../CSC8503Common/Opponent.h"

namespace NCL {
	namespace CSC8503 {
		class StateMachine;
		class Bonus;
		class CourseworkGame {
		public:
			CourseworkGame();
			~CourseworkGame();
			void StartGame();
			void UpdateGame(float dt);
			/*
			suddenly realised that I needed a way to increment score from
			bonus gameobjects OnCollisionBegin method so I've crammed this in
			not sure how else to handle this
			*/
			void OnBonusCollect() {
				score += 25;
			}
			float GetEndTimer() { return endTimer; }

			bool isRunning() { return gameRunning; }

			void InitAI();
		protected:
			//UpdateKeys - updates which keys are being pressed and makes changes
			GameTechRenderer* renderer;
			PhysicsSystem* physics;
			GameWorld* world;
			DiagonalNavigationGrid* grid;

			//GameObject pointer for debug selection
			GameObject* selectionObject = nullptr;

			//Meshes to be loaded in at initialization (from tutorial)
			OGLMesh* capsuleMesh = nullptr;
			OGLMesh* cubeMesh = nullptr;
			OGLMesh* sphereMesh = nullptr;
			OGLTexture* basicTex = nullptr;
			OGLTexture* iceTex = nullptr;
			OGLShader* basicShader = nullptr;

			OGLMesh* charMeshA = nullptr;
			OGLMesh* charMeshB = nullptr;
			OGLMesh* enemyMesh = nullptr;
			OGLMesh* bonusMesh = nullptr;

			void InitialiseAssets();
			void InitWorld();
			void InitSurfaces();
			void InitCharacters();
			void InitCamera();
			void InitObjects();
			void InitStateObjects();
			void InitStateMachine();

			void UpdateScore(float dt);
			void DrawStrings();
			/*
			* no subclass for player as there is only one instance of the player
			* and there is no need to override OnCollisionBegin/End (important
			* collisions such as with the finish line are handled through testing
			* if the object the finish line collides with is the player 
			* (i.e. if collidedObject == PlayerCharacter))
			*/
			void UpdatePlayer(float dt);
			void UpdatePlayerCamera();
			bool GameIsActive();
			void UpdatePlayerKeys(float dt);

			GameObject* AddFloorToWorld(const Vector3& pos,
				const Vector3& size, const Quaternion& rot = Quaternion());
			GameObject* AddPlayerToWorld(const Vector3& pos);
			GameObject* AddBonusToWorld(const Vector3& position);
			GameObject* AddSteelSphereToWorld(const Vector3& position, float radius);
			GameObject* AddRubberSphereToWorld(const Vector3& position, float radius);

			GameObject* PlayerCharacter;
			bool debugCamera = false;
			bool inSelectionMode = false;
			bool SelectDebugObject();
			void DebugWinLoss();
			void CheckDebugToggles();
			void DisplayDebugInfo();

			bool gameRunning = true;
			bool gameEnded = false;
			bool gameWon = false;
			float gameTimer = 1.0f;
			float endTimer = 20.0f;
			int score = 1000;
			float pMoveForce = 4300.0f;

			Vector3 camOffset = Vector3(-30,8,0);
			Vector4 textColour = Vector4(0, 1, 0, 1);
			StateMachine* machine;
			void EndGame(float dt);
			int data;

			std::vector<Opponent*> oppAll;

		};
	}
}