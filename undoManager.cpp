#include "pch.h"
#include "undoManager.h"

undoManager* undoManager::Instance = nullptr;

undoManager::undoManager()
{
}

undoManager::~undoManager()
{
}

void undoManager::addAction(action* newAction)
{
	undoActions.push_back(newAction);
}

void undoManager::undo(int actionsToUndo)
{
	if (undoActions.size() == 0 || actionsToUndo <= 0 || actionsToUndo > undoActions.size())
		return;

	pointCloud* currentPointCloud = nullptr;
	currentPointCloud = undoActions.back()->affectedPointCloud;

	LOG.addToLog("currentPointCloud ID: " + currentPointCloud->ID, "undoActions");
	if (currentPointCloud == nullptr)
		return;

	// Take original data.
	std::vector<MeshVertex> copyOfOriginalData = currentPointCloud->originalData;

	for (size_t i = undoActions.size() - 1; i >= 0; i--)
	{
		LOG.addToLog("i was : " + std::to_string(i), "undoActions");
		if (undoActions[i]->affectedPointCloud != currentPointCloud)
			continue;

		undoActions.erase(undoActions.begin() + i, undoActions.begin() + i + 1);
		actionsToUndo--;
		if (actionsToUndo == 0)
			break;
	}

	for (size_t i = 0; i < undoActions.size(); i++)
	{
		if (undoActions[i]->affectedPointCloud == currentPointCloud)
		{
			undoInternal(undoActions[i], copyOfOriginalData);
		}
	}

	// Copy back to main buffer.
	currentPointCloud->vertexInfo = copyOfOriginalData;

	// Update GPU buffer.
	const int kVertexSize = 12 + 4;
	ID3D11DeviceContext* ctx = NULL;

	if (GPU.getDevice() != nullptr)
		GPU.getDevice()->GetImmediateContext(&ctx);

#ifdef USE_COMPUTE_SHADER
	//if (currentPointCloud->currentBuffer_CS != nullptr && *currentPointCloud->currentBuffer_CS != nullptr)
	//	ctx->UpdateSubresource(*currentPointCloud->currentBuffer_CS, 0, NULL, currentPointCloud->vertexInfo.data(), currentPointCloud->getPointCount() * kVertexSize, currentPointCloud->getPointCount() * kVertexSize);
#else
	ctx->UpdateSubresource(currentPointCloud->mainVB, 0, NULL, currentPointCloud->vertexInfo.data(), currentPointCloud->getPointCount() * kVertexSize, currentPointCloud->getPointCount() * kVertexSize);
#endif // USE_COMPUTE_SHADER

	ctx->Release();
	undoActionWasApplied = true;
}

void undoManager::undoInternal(action* actionToUndo, std::vector<MeshVertex>& originalData)
{
	pointCloud* currentPointCloud = actionToUndo->affectedPointCloud;

	std::vector<int> deletedPoints;
	if (actionToUndo->type == "deleteAction")
	{
		LOG.addToLog("Brush location: ", reinterpret_cast<deleteAction*>(actionToUndo)->center, "undoActions");
		LOG.addToLog("Brush size: " + std::to_string(reinterpret_cast<deleteAction*>(actionToUndo)->radius), "undoActions");

		currentPointCloud->getSearchOctree()->searchForObjects(reinterpret_cast<deleteAction*>(actionToUndo)->center,
															   reinterpret_cast<deleteAction*>(actionToUndo)->radius, deletedPoints);

		LOG.addToLog("pointsToDelete size: " + std::to_string(deletedPoints.size()), "undoActions");
	}
	else if (actionToUndo->type == "deleteOutliersAction")
	{
		deletedPoints = reinterpret_cast<deleteOutliersAction*>(actionToUndo)->outliersIndexes;
		LOG.addToLog("deletedPoints size: " + std::to_string(deletedPoints.size()), "undoActions");
	}

	for (size_t i = 0; i < deletedPoints.size(); i++)
	{
		originalData[deletedPoints[i]].position[0] = DELETED_POINTS_COORDINATE;
		originalData[deletedPoints[i]].position[1] = DELETED_POINTS_COORDINATE;
		originalData[deletedPoints[i]].position[2] = DELETED_POINTS_COORDINATE;
	}
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API undo(int actionsToUndo)
{
	undoManager::getInstance().undo(actionsToUndo);
}

void undoManager::clear()
{
	undoActions.clear();
}

void undoManager::clear(pointCloud* PointCloud)
{
	for (size_t i = 0; i < undoActions.size(); i++)
	{
		if (undoActions[i]->affectedPointCloud == PointCloud)
		{
			delete undoActions[i];
			undoActions.erase(undoActions.begin() + i, undoActions.begin() + i + 1);
			i--;
		}
	}
}