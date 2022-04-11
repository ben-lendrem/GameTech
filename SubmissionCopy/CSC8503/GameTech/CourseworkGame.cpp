#include "CourseworkGame.h"
#include "../CSC8503Common/GameWorld.h"
#include "../CSC8503Common/Constraint.h"
#include "../CSC8503Common/PushdownMachine.h"
#include "../CSC8503Common/PushdownState.h"
#include "../../Plugins/OpenGLRendering/OGLMesh.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"
#include "../../Plugins/OpenGLRendering/OGLTexture.h"
#include "../../Common/TextureLoader.h"
#include "../CSC8503Common/Bonus.h"
#include "../CSC8503Common/StateGameObject.h"
#include "../CSC8503Common/StateMachine.h"
#include "../CSC8503Common/StateTransition.h"

#include <string>

using namespace NCL;
using namespace CSC8503;

/*
World IDs:
1: Player
2: Floors (and walls)
3: Bonus
4: SteelSphere
5: RubberSphere
6: State-based sphere
7: Opponent
*/


CourseworkGame::CourseworkGame() {
	world = new GameWorld();
	renderer = new GameTechRenderer(*world);
	physics = new PhysicsSystem(*world);
	machine = new StateMachine();
	grid = new DiagonalNavigationGrid("courseworkGrid.txt");
	Debug::SetRenderer(renderer);
	InitialiseAssets();
}

CourseworkGame::~CourseworkGame() {
	delete cubeMesh;
	delete sphereMesh;
	delete charMeshA;
	delete charMeshB;
	delete enemyMesh;
	delete bonusMesh;
	delete PlayerCharacter;

	delete basicTex;
	delete iceTex;
	delete basicShader;
	delete grid;
	delete machine;
	delete physics;
	delete renderer;
	delete world;
	while (!oppAll.empty()) {
		oppAll.erase(oppAll.begin());
	}
}

void CourseworkGame::StartGame() {
	score = 1000;
	data = 0;
	gameTimer = 1.0f;
	endTimer = 20.0f;
	gameRunning = true;
	gameEnded = false;
	gameWon = false;

	inSelectionMode = false;
}

void CourseworkGame::EndGame(float dt) {
	endTimer -= dt;
}


void CourseworkGame::UpdateGame(float dt) {
	DebugWinLoss();
	if (debugCamera) {
		world->GetMainCamera()->UpdateCamera(dt);
	}
	else {
		UpdatePlayerCamera();
	}
	if (machine) {
		machine->Update(dt);
	}
	CheckDebugToggles();
	SelectDebugObject();
	DisplayDebugInfo();
	UpdateScore(dt);
	UpdatePlayer(dt);
	if (!oppAll.empty()) {
		for (int i = 0; i < oppAll.size(); i++) {
			oppAll.at(i)->Update(dt);
		}
	}
	if (gameEnded) {
		EndGame(dt);
	}
	//update physics
	physics->Update(dt);
	//update gameworld
	world->UpdateWorld(dt);
	//update renderer
	renderer->Update(dt);
	//draw strings
	DrawStrings();
	//flush renderables
	Debug::FlushRenderables(dt);
	//render
	renderer->Render();
}

void CourseworkGame::InitialiseAssets() {
	auto loadFunc = [](const string& name, OGLMesh** into) {
		*into = new OGLMesh(name);
		(*into)->SetPrimitiveType(GeometryPrimitive::Triangles);
		(*into)->UploadToGPU();
	};

	loadFunc("cube.msh", &cubeMesh);
	loadFunc("sphere.msh", &sphereMesh);
	loadFunc("Male1.msh", &charMeshA);
	loadFunc("courier.msh", &charMeshB);
	loadFunc("security.msh", &enemyMesh);
	loadFunc("coin.msh", &bonusMesh);
	loadFunc("capsule.msh", &capsuleMesh);

	basicTex = (OGLTexture*)TextureLoader::LoadAPITexture("checkerboard.png");
	iceTex = (OGLTexture*)TextureLoader::LoadAPITexture("Snow.png"); //DOESN'T WORK
	basicShader = new OGLShader("GameTechVert.glsl", "GameTechFrag.glsl");
	InitCamera();
	InitWorld();

}

void CourseworkGame::InitCamera() {
	world->GetMainCamera()->SetNearPlane(0.1f);
	world->GetMainCamera()->SetFarPlane(500.0f);
	world->GetMainCamera()->SetPitch(-15.0f);
	world->GetMainCamera()->SetYaw(315.0f);
	world->GetMainCamera()->SetPosition(Vector3(-60, 40, 60));
}

