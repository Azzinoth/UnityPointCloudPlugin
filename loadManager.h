#pragma once

#include "computeShader.h"
#include "Octree.h"
#include <thread>
#include "thirdparty/laszip/include/laszip_api.h"
#include <filesystem>
#include <random>

//#define USE_QUADS_NOT_POINTS
//#define USE_COMPUTE_SHADER
//#define LOD_SYSTEM

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

#ifdef LOD_SYSTEM
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
#endif

struct ColorInfo
{
	unsigned char r, g, b;
};

struct NumPyPoints
{
	double X, Y, Z, Uncertainty;
};

struct NumPyMinMax
{
	double MinX = 0.0;
	double MinY = 0.0;
	double MinZ = 0.0;
	double MinUncertainty = 0.0;

	double MaxX = 0.0;
	double MaxY = 0.0;
	double MaxZ = 0.0;
	double MaxUncertainty = 0.0;
};

struct NumPyInfo
{
	std::vector<NumPyPoints> LoadedRawData;
	std::string WKT;
	std::vector<std::string> AllEPSG;
	NumPyMinMax MinMax;
};

class pointCloud
{
	octree* searchOctree = nullptr;
public:
	NumPyInfo* NumPy = nullptr;

#ifdef USE_COMPUTE_SHADER
	float* InputData_CS = nullptr;

	//ID3D11Buffer* allPointsDataBuffer_CS = nullptr;
	ID3D11Buffer* InputDataBuffer_CS = nullptr;
	//ID3D11Buffer* resultBuffer_CS = nullptr;
	//ID3D11Buffer* resultBufferSecond_CS = nullptr;
	//ID3D11Buffer** currentBuffer_CS = nullptr;

	//ID3D11ShaderResourceView* inputPoints_CS_SRV = nullptr;
	//ID3D11ShaderResourceView** currentInputPoints_CS_SRV = nullptr;
	ID3D11ShaderResourceView* InputData_CS_SRV = nullptr;
	ID3D11UnorderedAccessView* result_CS_UAV = nullptr;
	//ID3D11UnorderedAccessView* resultSecond_CS_UAV = nullptr;
	ID3D11UnorderedAccessView** current_CS_UAV = nullptr;

	ID3D11ShaderResourceView* result_CS_SRV = nullptr;
	//ID3D11ShaderResourceView* resultSecond_CS_SRV = nullptr;
	ID3D11ShaderResourceView** current_CS_SRV = nullptr;

	bool deletionOccuredThisFrame = false;
	int pointsToDraw = 0;
#endif
	ID3D11Buffer* mainVB;
	ID3D11Buffer* intermediateVB;
#ifdef USE_QUADS_NOT_POINTS
	ID3D11Buffer* pointPositionsVB;
#endif

	std::string ID = "";

	glm::mat4 worldMatrix;
	std::vector<MeshVertex> vertexInfo;
	std::vector<float> vertexIntensity;

	std::vector<int> lastHighlightedPoints;
	std::vector<MeshVertex> originalData;
	int highlightStep = 0;

#ifdef LOD_SYSTEM
	std::vector<LODInformation> LODs;
	static std::vector<LODSetting> LODSettings;
#endif

	bool wasInitialized = false;
	bool wasFullyLoaded = false;
	LAZFileInfo* loadedFrom = nullptr;
	glm::dvec3 min;
	glm::dvec3 max;
	glm::dvec3 adjustment;
	std::string spatialInfo;
	std::string UTMZone;

	std::string filePath;

	double initialXShift;
	double initialZShift;

	float lastDiscardDistance;
	int lastMinNeighborsInRange;
	std::vector<int> lastOutliers;

	unsigned short EPSG;
	float approximateGroundLevel;

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
		if (mainVB != nullptr)
			mainVB->Release();
		if (mainVB != nullptr)
			intermediateVB->Release();
		delete loadedFrom;

