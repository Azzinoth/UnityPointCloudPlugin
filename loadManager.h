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

class pointCloud
{
	octree* searchOctree = nullptr;
public:
	ID3D11Buffer* mainVB;
	ID3D11Buffer* intermediateVB;
	glm::mat4 worldMatrix;
	std::vector<MeshVertex> vertexInfo;
	std::vector<float> vertexIntensity;
	bool wasInitialized = false;
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
		D3D11_BUFFER_DESC desc;
		memset(&desc, 0, sizeof(desc));

		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = UINT(vertexInfo.size() * 16);
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		m_Device->CreateBuffer(&desc, NULL, &mainVB);

		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = UINT(vertexInfo.size() * 16);
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		m_Device->CreateBuffer(&desc, NULL, &intermediateVB);

		debugLog::getInstance().addToLog("m_Device->CreateBuffer", "OctreeMemory");

		double AABBsize = rangeX > rangeY ? rangeX : rangeY;
		AABBsize = AABBsize > rangeZ ? AABBsize : rangeZ;

		searchOctree->initialize(float(AABBsize), glm::vec3(rangeX, rangeY, rangeZ / 2.0f), &vertexInfo);
		for (int i = 0; i < vertexInfo.size(); i++)
		{
			searchOctree->addObject(i);
		}
	}

	int getPointCount()
	{
		return int(vertexInfo.size());
	}

	octree* getSearchOctree()
	{
		searchOctree->updateRawDataPointer(&vertexInfo);
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
private:
	SINGLETON_PRIVATE_PART(LoadManager)

	std::thread threadHandler;
	std::atomic<bool> loadingDone;
	std::atomic<bool> newJobReady;
	void loadFunc();
	std::string currentPath;
	pointCloud* currentPointCloud;
};