void CourseworkGame::InitWorld() {
	world->ClearAndErase();
	physics->Clear();

	//Initialise all meshes and stuff
	InitSurfaces();
	InitCharacters();
	InitObjects();
	InitStateMachine();
	InitStateObjects();
}

void CourseworkGame::InitSurfaces() {
	//starter floors
	AddFloorToWorld(Vector3(75, -2, 0), Vector3(75,2,50));

	AddFloorToWorld(Vector3(185, -2, 30), Vector3(35, 2, 20));
	AddFloorToWorld(Vector3(185, -2, -30), Vector3(35, 2, 20));

	//special floor types
	GameObject* tarFloor = AddFloorToWorld(Vector3(240, -2, -30), Vector3(20, 2, 20));
	GameObject* iceFloor = AddFloorToWorld(Vector3(240, -2, 30), Vector3(20, 2, 20));
	tarFloor->GetRenderObject()->SetColour(Vector4(0.2,0.2,0.2,1));
	iceFloor->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));

	//maze floor
	AddFloorToWorld(Vector3(370, -2, 0), Vector3(110, 2, 80));
	//maze opening
	AddFloorToWorld(Vector3(275, 5, -50), Vector3(5, 5, 30));
	AddFloorToWorld(Vector3(275, 5, 50), Vector3(5, 5, 30));
	//maze wiggle walls
	AddFloorToWorld(Vector3(310, 5, -35), Vector3(30, 5, 5));
	AddFloorToWorld(Vector3(310, 5, 25), Vector3(30, 5, 5));
	AddFloorToWorld(Vector3(335, 5, 55), Vector3(35, 5, 5));
	//maze middle wall
	AddFloorToWorld(Vector3(365, 5, 0), Vector3(5, 5, 50));
	//maze middle opening
	AddFloorToWorld(Vector3(395, 5, -50), Vector3(5, 5, 30));
	AddFloorToWorld(Vector3(395, 5, 50), Vector3(5, 5, 30));
	//maze end wall
	AddFloorToWorld(Vector3(430,5,0), Vector3(10,5,50));
	//maze end opening
	AddFloorToWorld(Vector3(465, 5, 50), Vector3(5, 5, 30));
	AddFloorToWorld(Vector3(465, 5, -50), Vector3(5, 5, 30));

	//final stretch
	AddFloorToWorld(Vector3(555, -2, 0), Vector3(75, 2, 10));

	//end platform
	AddFloorToWorld(Vector3(720, -2, 0), Vector3(90, 2, 80));

	//finishline (cheating by making finish posts with floors)
	AddFloorToWorld(Vector3(680, 20, 60), Vector3(10, 20, 10));
	AddFloorToWorld(Vector3(680, 20, -60), Vector3(10, 20, 10));
}

void CourseworkGame::InitCharacters() {
	GameObject* player = AddPlayerToWorld(Vector3(5, 5, 0));

	//messing around with orientation
	Quaternion startOrientation =
		Quaternion(Vector3(0, 1, 0), -1);
	player->GetTransform().SetOrientation(startOrientation);

	PlayerCharacter = player;
	InitAI();
}

void CourseworkGame::InitObjects() {
	AddBonusToWorld(Vector3(100,3,30));
	AddBonusToWorld(Vector3(100, 3, -30));
	AddBonusToWorld(Vector3(130, 3, -30));
	AddBonusToWorld(Vector3(160, 3, -30));
	AddBonusToWorld(Vector3(310, 3, -62));
	AddBonusToWorld(Vector3(310, 3, -72));
	AddBonusToWorld(Vector3(310, 3, 62));
	AddBonusToWorld(Vector3(310, 3, 72));
	AddBonusToWorld(Vector3(330, 3, 62));
	AddBonusToWorld(Vector3(330, 3, 72));
	AddSteelSphereToWorld(Vector3(50,5,-20), 3.5f);
	AddSteelSphereToWorld(Vector3(50, 5, 0), 3.5f);
	AddSteelSphereToWorld(Vector3(50, 5, 20), 3.5f);
	AddRubberSphereToWorld(Vector3(50, 5, -10), 3.5f);
	AddRubberSphereToWorld(Vector3(50, 5, 10), 3.5f);
}

