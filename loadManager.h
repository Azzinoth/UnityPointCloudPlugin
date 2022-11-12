#pragma once

#include "PointCloud.h"

class LoadManager
{
public:
	SINGLETON_PUBLIC_PART(LoadManager)

	void loadPointCloudAsync(std::string path, std::string projectPath, pointCloud* PointCloud);
	bool isLoadingDone();
private:
	SINGLETON_PRIVATE_PART(LoadManager)

	struct InfoForLoading
	{
		std::string currentPath = "";
		std::string currentProjectPath = "";
		pointCloud* currentPointCloud = nullptr;
	};

	static void LoadFunc(void* InputData, void* OutputData);

	static void LoadOwnFormat(pointCloud* PointCloud, std::string Path);
	static void LoadNPY(pointCloud* PointCloud, std::string Path);
	static void LoadLazLas(pointCloud* PointCloud, std::string Path);
};

class SaveManager
{
public:
	SINGLETON_PUBLIC_PART(SaveManager)

	bool SavePointCloudAsync(std::string path, pointCloud* PointCloud);
	bool isSaveDone();
private:
	SINGLETON_PRIVATE_PART(SaveManager)

	struct InfoForSaving
	{
		std::string currentPath = "";
		pointCloud* currentPointCloud = nullptr;
	};

	static void SaveFunc(void* InputData, void* OutputData);
};