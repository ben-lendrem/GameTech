#pragma once
#include <stack>

namespace NCL {
	namespace CSC8503 {
		class PushdownState;

		class PushdownMachine {
		public:
			PushdownMachine(PushdownState* initState) {
				initialState = initState;
				activeState = nullptr;
			}
			~PushdownMachine() {}

			bool Update(float dt);
		protected:
			PushdownState* activeState;
			PushdownState* initialState;

			std::stack<PushdownState*> stateStack;
		};
	}
}