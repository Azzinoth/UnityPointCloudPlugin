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

	LOG.Add("undoManager::addAction was called", "undoActions");
	LOG.Add("newAction->type: " + newAction->type, "undoActions");
}

void undoManager::undo(int actionsToUndo)
{
	LOG.Add("undoManager::undo was called", "undoActions");
	LOG.Add("actionsToUndo: " + std::to_string(actionsToUndo), "undoActions");
	LOG.Add("undoActions.size(): " + std::to_string(undoActions.size()), "undoActions");

	if (actionsToUndo > undoActions.size())
	{
		LOG.Add("actionsToUndo > undoActions.size()", "undoActions");
		actionsToUndo = undoActions.size();
	}

	if (undoActions.size() == 0 || actionsToUndo <= 0 )
		return;

	pointCloud* currentPointCloud = nullptr;
	currentPointCloud = undoActions.back()->affectedPointCloud;

	//LOG.Add("currentPointCloud ID: " + currentPointCloud->ID, "undoActions");
	if (currentPointCloud == nullptr)
		return;

	// Take original data.
	std::vector<VertexData> copyOfOriginalData = currentPointCloud->originalData;
	
	for (size_t i = undoActions.size() - 1; i >= 0; i--)
	{
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
			ReApply(undoActions[i], copyOfOriginalData);
		}
	}

	// Copy back to main buffer.
	currentPointCloud->vertexInfo = copyOfOriginalData;

	// Update GPU buffer.
	const int kVertexSize = sizeof(VertexData);
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

void undoManager::ReApply(action* actionToUndo, std::vector<VertexData>& originalData)
{
	pointCloud* currentPointCloud = actionToUndo->affectedPointCloud;

	std::vector<int> deletedPoints;
	if (actionToUndo->type == "deleteAction" || actionToUndo->type == "changeClassificationAction")
	{
		LOG.Add("Brush location: " + vec3ToString(reinterpret_cast<deleteAction*>(actionToUndo)->center), "undoActions");
		LOG.Add("Brush size: " + std::to_string(reinterpret_cast<deleteAction*>(actionToUndo)->radius), "undoActions");

		currentPointCloud->getSearchOctree()->searchForObjects(reinterpret_cast<deleteAction*>(actionToUndo)->center,
															   reinterpret_cast<deleteAction*>(actionToUndo)->radius, deletedPoints);

		LOG.Add("PointnsInSphere size: " + std::to_string(deletedPoints.size()), "undoActions");
	}
	else if (actionToUndo->type == "deleteOutliersAction")
	{
		deletedPoints = reinterpret_cast<deleteOutliersAction*>(actionToUndo)->outliersIndexes;
		LOG.Add("deletedPoints size: " + std::to_string(deletedPoints.size()), "undoActions");
	}

	if (actionToUndo->type == "deleteAction" || actionToUndo->type == "deleteOutliersAction")
	{
		for (size_t i = 0; i < deletedPoints.size(); i++)
		{
			originalData[deletedPoints[i]].position[0] = DELETED_POINTS_COORDINATE;
			originalData[deletedPoints[i]].position[1] = DELETED_POINTS_COORDINATE;
			originalData[deletedPoints[i]].position[2] = DELETED_POINTS_COORDINATE;
		}
	}
	else if (actionToUndo->type == "changeClassificationAction")
	{
		for (size_t i = 0; i < deletedPoints.size(); i++)
		{
			originalData[deletedPoints[i]].classification = reinterpret_cast<changeClassificationAction*>(actionToUndo)->newClassification;
		}
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