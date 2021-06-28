#pragma once

#include "debugLog.h"
#include "Octree.h"
#include <thread>
#include "thirdparty/laszip/include/laszip_api.h"

static ID3D11Device* m_Device;

class pointCloud;
struct LAZFileInfo
{
	laszip_header header = laszip_header();
	std::vector<laszip_point> LAZpoints;
	pointCloud* resultingPointCloud = nullptr;
	bool compressed = false;

	LAZFileInfo() {}
	~LAZFileInfo() {}
};

struct LODInformation
{
	ID3D11Buffer* VB;
	std::vector<MeshVertex> vertexInfo;
};

struct LODSetting
{
	float maxDistance;
	float targetPercentOFPoints;
	int takeEach_Nth_Point;
};

class pointCloud
{
	octree* searchOctree = nullptr;
public:
	ID3D11Buffer* mainVB;
	ID3D11Buffer* intermediateVB;

	glm::mat4 worldMatrix;
	std::vector<MeshVertex> vertexInfo;
	std::vector<float> vertexIntensity;
	std::vector<int> lastHighlightedPoints;

	std::vector<LODInformation> LODs;
	static std::vector<LODSetting> LODSettings;

	bool wasInitialized = false;
	bool wasFullyLoaded = false;
	LAZFileInfo* loadedFrom = nullptr;
	glm::vec3 min;
	glm::vec3 max;
	glm::vec3 adjustment;
	std::string spatialInfo;
	std::string UTMZone;

	double initialXShift;
	double initialZShift;

	pointCloud()
	{
		mainVB = nullptr;
		intermediateVB = nullptr;
		worldMatrix = glm::identity<glm::mat4>();
		searchOctree = new octree();
	}

	~pointCloud()
	{
		debugLog::getInstance().addToLog("start of ~pointCloud", "OctreeMemory");

		delete searchOctree;
		mainVB->Release();
		intermediateVB->Release();
		delete loadedFrom;

		debugLog::getInstance().addToLog("end of ~pointCloud", "OctreeMemory");
	}

	void initializeOctree(double rangeX, double rangeY, double rangeZ)
	{
		double AABBsize = rangeX > rangeY ? rangeX : rangeY;
		AABBsize = AABBsize > rangeZ ? AABBsize : rangeZ;

		searchOctree->initialize(float(AABBsize * 4.0f), glm::vec3(rangeX, rangeY, rangeZ / 2.0f), &vertexInfo);
		debugLog::getInstance().addToLog("after searchOctree->initialize", "testThread");
		debugLog::getInstance().addToLog("vertexInfo.size(): " + std::to_string(vertexInfo.size()), "testThread");
		
		for (int i = 0; i < vertexInfo.size(); i++)
		{
			bool accepted = searchOctree->addObject(i);
			/*if (!accepted)
			{
				debugLog::getInstance().addToLog("point was: rejected", "OctreeEvents");
				debugLog::getInstance().addToLog("point:", vertexInfo[i].position, "OctreeEvents");
			}*/

			//debugLog::getInstance().addToLog("after addObject i: " + std::to_string(i), "testThread");
		}
		debugLog::getInstance().addToLog(" after for (int i = 0; i < vertexInfo.size(); i++)", "testThread");
	}

	int getPointCount()
	{
		return int(vertexInfo.size());
	}

	octree* getSearchOctree()
	{
		//searchOctree->updateRawDataPointer(&vertexInfo);
		return searchOctree;
	}
};

class pointCloud;

class LoadManager
{
public:
	SINGLETON_PUBLIC_PART(LoadManager)

	bool tryLoadPointCloudAsync(std::string path, pointCloud* PointCloud);
	int getFreeTextureThreadCount();
	bool isLoadingDone();
private:
	SINGLETON_PRIVATE_PART(LoadManager)

	std::thread threadHandler;
	std::atomic<bool> loadingDone;
	std::atomic<bool> newJobReady;
	void loadFunc();
	std::string currentPath = "";
	pointCloud* currentPointCloud = nullptr;
};