		debugLog::getInstance().addToLog("end of ~pointCloud", "OctreeMemory");
	}

	void initializeOctree(double rangeX, double rangeY, double rangeZ, glm::vec3 worldCenter)
	{
		double AABBsize = rangeX > rangeY ? rangeX : rangeY;
		AABBsize = AABBsize > rangeZ ? AABBsize : rangeZ;

#ifdef USE_QUADS_NOT_POINTS
		searchOctree->initialize(float(AABBsize), worldCenter);
#else
		searchOctree->initialize(float(AABBsize), worldCenter);

		debugLog::getInstance().addToLog("after searchOctree->initialize", "testThread");
		debugLog::getInstance().addToLog("vertexInfo.size(): " + std::to_string(vertexInfo.size()), "testThread");

		for (int i = 0; i < vertexInfo.size(); i++)
		{
			bool accepted = searchOctree->addObject(vertexInfo[i], i);
			if (!accepted)
			{
				debugLog::getInstance().addToLog("point was: rejected", "OctreeEvents");
				debugLog::getInstance().addToLog("point:", vertexInfo[i].position, "OctreeEvents");
			}
		}
		debugLog::getInstance().addToLog(" after for (int i = 0; i < vertexInfo.size(); i++)", "testThread");
#endif
	}

	int getPointCount()
	{
		return int(vertexInfo.size());
	}

	octree* getSearchOctree()
	{
		return searchOctree;
	}

	void highlightOutliers(float discardDistance, int minNeighborsInRange)
	{
		lastDiscardDistance = discardDistance;
		lastMinNeighborsInRange = minNeighborsInRange;

		lastOutliers.clear();

		// Subsample
		//for (size_t i = 0; i < vertexInfo.size(); i++)
		//{
		//	if (vertexInfo[i].position[0] == DELETED_POINTS_COORDINATE &&
		//		vertexInfo[i].position[1] == DELETED_POINTS_COORDINATE &&
		//		vertexInfo[i].position[2] == DELETED_POINTS_COORDINATE)
		//		continue;

		//	// We rely on fact that points are somewhat sorted by their position in an array.
		//	bool earlyExit = false;
		//	for (size_t j = i - 10; j < i + 10; j++)
		//	{
		//		if (j < 0 || j >= vertexInfo.size() || j == i)
		//			continue;

		//		float distance = glm::length(vertexInfo[i].position - vertexInfo[j].position);
		//		if (distance < discardDistance)
		//		{
		//			earlyExit = true;
		//			break;
		//		}
		//	}

		//	if (earlyExit)
		//	{
		//		vertexInfo[i].color[0] = 255;
		//		vertexInfo[i].color[1] = 0;
		//		vertexInfo[i].color[2] = 0;
		//		vertexInfo[i].color[3] = 255;

		//		lastOutliers.push_back(int(i));

		//		continue;
		//	}

		//	float distance = getSearchOctree()->closestPointDistance(vertexInfo[i].position, discardDistance);
		//	if (distance < discardDistance)
		//	{
		//		vertexInfo[i].color[0] = 255;
		//		vertexInfo[i].color[1] = 0;
		//		vertexInfo[i].color[2] = 0;
		//		vertexInfo[i].color[3] = 255;

		//		lastOutliers.push_back(int(i));
		//	}
		//	else
		//	{
		//		vertexInfo[i].color[0] = originalData[i].color[0];
		//		vertexInfo[i].color[1] = originalData[i].color[1];
		//		vertexInfo[i].color[2] = originalData[i].color[2];
		//		vertexInfo[i].color[3] = originalData[i].color[3];
		//	}
		//}


		for (size_t i = 0; i < vertexInfo.size(); i++)
		{
			float distance = FLT_MAX;
			// We rely on fact that points are somewhat sorted by their position in an array.
			int localNeighbors = 0;
			for (size_t j = i - 10; j < i + 10; j++)
			{
				if (j < 0 || j >= vertexInfo.size() || j == i)
					continue;

				distance = glm::length(vertexInfo[i].position - vertexInfo[j].position);
				if (distance < discardDistance)
				{
					localNeighbors++;
					if (localNeighbors >= minNeighborsInRange)
						break;
				}
			}

			if (localNeighbors < minNeighborsInRange)
			{
				distance = getSearchOctree()->closestPointDistance(vertexInfo[i].position, discardDistance, minNeighborsInRange);

				if (distance > discardDistance)
				{
					vertexInfo[i].color[0] = 255;
					vertexInfo[i].color[1] = 0;
					vertexInfo[i].color[2] = 0;
					vertexInfo[i].color[3] = 255;

					lastOutliers.push_back(int(i));
				}
				else
				{
					vertexInfo[i].color[0] = originalData[i].color[0];
					vertexInfo[i].color[1] = originalData[i].color[1];
					vertexInfo[i].color[2] = originalData[i].color[2];
					vertexInfo[i].color[3] = originalData[i].color[3];
				}
			}
			else
			{
				vertexInfo[i].color[0] = originalData[i].color[0];
				vertexInfo[i].color[1] = originalData[i].color[1];
				vertexInfo[i].color[2] = originalData[i].color[2];
				vertexInfo[i].color[3] = originalData[i].color[3];
			}
		}

		// Update GPU buffer.
		const int kVertexSize = 12 + 4;
		ID3D11DeviceContext* ctx = NULL;

		if (GPU.getDevice() != nullptr)
			GPU.getDevice()->GetImmediateContext(&ctx);

		ctx->UpdateSubresource(mainVB, 0, NULL, vertexInfo.data(), getPointCount() * kVertexSize, getPointCount() * kVertexSize);

//#ifdef USE_COMPUTE_SHADER
//		if (currentBuffer_CS != nullptr && *currentBuffer_CS != nullptr)
//			ctx->UpdateSubresource(*currentBuffer_CS, 0, NULL, vertexInfo.data(), getPointCount() * kVertexSize, getPointCount() * kVertexSize);
//#else
//		ctx->UpdateSubresource(mainVB, 0, NULL, vertexInfo.data(), getPointCount() * kVertexSize, getPointCount() * kVertexSize);
//#endif // USE_COMPUTE_SHADER
	}

	float getApproximateGroundLevel()
	{
		return approximateGroundLevel;
	}

	void calculateApproximateGroundLevel()
	{
		std::vector<float> allYValues;
		allYValues.resize(vertexInfo.size());
		for (size_t i = 0; i < vertexInfo.size(); i++)
		{
			allYValues[i] = vertexInfo[i].position.y;
		}

		std::sort(allYValues.begin(), allYValues.end());

		int numbOfPoints = int(vertexInfo.size() * 0.01f);
		float mean = 0.0f;
		for (size_t i = 0; i < numbOfPoints; i++)
		{
			mean += allYValues[i];
		}

		if (numbOfPoints != 0)
			mean /= numbOfPoints;

		approximateGroundLevel = mean;
	}
};

