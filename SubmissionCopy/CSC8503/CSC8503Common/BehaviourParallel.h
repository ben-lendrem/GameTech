#pragma once
#include "BehaviourNodeWithChildren.h"
/*
Parallel behaviour node attempts to execute all children nodes regardless of 
the outcome of any one child node
*/
class BehaviourParallel : public BehaviourNodeWithChildren {
public:
	BehaviourParallel(const std::string& nodeName)
		: BehaviourNodeWithChildren(nodeName) {}
	~BehaviourParallel() {}
	BehaviourState Execute(float dt) override {
		for (auto& i : childNodes) {
			i->Execute(dt);
		}
		return Success;
	}
};