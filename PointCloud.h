#pragma once

#include "computeShader.h"
#include "Octree.h"
#include <thread>
#include "thirdparty/laszip/include/laszip_api.h"
#include <filesystem>
#include <random>

using namespace FocalEngine;

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
	glm::dvec3 RawMin;
	glm::dvec3 RawMax;
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

	unsigned short EPSG = 0;
	float approximateGroundLevel;

	pointCloud();
	~pointCloud();

	void initializeOctree(double rangeX, double rangeY, double rangeZ, glm::vec3 worldCenter);

	int getPointCount();
	octree* getSearchOctree();

	void highlightOutliers(float discardDistance, int minNeighborsInRange);

	float getApproximateGroundLevel();
	void calculateApproximateGroundLevel();
};