#pragma once

#include "thirdparty/cnpy/cnpy.h"
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
	static void ExtractLazLasHeaderInfo(pointCloud* PointCloud, laszip_header* Header);

	static void copyLAZvlr(laszip_header* dest, laszip_vlr_struct source);
	static void copyLAZFileHeader(laszip_header* dest, laszip_header* source);
	static std::string GetWKT(std::string FileName, std::string VariableName = "wkt");
	static std::vector<std::string> GetEPSG(std::string TotalString);
	static NumPyMinMax GetMinMax(std::string FileName, std::string VariableName = "minmax");
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