GameObject* CourseworkGame::AddFloorToWorld(const Vector3& pos,
	const Vector3& size, const Quaternion& rot) {
	//modified tutorial function to allow for custom size floors
	GameObject* floor = new GameObject();
	AABBVolume* volume = new AABBVolume(size);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(size * 2)
		.SetPosition(pos)
		.SetOrientation(rot);
	floor->SetRenderObject(new RenderObject(
		&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(
		&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();
	floor->GetPhysicsObject()->SetElasticity(0.5f);
	floor->SetWorldID(2);

	world->AddGameObject(floor);
	return floor;
}

GameObject* CourseworkGame::AddPlayerToWorld(const Vector3& pos) {
	float meshSize = 3.0f;
	float inverseMass = 0.5f;

	GameObject* character = new GameObject();

	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.85f, 0.3f) * meshSize);

	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(pos);

	if (rand() % 2) {
		character->SetRenderObject(new RenderObject(
			&character->GetTransform(), charMeshA, nullptr, basicShader));
	}
	else {
		character->SetRenderObject(new RenderObject(
			&character->GetTransform(), charMeshB, nullptr, basicShader));
	}
	character->SetPhysicsObject(new PhysicsObject(
		&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();
	character->SetWorldID(1);
	world->AddGameObject(character);

	return character;
}

GameObject* CourseworkGame::AddBonusToWorld(const Vector3& position) {
	Bonus* apple = new Bonus(this);
	SphereVolume* volume = new SphereVolume(0.25f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(Vector3(0.25, 0.25, 0.25))
		.SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), bonusMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(0.0f);
	apple->GetPhysicsObject()->InitSphereInertia();
	apple->SetWorldID(3);
	world->AddGameObject(apple);

	return apple;
}

void CourseworkGame::DrawStrings() {
	
	std::string currentScore = std::to_string(score);
	if (!gameEnded) {
		std::string debugCameraOn;
		switch (debugCamera) {
		case true:
			debugCameraOn = "true";
			break;
		case false:
			debugCameraOn = "false";
		}
		renderer->DrawString("Debug camera: " + debugCameraOn, Vector2(64, 3),
			textColour);
		renderer->DrawString("Press Q to toggle debug camera", Vector2(56, 7),
			textColour);
		std::string debugSelectionOn;
		switch (inSelectionMode) {
		case true:
			debugSelectionOn = "true";
			break;
		case false:
			debugSelectionOn = "false";
		}
		renderer->DrawString("Debug Selection: " + debugSelectionOn, Vector2(58, 93),
			textColour);
		renderer->DrawString("Press L to toggle debug select", Vector2(43, 97),
			textColour);

		renderer->DrawString(("Score: " + currentScore), Vector2(5, 5),
			textColour);
	}
	else {
		std::string s;
		switch (gameWon) {
		case true:
			 s = "won";
			break;
		case false:
			s = "lost";
		}
		std::string endTime = std::to_string((int)endTimer);
		std::string endText1 = "You " + s + "!";
		std::string endText2 = "Your final score was " + currentScore;
		std::string endText3 = "The game will close in " + endTime + " seconds";
		renderer->DrawString(endText1, Vector2(42, 30), textColour);
		renderer->DrawString(endText2, Vector2(33, 37), textColour);
		renderer->DrawString(endText3, Vector2(25, 44), textColour);
	}
	
}

void CourseworkGame::UpdateScore(float dt) {
	if (gameRunning && !gameEnded) {
		gameTimer -= dt;
		if (gameTimer <= 0.0f) {
			score -= 10;
			gameTimer += 1.0f;
		}
		if (score <= 0) {
			gameEnded = true;
		}
	}
}

void CourseworkGame::UpdatePlayer(float dt) {
	if (PlayerCharacter->IsActive() && GameIsActive()) {
		UpdatePlayerKeys(dt);
	}
	//cheatsy method for checking if player has crossed the finish line
	Vector3 playerPos = PlayerCharacter->GetTransform().GetPosition();
	if (playerPos.x >= 670 &&
		playerPos.y >= -1 &&
		playerPos.z >= -50 && playerPos.z <= 50) {
		gameEnded = true;
		gameRunning = false;
		gameWon = true;
	}
	if (playerPos.y < -40 || score <= 0) {
		gameEnded = true;
		gameRunning = false;
		gameWon = false;
	}
}

bool CourseworkGame::GameIsActive() {
	return (gameRunning && !gameEnded);
}

void CourseworkGame::UpdatePlayerKeys(float dt) {
	/*
	Player movement needs to be updated so that left/right is 
	done based on camera position for now simple linear forces
	*/
	Vector3 playerPos = PlayerCharacter->GetTransform()
		.GetPosition();
	Vector3 playerRot = PlayerCharacter->GetTransform()
		.GetOrientation().ToEuler().Normalised();
	Vector3 playerXZ = Vector3(playerRot.x, 0, playerRot.z).Normalised();
	
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
		PlayerCharacter->GetPhysicsObject()->AddForce(
			Vector3(1, 0, 0) * pMoveForce * dt
		);
	}
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
		PlayerCharacter->GetPhysicsObject()->AddForce(
			Vector3(-1, 0, 0) * pMoveForce * dt
		);
	}
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
		PlayerCharacter->GetPhysicsObject()->AddForce(
			Vector3(0, 0, -1) * pMoveForce * dt
		);
	}
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
		PlayerCharacter->GetPhysicsObject()->AddForce(
			Vector3(0, 0, 1) * pMoveForce * dt
		);
	}
}

