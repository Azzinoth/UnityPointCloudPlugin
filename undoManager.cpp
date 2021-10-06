#include "pch.h"
#include "undoManager.h"

undoManager* undoManager::_instance = nullptr;

undoManager::undoManager()
{
}

undoManager::~undoManager()
{
}

static bool firstTime = true;
void undoManager::setPointCloud(pointCloud* currentPointCloud)
{
	if (currentPointCloud == nullptr)
		return;

	if (firstTime)
	{
		this->currentPointCloud = currentPointCloud;
		firstTime = false;
	}
}

void undoManager::addDeleteAction(glm::vec3 center, float radius)
{
	//undoActions.push(deleteAction(center, radius));
	undoActions.push_back(deleteAction(center, radius));
}

void undoManager::undo()
{
	if (currentPointCloud == nullptr || undoActions.size() == 0)
		return;

	LOG.addToLog("==============================================================", "undoActions");
	// Take original data.
	std::vector<MeshVertex> copyOfOriginalData = currentPointCloud->originalData;
	// Apply all actions to it except last one that we want to cancel.
	LOG.addToLog("undoActions.size(): " + std::to_string(undoActions.size()), "undoActions");
	undoActions.pop_back();
	LOG.addToLog("undoActions.size() after pop_back: " + std::to_string(undoActions.size()), "undoActions");
	for (size_t i = 0; i < undoActions.size() /*- 1*/; i++)
	{
		LOG.addToLog("i: " + std::to_string(i), "undoActions");
		LOG.addToLog("Brush location: ", undoActions[i].center, "undoActions");
		LOG.addToLog("Brush size: " + std::to_string(undoActions[i].radius), "undoActions");

		std::vector<int> deletedPoints;
		currentPointCloud->getSearchOctree()->searchForObjects(undoActions[i].center, undoActions[i].radius, deletedPoints);

		LOG.addToLog("pointsToDelete size: " + std::to_string(deletedPoints.size()), "undoActions");

		for (size_t j = 0; j < deletedPoints.size(); j++)
		{
			copyOfOriginalData[deletedPoints[j]].position[0] = DELETED_POINTS_COORDINATE;
			copyOfOriginalData[deletedPoints[j]].position[1] = DELETED_POINTS_COORDINATE;
			copyOfOriginalData[deletedPoints[j]].position[2] = DELETED_POINTS_COORDINATE;
		}
	}

	// Copy back to main buffer.
	currentPointCloud->vertexInfo = copyOfOriginalData;

	// Update GPU buffer.
	const int kVertexSize = 12 + 4;
	ID3D11DeviceContext* ctx = NULL;

	if (GPU.getDevice() != nullptr)
		GPU.getDevice()->GetImmediateContext(&ctx);
	ctx->UpdateSubresource(currentPointCloud->mainVB, 0, NULL, currentPointCloud->vertexInfo.data(), currentPointCloud->getPointCount() * kVertexSize, currentPointCloud->getPointCount() * kVertexSize);
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API undo()
{
	undoManager::getInstance().undo();
}