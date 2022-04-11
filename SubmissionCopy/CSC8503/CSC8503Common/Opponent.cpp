#include "Opponent.h"
#include "Ray.h"

using namespace NCL;
using namespace CSC8503;

Opponent::Opponent(GameObject* p, DiagonalNavigationGrid* g,
	GameWorld* w, Vector3 startPos) {
	Player = p;
	grid = g;
	world = w;
	scanTimer = 1.5f;
	canMove = true;
	transform.SetPosition(startPos);
	InitPath(startPos);
}

Opponent::~Opponent() {
}

void Opponent::Update(float dt) {
	if (Player->GetTransform().GetPosition().x - transform.GetPosition().x > 300 || transform.GetPosition().y < -5) {
		transform.SetPosition(lastWaypoint + Vector3(0,5,0));
		physicsObject->SetLinearVelocity(Vector3(0, 0, 0));
		physicsObject->ClearForces();
	}
	if (canMove) {
		HandleMovement(dt);
	}
	if (hasFinished()) {
		canMove = false;
	}
}


void Opponent::InitPath(Vector3 startPosition) {
	NavigationPath path;
	bool found = grid->FindPath(startPosition, Vector3(670, 0, 0), path);
	if (found) {
		Vector3 pos;
		while (path.PopWaypoint(pos)) {
			waypoints.emplace_back(pos);
		}
	}
	AdvanceWaypoints();
}

RayCollision* Opponent::scanForBonuses() {
	for (int i = 0; i < 16; ++i) {
		float degrees = 22.5f * i;
		Quaternion rotation = Quaternion::AxisAngleToQuaterion(Vector3(0, 1, 0), degrees);
		Vector3 rayDirection = rotation * Vector3(15, 0, 0);
		Ray r = Ray(transform.GetPosition(), rayDirection);
		RayCollision* collision = new RayCollision();
		if (world->Raycast(r, *collision, true)) {
			return collision;
		}
	}
	return nullptr;
}

void Opponent::MoveTowardsWaypoint(float dtInAgain) {
	physicsObject->AddForce(VectorToCurrentWaypoint().Normalised() * moveForce * dtInAgain);
}

Vector3 Opponent::VectorToCurrentWaypoint() {
	return (currentWaypoint - transform.GetPosition());
}

void Opponent::HandleMovement(float dtIn) {
	MoveTowardsWaypoint(dtIn);
	if (VectorToCurrentWaypoint().Length() < 10.0f || VectorToNextWaypoint().Length() < VectorToCurrentWaypoint().Length()) {
		AdvanceWaypoints();
	}
}

void Opponent::AdvanceWaypoints() {
	if (!waypoints.empty()) {
		lastWaypoint = currentWaypoint;
		currentWaypoint = waypoints.at(0);
		waypoints.erase(waypoints.begin());
	}
	
}

Vector3 Opponent::VectorToNextWaypoint() {
	if (!waypoints.empty()) {
		return (waypoints.at(0) - transform.GetPosition());
	}
	else {
		return Vector3(0, 0, 0);
	}
}

