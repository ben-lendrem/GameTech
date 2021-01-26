#include "StateGameObject.h"
#include "../CSC8503Common/StateTransition.h"
#include "../CSC8503Common/StateMachine.h"
#include "../CSC8503Common/State.h"

using namespace NCL;
using namespace CSC8503;

StateGameObject::StateGameObject() {
	counter = 0.0f;
	stateMachine = new StateMachine();

	State* stateA = new State([&](float dt)->void
		{
			this->MoveLeft(dt);
		});
	State* stateB = new State([&](float dt)->void
		{
			this->MoveRight(dt);
		});
	stateMachine->AddState(stateA);
	stateMachine->AddState(stateB);

	StateTransition* transitionAB = new StateTransition(
		stateA, stateB, [&]()->bool {
			return this->counter > 3.0f;
		});
	StateTransition* transitionBA = new StateTransition(
		stateB, stateA, [&]()->bool {
			return this->counter < 0.0f;
		});
	stateMachine->AddTransition(transitionAB);
	stateMachine->AddTransition(transitionBA);
}

StateGameObject::~StateGameObject() {
	delete stateMachine;
}

void StateGameObject::Update(float dt) {
	stateMachine->Update(dt);
}

void StateGameObject::MoveLeft(float dt) {
	GetPhysicsObject()->AddForce({ -100, 0, 0 });
	counter += dt;
}

void StateGameObject::MoveRight(float dt) {
	GetPhysicsObject()->AddForce({ 100, 0, 0 });
	counter -= dt;
}