void CourseworkGame::UpdatePlayerCamera() {
	Vector3 playerPos = PlayerCharacter->GetTransform().GetPosition();
	/*
	camPos needs to be updated to be based on player orientation.
	For now simple offset
	*/
	Vector3 camPos = playerPos + camOffset;
	Ray playerToIdealCam = Ray(playerPos, camOffset);

	RayCollision closestCollision;
	if (world->Raycast(playerToIdealCam, closestCollision, true) && ((GameObject*)closestCollision.node)->GetWorldID() == 2) {
		camPos = closestCollision.collidedAt -
			(playerToIdealCam.GetDirection() * 0.1);
	}
	Matrix4 temp = Matrix4::BuildViewMatrix(camPos, playerPos, Vector3(0, 1, 0));
	Matrix4 modelMat = temp.Inverse();
	Quaternion q(modelMat);
	Vector3 angles = q.ToEuler();

	world->GetMainCamera()->SetPosition(camPos);
	world->GetMainCamera()->SetPitch(angles.x);
	world->GetMainCamera()->SetYaw(angles.y);
	
}

void CourseworkGame::DebugWinLoss() {
	
	//win
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NUM1)) {
		gameEnded = true;
		gameRunning = false;
		gameWon = true;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NUM2)) {
		gameEnded = true;
		gameRunning = false;
		gameWon = false;
	}
}

void CourseworkGame::CheckDebugToggles() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::Q)) {
		debugCamera = !debugCamera;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::L)) {
		inSelectionMode = !inSelectionMode;
	}
	if (inSelectionMode) {
		Window::GetWindow()->ShowOSPointer(true);
		Window::GetWindow()->LockMouseToWindow(false);
	}
	else {
		Window::GetWindow()->ShowOSPointer(false);
		Window::GetWindow()->LockMouseToWindow(true);
	}
}

bool CourseworkGame::SelectDebugObject() {
	if (inSelectionMode) {
		if (Window::GetMouse()->ButtonDown(MouseButtons::LEFT)) {
			if (selectionObject) {
				selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
				selectionObject = nullptr;
			}

			Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());

			RayCollision closestCollision;
			if (world->Raycast(ray, closestCollision, true)) {
				selectionObject = (GameObject*)closestCollision.node;
				selectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
				return true;
			}
			else {
				return false;
			}
		}
	}
}

void CourseworkGame::DisplayDebugInfo() {
	if (selectionObject) {
		Vector3 selectedPos = selectionObject->GetTransform().GetPosition();
		std::string posX = std::to_string(selectedPos.x);
		std::string posY = std::to_string(selectedPos.y);
		std::string posZ = std::to_string(selectedPos.z);
		std::string isActive;
		switch (selectionObject->IsActive()) {
		case true: isActive = "true"; break;
		case false: isActive = "false";
		}
		std::string selectedID = std::to_string(selectionObject->GetWorldID());
		std::string selectedName = selectionObject->GetName();
		std::string debugText1 = "Object name: " + selectedName;
		std::string debugText2 = "Object id: " + selectedID;
		std::string debugText3 = "Object position: (" + posX +
			", " + posY + ", " + posZ + ")";
		std::string debugText4 = "Object active: " + isActive;
		renderer->DrawString(debugText1, Vector2(3, 60), textColour);
		renderer->DrawString(debugText2, Vector2(3, 64), textColour);
		renderer->DrawString(debugText3, Vector2(3, 68), textColour);
		renderer->DrawString(debugText4, Vector2(3, 72), textColour);
	}
	
}

GameObject* CourseworkGame::AddSteelSphereToWorld(const Vector3& position,
	float radius) {
	GameObject* sphere = new GameObject();

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);

	sphere->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(0.1f);
	sphere->GetPhysicsObject()->SetElasticity(0.3f);
	sphere->GetPhysicsObject()->InitSphereInertia();
	sphere->GetRenderObject()->SetColour(Vector4(0.5, 0.5, 0.52, 1));
	sphere->SetWorldID(4);
	world->AddGameObject(sphere);

	return sphere;
}

