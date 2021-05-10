#include "pch.h"
#include "loadManager.h"

LoadManager* LoadManager::_instance = nullptr;

void LoadManager::loadFunc()
{

}

LoadManager::LoadManager()
{
	loadingDone = false;
	newJobReady = false;
	threadHandler = std::thread(&LoadManager::loadFunc, this);
	threadHandler.detach();
}

LoadManager::~LoadManager()
{
}

bool LoadManager::tryLoadPointCloudAsync(std::string path, pointCloud* PointCloud)
{
	//PointCloud->

	return false;
}