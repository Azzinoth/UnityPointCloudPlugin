#pragma once

#include "loadManager.h"
#include <stack>

struct action
{
	std::string type = "none";
	pointCloud* affectedPointCloud = nullptr;
	action() {};
};

struct deleteAction : public action
{
	glm::vec3 center;
	float radius;

	deleteAction() {};
	deleteAction(glm::vec3 center, float radius, pointCloud* AffectedPointCloud)
	{
		this->type = "deleteAction";
		this->center = center;
		this->radius = radius;
		this->affectedPointCloud = AffectedPointCloud;
	};
};

struct deleteOutliersAction : public action
{
	std::vector<int> outliersIndexes;

	deleteOutliersAction() {};
	deleteOutliersAction(std::vector<int> OutliersIndexes, pointCloud* AffectedPointCloud)
	{
		this->type = "deleteOutliersAction";
		this->outliersIndexes = OutliersIndexes;
		this->affectedPointCloud = AffectedPointCloud;
	};
};

class undoManager
{
public:
	SINGLETON_PUBLIC_PART(undoManager)

	void addAction(action* newAction);
	void undo(int actionsToUndo = 1);
	void clear(pointCloud* PointCloud);
	void clear();

	bool undoActionWasApplied = false;
private:
	SINGLETON_PRIVATE_PART(undoManager)

	void undoInternal(action* actionToUndo, std::vector<MeshVertex>& originalData);
	std::vector<action*> undoActions;
};

#define UNDO_MANAGER undoManager::getInstance()