GameObject* CourseworkGame::AddRubberSphereToWorld(const Vector3& position,
	float radius) {
	GameObject* sphere = new GameObject();

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);

	sphere->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(5.0f);
	sphere->GetPhysicsObject()->SetElasticity(0.9f);
	sphere->GetPhysicsObject()->InitSphereInertia();
	sphere->GetRenderObject()->SetColour(Vector4(0.8, 0.73, 0.45, 1));
	sphere->SetWorldID(5);
	world->AddGameObject(sphere);

	return sphere;
}

void CourseworkGame::InitStateMachine() {
	State* A = new State([&](float dt)->void
		{
			data++;
		});
	State* B = new State([&](float dt)->void
		{
			data--;
		});
	StateTransition* stateAB = new StateTransition(A, B, [&](void)->bool
		{
			return data > 10;
		});
	StateTransition* stateBA = new StateTransition(B, A, [&](void)->bool
		{
			return data < 0;
		});
	machine->AddState(A);
	machine->AddState(B);
	machine->AddTransition(stateAB);
	machine->AddTransition(stateBA);
}

void CourseworkGame::InitStateObjects() {
	StateGameObject* steelSphere = new StateGameObject();
	SphereVolume* volume = new SphereVolume(3.5f);
	steelSphere->SetBoundingVolume((CollisionVolume*)volume);
	steelSphere->GetTransform()
		.SetScale(Vector3(3.5f, 3.5f, 3.5f))
		.SetPosition(Vector3(540, 5, 10));
	steelSphere->SetRenderObject(new RenderObject(&steelSphere->GetTransform(),
		sphereMesh, basicTex, basicShader));
	steelSphere->SetPhysicsObject(new PhysicsObject(&steelSphere->GetTransform(),
		steelSphere->GetBoundingVolume()));
	steelSphere->GetPhysicsObject()->SetInverseMass(0.1f);
	steelSphere->GetPhysicsObject()->SetElasticity(0.3f);
	steelSphere->GetPhysicsObject()->InitSphereInertia();
	steelSphere->GetRenderObject()->SetColour(Vector4(0.5, 0.5, 0.52, 1));
	steelSphere->SetWorldID(6);
	world->AddGameObject(steelSphere);
}

static void AddAIToGame(Vector3 startPos, GameObject* pc, DiagonalNavigationGrid* gridIn,
	GameWorld* worldIn, OGLMesh* enemyMeshIn, OGLShader* basicShaderIn, std::vector<Opponent*>* vecIn) {
	float meshSize = 3.0f;
	float inverseMass = 0.5f + (((double) rand() / (RAND_MAX + 1))/50.0f);
	Opponent* oppCurrent = new Opponent(pc, gridIn, worldIn, startPos);
	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.85f, 0.3f) * meshSize);
	oppCurrent->SetBoundingVolume((CollisionVolume*)volume);

	oppCurrent->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize));
	oppCurrent->SetPhysicsObject(new PhysicsObject(
		&oppCurrent->GetTransform(), oppCurrent->GetBoundingVolume()));

	oppCurrent->SetRenderObject(new RenderObject(
		&oppCurrent->GetTransform(), enemyMeshIn, nullptr, basicShaderIn));

	Quaternion startOrientation =
		Quaternion(Vector3(0, 1, 0), -1);
	oppCurrent->GetTransform().SetOrientation(startOrientation);

	oppCurrent->GetRenderObject()->SetColour(Vector4(1,0.3,0.3,1));
	oppCurrent->SetWorldID(7);
	worldIn->AddGameObject(oppCurrent);
	vecIn->emplace_back(oppCurrent);
}


void CourseworkGame::InitAI() {
	if (PlayerCharacter && grid && world && enemyMesh && basicShader) {
		AddAIToGame(Vector3(5, 5, -30), PlayerCharacter, grid, world, enemyMesh, basicShader, &oppAll);
		AddAIToGame(Vector3(5, 5, -20), PlayerCharacter, grid, world, enemyMesh, basicShader, &oppAll);
		AddAIToGame(Vector3(5, 5, -10), PlayerCharacter, grid, world, enemyMesh, basicShader, &oppAll);
		AddAIToGame(Vector3(5, 5, 10), PlayerCharacter, grid, world, enemyMesh, basicShader, &oppAll);
		AddAIToGame(Vector3(5, 5, 20), PlayerCharacter, grid, world, enemyMesh, basicShader, &oppAll);
		AddAIToGame(Vector3(5, 5, 30), PlayerCharacter, grid, world, enemyMesh, basicShader, &oppAll);
	}
}


