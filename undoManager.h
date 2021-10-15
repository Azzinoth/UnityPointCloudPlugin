#pragma once

#include "loadManager.h"
#include <stack>

//static ID3D11Device* m_Device = nullptr;

struct deleteAction
{
	glm::vec3 center;
	float radius;

	deleteAction() {};
	deleteAction(glm::vec3 center, float radius)
	{
		this->center = center;
		this->radius = radius;
	};
};

class undoManager
{
public:
	SINGLETON_PUBLIC_PART(undoManager)

	void setPointCloud(pointCloud* currentPointCloud);
	void addDeleteAction(glm::vec3 center, float radius);

	void undo(int actionsToUndo = 1);
	void clear();
private:
	SINGLETON_PRIVATE_PART(undoManager)

	pointCloud* currentPointCloud = nullptr;
	//std::stack<deleteAction> undoActions;
	std::vector<deleteAction> undoActions;
};

#define UNDO_MANAGER undoManager::getInstance()