class LoadManager
{
public:
	SINGLETON_PUBLIC_PART(LoadManager)

	bool tryLoadPointCloudAsync(std::string path, std::string projectPath, pointCloud* PointCloud);
	void loadPointCloudAsync(std::string path, std::string projectPath, pointCloud* PointCloud);
	int getFreeTextureThreadCount();
	bool isLoadingDone();

	void update();
private:
	SINGLETON_PRIVATE_PART(LoadManager)

	std::thread threadHandler;
	std::atomic<bool> loadingDone;
	std::atomic<bool> newJobReady;
	void loadFunc();
	std::string currentPath = "";
	std::string currentProjectPath = "";
	pointCloud* currentPointCloud = nullptr;
	std::vector<std::pair<std::string, pointCloud*>> pointCloudsToLoad;
};

class SaveManager
{
public:
	SINGLETON_PUBLIC_PART(SaveManager)

	bool trySavePointCloudAsync(std::string path, pointCloud* PointCloud);
	bool isSaveDone();
private:
	SINGLETON_PRIVATE_PART(SaveManager)

	std::thread threadHandler;
	std::atomic<bool> savingDone;
	std::atomic<bool> newJobReady;
	void saveFunc();
	std::string currentPath = "";
	std::string currentProjectPath = "";
	pointCloud* currentPointCloud = nullptr;
};