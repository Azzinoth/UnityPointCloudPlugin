#include "pch.h"

using namespace FocalEngine;

std::string getVersion()
{
	SYSTEMTIME st, lt;

	GetSystemTime(&st);
	GetLocalTime(&lt);

	std::string result = "version ";
	result += std::to_string(st.wYear);
	result += ".";
	result += std::to_string(st.wMonth);
	result += ".";
	result += std::to_string(st.wDay);
	result += ".";
	result += std::to_string(st.wHour * 1000 + st.wMinute * 100 + st.wSecond);

	return result;
}

static std::string currentVersion = "version 2022.11.18.18309";

static ID3D11Buffer* m_CB = nullptr;
static ID3D11VertexShader* m_VertexShader;
static ID3D11PixelShader* m_PixelShader;
static ID3D11InputLayout* m_InputLayout;
static ID3D11RasterizerState* m_RasterState;
static ID3D11BlendState* m_BlendState;
static ID3D11DepthStencilState* m_DepthState;
static float pointsWorldMatrix[16];
static float worldToViewMatrix[16];
static float projectionMatrix[16];

//static std::atomic<bool> requestToDelete;
static bool localHighlightDeletedPoints = false;
const int kVertexSize = sizeof(VertexData);

static int screenIndex = -1;
static int internalScreenIndex = 0;

static std::unordered_map<int, float*> viewProjectionMatrices;
static std::unordered_map<int, ID3D11Buffer*> viewProjectionMatricesBuffers;

ID3D11ComputeShader* computeShader = nullptr;

VertexData* allPointsData_CS;

#define FLOAT_TEST

#ifdef FLOAT_TEST
	//float* InputData_CS;
#else
	VertexData* InputData_CS;
#endif // FLOAT_TEST

#ifdef USE_COMPUTE_SHADER
	/*ID3D11Buffer* allPointsDataBuffer_CS = nullptr;
	ID3D11Buffer* InputDataBuffer_CS = nullptr;
	ID3D11Buffer* resultBuffer_CS = nullptr;
	ID3D11Buffer* resultBufferSecond_CS = nullptr;
	ID3D11Buffer** currentBuffer_CS = nullptr;

	ID3D11ShaderResourceView* inputPoints_CS_SRV = nullptr;
	ID3D11ShaderResourceView** currentInputPoints_CS_SRV = nullptr;
	ID3D11ShaderResourceView* InputData_CS_SRV = nullptr;
	ID3D11UnorderedAccessView* result_CS_UAV = nullptr;
	ID3D11UnorderedAccessView* resultSecond_CS_UAV = nullptr;
	ID3D11UnorderedAccessView** current_CS_UAV = nullptr;

	ID3D11ShaderResourceView* result_CS_SRV = nullptr;
	ID3D11ShaderResourceView* resultSecond_CS_SRV = nullptr;
	ID3D11ShaderResourceView** current_CS_SRV = nullptr;*/

	void runMyDeleteComputeShader(pointCloud* pointCloudToRender);
#endif

ID3D11ComputeShader* outliers_CS = nullptr;
//ID3D11ComputeShader* outliers_CS = nullptr;
//ID3D11Buffer* outliersBuffer_CS = nullptr;
//ID3D11ShaderResourceView* outliers_CS_SRV = nullptr;
//ID3D11UnorderedAccessView* outliers_result_CS_UAV = nullptr;

static float testLevel = -830.0f;
static float** testFrustum = nullptr;

static float** frustum = nullptr;
static void updateFrustumPlanes();
static bool frustumCulling = false;

#ifdef LOD_SYSTEM
static bool LODSystemActive = false;
static float LODTransitionDistance = 3500.0f;
#endif

static bool highlightDeletedPoints = false;
static glm::vec3 deletionSpherePosition = glm::vec3(0.0f);
static float deletionSphereSize = 0.0f;

std::vector<pointCloud*> pointClouds;

void WorkOnRequests()
{
	/*std::vector<ModificationRequest>*/ RenderingThreadLocalCopy = ModificationRequests;
	ModificationRequests.clear();

	for (size_t i = 0; i < RenderingThreadLocalCopy.size(); i++)
	{
		RenderingThreadLocalCopy[i].Result = 0;
#ifdef USE_COMPUTE_SHADER

		deletionSpherePosition = glm::vec3(center[0], center[1], center[2]);
		deletionSphereSize = size / 2.0f;

		for (size_t j = 0; j < pointClouds.size(); j++)
		{
			if (!pointClouds[i]->wasFullyLoaded)
				continue;

			glm::vec3 localPosition = glm::inverse(glm::transpose(pointClouds[i]->worldMatrix)) * glm::vec4(deletionSpherePosition, 1.0f);
			float extractedScale = glm::length(glm::transpose(pointClouds[i]->worldMatrix)[0]);
			size /= extractedScale;

			if (pointClouds[i]->getSearchOctree()->isInOctreeBound(localPosition, size))
			{
				//pointClouds[i]->getSearchOctree()->deleteObjects(localPosition, size);
				//pointClouds[i]->deletionOccuredThisFrame = true;

				runMyDeleteComputeShader(pointClouds[i]);

				UNDO_MANAGER.addAction(new deleteAction(localPosition, size, pointClouds[i]));
				//anyPointWasDeleted = true;
			}

			/*if (pointClouds[i]->getSearchOctree()->PointnsInSphere.size() > 0)
			{
				UNDO_MANAGER.addAction(new deleteAction(localPosition, size, pointClouds[i]));

				LOG.Add("==============================================================", "deleteEvents");
				LOG.Add("Brush location: ", localPosition, "deleteEvents");
				LOG.Add("Brush size: " + std::to_string(size), "deleteEvents");
				LOG.Add("PointnsInSphere size: " + std::to_string(pointClouds[i]->getSearchOctree()->PointnsInSphere.size()), "deleteEvents");

				anyPointWasDeleted = true;
			}*/
}
#else
		for (size_t j = 0; j < pointClouds.size(); j++)
		{
			if (!pointClouds[j]->wasFullyLoaded)
				continue;

			glm::vec3 localPosition = glm::inverse(glm::transpose(pointClouds[j]->worldMatrix)) * glm::vec4(RenderingThreadLocalCopy[i].Center, 1.0f);
			float extractedScale = glm::length(glm::transpose(pointClouds[j]->worldMatrix)[0]);
			RenderingThreadLocalCopy[i].Size /= extractedScale;

			if (pointClouds[j]->getSearchOctree()->isInOctreeBound(localPosition, RenderingThreadLocalCopy[i].Size))
			{
				pointClouds[j]->getSearchOctree()->searchForObjects(localPosition, RenderingThreadLocalCopy[i].Size, pointClouds[j]->getSearchOctree()->PointnsInSphere);
			}

			if (pointClouds[j]->getSearchOctree()->PointnsInSphere.size() > 0)
			{
				if (RenderingThreadLocalCopy[i].TypeOfModification == 0)
				{
					UNDO_MANAGER.addAction(new deleteAction(localPosition, RenderingThreadLocalCopy[i].Size, pointClouds[j]));

					LOG.Add("==============================================================", "deleteEvents");
					LOG.Add("Brush location: " + vec3ToString(localPosition), "deleteEvents");
					LOG.Add("Brush size: " + std::to_string(RenderingThreadLocalCopy[i].Size), "deleteEvents");
					LOG.Add("PointnsInSphere size: " + std::to_string(pointClouds[j]->getSearchOctree()->PointnsInSphere.size()), "deleteEvents");

					octree* currentOctree = pointClouds[j]->getSearchOctree();
					for (size_t k = 0; k < currentOctree->PointnsInSphere.size(); k++)
					{
						if (pointClouds[j]->vertexInfo[currentOctree->PointnsInSphere[k]].position[0] != DELETED_POINTS_COORDINATE &&
							pointClouds[j]->vertexInfo[currentOctree->PointnsInSphere[k]].position[1] != DELETED_POINTS_COORDINATE &&
							pointClouds[j]->vertexInfo[currentOctree->PointnsInSphere[k]].position[2] != DELETED_POINTS_COORDINATE)
						{
							RenderingThreadLocalCopy[i].Result++;
						}
					}
				}
				else if (RenderingThreadLocalCopy[i].TypeOfModification == 1)
				{
					octree* currentOctree = pointClouds[j]->getSearchOctree();
					for (size_t k = 0; k < currentOctree->PointnsInSphere.size(); k++)
					{
						if (pointClouds[j]->vertexInfo[currentOctree->PointnsInSphere[k]].classification != RenderingThreadLocalCopy[i].AdditionalData)
						{
							RenderingThreadLocalCopy[i].Result++;
						}
					}
				}

				ID3D11DeviceContext* ctx = NULL;
				GPU.getDevice()->GetImmediateContext(&ctx);
				ApplyPoindModificationRequest(pointClouds[j], ctx, RenderingThreadLocalCopy[i]);
				ctx->Release();
			}
		}

#endif // USE_COMPUTE_SHADER

		//requestToDelete = true;
	}
}

static string NextTextToSend = "";
extern "C" int UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API GetNextTextLengthFromDLL()
{
	return int(NextTextToSend.size());
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API FillTextFromDLL(int* Data)
{
	//LOG.Add(std::string("NextTextToSend.size(): ") + std::to_string(NextTextToSend.size()), "GetTextFromDLL");

	for (size_t i = 0; i < NextTextToSend.size(); i++)
	{
		Data[i] = int(NextTextToSend[i]);
	}

	NextTextToSend = "";
}

std::unordered_map<std::string, float> FloatsToSync;
extern "C" int UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API GetFloatsToSyncCount()
{
	return int(FloatsToSync.size());
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API RequestFloatsToSyncVariableName(int index)
{
	int currentIndex = 0;
	auto it = FloatsToSync.begin();
	while (it != FloatsToSync.end())
	{
		if (currentIndex == index)
		{
			NextTextToSend = it->first;
			return;
		}

		it++;
		currentIndex++;
	}
}

extern "C" float UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API GetFloatToSyncValue(char* Name)
{
	if (FloatsToSync.find(Name) == FloatsToSync.end())
		return 0.0f;

	return FloatsToSync[Name];
}

extern "C" bool UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API SetFloatToSyncValue(char* Name, float NewValue)
{
	if (FloatsToSync.find(Name) == FloatsToSync.end())
		return false;

	FloatsToSync[Name] = NewValue;

	return true;
}

static bool DLLWasLoadedCorrectly = false;
static std::string resultString;
static std::string projectPath = "";

void runMyComputeShader();

pointCloud* getPointCloud(std::string ID)
{
	if (ID.size() != 24)
		return nullptr;

	for (size_t i = 0; i < pointClouds.size(); i++)
	{
		if (pointClouds[i]->ID == ID)
			return pointClouds[i];
	}

	return nullptr;
}

extern "C" bool UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API IsLastAsyncLoadFinished()
{
	return LoadManager::getInstance().isLoadingDone();
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API getNewUniqueID(int* IDToFill)
{
	std::string newID = APPLICATION.GetUniqueHexID();
	for (size_t i = 0; i < 24; i++)
	{
		IDToFill[i] = int(newID[i]);
	}
}

void AsynOpenLAZFileFromUnity(char* filePath, char* ID)
{
	LOG.Add(std::string("File name: ") + filePath, "File_Load_Log");

	if (strlen(filePath) < 4)
	{
		LOG.Add(std::string("Call of AsynOpenLAZFileFromUnity can't be executed because file name is incorrect: ") + filePath, "ERRORS");
		return;
	}

	// if file is in our own format we will not need dll functionality
	if (filePath[strlen(filePath) - 4] != '.' && filePath[strlen(filePath) - 3] != 'c' && filePath[strlen(filePath) - 2] != 'p' && filePath[strlen(filePath) - 1] != 'c')
	{
		if (!DLLWasLoadedCorrectly)
		{
			LOG.Add("Call of AsynOpenLAZFileFromUnity can't be executed because DLL was not loaded correctly", "ERRORS");
			return;
		}
	}

	pointCloud* temp = new pointCloud();
	temp->ID = ID;
	LoadManager::getInstance().loadPointCloudAsync(std::string(filePath), projectPath, temp);
	pointClouds.push_back(temp);
}

extern "C" bool UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API OpenLAZFileFromUnity(char* filePath, char* ID)
{
	LOG.Add(std::string("File name: ") + filePath, "File_Name");

	if (strlen(filePath) < 4)
	{
		LOG.Add(std::string("Call of OpenLAZFileFromUnity can't be executed because file name is incorrect: ") + filePath, "ERRORS");
		return false;
	}

	// if file is in our own format we will not need dll functionality
	if (filePath[strlen(filePath) - 4] != '.' && filePath[strlen(filePath) - 3] != 'c' && filePath[strlen(filePath) - 2] != 'p' && filePath[strlen(filePath) - 1] != 'c')
	{
		if (!DLLWasLoadedCorrectly)
		{
			LOG.Add("Call of OpenLAZFileFromUnity can't be executed because DLL was not loaded correctly", "ERRORS");
			return false;
		}
	}

	AsynOpenLAZFileFromUnity(filePath, ID);
	return true;
}

extern "C" bool UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API ValidatePointCloudGMFromUnity(char* filePath, char* pointCloudID)
{
	LOG.Add("path: " + std::string(filePath), "ReLoad");
	LOG.Add("ID: " + std::string(pointCloudID), "ReLoad");

	if (strlen(filePath) < 4)
		return false;

	// Looking at point clouds that we have in RAM to validate game object from scene.
	for (size_t i = 0; i < pointClouds.size(); i++)
	{
		if (pointClouds[i]->ID == pointCloudID)
			return false;
	}

	LOG.Add("Try to load: " + std::string(filePath), "ReLoad");
	AsynOpenLAZFileFromUnity(filePath, pointCloudID);

	return true;
}

//extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API AddClassificationEntry(int Type, float R, float G, float B);

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API OnSceneStartFromUnity(char* projectFilePath)
{
	LOG.SetFileOutput(true);

	LOG.DisableTopicFileOutput("camera");
	LOG.DisableTopicFileOutput("precision");
	LOG.DisableTopicFileOutput("screens");
	LOG.DisableTopicFileOutput("renderLog");
	LOG.DisableTopicFileOutput("Threads");

	if (THREAD_POOL.GetThreadCount() == 0)
		THREAD_POOL.SetConcurrentThreadCount(2);

	ClassificationCount = 0;

	/*for (size_t i = 4500; i < 4600; i++)
	{
		AddClassificationEntry(i, 1.0f, 0.1f, 0.1f);
	}*/
	//AddClassificationEntry(0, 0.1f, 1.0f, 0.1f);
	//AddClassificationEntry(1, 0.1f, 0.1f, 1.0f);
	//AddClassificationEntry(155, 1.0f, 0.1f, 1.0f);

	if (FloatsToSync.size() == 0)
	{
		FloatsToSync["VisualizeClassification"] = 0.0f;
	}

	/*UNDO_MANAGER.clear();

	for (size_t i = 0; i < pointClouds.size(); i++)
	{
		delete pointClouds[i];
	}
	pointClouds.clear();*/

	projectPath = projectFilePath;
	LOG.Add("Project path: " + std::string(projectFilePath), "Project_File_Path");

	if (!DLLWasLoadedCorrectly)
	{
		if (laszip_load_dll(projectFilePath))
		{
			DLLWasLoadedCorrectly = false;
			LOG.Add("project path: " + std::string(projectFilePath), "DLL_ERRORS");
			LOG.Add("loading LASzip DLL failed", "DLL_ERRORS");
			return;
		}

		DLLWasLoadedCorrectly = true;
	}

	laszip_U8 version_major;
	laszip_U8 version_minor;
	laszip_U16 version_revision;
	laszip_U32 version_build;
	if (laszip_get_version(&version_major, &version_minor, &version_revision, &version_build))
	{
		LOG.Add("getting LASzip DLL version number failed", "DLL_ERRORS");
	}

	LOG.Add("LASzip DLL v" + std::to_string((int)version_major) + "." + std::to_string((int)version_minor) + "r" + std::to_string((int)version_revision) + " (build " + std::to_string((int)version_build) + ")", "DLL_START");

#ifdef LOD_SYSTEM
	pointCloud::LODSettings.resize(4);
	pointCloud::LODSettings[0].targetPercentOFPoints = 50.0f;
	pointCloud::LODSettings[0].maxDistance = 1000.0f;
	pointCloud::LODSettings[1].targetPercentOFPoints = 33.0f;
	pointCloud::LODSettings[1].maxDistance = 3500.0f;
	pointCloud::LODSettings[2].targetPercentOFPoints = 25.0f;
	pointCloud::LODSettings[2].maxDistance = 5000.0f;
	pointCloud::LODSettings[3].targetPercentOFPoints = 12.5f;
	pointCloud::LODSettings[3].maxDistance = 21000.0f;

	for (size_t i = 0; i < 4; i++)
	{
		pointCloud::LODSettings[i].takeEach_Nth_Point = (int)(100.0f / pointCloud::LODSettings[i].targetPercentOFPoints);
	}
#endif
}

extern "C" bool UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API SaveToLAZFileFromUnity(char* filePath, char* pointCloudID)
{
	if (strlen(filePath) < 4)
		return false;

	// if file is in our own format we will not need dll functionality
	if (filePath[strlen(filePath) - 4] != '.' && filePath[strlen(filePath) - 3] != 'c' && filePath[strlen(filePath) - 2] != 'p' && filePath[strlen(filePath) - 1] != 'c')
	{
		if (!DLLWasLoadedCorrectly)
		{
			LOG.Add("Call of SaveToLAZFileFromUnity can't be executed because DLL was not loaded correctly", "ERRORS");
			return false;
		}
	}

	if (getPointCloud(pointCloudID) == nullptr)
	{
		LOG.Add("Call of SaveToLAZFileFromUnity can't be executed because pointCloud can't be found", "ERRORS");
		return false;
	}

	bool willBeSaved = false;
	willBeSaved = SaveManager::getInstance().SavePointCloudAsync(std::string(filePath), getPointCloud(pointCloudID));
	return willBeSaved;
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API SaveToOwnFormatFileFromUnity(char* filePath, char* pointCloudID)
{
	pointCloud* currentPointCloud = getPointCloud(pointCloudID);
	if (currentPointCloud == nullptr)
		return;

	int pointsToWrite = 0;
	// Count how many points left.
	for (size_t j = 0; j < currentPointCloud->getPointCount(); j++)
	{
		if (currentPointCloud->vertexInfo[j].position[0] != DELETED_POINTS_COORDINATE &&
			currentPointCloud->vertexInfo[j].position[1] != DELETED_POINTS_COORDINATE &&
			currentPointCloud->vertexInfo[j].position[2] != DELETED_POINTS_COORDINATE)
			pointsToWrite++;
	}

	std::string fileName = filePath;
	std::fstream file;
	file.open(fileName, std::ios::out | std::ios::binary);

	// Write number of points.
	file.write((char*)&pointsToWrite, sizeof(int));

	// Write initialXShift and initialZShift.
	file.write((char*)&currentPointCloud->Metrics.InitialXShift, sizeof(double));
	file.write((char*)&currentPointCloud->Metrics.InitialZShift, sizeof(double));

	// Write adjustment.
	file.write((char*)&currentPointCloud->Metrics.Adjustment[0], sizeof(float));
	file.write((char*)&currentPointCloud->Metrics.Adjustment[1], sizeof(float));
	file.write((char*)&currentPointCloud->Metrics.Adjustment[2], sizeof(float));

	// Write min and max.
	file.write((char*)&currentPointCloud->Metrics.Min[0], sizeof(float));
	file.write((char*)&currentPointCloud->Metrics.Min[1], sizeof(float));
	file.write((char*)&currentPointCloud->Metrics.Min[2], sizeof(float));

	file.write((char*)&currentPointCloud->Metrics.Max[0], sizeof(float));
	file.write((char*)&currentPointCloud->Metrics.Max[1], sizeof(float));
	file.write((char*)&currentPointCloud->Metrics.Max[2], sizeof(float));

	// Write root node size.
	//file.write((char*)&currentPointCloud->getSearchOctree()->root->size, sizeof(float));

	for (size_t j = 0; j < currentPointCloud->getPointCount(); j++)
	{
		// Write point only if it is not "deleted".
		if (currentPointCloud->vertexInfo[j].position[0] != DELETED_POINTS_COORDINATE &&
			currentPointCloud->vertexInfo[j].position[1] != DELETED_POINTS_COORDINATE &&
			currentPointCloud->vertexInfo[j].position[2] != DELETED_POINTS_COORDINATE)
		{
			// Write position of point.
			file.write((char*)&currentPointCloud->vertexInfo[j].position[0], sizeof(float));
			file.write((char*)&currentPointCloud->vertexInfo[j].position[1], sizeof(float));
			file.write((char*)&currentPointCloud->vertexInfo[j].position[2], sizeof(float));

			// Write color of point.
			file.write((char*)&currentPointCloud->vertexInfo[j].color[0], sizeof(unsigned char));
			file.write((char*)&currentPointCloud->vertexInfo[j].color[1], sizeof(unsigned char));
			file.write((char*)&currentPointCloud->vertexInfo[j].color[2], sizeof(unsigned char));
			file.write((char*)&currentPointCloud->vertexInfo[j].color[3], sizeof(unsigned char));
		}
	}

	file.close();
}

void highlightDeletedPointsFunction(pointCloud* pointCloud, ID3D11DeviceContext* ctx)
{
	return;
	std::vector<int> pointsToHighlight;
	glm::vec3 localDeletionSpherePosition = glm::inverse(glm::transpose(pointCloud->worldMatrix)) * glm::vec4(deletionSpherePosition, 1.0f);
	if (pointCloud->getSearchOctree()->isInOctreeBound(localDeletionSpherePosition, deletionSphereSize))
		pointCloud->getSearchOctree()->searchForObjects(localDeletionSpherePosition, deletionSphereSize, pointsToHighlight);
	
	//LOG.Add("pointsToHighlight.size():" + std::to_string(pointsToHighlight.size()), "11");

	int minIndex = INT_MAX;
	int maxIndex = INT_MIN;

	// In case user switched off highlighting of points we need to clean up color data.
	if (!highlightDeletedPoints && pointCloud->lastHighlightedPoints.size() != 0)
	{
		minIndex = 0;
		maxIndex = int(pointCloud->vertexInfo.size());

		for (size_t i = 0; i < pointCloud->vertexInfo.size(); i++)
		{
			pointCloud->vertexInfo[i].color[0] = pointCloud->originalData[i].color[0];
			pointCloud->vertexInfo[i].color[1] = pointCloud->originalData[i].color[1];
			pointCloud->vertexInfo[i].color[2] = pointCloud->originalData[i].color[2];
		}

		pointCloud->lastHighlightedPoints.clear();
	}

	pointCloud->highlightStep += 10;
	if (pointCloud->highlightStep > 100)
		pointCloud->highlightStep = 0;

	D3D11_BOX box{};
	box.left = 0;
	box.right = 0 + pointCloud->getPointCount() * kVertexSize;
	box.top = 0;
	box.bottom = 1;
	box.front = 0;
	box.back = 1;

	for (size_t i = 0; i < pointCloud->lastHighlightedPoints.size(); i++)
	{
		if (pointCloud->lastHighlightedPoints[i] > maxIndex)
			maxIndex = pointCloud->lastHighlightedPoints[i];

		if (pointCloud->lastHighlightedPoints[i] < minIndex)
			minIndex = pointCloud->lastHighlightedPoints[i];

		pointCloud->vertexInfo[pointCloud->lastHighlightedPoints[i]].color[0] = pointCloud->originalData[pointCloud->lastHighlightedPoints[i]].color[0];
		pointCloud->vertexInfo[pointCloud->lastHighlightedPoints[i]].color[1] = pointCloud->originalData[pointCloud->lastHighlightedPoints[i]].color[1];
		pointCloud->vertexInfo[pointCloud->lastHighlightedPoints[i]].color[2] = pointCloud->originalData[pointCloud->lastHighlightedPoints[i]].color[2];
	}
	pointCloud->lastHighlightedPoints = pointsToHighlight;

	for (size_t i = 0; i < pointsToHighlight.size(); i++)
	{
		if (pointsToHighlight[i] > maxIndex)
			maxIndex = pointsToHighlight[i];

		if (pointsToHighlight[i] < minIndex)
			minIndex = pointsToHighlight[i];

		unsigned char originalR = pointCloud->originalData[pointsToHighlight[i]].color[0];
		unsigned char originalG = pointCloud->originalData[pointsToHighlight[i]].color[1];
		unsigned char originalB = pointCloud->originalData[pointsToHighlight[i]].color[2];

		unsigned char invertedR = 255 - originalR;
		unsigned char invertedG = 255 - originalG;
		unsigned char invertedB = 255 - originalB;

		pointCloud->vertexInfo[pointsToHighlight[i]].color[0] = originalR + unsigned char((invertedR - originalR) * (pointCloud->highlightStep / 100.0f));
		pointCloud->vertexInfo[pointsToHighlight[i]].color[1] = originalG + unsigned char((invertedG - originalG) * (pointCloud->highlightStep / 100.0f));
		pointCloud->vertexInfo[pointsToHighlight[i]].color[2] = originalB + unsigned char((invertedB - originalB) * (pointCloud->highlightStep / 100.0f));
	}

	if (minIndex == INT_MAX || maxIndex == INT_MIN)
		return;
	
	box.left = minIndex * kVertexSize;
	box.right = minIndex * kVertexSize + (maxIndex - minIndex + 1) * kVertexSize;

	D3D11_BOX dbox{};
	dbox.left = 0;
	dbox.right = (maxIndex - minIndex + 1) * kVertexSize;
	dbox.top = 0;
	dbox.bottom = 1;
	dbox.front = 0;
	dbox.back = 1;

	if (box.right / kVertexSize > pointCloud->vertexInfo.size())
		return;
	
	ctx->UpdateSubresource(pointCloud->intermediateVB, 0, &dbox, pointCloud->vertexInfo.data() + minIndex, pointCloud->getPointCount() * kVertexSize, pointCloud->getPointCount() * kVertexSize);
	ctx->CopySubresourceRegion(pointCloud->mainVB, 0, box.left, 0, 0, pointCloud->intermediateVB, 0, &dbox);
}

static DWORD timeLastTimeCall = GetTickCount();
static int iteration = 0;

void UpdatePointsInGPUMem(pointCloud* pointCloud, std::vector<int> &PointnsIndexes, ID3D11DeviceContext* ctx)
{
	static int LastModifiedMinIndex = -1;
	static int LastModifiedMaxIndex = -1;
	static int LastModifiedPointsCount = -1;

	int minIndex = INT_MAX;
	int maxIndex = INT_MIN;

	D3D11_BOX box{};
	box.left = 0;
	box.right = 0 + pointCloud->getPointCount() * kVertexSize;
	box.top = 0;
	box.bottom = 1;
	box.front = 0;
	box.back = 1;

	LOG.Add("pointCloud->getSearchOctree()->PointnsInSphere.size(): " + std::to_string(PointnsIndexes.size()), "UpdatePointsInGPUMem");

	if (PointnsIndexes.size() != 0)
		LOG.Add("DrawPointCloud with PointnsInSphere first element: " + std::to_string(PointnsIndexes[0]), "UpdatePointsInGPUMem");

	for (size_t i = 0; i < PointnsIndexes.size(); i++)
	{
		if (PointnsIndexes[i] > maxIndex)
			maxIndex = PointnsIndexes[i];

		if (PointnsIndexes[i] < minIndex)
			minIndex = PointnsIndexes[i];
	}

	LOG.Add("maxIndex: " + std::to_string(maxIndex), "UpdatePointsInGPUMem");
	LOG.Add("minIndex: " + std::to_string(minIndex), "UpdatePointsInGPUMem");

	if (minIndex == INT_MAX || maxIndex == INT_MIN)
		return;

	if (minIndex == LastModifiedMinIndex && maxIndex == LastModifiedMaxIndex && PointnsIndexes.size() == LastModifiedPointsCount)
	{
		if (UNDO_MANAGER.undoActionWasApplied)
		{
			UNDO_MANAGER.undoActionWasApplied = false;
		}
		else
		{
			LOG.Add("minIndex == LastModifiedMinIndex && maxIndex == LastModifiedMaxIndex && PointnsIndexes.size() == LastModifiedPointsCount", "UpdatePointsInGPUMem");
			return;
		}
	}
	LastModifiedMinIndex = minIndex;
	LastModifiedMaxIndex = maxIndex;
	LastModifiedPointsCount = PointnsIndexes.size();

	box.left = minIndex * kVertexSize;
	box.right = minIndex * kVertexSize + (maxIndex - minIndex + 1) * kVertexSize;

	D3D11_BOX dbox{};
	dbox.left = 0;
	dbox.right = (maxIndex - minIndex + 1) * kVertexSize;
	dbox.top = 0;
	dbox.bottom = 1;
	dbox.front = 0;
	dbox.back = 1;

	LOG.Add("min: " + std::to_string(minIndex), "UpdatePointsInGPUMem");
	LOG.Add("dbox.right: " + std::to_string(dbox.right), "UpdatePointsInGPUMem");

	LOG.Add("box.left: " + std::to_string(box.left), "UpdatePointsInGPUMem");
	LOG.Add("box.right: " + std::to_string(box.right), "UpdatePointsInGPUMem");

	if (box.right / kVertexSize > pointCloud->vertexInfo.size())
	{
		LOG.Add("Error ! box.right / kVertexSize > pointCloudToRender->vertexInfo.size()", "UpdatePointsInGPUMem");
		return;
	}

	ctx->UpdateSubresource(pointCloud->intermediateVB, 0, &dbox, pointCloud->vertexInfo.data() + minIndex, pointCloud->getPointCount() * kVertexSize, pointCloud->getPointCount() * kVertexSize);
	ctx->CopySubresourceRegion(pointCloud->mainVB, 0, box.left, 0, 0, pointCloud->intermediateVB, 0, &dbox);
}

void ApplyPoindModificationRequest(pointCloud* pointCloud, ID3D11DeviceContext* ctx, ModificationRequest Request)
{
	auto& PointToModify = pointCloud->getSearchOctree()->PointnsInSphere;

	if (PointToModify.empty())
	{
		if (UNDO_MANAGER.undoActionWasApplied)
			UNDO_MANAGER.undoActionWasApplied = false;
		
		return;
	}

	if (Request.TypeOfModification == 0)
	{
		for (size_t i = 0; i < pointCloud->getSearchOctree()->PointnsInSphere.size(); i++)
		{
			pointCloud->vertexInfo[PointToModify[i]].position[0] = DELETED_POINTS_COORDINATE;
			pointCloud->vertexInfo[PointToModify[i]].position[1] = DELETED_POINTS_COORDINATE;
			pointCloud->vertexInfo[PointToModify[i]].position[2] = DELETED_POINTS_COORDINATE;
		}
	}
	else if (Request.TypeOfModification == 1)
	{
		LOG.Add("Request.TypeOfModification == 1 in ApplyPoindModificationRequest", "Classification");

		for (size_t i = 0; i < pointCloud->getSearchOctree()->PointnsInSphere.size(); i++)
		{
			pointCloud->vertexInfo[PointToModify[i]].classification = Request.AdditionalData;
		}
	}
	
	UpdatePointsInGPUMem(pointCloud, pointCloud->getSearchOctree()->PointnsInSphere, ctx);
	PointToModify.clear();
}

static glm::vec3 LastRequestSphereCenter = glm::vec3(-10000.0f);
static float LastRequestSphereSize = -100.0f;
static int LastRequestType = -1;
static float LastRequestAdditionalData = -FLT_MAX;
static DWORD alternativeTime = GetTickCount();

bool IsRequestNew(float* Center, float Size, int TypeOfModification, float AdditionalData)
{
	if (Size <= 0.0f)
	{
		LOG.Add("Sphere size was: " + std::to_string(Size), "ModificationEvents");
		return false;
	}

	glm::vec3 centerOfBrush = glm::vec3(Center[0], Center[1], Center[2]);
	if (LastRequestSphereSize == Size && LastRequestSphereCenter == centerOfBrush &&
		LastRequestType == TypeOfModification && LastRequestAdditionalData == AdditionalData)
	{
		if (UNDO_MANAGER.undoActionWasApplied)
		{
			UNDO_MANAGER.undoActionWasApplied = false;
		}
		else
		{
			LOG.Add("LastRequestSphereSize == Size && LastRequestSphereCenter == centerOfBrush && LastRequestType == TypeOfModification && LastRequestAdditionalData == AdditionalData", "ModificationEvents");
			return false;
		}
	}
	LastRequestSphereSize = Size;
	LastRequestSphereCenter = centerOfBrush;
	LastRequestType = TypeOfModification;
	LastRequestAdditionalData = AdditionalData;

	if (GetTickCount() - timeLastTimeCall < 20)
	{
		if (UNDO_MANAGER.undoActionWasApplied)
		{
			UNDO_MANAGER.undoActionWasApplied = false;
		}

		return false;
	}
	timeLastTimeCall = GetTickCount();

	return true;
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API RequestToDeleteFromUnity(float* center, float size, int* RequestID)
{
	if (!IsRequestNew(center, size, 0, 0.0f))
		return;
	
	ModificationRequest NewRequest;
	NewRequest.Center = glm::vec3(center[0], center[1], center[2]);
	NewRequest.Size = size;
	NewRequest.TypeOfModification = 0;
	NewRequest.AdditionalData = 0.0f;
	NewRequest.ID = APPLICATION.GetUniqueHexID();

	LOG.Add("NewRequest.ID = " + NewRequest.ID, "NewRequest.ID");
	for (size_t i = 0; i < 24; i++)
	{
		RequestID[i] = int(NewRequest.ID[i]);
	}

	ModificationRequests.push_back(NewRequest);
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API RequestToModifyPointsFromUnity(float* Center, float Size, int TypeOfModification, float AdditionalData, int* RequestID)
{
	if (!IsRequestNew(Center, Size, TypeOfModification, AdditionalData))
		return;

	ModificationRequest NewRequest;
	NewRequest.Center = glm::vec3(Center[0], Center[1], Center[2]);
	NewRequest.Size = Size;
	NewRequest.TypeOfModification = TypeOfModification;
	NewRequest.AdditionalData = AdditionalData;
	NewRequest.ID = APPLICATION.GetUniqueHexID();

	LOG.Add("NewRequest.ID = " + NewRequest.ID, "NewRequest.ID");
	for (size_t i = 0; i < 24; i++)
	{
		RequestID[i] = int(NewRequest.ID[i]);
	}

	ModificationRequests.push_back(NewRequest);
}

extern "C" int UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API GetRequestResult(char* RequestID)
{
	for (size_t i = 0; i < RenderingThreadLocalCopy.size(); i++)
	{
		if (RenderingThreadLocalCopy[i].ID == RequestID)
		{
			return RenderingThreadLocalCopy[i].Result;
		}
	}

	return -1;
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API RequestClassificationVisualizationFromUnity(bool VisualizeClassification)
{
	FloatsToSync["VisualizeClassification"] = VisualizeClassification;
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API AddClassificationEntry(int Type, float R, float G, float B)
{
	if (ClassificationCount == CLASSIFICATION_MAX_COUNT)
		return;

	ConstBufferData->ClassToColor[ClassificationCount] = glm::vec4(Type, R, G, B);

	ClassificationCount++;
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API ClearClassificationTable()
{
	for (size_t i = 0; i < CLASSIFICATION_MAX_COUNT; i++)
	{
		ConstBufferData->ClassToColor[i] = glm::vec4(0.0f);
	}

	ClassificationCount = 0;
}

extern "C" bool UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UpdateClassificationEntry(int Type, float R, float G, float B)
{
	for (size_t i = 0; i < CLASSIFICATION_MAX_COUNT; i++)
	{
		if (ConstBufferData->ClassToColor[i].x == Type)
		{
			ConstBufferData->ClassToColor[i] = glm::vec4(Type, R, G, B);
			return true;
		}
	}

	return false;
}

extern "C" int UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API RequestOctreeDebugMaxNodeDepthFromUnity()
{
	return pointClouds[0]->getSearchOctree()->getDebugMaxNodeDepth();
}

extern "C" int UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API RequestOctreeBoundsCountFromUnity()
{
	return int(pointClouds[0]->getSearchOctree()->debugGetNodesPositionAndSize().size());
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API RequestOctreeBoundsFromUnity(float* arrayToFill)
{
	auto arrayToSend = pointClouds[0]->getSearchOctree()->debugGetNodesPositionAndSize();

	for (size_t i = 0; i < arrayToSend.size(); i++)
	{
		arrayToFill[i * 5] = float(arrayToSend[i].center.x);
		arrayToFill[i * 5 + 1] = float(arrayToSend[i].center.y);
		arrayToFill[i * 5 + 2] = float(arrayToSend[i].center.z);
		arrayToFill[i * 5 + 3] = arrayToSend[i].size;
		arrayToFill[i * 5 + 4] = float(arrayToSend[i].depth);
	}
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API RequestPointCloudAdjustmentFromUnity(double* adjustment, char* pointCloudID)
{
	pointCloud* currentPointCloud = getPointCloud(pointCloudID);
	if (currentPointCloud == nullptr)
		return;

	adjustment[0] = currentPointCloud->Metrics.Adjustment.x;
	adjustment[1] = currentPointCloud->Metrics.Adjustment.y;
	adjustment[2] = currentPointCloud->Metrics.Adjustment.z;

	adjustment[3] = currentPointCloud->Metrics.InitialXShift;
	adjustment[4] = currentPointCloud->Metrics.InitialZShift;

	if (currentPointCloud->NumPy == nullptr)
	{
		adjustment[5] = currentPointCloud->Metrics.RawMin.x;
		adjustment[7] = currentPointCloud->Metrics.RawMin.z;

		adjustment[5] *= currentPointCloud->loadedFrom->header.x_scale_factor;
	    adjustment[5] += currentPointCloud->loadedFrom->header.x_offset;

		adjustment[6] = currentPointCloud->Metrics.Min.y;
		//adjustment[6] = currentPointCloud->Metrics.RawMin.y;
		//adjustment[6] *= currentPointCloud->loadedFrom->header.z_scale_factor;
		//adjustment[6] += currentPointCloud->loadedFrom->header.z_offset;

		adjustment[7] *= currentPointCloud->loadedFrom->header.y_scale_factor;
		adjustment[7] += currentPointCloud->loadedFrom->header.y_offset;

		//std::swap(adjustment[6], adjustment[7]);

		adjustment[8] = currentPointCloud->Metrics.RawMax.x;

		adjustment[8] *= currentPointCloud->loadedFrom->header.x_scale_factor;
		adjustment[8] += currentPointCloud->loadedFrom->header.x_offset;

		adjustment[9] = currentPointCloud->Metrics.Max.y;
		//adjustment[9] = currentPointCloud->Metrics.RawMax.y;
		//adjustment[9] *= currentPointCloud->loadedFrom->header.z_scale_factor;
		//adjustment[9] += currentPointCloud->loadedFrom->header.z_offset;

		adjustment[10] = currentPointCloud->Metrics.RawMax.z;
		adjustment[10] *= currentPointCloud->loadedFrom->header.y_scale_factor;
		adjustment[10] += currentPointCloud->loadedFrom->header.y_offset;

		//std::swap(adjustment[9], adjustment[10]);
	}
	else
	{
		adjustment[5] = currentPointCloud->Metrics.Min.x;
		adjustment[6] = currentPointCloud->Metrics.Min.y;
		adjustment[7] = currentPointCloud->Metrics.Min.z;

		adjustment[8] = currentPointCloud->Metrics.Max.x;
		adjustment[9] = currentPointCloud->Metrics.Max.y;
		adjustment[10] = currentPointCloud->Metrics.Max.z;
	}

	adjustment[11] = currentPointCloud->EPSG;
	adjustment[12] = currentPointCloud->getApproximateGroundLevel();

	std::string Result;
	for (size_t i = 0; i < 13; i++)
	{
		Result += std::to_string(i) + ": ";
		Result += std::to_string(adjustment[i]);
		Result += '\n';
	}

	LOG.Add(Result, "RequestPointCloudAdjustmentFromUnity");
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API RequestPointCloudUTMZoneFromUnity(int* UTMZone, int* North, char* pointCloudID)
{
	UTMZone[0] = 0;
	North[0] = 0;

	pointCloud* currentPointCloud = getPointCloud(pointCloudID);
	if (currentPointCloud == nullptr)
		return;

	if (currentPointCloud->UTMZone.size() == 0 || currentPointCloud->UTMZone.size() == 0)
	{
		LOG.Add("RequestPointCloudUTMZoneFromUnity was called but \"UTMZone\" or/and \"North\" was empty!", "DLL_ERRORS");
	}
	else if (currentPointCloud->UTMZone.size() == 1)
	{
		UTMZone[0] = atoi(currentPointCloud->UTMZone.substr(0, 1).c_str());
	}
	else if (currentPointCloud->UTMZone.size() > 2)
	{
		UTMZone[0] = atoi(currentPointCloud->UTMZone.substr(0, 2).c_str());
		North[0] = currentPointCloud->UTMZone.substr(2, 1) == "N" ? 1 : 0;
	}
}

static void UNITY_INTERFACE_API OnGraphicsDeviceEvent(UnityGfxDeviceEventType eventType);

static IUnityInterfaces* s_UnityInterfaces = NULL;
static IUnityGraphics* s_Graphics = NULL;

extern "C" void	UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces* unityInterfaces)
{
	s_UnityInterfaces = unityInterfaces;
	s_Graphics = s_UnityInterfaces->Get<IUnityGraphics>();
	s_Graphics->RegisterDeviceEventCallback(OnGraphicsDeviceEvent);

	OnGraphicsDeviceEvent(kUnityGfxDeviceEventInitialize);
	// Write file with version
	LOG.Add(currentVersion, "version");
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginUnload()
{
	s_Graphics->UnregisterDeviceEventCallback(OnGraphicsDeviceEvent);
}

static void createConstantBuffer(ID3D11Buffer** Buffer)
{
	LOG.Add("createConstantBuffer begin.", "AddVariableToShader");
	D3D11_BUFFER_DESC desc;
	memset(&desc, 0, sizeof(desc));

	desc.Usage = D3D11_USAGE_DEFAULT;
#ifdef USE_QUADS_NOT_POINTS
	desc.ByteWidth = 64 * 3;
#else
	desc.ByteWidth = sizeof(MyConstBuffer); // You must set the ByteWidth value of D3D11_BUFFER_DESC in multiples of 16.
#endif
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = 0;
	auto result = GPU.getDevice()->CreateBuffer(&desc, NULL, Buffer);

	if (*Buffer == nullptr)
	{
		LOG.Add("*Buffer == nullptr", "AddVariableToShader");
		LOG.Add("result: " + std::to_string(result), "AddVariableToShader");
	}

	LOG.Add("createConstantBuffer end.", "AddVariableToShader");
}

static void CreateResources()
{
	createConstantBuffer(&m_CB);

	// shaders
	ID3DBlob* pVSBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	HRESULT hr;
	// C:/Users/kberegovyi/Downloads/ARNav2_compute_08.16.2021/Assets/Plugins/PointCloudPlugin/

	long VSLen = 0;
	//void* src = LoadShaderFile(L"C:/Users/kandr/OneDrive/University/ocean_lab/PointCloudPlugin/shaders/VS_POINTS.hlsl", "hlsl", &VSLen);
	hr = D3DCompile(VertexShaderSource.c_str(),
		VertexShaderSource.length(),
		nullptr, nullptr, nullptr, "VS", "vs_5_0",
		D3DCOMPILE_ENABLE_STRICTNESS, 0, &pVSBlob, &errorBlob);

	/*hr = D3DCompileFromFile(L"C:/Users/kandr/OneDrive/University/ocean_lab/PointCloudPlugin/shaders/VS_POINTS.hlsl", nullptr,
							D3D_COMPILE_STANDARD_FILE_INCLUDE,
							"VS", "vs_5_0",
							D3DCOMPILE_ENABLE_STRICTNESS, 0, &pVSBlob, &errorBlob);*/

	LOG.Add("shaderResult: " + std::system_category().message(hr), "computeShader");
	if (errorBlob)
	{
		LOG.Add("shaderResult: " + std::string((char*)errorBlob->GetBufferPointer()), "computeShader");
		//OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		errorBlob->Release();
	}

	//hr = GPU.getDevice()->CreateVertexShader(kVertexShaderCode, sizeof(kVertexShaderCode), nullptr, &m_VertexShader);
	hr = GPU.getDevice()->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_VertexShader);

	if (FAILED(hr))
		LOG.Add("Failed to create vertex shader.", "computeShader");
	hr = GPU.getDevice()->CreatePixelShader(kPixelShaderCode, sizeof(kPixelShaderCode), nullptr, &m_PixelShader);
	if (FAILED(hr))
		LOG.Add("Failed to create pixel shader.", "computeShader");

	// input layout
	if (m_VertexShader)
	{
		D3D11_INPUT_ELEMENT_DESC s_DX11InputElementDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "CLASSIFICATION", 0,  DXGI_FORMAT_R16_UINT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		//GPU.getDevice()->CreateInputLayout(s_DX11InputElementDesc, 3, kVertexShaderCode, sizeof(kVertexShaderCode), &m_InputLayout);
		GPU.getDevice()->CreateInputLayout(s_DX11InputElementDesc, 3, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_InputLayout);
	}

	// render states
	D3D11_RASTERIZER_DESC rsdesc;
	memset(&rsdesc, 0, sizeof(rsdesc));
	rsdesc.FillMode = D3D11_FILL_SOLID;
	rsdesc.CullMode = D3D11_CULL_NONE;
	rsdesc.DepthClipEnable = TRUE;
	GPU.getDevice()->CreateRasterizerState(&rsdesc, &m_RasterState);

	D3D11_DEPTH_STENCIL_DESC dsdesc;
	memset(&dsdesc, 0, sizeof(dsdesc));
	dsdesc.DepthEnable = TRUE;
	dsdesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsdesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
	GPU.getDevice()->CreateDepthStencilState(&dsdesc, &m_DepthState);

	D3D11_BLEND_DESC bdesc;
	memset(&bdesc, 0, sizeof(bdesc));
	bdesc.RenderTarget[0].BlendEnable = FALSE;
	bdesc.RenderTarget[0].RenderTargetWriteMask = 0xF;
	GPU.getDevice()->CreateBlendState(&bdesc, &m_BlendState);

#ifdef USE_COMPUTE_SHADER
	// Compute shader.
	// C:/Users/kberegovyi/Downloads/ARNav2_compute_08.16.2021/Assets/Plugins/PointCloudPlugin/computeShader.hlsl
	// C:/Users/kandr/OneDrive/University/ocean_lab/PointCloudPlugin/shaders/computeShader.hlsl

	//"C:/Users/kberegovyi/Downloads/ARNav2_compute_08.16.2021/Assets/Plugins/PointCloudPlugin/computeShader_DELETE.hlsl"
	
	//compileAndCreateComputeShader(GPU.getDevice(), (BYTE*)g_CSMain, &computeShader);
	// Kindr
	// kandr
	compileAndCreateComputeShader(GPU.getDevice(), "C:/Users/Kindr/OneDrive/University/ocean_lab/PointCloudPlugin/shaders/computeShader_DELETE_NEW.hlsl", &computeShader);

	//outliers_CS = new CShader("C:/Users/kandr/OneDrive/University/ocean_lab/PointCloudPlugin/shaders/computeShader_Outliers.hlsl");
	compileAndCreateComputeShader(GPU.getDevice(), (BYTE*)g_CSOutLiers, &outliers_CS);
	

	//compileAndCreateComputeShader(GPU.getDevice(), "C:/Users/kberegovyi/Downloads/ARNav2_compute_08.16.2021/Assets/Plugins/PointCloudPlugin/computeShader_Outliers.hlsl", &outliers_CS);
#endif
}



static int drawCount = 0;
static void DrawPointCloud(pointCloud* pointCloudToRender, bool HighlightDeletedPoints)
{
	if (!pointCloudToRender->wasFullyLoaded)
		return;

	//static int pointsToDraw = 0;
#ifdef USE_COMPUTE_SHADER
	//getComputeShaderResultCounter(GPU.getDevice(), *current_CS_UAV/*result_CS_UAV*/);
	//runMyComputeShader();

	//if (pointCloudToRender->inputPoints_CS_SRV == nullptr)
	//{
	//	deletionSpherePosition = glm::vec3(0.0f);
	//	deletionSphereSize = 0.0f;
	//	runMyDeleteComputeShader(pointCloudToRender);
	//	LOG.Add("allPointsData_CS_SRV == nullptr updated", "computeShader");
	//	if (pointCloudToRender->current_CS_UAV == nullptr)
	//		LOG.Add("current_CS_UAV == nullptr", "computeShader");
	//	//pointCloudToRender->pointsToDraw = getComputeShaderResultCounter(GPU.getDevice(), *pointCloudToRender->current_CS_UAV/*result_CS_UAV*/);
	//	//LOG.Add("getComputeShaderResultCounter(GPU.getDevice(), *current_CS_UAV/*result_CS_UAV*/);", "computeShader");
	//	//pointCloudToRender->currentInputPoints_CS_SRV = &pointCloudToRender->result_CS_SRV;
	//	//pointCloudToRender->current_CS_UAV = &pointCloudToRender->resultSecond_CS_UAV;
	//}
#endif

	if (pointCloudToRender->getSearchOctree()->PointnsInSphere.size() != 0)
	{
		LOG.Add("DrawPointCloud begin with PointnsInSphere size: " + std::to_string(pointCloudToRender->getSearchOctree()->PointnsInSphere.size()), "deleteEvents");
	}

	glm::mat4 glmWorldMatrix = pointCloudToRender->worldMatrix;
	glm::mat4 glmViewMatrix;
	glm::mat4 glmProjectionMatrix; 
	//LOG.Add("screenIndex in DrawPointCloud: " + std::to_string(internalScreenIndex), "screens");

	if (internalScreenIndex == -1 || viewProjectionMatrices.find(internalScreenIndex) == viewProjectionMatrices.end())
	{
		glmViewMatrix = glm::make_mat4(worldToViewMatrix);
		glmProjectionMatrix = glm::make_mat4(projectionMatrix);
	}
	else
	{
		glmViewMatrix = glm::make_mat4(viewProjectionMatrices[internalScreenIndex]);
		glmProjectionMatrix = glm::make_mat4(&viewProjectionMatrices[internalScreenIndex][16]);

		if (viewProjectionMatricesBuffers.find(internalScreenIndex) == viewProjectionMatricesBuffers.end())
		{
			LOG.Add("adding entry in viewProjectionMatricesBuffers", "screens");
			LOG.Add("viewProjectionMatricesBuffers.size(): " + std::to_string(viewProjectionMatricesBuffers.size()), "screens");

			createConstantBuffer(&viewProjectionMatricesBuffers[internalScreenIndex]);
		}
	}

	glmWorldMatrix = glm::transpose(glmWorldMatrix);
	glmViewMatrix = glm::transpose(glmViewMatrix);
	glmProjectionMatrix = glm::transpose(glmProjectionMatrix);

	//LOG.Add("ID: " + pointCloudToRender->ID, "renderLog");
	//LOG.Add("x: " + std::to_string(glmWorldMatrix[0][0]) + " y: " + std::to_string(glmWorldMatrix[0][1]) +
	//								 " z: " + std::to_string(glmWorldMatrix[0][2]) + " w: " + std::to_string(glmWorldMatrix[0][3]), "renderLog");

	//LOG.Add("x: " + std::to_string(glmWorldMatrix[1][0]) + " y: " + std::to_string(glmWorldMatrix[1][1]) +
	//	" z: " + std::to_string(glmWorldMatrix[1][2]) + " w: " + std::to_string(glmWorldMatrix[1][3]), "renderLog");

	//LOG.Add("x: " + std::to_string(glmWorldMatrix[2][0]) + " y: " + std::to_string(glmWorldMatrix[2][1]) +
	//	" z: " + std::to_string(glmWorldMatrix[2][2]) + " w: " + std::to_string(glmWorldMatrix[2][3]), "renderLog");

	//LOG.Add("x: " + std::to_string(glmWorldMatrix[3][0]) + " y: " + std::to_string(glmWorldMatrix[3][1]) +
	//	" z: " + std::to_string(glmWorldMatrix[3][2]) + " w: " + std::to_string(glmWorldMatrix[3][3]), "renderLog");

	// WHY ??))
	//if (frustumCulling)
	//{
		//float distance;
		//for (int p = 0; p < 6; p++)
		//{
		//	distance = frustum[p][0] * glmWorldMatrix[3][0] + frustum[p][1] * glmWorldMatrix[3][1] + frustum[p][2] * glmWorldMatrix[3][2] + frustum[p][3];
		//	if (distance <= -pointCloudToRender->getSearchOctree()->root->nodeAABB.size)
		//	{
		//		/*LOG.Add("distance: " + std::to_string(distance), "renderTest");
		//		LOG.Add("nodeAABB.size: " + std::to_string(pointCloudToRender->getSearchOctree()->root->nodeAABB.size), "renderTest");

		//		LOG.Add("glmWorldMatrix[3][0]: " + std::to_string(glmWorldMatrix[3][0]), "renderTest");
		//		LOG.Add("glmWorldMatrix[3][1]: " + std::to_string(glmWorldMatrix[3][1]), "renderTest");
		//		LOG.Add("glmWorldMatrix[3][2]: " + std::to_string(glmWorldMatrix[3][2]), "renderTest");*/
		//		return;
		//	}
		//}
	//}

	ID3D11DeviceContext* ctx = NULL;
	GPU.getDevice()->GetImmediateContext(&ctx);

	ctx->OMSetDepthStencilState(m_DepthState, 0);
	ctx->RSSetState(m_RasterState);
	ctx->OMSetBlendState(m_BlendState, NULL, 0xFFFFFFFF);

	glm::mat4 finalMatrix = glmProjectionMatrix * glmViewMatrix * glmWorldMatrix;
	LOG.Add("finalMatrix :" + mat4ToString(finalMatrix), "camera");
	LOG.Add("glmViewMatrix :" + mat4ToString(glmViewMatrix), "camera");
	LOG.Add("glmWorldMatrix :" + mat4ToString(glmWorldMatrix), "camera");

	//LOG.Add("m_CB update begin.", "AddVariableToShader");
#ifdef USE_QUADS_NOT_POINTS
	ctx->UpdateSubresource(m_CB, 0, NULL, glm::value_ptr(glmWorldMatrix), 64, 0);
	ctx->UpdateSubresource(m_CB, 1, NULL, glm::value_ptr(glmViewMatrix), 128, 0);
	ctx->UpdateSubresource(m_CB, 2, NULL, glm::value_ptr(glmProjectionMatrix), 192, 0);
#else
	if (internalScreenIndex != -1 || viewProjectionMatricesBuffers.find(internalScreenIndex) == viewProjectionMatricesBuffers.end())
	{
		ConstBufferData->finalMat = finalMatrix;
		ConstBufferData->glmViewMatrix = glmViewMatrix;
		ConstBufferData->worldMatrix = glmWorldMatrix;
		ConstBufferData->additionalFloat.x = FloatsToSync["VisualizeClassification"];

		ctx->UpdateSubresource(m_CB, 0, NULL, ConstBufferData, 0, 0);

		//ctx->UpdateSubresource(m_CB, 0, NULL, glm::value_ptr(finalMatrix), 64, 64);
		//ctx->UpdateSubresource(m_CB, 1, NULL, glm::value_ptr(finalMatrix), 64, 64);
		//ctx->UpdateSubresource(m_CB, 2, NULL, glm::value_ptr(glmViewMatrix), 64, 64);

		//glm::vec4 DataStorage = glm::vec4(0.0f);
		//DataStorage.x = 0.5f/*FloatsToSync["FirstShaderFloat"]*/;
		//ctx->UpdateSubresource(m_CB, 3, NULL, glm::value_ptr(DataStorage), 16, 0);

		//LOG.Add("FloatsToSync[\"FirstShaderFloat\"]: " + std::to_string(FloatsToSync["FirstShaderFloat"]), "AddVariableToShader");
	}
	else
	{
		ctx->UpdateSubresource(viewProjectionMatricesBuffers[internalScreenIndex], 0, NULL, glm::value_ptr(finalMatrix), 64, 0);
		ctx->UpdateSubresource(viewProjectionMatricesBuffers[internalScreenIndex], 1, NULL, glm::value_ptr(glmViewMatrix), 128, 0);
		ctx->UpdateSubresource(viewProjectionMatricesBuffers[internalScreenIndex], 2, NULL, glm::value_ptr(glmViewMatrix), 192, 0);
	}

	//LOG.Add("m_CB update end.", "AddVariableToShader");
	
#endif

	// Set shaders
#ifndef USE_COMPUTE_SHADER
	if (internalScreenIndex != -1 || viewProjectionMatricesBuffers.find(internalScreenIndex) == viewProjectionMatricesBuffers.end())
	{
		ctx->VSSetConstantBuffers(0, 1, &m_CB);
	}
	else
	{
		ctx->VSSetConstantBuffers(0, 1, &viewProjectionMatricesBuffers[internalScreenIndex]);
	}
	
	ctx->VSSetShader(m_VertexShader, NULL, 0);
	ctx->PSSetShader(m_PixelShader, NULL, 0);
#endif
	
	if (!pointCloudToRender->wasInitialized)
	{
		D3D11_BUFFER_DESC desc;
		memset(&desc, 0, sizeof(desc));

#ifdef USE_COMPUTE_SHADER
		//desc.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.ByteWidth = sizeof(VertexData) * pointCloudToRender->vertexInfo.size();
		desc.StructureByteStride = sizeof(VertexData);
#else
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = UINT(pointCloudToRender->vertexInfo.size() * kVertexSize);
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
#endif
		GPU.getDevice()->CreateBuffer(&desc, NULL, &pointCloudToRender->mainVB);

#ifndef USE_COMPUTE_SHADER
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = UINT(pointCloudToRender->vertexInfo.size() * kVertexSize);
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		GPU.getDevice()->CreateBuffer(&desc, NULL, &pointCloudToRender->intermediateVB);

#ifdef LOD_SYSTEM
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		for (size_t i = 0; i < pointCloudToRender->LODs.size(); i++)
		{
			desc.ByteWidth = UINT(pointCloudToRender->LODs[i].vertexInfo.size() * 16);
			GPU.getDevice()->CreateBuffer(&desc, NULL, &pointCloudToRender->LODs[i].VB);
		}
#endif
		
#endif
		ctx->UpdateSubresource(pointCloudToRender->mainVB, 0, nullptr, pointCloudToRender->vertexInfo.data(), pointCloudToRender->getPointCount() * kVertexSize, pointCloudToRender->getPointCount() * kVertexSize);
		LOG.Add("copy data to vertex buffer, vertexInfo size: " + std::to_string(pointCloudToRender->vertexInfo.size()), "OctreeEvents");

#ifdef USE_QUADS_NOT_POINTS
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = UINT(pointCloudToRender->vertexInfo.size() / 6 * 12);
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.StructureByteStride = sizeof(glm::vec3);
		GPU.getDevice()->CreateBuffer(&desc, NULL, &pointCloudToRender->pointPositionsVB);

		std::vector<glm::vec3> temp;
		temp.resize(pointCloudToRender->vertexInfo.size() / 6);
		for (size_t i = 0; i < pointCloudToRender->vertexInfo.size() / 6; i++)
		{
			temp[i].x = float(i);
		}
		ctx->UpdateSubresource(pointCloudToRender->pointPositionsVB, 0, nullptr, temp.data(), pointCloudToRender->getPointCount() / 6 * sizeof(glm::vec3), pointCloudToRender->getPointCount() / 6 * sizeof(glm::vec3));
#endif


#ifndef USE_COMPUTE_SHADER
#ifdef LOD_SYSTEM
		for (size_t i = 0; i < pointCloudToRender->LODs.size(); i++)
		{
			ctx->UpdateSubresource(pointCloudToRender->LODs[i].VB, 0, nullptr, pointCloudToRender->LODs[i].vertexInfo.data(), UINT(pointCloudToRender->LODs[i].vertexInfo.size() * kVertexSize), UINT(pointCloudToRender->LODs[i].vertexInfo.size() * kVertexSize));
		}
#endif
#endif
		pointCloudToRender->wasInitialized = true;
	}

	//Sleep(5);
	//if (localHighlightDeletedPoints)
		highlightDeletedPointsFunction(pointCloudToRender, ctx);
	//onDrawDeletePointsinGPUMem(pointCloudToRender, ctx, HighlightDeletedPoints);

#ifdef USE_COMPUTE_SHADER

	if (pointCloudToRender->current_CS_SRV == nullptr)
	{
		deletionSpherePosition = glm::vec3(0.0f);
		deletionSphereSize = 0.0f;
		runMyDeleteComputeShader(pointCloudToRender);
		LOG.Add("current_CS_SRV == nullptr updated", "computeShader");
	}

	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	ctx->VSSetConstantBuffers(0, 1, &m_CB);
	ctx->VSSetShaderResources(0, 1, pointCloudToRender->current_CS_SRV/*&result_CS_SRV*/);
	ctx->VSSetShader(m_VertexShader, NULL, 0);
	ctx->PSSetShader(m_PixelShader, NULL, 0);
	//int pointsToDraw = getComputeShaderResultCounter(GPU.getDevice(), *current_CS_UAV/*result_CS_UAV*/);

	/*if (pointCloudToRender->deletionOccuredThisFrame)
	{
		pointCloudToRender->pointsToDraw = getComputeShaderResultCounter(GPU.getDevice(), *pointCloudToRender->current_CS_UAV);

		pointCloudToRender->currentBuffer_CS = pointCloudToRender->currentBuffer_CS == &pointCloudToRender->resultBuffer_CS ?
																					   &pointCloudToRender->resultBufferSecond_CS : &pointCloudToRender->resultBuffer_CS;

		pointCloudToRender->currentInputPoints_CS_SRV = pointCloudToRender->current_CS_SRV == &pointCloudToRender->result_CS_SRV ? &pointCloudToRender->resultSecond_CS_SRV : &pointCloudToRender->result_CS_SRV;

		pointCloudToRender->current_CS_SRV = pointCloudToRender->current_CS_SRV == &pointCloudToRender->result_CS_SRV ?
																				   &pointCloudToRender->resultSecond_CS_SRV : &pointCloudToRender->result_CS_SRV;
		pointCloudToRender->current_CS_UAV = pointCloudToRender->current_CS_UAV == &pointCloudToRender->result_CS_UAV ?
																				   &pointCloudToRender->resultSecond_CS_UAV : &pointCloudToRender->result_CS_UAV;

		pointCloudToRender->deletionOccuredThisFrame = false;
	}*/

	//LOG.Add("pointsToDraw: " + std::to_string(pointCloudToRender->pointsToDraw), "computeShader");
	

	if (pointCloudToRender->pointsToDraw != 0 && pointCloudToRender->pointsToDraw <= pointCloudToRender->getPointCount())
	{
		float distanceToCamera = glm::distance(glm::vec3(glmWorldMatrix[3][0], glmWorldMatrix[3][1], glmWorldMatrix[3][2]), glm::vec3(glmViewMatrix[3][0], glmViewMatrix[3][1], glmViewMatrix[3][2]));
#ifdef LOD_SYSTEM
		if (!LODSystemActive)
		{
			//LOG.Add("pointCloudToRender->pointsToDraw: " + std::to_string(pointCloudToRender->pointsToDraw), "computeShader");
			ctx->Draw(pointCloudToRender->pointsToDraw, 0);
		}
		else
		{
			for (size_t i = 0; i < pointCloud::LODSettings.size(); i++)
			{
				if (distanceToCamera <= pointCloud::LODSettings[i].maxDistance)
				{
					ctx->Draw(UINT(pointCloudToRender->LODs[i].vertexInfo.size()), 0);
					break;
				}
			}
		}
#else
		ctx->Draw(pointCloudToRender->pointsToDraw, 0);
#endif
	}

#else
	ctx->IASetInputLayout(m_InputLayout);
#ifdef USE_QUADS_NOT_POINTS
	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
#else
	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
#endif
	
	UINT stride = kVertexSize;
	UINT offset = 0;

	float distanceToCamera = glm::distance(glm::vec3(glmWorldMatrix[3][0], glmWorldMatrix[3][1], glmWorldMatrix[3][2]), glm::vec3(glmViewMatrix[3][0], glmViewMatrix[3][1], glmViewMatrix[3][2]));
#ifdef LOD_SYSTEM
	if (!LODSystemActive)
	{
		ctx->IASetVertexBuffers(0, 1, &pointCloudToRender->mainVB, &stride, &offset);
		ctx->Draw(pointCloudToRender->getPointCount(), 0);
	}
	else
	{
		for (size_t i = 0; i < pointCloud::LODSettings.size(); i++)
		{
			if (distanceToCamera <= pointCloud::LODSettings[i].maxDistance)
			{
				ctx->IASetVertexBuffers(0, 1, &pointCloudToRender->LODs[i].VB, &stride, &offset);
				ctx->Draw(UINT(pointCloudToRender->LODs[i].vertexInfo.size()), 0);
				break;
			}
		}
	}
#else
	ctx->IASetVertexBuffers(0, 1, &pointCloudToRender->mainVB, &stride, &offset);

#ifdef USE_QUADS_NOT_POINTS
	ctx->IASetVertexBuffers(1, 1, &pointCloudToRender->pointPositionsVB, &stride, &offset);
	ctx->Draw(pointCloudToRender->getPointCount() / 6, 0);
#else
	ctx->Draw(pointCloudToRender->getPointCount(), 0);
#endif

#endif
#endif

	ctx->Release();
}

static void ProcessDeviceEvent(UnityGfxDeviceEventType type, IUnityInterfaces* interfaces)
{
	switch (type)
	{
		case kUnityGfxDeviceEventInitialize:
		{
			GPU.initialize(interfaces);
			//IUnityGraphicsD3D11* d3d = interfaces->Get<IUnityGraphicsD3D11>();
			//m_Device = d3d->GetDevice();
			CreateResources();
			break;
		}

		case kUnityGfxDeviceEventShutdown:
		{
			//ReleaseResources();
			break;
		}
	}
}

//static RenderAPI* s_CurrentAPI = NULL;
static UnityGfxRenderer s_DeviceType = kUnityGfxRendererNull;

static void UNITY_INTERFACE_API OnGraphicsDeviceEvent(UnityGfxDeviceEventType eventType)
{
	// Create graphics API implementation upon initialization
	if (eventType == kUnityGfxDeviceEventInitialize)
	{
		s_DeviceType = s_Graphics->GetRenderer();
		//s_CurrentAPI = CreateRenderAPI(s_DeviceType);
	}

	ProcessDeviceEvent(eventType, s_UnityInterfaces);
	
	// Cleanup graphics API implementation upon shutdown
	if (eventType == kUnityGfxDeviceEventShutdown)
	{
		//delete s_CurrentAPI;
		//s_CurrentAPI = NULL;
		//s_DeviceType = kUnityGfxRendererNull;
	}
}

std::unordered_map<DWORD, int> renderThreads;
static void Render()
{
	//internalScreenIndex++;
	//if (internalScreenIndex > 1)
	//	internalScreenIndex = 0;

	//internalScreenIndex = -1;

	/*DWORD currentThread = GetCurrentThreadId();
	if (renderThreads.find(currentThread) == renderThreads.end())
	{
		renderThreads[currentThread]++;
		LOG.Add("render thread: " + std::to_string(GetCurrentThreadId()), "camera");
	}*/

	LOG.Add("function: " + std::string(__FUNCTION__), "Threads");
	LOG.Add("line: " + std::to_string(__LINE__), "Threads");
	LOG.Add("thread: " + std::to_string(GetCurrentThreadId()), "Threads");
	LOG.Add("=========================================", "Threads");

	static DWORD timeLastTimeMemoryModification = GetTickCount();
	if (/*highlightDeletedPoints &&*/ GetTickCount() - timeLastTimeMemoryModification > 33)
	{
		timeLastTimeMemoryModification = GetTickCount();
		localHighlightDeletedPoints = true;
	}
	
	WorkOnRequests();

	for (size_t i = 0; i < pointClouds.size(); i++)
	{
		DrawPointCloud(pointClouds[i], localHighlightDeletedPoints);
	}
	
	THREAD_POOL.Update();
}

extern "C" bool UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API updateWorldMatrix(void* worldMatrix, char* pointCloudID) 
{
	pointCloud* currentPointCloud = getPointCloud(pointCloudID);
	if (currentPointCloud == nullptr)
		return false;
	
	for (size_t i = 0; i < 16; i++)
	{
		pointsWorldMatrix[i] = ((float*)(worldMatrix))[i];
	}

	currentPointCloud->worldMatrix = glm::make_mat4(pointsWorldMatrix);
	return true;
}

std::vector<std::vector<float>> recordedViewMatrix;
bool addIfNewView(float* newViewMatrix)
{
	std::vector<float> newData;
	newData.resize(16);

	for (size_t i = 0; i < 16; i++)
	{
		newData[i] = newViewMatrix[i];
	}

	bool needToAdd = true;

	for (size_t i = 0; i < recordedViewMatrix.size(); i++)
	{
		bool equal = true;
		for (size_t j = 0; j < recordedViewMatrix[i].size(); j++)
		{
			if (abs(recordedViewMatrix[i][j] - newData[j]) > 0.01)
			{
				//LOG.Add("should add", "camera");
				//LOG.Add("recordedViewMatrix[i][j]: " + std::to_string(recordedViewMatrix[i][j]), "camera");
				//LOG.Add("newData[j]: " + std::to_string(newData[j]), "camera");
				equal = false;
				break;
			}
		}

		if (equal)
		{
			needToAdd = false;
			break;
		}
	}

	if (needToAdd)
	{
		LOG.Add("==================== new view matrix added ====================", "camera");
		recordedViewMatrix.push_back(newData);
	}

	return needToAdd;
}

std::vector<std::vector<float>> recordedProjectionMatrix;
bool addIfNewProjection(float* newProjectionMatrix)
{
	std::vector<float> newData;
	newData.resize(16);

	for (size_t i = 0; i < 16; i++)
	{
		newData[i] = newProjectionMatrix[i];
	}

	bool needToAdd = true;

	for (size_t i = 0; i < recordedProjectionMatrix.size(); i++)
	{
		bool equal = true;
		for (size_t j = 0; j < recordedProjectionMatrix[i].size(); j++)
		{
			if (abs(recordedProjectionMatrix[i][j] - newData[j]) > 0.01)
			{
				equal = false;
				break;
			}
		}

		if (equal)
		{
			needToAdd = false;
			break;
		}
	}

	if (needToAdd)
	{
		LOG.Add("==================== new projection matrix added ====================", "camera");
		recordedProjectionMatrix.push_back(newData);
	}

	return needToAdd;
}

//std::unordered_map<DWORD, int> updateCameraThreads;
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API updateCamera(void* cameraWorldMatrix, void* cameraProjectionMatrix, int cameraScreenIndex = -1)
{
	if (cameraScreenIndex != -1)
	{
		LOG.Add("screenIndex in updateCamera: " + std::to_string(cameraScreenIndex), "screens");

		if (viewProjectionMatrices.find(cameraScreenIndex) == viewProjectionMatrices.end())
		{
			LOG.Add("adding entry in viewProjectionMatrices", "screens");
			LOG.Add("viewProjectionMatrices.size(): " + std::to_string(viewProjectionMatrices.size()), "screens");

			float* newStorage = new float[32];
			viewProjectionMatrices[cameraScreenIndex] = newStorage;
		}

		for (size_t i = 0; i < 16; i++)
		{
			viewProjectionMatrices[cameraScreenIndex][i] = ((float*)(cameraWorldMatrix))[i];
		}

		for (size_t i = 16; i < 32; i++)
		{
			viewProjectionMatrices[cameraScreenIndex][i] = ((float*)(cameraProjectionMatrix))[i - 16];
		}

		return;
	}

	/*DWORD currentThread = GetCurrentThreadId();
	if (updateCameraThreads.find(currentThread) == updateCameraThreads.end())
	{
		updateCameraThreads[currentThread]++;
		LOG.Add("updateCamera thread: " + std::to_string(GetCurrentThreadId()), "camera");
	}*/

	bool newData = addIfNewView((float*)(cameraWorldMatrix));
		
	for (size_t i = 0; i < 16; i++)
	{
		worldToViewMatrix[i] = ((float*)(cameraWorldMatrix))[i];
		if (newData)
			LOG.Add("worldToViewMatrix[" + std::to_string(i) + "]: " + std::to_string(worldToViewMatrix[i]), "camera");
	}

	newData = addIfNewProjection((float*)(cameraProjectionMatrix));

	for (size_t i = 0; i < 16; i++)
	{
		projectionMatrix[i] = ((float*)(cameraProjectionMatrix))[i];
		if (newData)
			LOG.Add("projectionMatrix[" + std::to_string(i) + "]: " + std::to_string(projectionMatrix[i]), "camera");
	}

	/*glm::mat4 glmViewMatrix = glm::make_mat4(worldToViewMatrix);
	glmViewMatrix = glm::transpose(glmViewMatrix);
	for (size_t i = 0; i < 4; i++)
	{
		for (size_t j = 0; j < 4; j++)
		{
			LOG.Add("glmViewMatrix[" + std::to_string(i) + "][" + std::to_string(j) + "]: " + std::to_string(glmViewMatrix[i][j]), "camera");
		}
	}*/

	//LOG.Add("glmWorldMatrix[0][3]: " + std::to_string(glmViewMatrix[0][3]), "camera");
	//LOG.Add("glmWorldMatrix[1][3]: " + std::to_string(glmViewMatrix[1][3]), "camera");
	//LOG.Add("glmWorldMatrix[2][3]: " + std::to_string(glmViewMatrix[2][3]), "camera");

	updateFrustumPlanes();
}

static void UNITY_INTERFACE_API OnRenderEvent(int eventID)
{
	LOG.Add("eventID: " + std::to_string(eventID), "renderLog");
	internalScreenIndex = eventID;
	Render();
}

extern "C" UnityRenderingEvent UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API GetRenderEventFunc()
{
	return OnRenderEvent;
}

static void UNITY_INTERFACE_API OnRenderAndDataEvent(int eventID, void* data)
{
	//LOG.Add("OnRenderAndDataEvent(int eventID, void* data)", "OnRenderAndDataEvent");

	for (size_t i = 0; i < 16; i++)
	{
		worldToViewMatrix[i] = ((float*)(data))[i];
	}

	for (size_t i = 16; i < 32; i++)
	{
		projectionMatrix[i - 16] = ((float*)(data))[i];
	}

	bool newData = addIfNewView(worldToViewMatrix);
	if (newData)
	{
		for (size_t i = 0; i < 16; i++)
		{
			LOG.Add("worldToViewMatrix[" + std::to_string(i) + "]: " + std::to_string(worldToViewMatrix[i]), "camera");
		}
	}
	
	newData = addIfNewProjection(projectionMatrix);

	if (newData)
	{
		for (size_t i = 0; i < 16; i++)
		{
			LOG.Add("projectionMatrix[" + std::to_string(i) + "]: " + std::to_string(projectionMatrix[i]), "camera");
		}
	}

	//for (size_t i = 0; i < 32; i++)
	//{
	//	LOG.Add("data[" + std::to_string(i) + "] = " + std::to_string(((float*)(data))[i]), "OnRenderAndDataEvent");
	//	//viewProjectionMatrices[screenIndex][i] = ((float*)(data))[i];
	//}

	Render();
}

extern "C" UnityRenderingEventAndData UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API GetRenderEventAndDataFunc()
{
	//LOG.Add("GetRenderEventAndDataFunc()", "OnRenderAndDataEvent");
	return OnRenderAndDataEvent;
}

void updateFrustumPlanes()
{
	if (frustum == nullptr)
	{
		frustum = new float* [6];
		for (size_t i = 0; i < 6; i++)
		{
			frustum[i] = new float[4];
		}
	}

	float   clip[16];
	float   t;

	glm::mat4 glmViewMatrix = glm::make_mat4(worldToViewMatrix);
	glm::mat4 glmProjectionMatrix = glm::make_mat4(projectionMatrix);

	glmViewMatrix = glm::transpose(glmViewMatrix);
	glmProjectionMatrix = glm::transpose(glmProjectionMatrix);

	glm::mat4 cliping = glmProjectionMatrix * glmViewMatrix;
	// This huge and ambiguous type of i is just to get rid of warning.
	for (glm::mat<4, 4, float, glm::packed_highp>::length_type i = 0; i < 4; i++)
	{
		clip[i * 4] = cliping[i][0];
		clip[i * 4 + 1] = cliping[i][1];
		clip[i * 4 + 2] = cliping[i][2];
		clip[i * 4 + 3] = cliping[i][3];
	}

	/* Extract the numbers for the RIGHT plane */
	frustum[0][0] = clip[3] - clip[0];
	frustum[0][1] = clip[7] - clip[4];
	frustum[0][2] = clip[11] - clip[8];
	frustum[0][3] = clip[15] - clip[12];

	/* Normalize the result */
	t = sqrt(frustum[0][0] * frustum[0][0] + frustum[0][1] * frustum[0][1] + frustum[0][2] * frustum[0][2]);
	frustum[0][0] /= t;
	frustum[0][1] /= t;
	frustum[0][2] /= t;
	frustum[0][3] /= t;

	/* Extract the numbers for the LEFT plane */
	frustum[1][0] = clip[3] + clip[0];
	frustum[1][1] = clip[7] + clip[4];
	frustum[1][2] = clip[11] + clip[8];
	frustum[1][3] = clip[15] + clip[12];

	/* Normalize the result */
	t = sqrt(frustum[1][0] * frustum[1][0] + frustum[1][1] * frustum[1][1] + frustum[1][2] * frustum[1][2]);
	frustum[1][0] /= t;
	frustum[1][1] /= t;
	frustum[1][2] /= t;
	frustum[1][3] /= t;

	/* Extract the BOTTOM plane */
	frustum[2][0] = clip[3] + clip[1];
	frustum[2][1] = clip[7] + clip[5];
	frustum[2][2] = clip[11] + clip[9];
	frustum[2][3] = clip[15] + clip[13];

	/* Normalize the result */
	t = sqrt(frustum[2][0] * frustum[2][0] + frustum[2][1] * frustum[2][1] + frustum[2][2] * frustum[2][2]);
	frustum[2][0] /= t;
	frustum[2][1] /= t;
	frustum[2][2] /= t;
	frustum[2][3] /= t;

	/* Extract the TOP plane */
	frustum[3][0] = clip[3] - clip[1];
	frustum[3][1] = clip[7] - clip[5];
	frustum[3][2] = clip[11] - clip[9];
	frustum[3][3] = clip[15] - clip[13];

	/* Normalize the result */
	t = sqrt(frustum[3][0] * frustum[3][0] + frustum[3][1] * frustum[3][1] + frustum[3][2] * frustum[3][2]);
	frustum[3][0] /= t;
	frustum[3][1] /= t;
	frustum[3][2] /= t;
	frustum[3][3] /= t;

	/* Extract the FAR plane */
	frustum[4][0] = clip[3] - clip[2];
	frustum[4][1] = clip[7] - clip[6];
	frustum[4][2] = clip[11] - clip[10];
	frustum[4][3] = clip[15] - clip[14];

	/* Normalize the result */
	t = sqrt(frustum[4][0] * frustum[4][0] + frustum[4][1] * frustum[4][1] + frustum[4][2] * frustum[4][2]);
	frustum[4][0] /= t;
	frustum[4][1] /= t;
	frustum[4][2] /= t;
	frustum[4][3] /= t;

	/* Extract the NEAR plane */
	frustum[5][0] = clip[3] + clip[2];
	frustum[5][1] = clip[7] + clip[6];
	frustum[5][2] = clip[11] + clip[10];
	frustum[5][3] = clip[15] + clip[14];

	/* Normalize the result */
	t = sqrt(frustum[5][0] * frustum[5][0] + frustum[5][1] * frustum[5][1] + frustum[5][2] * frustum[5][2]);
	frustum[5][0] /= t;
	frustum[5][1] /= t;
	frustum[5][2] /= t;
	frustum[5][3] /= t;
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API setFrustumCulling(bool active)
{
	frustumCulling = active;
}


extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API setLODSystemActive(bool active)
{
#ifdef LOD_SYSTEM
	LODSystemActive = active;
#endif
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API setLODInfo(float* values, int LODIndex, int pointCloudIndex)
{
#ifdef LOD_SYSTEM
	if (pointCloudIndex >= pointClouds.size() || pointCloudIndex < 0)
		return;

	if (LODIndex >= pointCloud::LODSettings.size() || LODIndex < 0)
		return;

	pointCloud::LODSettings[LODIndex].maxDistance = ((float*)(values))[0];
	pointCloud::LODSettings[LODIndex].targetPercentOFPoints = ((float*)(values))[1];
	pointCloud::LODSettings[LODIndex].takeEach_Nth_Point = (int)(100.0f / pointCloud::LODSettings[LODIndex].targetPercentOFPoints);
#endif
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API RequestLODInfoFromUnity(float* maxDistance, float* targetPercentOFPoints, int LODIndex, int pointCloudIndex)
{
#ifdef LOD_SYSTEM
	if (pointCloudIndex >= pointClouds.size() || pointCloudIndex < 0)
		return;

	if (LODIndex >= pointCloud::LODSettings.size() || LODIndex < 0)
		return;

	//LOG.Add("pointCloud::LODSettings[LODIndex].maxDistance: " + std::to_string(pointCloud::LODSettings[LODIndex].maxDistance), "TEST");
	//LOG.Add("pointCloud::LODSettings[LODIndex].targetPercentOFPoints: " + std::to_string(pointCloud::LODSettings[LODIndex].targetPercentOFPoints), "TEST");

	maxDistance[0] = pointCloud::LODSettings[LODIndex].maxDistance;
	targetPercentOFPoints[0] = pointCloud::LODSettings[LODIndex].targetPercentOFPoints;
#endif
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API RequestClosestPointToPointFromUnity(float* initialPointPosition)
{
	glm::vec3 initialPoint = glm::vec3(0.0f);
	initialPoint.x = initialPointPosition[0];
	initialPoint.y = initialPointPosition[1];
	initialPoint.z = initialPointPosition[2];

	glm::vec3 closestPoint = glm::vec3(0.0f);
	int pointCloudIndex = -1;

	float minDistance = FLT_MAX;
	for (size_t i = 0; i < pointClouds.size(); i++)
	{
		if (!pointClouds[i]->wasFullyLoaded)
			continue;

		glm::mat4 glmWorldMatrix = pointClouds[i]->worldMatrix;
		glmWorldMatrix = glm::transpose(glmWorldMatrix);
		glm::vec3 pointCloudTranslation = glm::vec3(glmWorldMatrix[3].x, glmWorldMatrix[3].y, glmWorldMatrix[3].z);

		for (size_t j = 0; j < pointClouds[i]->vertexInfo.size(); j++)
		{
			float distance = glm::distance(pointClouds[i]->vertexInfo[j].position + pointCloudTranslation, initialPoint);
			if (distance < minDistance)
			{
				pointCloudIndex = int(i);
				minDistance = distance;
				closestPoint = pointClouds[i]->vertexInfo[j].position + pointCloudTranslation;
			}
		}
	}

	initialPointPosition[0] = closestPoint.x;
	initialPointPosition[1] = closestPoint.y;
	initialPointPosition[2] = closestPoint.z;
}

bool isAtleastOnePointInSphere(int pointCloudIndex, float* center, float size)
{
	if (pointCloudIndex >= pointClouds.size() || pointCloudIndex < 0)
		return false;

	if (!pointClouds[pointCloudIndex]->wasFullyLoaded)
		return false;

	glm::vec3 centerOfBrush = glm::vec3(center[0], center[1], center[2]);
	glm::vec3 localPosition = glm::inverse(glm::transpose(pointClouds[pointCloudIndex]->worldMatrix)) * glm::vec4(centerOfBrush, 1.0f);
	LOG.Add("centerOfBrush: " + vec3ToString(centerOfBrush), "isAtleastOnePointInSphere");
	LOG.Add("localPosition: " + vec3ToString(localPosition), "isAtleastOnePointInSphere");

	if (pointClouds[pointCloudIndex]->getSearchOctree()->isInOctreeBound(localPosition, size))
	{
		if (pointClouds[pointCloudIndex]->getSearchOctree()->isAtleastOnePointInSphere(localPosition, size))
			return true;
	}

	return false;
}

extern "C" bool UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API RequestIsAtleastOnePointInSphereFromUnity(float* center, float size)
{
	glm::vec3 centerOfBrush = glm::vec3(center[0], center[1], center[2]);

	for (size_t i = 0; i < pointClouds.size(); i++)
	{
		if (!pointClouds[i]->wasFullyLoaded)
			continue;

		glm::vec3 localPosition = glm::inverse(glm::transpose(pointClouds[i]->worldMatrix)) * glm::vec4(centerOfBrush, 1.0f);

		if (pointClouds[i]->getSearchOctree()->isInOctreeBound(localPosition, size))
		{
			if (pointClouds[i]->getSearchOctree()->isAtleastOnePointInSphere(localPosition, size))
				return true;
		}
	}

	return false;
}

//#define CLOSEST_POINT_FAST_SEARCH
glm::vec3 getClosestPoint(int pointCloudIndex, glm::vec3 referencePoint)
{
	if (pointCloudIndex >= pointClouds.size() || pointCloudIndex < 0)
		return glm::vec3(FLT_MAX);

	if (!pointClouds[pointCloudIndex]->wasFullyLoaded)
		return glm::vec3(FLT_MAX);

	glm::vec3 closestPoint = glm::vec3(0.0f);

#ifdef CLOSEST_POINT_FAST_SEARCH

	LOG.Add("referencePoint: ", referencePoint, "getClosestPoint");

	glm::mat4 glmWorldMatrix = pointClouds[pointCloudIndex]->worldMatrix;
	glmWorldMatrix = glm::transpose(glmWorldMatrix);
	glm::vec3 pointCloudTranslation = glm::vec3(glmWorldMatrix[3].x, glmWorldMatrix[3].y, glmWorldMatrix[3].z);

	float distanceToPointCloud = glm::distance(referencePoint, pointCloudTranslation);
	LOG.Add("distanceToPointCloud: " + std::to_string(distanceToPointCloud), "getClosestPoint");

	// Multiplying by 4 to decrease chance that we will not "caught" any points.
	float workingRange = distanceToPointCloud * 4.0f;
	bool downScale = true;
	float sizeDifference = workingRange / 2.0f;
	bool pointInRange = isAtleastOnePointInSphere(pointCloudIndex, glm::value_ptr(referencePoint), workingRange);
	float lastScaleWithPoints = 0.0f;
	if (pointInRange)
		lastScaleWithPoints = workingRange;

	bool pointInRangeLastStep = pointInRange;
	for (int i = 0; i < 20; i++)
	{
		LOG.Add("step: " + std::to_string(i), "getClosestPoint");
		LOG.Add("sizeDifference: " + std::to_string(sizeDifference), "getClosestPoint");

		LOG.Add("workingRange_BEFORE: " + std::to_string(workingRange), "getClosestPoint");
		if (downScale)
		{
			workingRange += -sizeDifference;
		}
		else
		{
			workingRange += sizeDifference;
		}
		sizeDifference = sizeDifference / 2.0f;
		
		LOG.Add("workingRange_AFTER: " + std::to_string(workingRange), "getClosestPoint");

		pointInRange = isAtleastOnePointInSphere(pointCloudIndex, glm::value_ptr(referencePoint), workingRange);

		
		LOG.Add("pointInRange: " + std::to_string(pointInRange), "getClosestPoint");
		
		if (pointInRange)
			lastScaleWithPoints = workingRange;

		if (pointInRange != pointInRangeLastStep)
		{
			LOG.Add("pointInRange != pointInRangeLastStep", "getClosestPoint");
			pointInRangeLastStep = pointInRange;
			downScale = !downScale;
			//sizeDifference = sizeDifference / 2.0f;
		}
	}

	float size = lastScaleWithPoints;
	LOG.Add("lastScaleWithPoints: " + std::to_string(lastScaleWithPoints), "getClosestPoint");

	float minDistance = FLT_MAX;
	int totalCountOFPoints = 0;

	// With final size of sphere we are looking closest point in it is bounds.
	glm::vec3 localPosition = glm::inverse(glm::transpose(pointClouds[pointCloudIndex]->worldMatrix)) * glm::vec4(referencePoint, 1.0f);
	pointClouds[pointCloudIndex]->getSearchOctree()->deleteObjects(localPosition, size);
	totalCountOFPoints += int(pointClouds[pointCloudIndex]->getSearchOctree()->PointnsInSphere.size());

	for (size_t j = 0; j < pointClouds[pointCloudIndex]->getSearchOctree()->PointnsInSphere.size(); j++)
	{
		float distance = glm::distance(pointClouds[pointCloudIndex]->vertexInfo[pointClouds[pointCloudIndex]->getSearchOctree()->PointnsInSphere[j]].position + pointCloudTranslation, referencePoint);
		if (distance < minDistance)
		{
			minDistance = distance;
			closestPoint = pointClouds[pointCloudIndex]->vertexInfo[pointClouds[pointCloudIndex]->getSearchOctree()->PointnsInSphere[j]].position + pointCloudTranslation;
		}
	}
	pointClouds[pointCloudIndex]->getSearchOctree()->PointnsInSphere.clear();
#else
	float minDistance = FLT_MAX;
	for (size_t i = 0; i < pointClouds[pointCloudIndex]->getPointCount(); i++)
	{
		if (pointClouds[pointCloudIndex]->vertexInfo[i].position == referencePoint)
			continue;

		float distanceToClosest = glm::length(pointClouds[pointCloudIndex]->vertexInfo[i].position - referencePoint);
		//LOG.Add("First point: ", pointClouds[pointCloudIndex]->vertexInfo[i].position, "getClosestPoint");
		//LOG.Add("ReferencePoint point: ", referencePoint, "getClosestPoint");
		//LOG.Add("distanceToClosest: " + std::to_string(distanceToClosest), "getClosestPoint");

		if (minDistance > distanceToClosest)
		{
			closestPoint = pointClouds[pointCloudIndex]->vertexInfo[i].position;
			minDistance = distanceToClosest;
		}
	}

#endif

	return closestPoint;
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API DeleteOutliers_OLD(int pointCloudIndex, float outliersRange)
{
	if (pointCloudIndex >= pointClouds.size() || pointCloudIndex < 0)
		return;

	DWORD totalTime = GetTickCount();

	for (size_t i = 0; i < pointClouds[pointCloudIndex]->getPointCount(); i++)
	{
		if (i > 2000)
			break;

		glm::vec3 point = getClosestPoint(pointCloudIndex, pointClouds[pointCloudIndex]->vertexInfo[i].position);
		//LOG.Add("referencePoint: ", pointClouds[pointCloudIndex]->vertexInfo[i].position, "DeleteOutliers");
		//LOG.Add("closestPoint: ", point, "DeleteOutliers");
		if (point != glm::vec3(FLT_MAX))
		{
			float distanceToClosest = glm::length(point - pointClouds[pointCloudIndex]->vertexInfo[i].position);
			if (distanceToClosest > outliersRange)
			{
				LOG.Add("distanceToClosest: " + std::to_string(distanceToClosest), "DeleteOutliers");
			}
		}
	}

	LOG.Add("Time spent in DeleteOutliers: " + std::to_string(GetTickCount() - totalTime) + " ms", "DeleteOutliers");
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API RequestClosestPointInSphereFromUnity(float* center, float size)
{
	//LOG.Add("===============================", "RequestClosestPointInSphereFromUnity");

	DWORD totalTime = GetTickCount();

	if (pointClouds.size() == 0)
		return;

	glm::vec3 referencePoint = glm::vec3(center[0], center[1], center[2]);
	//LOG.Add("point", centerOfBrush, "RequestClosestPointInSphereFromUnity");
	//LOG.Add("pointClouds.size(): " + std::to_string(pointClouds.size()), "RequestClosestPointInSphereFromUnity");

	DWORD time = GetTickCount();
	float minDistance = FLT_MAX;
	for (int i = 0; i < pointClouds.size(); i++)
	{
		if (!pointClouds[i]->wasFullyLoaded)
			continue;

		glm::mat4 glmWorldMatrix = pointClouds[i]->worldMatrix;
		glmWorldMatrix = glm::transpose(glmWorldMatrix);
		glm::vec3 pointCloudTranslation = glm::vec3(glmWorldMatrix[3].x, glmWorldMatrix[3].y, glmWorldMatrix[3].z);

		float currentDistance = glm::distance(referencePoint, pointCloudTranslation);
		if (currentDistance < minDistance)
		{
			minDistance = currentDistance;
		}
	}

	//LOG.Add("minDistance: " + std::to_string(minDistance), "RequestClosestPointInSphereFromUnity");
	//LOG.Add("Time spent on looking for closest point cloud: " + std::to_string(GetTickCount() - time) + " ms", "RequestClosestPointInSphereFromUnity");

	if (minDistance == FLT_MAX)
		return;

	time = GetTickCount();

	// Multiplying by 4 to decrease chance that we will not "caught" any points.
	float workingRange = minDistance * 4.0f;
	//pointCloudManager.getTestSphereGameObject().transform.localScale = new Vector3(workingRange, workingRange, workingRange);
	bool downScale = true;
	float sizeDifference = workingRange / 2.0f;
	bool pointInRange = RequestIsAtleastOnePointInSphereFromUnity(glm::value_ptr(referencePoint), workingRange);
	float lastScaleWithPoints = 0.0f;
	if (pointInRange)
		lastScaleWithPoints = workingRange;

	bool pointInRangeLastStep = pointInRange;

	for (int i = 0; i < 20; i++)
	{
		workingRange += downScale ? -sizeDifference : sizeDifference;
		//pointCloudManager.getTestSphereGameObject().transform.localScale = new Vector3(workingRange, workingRange, workingRange);
		pointInRange = RequestIsAtleastOnePointInSphereFromUnity(glm::value_ptr(referencePoint), workingRange);
		if (pointInRange)
			lastScaleWithPoints = workingRange;

		if (pointInRange != pointInRangeLastStep)
		{
			pointInRangeLastStep = pointInRange;
			downScale = !downScale;
			sizeDifference = sizeDifference / 2.0f;
		}
	}

	//LOG.Add("Time spent on binary tightening of search sphere: " + std::to_string(GetTickCount() - time) + " ms", "RequestClosestPointInSphereFromUnity");

	size = lastScaleWithPoints;
	glm::vec3 closestPoint = glm::vec3(0.0f);

	minDistance = FLT_MAX;
	time = GetTickCount();
	int totalCountOFPoints = 0;

	for (size_t i = 0; i < pointClouds.size(); i++)
	{
		if (!pointClouds[i]->wasFullyLoaded)
			continue;

		glm::mat4 glmWorldMatrix = pointClouds[i]->worldMatrix;
		glmWorldMatrix = glm::transpose(glmWorldMatrix);
		glm::vec3 pointCloudTranslation = glm::vec3(glmWorldMatrix[3].x, glmWorldMatrix[3].y, glmWorldMatrix[3].z);

		glm::vec3 localPosition = glm::inverse(glm::transpose(pointClouds[i]->worldMatrix)) * glm::vec4(referencePoint, 1.0f);

		auto timeDeleteObjects = GetTickCount();

		pointClouds[i]->getSearchOctree()->deleteObjects(localPosition, size);
		//LOG.Add("timeDeleteObjects: " + std::to_string(GetTickCount() - timeDeleteObjects), "RequestClosestPointInSphereFromUnity");

		LOG.Add("pointClouds[i]->getSearchOctree()->PointnsInSphere.size(): " + std::to_string(pointClouds[i]->getSearchOctree()->PointnsInSphere.size()), "deleteEvents");
		totalCountOFPoints += int(pointClouds[i]->getSearchOctree()->PointnsInSphere.size());

		for (size_t j = 0; j < pointClouds[i]->getSearchOctree()->PointnsInSphere.size(); j++)
		{
			float distance = glm::distance(pointClouds[i]->vertexInfo[pointClouds[i]->getSearchOctree()->PointnsInSphere[j]].position + pointCloudTranslation, referencePoint);
			if (distance < minDistance)
			{
				minDistance = distance;
				closestPoint = pointClouds[i]->vertexInfo[pointClouds[i]->getSearchOctree()->PointnsInSphere[j]].position + pointCloudTranslation;
			}
		}

		pointClouds[i]->getSearchOctree()->PointnsInSphere.clear();
	}

	//if (GetTickCount() - time > 0)
	//	LOG.Add("totalCountOFPoints: " + std::to_string(totalCountOFPoints), "RequestClosestPointInSphereFromUnity");
	
	//LOG.Add("Time spent on search of closest point in search sphere: " + std::to_string(GetTickCount() - time) + " ms", "RequestClosestPointInSphereFromUnity");

	center[0] = closestPoint.x;
	center[1] = closestPoint.y;
	center[2] = closestPoint.z;

	//LOG.Add("totalTime: " + std::to_string(GetTickCount() - totalTime) + " ms", "RequestClosestPointInSphereFromUnity");
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API setHighlightDeletedPointsActive(bool active)
{
	highlightDeletedPoints = active;
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UpdateDeletionSpherePositionFromUnity(float* center, float size)
{
	//LOG.Add("UpdateDeletionSpherePositionFromUnity: ", "UpdateDeletionSpherePositionFromUnity");
	deletionSpherePosition = glm::vec3(center[0], center[1], center[2]);
	deletionSphereSize = size;
}

#ifdef USE_COMPUTE_SHADER
//void runMyComputeShader()
//{
//	static DWORD startTime = GetTickCount();
//
//	if (pointClouds.size() == 0 || !pointClouds[0]->wasFullyLoaded || testFrustum == nullptr || (GetTickCount() - startTime < 1000))
//		return;
//
//	//LOG.Add("runMyComputeShader()_START: " + std::system_category().message(GPU.getDevice()->GetDeviceRemovedReason()), "computeShader");
//
//	ID3D11DeviceContext* ctx = NULL;
//	GPU.getDevice()->GetImmediateContext(&ctx);
//
//	if (inputPoints_CS_SRV == nullptr)
//	{
//		allPointsData_CS = new VertexData[pointClouds[0]->vertexInfo.size()];
//#ifdef FLOAT_TEST
//		InputData_CS = new float[24];
//#else
//		InputData_CS = new VertexData[24];
//#endif // FLOAT_TEST
//
//		for (int i = 0; i < pointClouds[0]->vertexInfo.size(); ++i)
//		{
//			allPointsData_CS[i] = pointClouds[0]->vertexInfo[i];
//		}
//
//		D3D11_BUFFER_DESC descBuffer = {};
//		descBuffer.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
//		descBuffer.ByteWidth = sizeof(VertexData) * pointClouds[0]->vertexInfo.size();
//		descBuffer.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
//		descBuffer.StructureByteStride = sizeof(VertexData);
//		D3D11_SUBRESOURCE_DATA InitData;
//		InitData.pSysMem = &allPointsData_CS[0];
//		auto result = GPU.getDevice()->CreateBuffer(&descBuffer, &InitData, &allPointsDataBuffer_CS);
//		//HRESULT result = 0;
//		//LOG.Add("pointClouds[0]->vertexInfo.size(): " + std::to_string(pointClouds[0]->vertexInfo.size()), "computeShader");
//		//LOG.Add("GPU.getDevice()->CreateBuffer(&descBuffer, &InitData, &g_pBuf0);: " + std::system_category().message(result), "computeShader");
//		//LOG.Add("GPU.getDevice()->CreateBuffer(&descBuffer, &InitData, &g_pBuf0);: " + std::system_category().message(GPU.getDevice()->GetDeviceRemovedReason()), "computeShader");
//
//		InitData.pSysMem = &InputData_CS[0];
//#ifdef FLOAT_TEST
//		descBuffer.ByteWidth = sizeof(float) * 24;
//		descBuffer.StructureByteStride = sizeof(float);
//#else
//		descBuffer.ByteWidth = sizeof(VertexData) * 24;
//#endif // FLOAT_TEST
//
//		result = GPU.getDevice()->CreateBuffer(&descBuffer, &InitData, &InputDataBuffer_CS);
//		//LOG.Add("GPU.getDevice()->CreateBuffer(&descBuffer, &InitData, &g_pBuf1);: " + std::system_category().message(result), "computeShader");
//		//LOG.Add("GPU.getDevice()->CreateBuffer(&descBuffer, &InitData, &g_pBuf1);: " + std::system_category().message(GPU.getDevice()->GetDeviceRemovedReason()), "computeShader");
//		descBuffer.ByteWidth = sizeof(VertexData) * pointClouds[0]->vertexInfo.size();
//		descBuffer.StructureByteStride = sizeof(VertexData);
//		result = GPU.getDevice()->CreateBuffer(&descBuffer, nullptr, &resultBuffer_CS);
//		result = GPU.getDevice()->CreateBuffer(&descBuffer, nullptr, &resultBufferSecond_CS);
//		currentBuffer_CS = &resultBuffer_CS;
//		//LOG.Add("GPU.getDevice()->CreateBuffer(&descBuffer, nullptr, &g_pBufResult);: " + std::system_category().message(result), "computeShader");
//		//LOG.Add("GPU.getDevice()->CreateBuffer(&descBuffer, nullptr, &g_pBufResult);: " + std::system_category().message(GPU.getDevice()->GetDeviceRemovedReason()), "computeShader");
//
//		// DEBUG
//		result = allPointsDataBuffer_CS->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof("Buffer0") - 1, "Buffer0");
//		//LOG.Add("g_pBuf0->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(\"Buffer0\") - 1, \"Buffer0\");: " + std::system_category().message(result), "computeShader");
//		//LOG.Add("g_pBuf0->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(\"Buffer0\") - 1, \"Buffer0\");: " + std::system_category().message(GPU.getDevice()->GetDeviceRemovedReason()), "computeShader");
//		InputDataBuffer_CS->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof("Buffer1") - 1, "Buffer1");
//		resultBuffer_CS->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof("Result") - 1, "Result");
//		// DEBUG
//
//		CreateBufferSRV(GPU.getDevice(), allPointsDataBuffer_CS, &inputPoints_CS_SRV);
//		CreateBufferSRV(GPU.getDevice(), InputDataBuffer_CS, &InputData_CS_SRV);
//		CreateBufferUAV(GPU.getDevice(), resultBuffer_CS, &result_CS_UAV);
//		CreateBufferSRV(GPU.getDevice(), resultBuffer_CS, &result_CS_SRV);
//		CreateBufferUAV(GPU.getDevice(), resultBufferSecond_CS, &resultSecond_CS_UAV);
//		CreateBufferSRV(GPU.getDevice(), resultBufferSecond_CS, &resultSecond_CS_SRV);
//		current_CS_UAV = &result_CS_UAV;
//		current_CS_SRV = &result_CS_SRV;
//
//		// DEBUG
//		inputPoints_CS_SRV->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof("Buffer0 SRV") - 1, "Buffer0 SRV");
//		InputData_CS_SRV->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof("Buffer1 SRV") - 1, "Buffer1 SRV");
//		result_CS_UAV->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof("Result UAV") - 1, "Result UAV");
//		// DEBUG
//
//		currentInputPoints_CS_SRV = &inputPoints_CS_SRV;
//	}
//
//	int linearIndex = 0;
//	for (size_t i = 0; i < 6; i++)
//	{
//		for (size_t j = 0; j < 4; j++)
//		{
//#ifdef FLOAT_TEST
//			InputData_CS[linearIndex++] = testFrustum[i][j];
//#else
//			InputData_CS[linearIndex++].position.x = testFrustum[i][j];
//#endif // FLOAT_TEST
//		}
//	}
//
//	glm::vec3 localDeletionSpherePosition = glm::inverse(glm::transpose(pointClouds[0]->worldMatrix)) * glm::vec4(deletionSpherePosition, 1.0f);
//	InputData_CS[0] = localDeletionSpherePosition.x;
//	InputData_CS[1] = localDeletionSpherePosition.y;
//	InputData_CS[2] = localDeletionSpherePosition.z;
//	InputData_CS[3] = deletionSphereSize * 2.0f;
//
//#ifdef FLOAT_TEST
//	ctx->UpdateSubresource(InputDataBuffer_CS, 0, nullptr, InputData_CS, sizeof(float) * 24, sizeof(float) * 24);
//#else
//	ctx->UpdateSubresource(InputDataBuffer_CS, 0, nullptr, InputData_CS, sizeof(VertexData) * 24, sizeof(VertexData) * 24);
//#endif // FLOAT_TEST
//
//	ID3D11ShaderResourceView* aRViews[2] = { inputPoints_CS_SRV, InputData_CS_SRV };
//	RunComputeShader(ctx, computeShader, 2, aRViews, nullptr, nullptr, 0, result_CS_UAV, ceil(pointClouds[0]->vertexInfo.size() / 64), 1, 1);
//
//	if (false)
//	{
//		ID3D11Buffer* debugbuf = CreateAndCopyToDebugBuf(GPU.getDevice(), ctx, resultBuffer_CS);
//		D3D11_MAPPED_SUBRESOURCE MappedResource;
//		VertexData* p;
//		ctx->Map(debugbuf, 0, D3D11_MAP_READ, 0, &MappedResource);
//
//		// Set a break point here and put down the expression "p, 1024" in your watch window to see what has been written out by our CS
//		// This is also a common trick to debug CS programs.
//		p = (VertexData*)MappedResource.pData;
//
//		/*LOG.Add("resultBufferSize: " + std::to_string(MappedResource.RowPitch / sizeof(VertexData)), "computeShader");*/
//
//		//for (int i = 0; i < MappedResource.RowPitch / sizeof(VertexData); i++)
//		//{
//		//	LOG.Add("result[" + std::to_string(i) + "] - X: " + std::to_string(p[i].position.x) + " Y : " + std::to_string(p[i].position.y) + " Z : " + std::to_string(p[i].position.z), "computeShader");
//		//	//LOG.Add("result[" + std::to_string(i) + "] - R: " + std::to_string(p[i].color[0]) + " G : " + std::to_string(p[i].color[1]) + " B : " + std::to_string(p[i].color[2]), "computeShader");
//		//}
//
//		DWORD beforeTime = GetTickCount();
//
//		D3D11_BOX dbox{};
//		dbox.left = 0;
//		dbox.right = MappedResource.RowPitch;
//		dbox.top = 0;
//		dbox.bottom = 1;
//		dbox.front = 0;
//		dbox.back = 1;
//
//		ctx->UpdateSubresource(pointClouds[0]->mainVB, 0, &dbox, MappedResource.pData, pointClouds[0]->getPointCount() * kVertexSize, pointClouds[0]->getPointCount() * kVertexSize);
//		//ctx->UpdateSubresource(pointClouds[0]->intermediateVB, 0, &dbox, MappedResource.pData, pointClouds[0]->getPointCount() * kVertexSize, pointClouds[0]->getPointCount() * kVertexSize);
//		//ctx->CopySubresourceRegion(pointClouds[0]->mainVB, 0, dbox.left, 0, 0, pointClouds[0]->intermediateVB, 0, &dbox);
//
//		/*LOG.Add("ctx->CopySubresourceRegion was called", "computeShader");
//		LOG.Add("CPU time :" + std::to_string(GetTickCount() - beforeTime), "computeShader");
//		LOG.Add("testLevel" + std::to_string(testLevel), "computeShader");*/
//
//		//for (int i = 0; i < pointClouds[0]->vertexInfo.size(); i++)
//		//{
//		//	float distance = sqrt(pow(pointClouds[0]->vertexInfo[i].position.x, 2) + pow(pointClouds[0]->vertexInfo[i].position.y, 2) + pow(pointClouds[0]->vertexInfo[i].position.z, 2));
//		//	LOG.Add("p[" + std::to_string(i) + "].x :" + std::to_string(p[i].position.x) + "CPU: " + std::to_string(distance), "computeShader");
//		//	
//		//	//LOG.Add("p[" + std::to_string(i) + "].x :" + std::to_string(p[i].position.x) + "CPU: " + std::to_string(pointClouds[0]->vertexInfo[i].position.x), "computeShader");
//		//}
//
//
//
//
//		/*DWORD beforeTime = GetTickCount();
//		float smallestDistance = FLT_MAX;
//		int index = -1;
//
//		for (int i = 0; i < pointClouds[0]->vertexInfo.size(); ++i)
//		{
//			float distance = sqrt(pow(pointClouds[0]->vertexInfo[i].position.x, 2) + pow(pointClouds[0]->vertexInfo[i].position.y, 2) + pow(pointClouds[0]->vertexInfo[i].position.z, 2));
//			if (distance < smallestDistance)
//			{
//				smallestDistance = distance;
//				index = i;
//			}
//		}
//
//		LOG.Add("CPU time :" + std::to_string(GetTickCount() - beforeTime), "computeShader");
//
//		LOG.Add("CPU min distance :" + std::to_string(smallestDistance), "computeShader");
//		LOG.Add("CPU point index :" + std::to_string(index), "computeShader");*/
//
//		//LOG.Add("min distance :" + std::to_string(p[0].position.x), "computeShader");
//		//LOG.Add("point index :" + std::to_string(p[1].position.x), "computeShader");
//
//		ctx->Unmap(debugbuf, 0);
//	}
//
//	//LOG.Add("runMyComputeShader()_END: " + std::system_category().message(GPU.getDevice()->GetDeviceRemovedReason()), "computeShader");
//}

void runMyDeleteComputeShader(pointCloud* pointCloud)
{
	if (pointClouds.size() == 0 || pointCloud == nullptr || !pointCloud->wasFullyLoaded || pointCloud->mainVB == nullptr)
		return;

	//LOG.Add("runMyDeleteComputeShader()_START: " + std::system_category().message(GPU.getDevice()->GetDeviceRemovedReason()), "computeShader");

	ID3D11DeviceContext* ctx = NULL;
	GPU.getDevice()->GetImmediateContext(&ctx);

	if (pointCloud->current_CS_SRV == nullptr)
	{
		pointCloud->InputData_CS = new float[24];
		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = &pointCloud->InputData_CS[0];

		D3D11_BUFFER_DESC descBuffer = {};
		descBuffer.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		descBuffer.ByteWidth = sizeof(float) * 24;
		descBuffer.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		descBuffer.StructureByteStride = sizeof(float);
		auto result = GPU.getDevice()->CreateBuffer(&descBuffer, &InitData, &pointCloud->InputDataBuffer_CS);
		LOG.Add("pointCloud->InputDataBuffer_CS result : " + std::to_string (result), "deleteEvents");
		CreateBufferSRV(GPU.getDevice(), pointCloud->InputDataBuffer_CS, &pointCloud->InputData_CS_SRV);

		/*allPointsData_CS = new VertexData[pointCloud->vertexInfo.size()];
		InputData_CS = new float[24];

		for (int i = 0; i < pointCloud->vertexInfo.size(); ++i)
		{
			allPointsData_CS[i] = pointCloud->vertexInfo[i];
		}

		D3D11_BUFFER_DESC descBuffer = {};
		descBuffer.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		descBuffer.ByteWidth = sizeof(VertexData) * pointCloud->vertexInfo.size();
		descBuffer.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		descBuffer.StructureByteStride = sizeof(VertexData);
		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = &allPointsData_CS[0];
		auto result = GPU.getDevice()->CreateBuffer(&descBuffer, &InitData, &pointCloud->allPointsDataBuffer_CS);

		InitData.pSysMem = &InputData_CS[0];
		descBuffer.ByteWidth = sizeof(float) * 24;
		descBuffer.StructureByteStride = sizeof(float);

		result = GPU.getDevice()->CreateBuffer(&descBuffer, &InitData, &pointCloud->InputDataBuffer_CS);
		descBuffer.ByteWidth = sizeof(VertexData) * pointCloud->vertexInfo.size();
		descBuffer.StructureByteStride = sizeof(VertexData);
		result = GPU.getDevice()->CreateBuffer(&descBuffer, nullptr, &pointCloud->resultBuffer_CS);
		result = GPU.getDevice()->CreateBuffer(&descBuffer, nullptr, &pointCloud->resultBufferSecond_CS);
		pointCloud->currentBuffer_CS = &pointCloud->resultBuffer_CS;

		//new 
		ctx->UpdateSubresource(pointCloud->resultBuffer_CS, 0, nullptr, pointCloud->vertexInfo.data(), pointCloud->getPointCount() * kVertexSize, pointCloud->getPointCount() * kVertexSize);
		//new

		CreateBufferSRV(GPU.getDevice(), pointCloud->allPointsDataBuffer_CS, &pointCloud->inputPoints_CS_SRV);
		CreateBufferSRV(GPU.getDevice(), pointCloud->InputDataBuffer_CS, &pointCloud->InputData_CS_SRV);
		CreateBufferUAV(GPU.getDevice(), pointCloud->resultBuffer_CS, &pointCloud->result_CS_UAV);
		CreateBufferSRV(GPU.getDevice(), pointCloud->resultBuffer_CS, &pointCloud->result_CS_SRV);
		CreateBufferUAV(GPU.getDevice(), pointCloud->resultBufferSecond_CS, &pointCloud->resultSecond_CS_UAV);
		CreateBufferSRV(GPU.getDevice(), pointCloud->resultBufferSecond_CS, &pointCloud->resultSecond_CS_SRV);
		pointCloud->current_CS_UAV = &pointCloud->result_CS_UAV;
		pointCloud->current_CS_SRV = &pointCloud->result_CS_SRV;

		pointCloud->currentInputPoints_CS_SRV = &pointCloud->inputPoints_CS_SRV;*/

		//new
		CreateBufferSRV(GPU.getDevice(), pointCloud->mainVB, &pointCloud->result_CS_SRV);
		CreateBufferUAV(GPU.getDevice(), pointCloud->mainVB, &pointCloud->result_CS_UAV);
		pointCloud->current_CS_UAV = &pointCloud->result_CS_UAV;
		pointCloud->current_CS_SRV = &pointCloud->result_CS_SRV;
		pointCloud->pointsToDraw = pointCloud->getPointCount();
		return;
		//new
	}

	LOG.Add("RequestToDeleteFromUnity", "deleteEvents");
	
	if (GetTickCount() - timeLastTimeCall < 100)
	{
		LOG.Add("denial RequestToDeleteFromUnity", "deleteEvents");
		LOG.Add("time: " + std::to_string(GetTickCount() - timeLastTimeCall), "deleteEvents");
		//return;
	}

	LOG.Add("NOT denial RequestToDeleteFromUnity", "deleteEvents");
	timeLastTimeCall = GetTickCount();

	glm::vec3 localDeletionSpherePosition = glm::inverse(glm::transpose(pointClouds[0]->worldMatrix)) * glm::vec4(deletionSpherePosition, 1.0f);
	pointCloud->InputData_CS[0] = localDeletionSpherePosition.x;
	pointCloud->InputData_CS[1] = localDeletionSpherePosition.y;
	pointCloud->InputData_CS[2] = localDeletionSpherePosition.z;
	/*pointCloud->InputData_CS[0] = deletionSpherePosition.x;
	pointCloud->InputData_CS[1] = deletionSpherePosition.y;
	pointCloud->InputData_CS[2] = deletionSpherePosition.z;*/
	pointCloud->InputData_CS[3] = deletionSphereSize * 2.0f;

	ctx->UpdateSubresource(pointCloud->InputDataBuffer_CS, 0, nullptr, pointCloud->InputData_CS, sizeof(float) * 24, sizeof(float) * 24);
	ID3D11ShaderResourceView* aRViews[2] = { *pointCloud->current_CS_SRV, pointCloud->InputData_CS_SRV };

	if (pointCloud->InputData_CS_SRV == nullptr)
	{
		LOG.Add("pointCloud->InputData_CS_SRV == nullptr", "deleteEvents");
	}

	RunComputeShader(ctx, computeShader, 2, aRViews, nullptr, nullptr, 0, *pointCloud->current_CS_UAV, ceil(pointCloud->vertexInfo.size() / 1024), 1, 1);
	pointCloud->pointsToDraw = pointCloud->getPointCount();
}
#endif

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API setTestLevel(float unityTestLevel)
{
	testLevel = unityTestLevel;
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API highlightOutliers(float discardDistance, int minNeighborsInRange, char* pointCloudID)
{
	pointCloud* currentPointCloud = getPointCloud(pointCloudID);
	if (currentPointCloud == nullptr)
		return;

	for (size_t i = 0; i < pointClouds.size(); i++)
	{
		if (pointClouds[i]->ID == pointCloudID)
			currentPointCloud = pointClouds[i];
	}

	if (currentPointCloud == nullptr)
		return;
	
	currentPointCloud->highlightOutliers(discardDistance, minNeighborsInRange);
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API deleteOutliers(char* pointCloudID)
{
	pointCloud* currentPointCloud = getPointCloud(pointCloudID);
	if (currentPointCloud == nullptr)
		return;

	UNDO_MANAGER.addAction(new deleteOutliersAction(currentPointCloud->lastOutliers, currentPointCloud));

	currentPointCloud->getSearchOctree()->PointnsInSphere = currentPointCloud->lastOutliers;
#ifdef USE_COMPUTE_SHADER
	for (size_t i = 0; i < currentPointCloud->getSearchOctree()->PointnsInSphere.size(); i++)
	{
		currentPointCloud->vertexInfo[currentPointCloud->getSearchOctree()->PointnsInSphere[i]].position[0] = DELETED_POINTS_COORDINATE;
		currentPointCloud->vertexInfo[currentPointCloud->getSearchOctree()->PointnsInSphere[i]].position[1] = DELETED_POINTS_COORDINATE;
		currentPointCloud->vertexInfo[currentPointCloud->getSearchOctree()->PointnsInSphere[i]].position[2] = DELETED_POINTS_COORDINATE;
	}
	currentPointCloud->getSearchOctree()->PointnsInSphere.clear();

	// Update GPU buffer.
	const int kVertexSize = 12 + 4;
	ID3D11DeviceContext* ctx = NULL;

	if (GPU.getDevice() != nullptr)
		GPU.getDevice()->GetImmediateContext(&ctx);

	ctx->UpdateSubresource(currentPointCloud->mainVB, 0, NULL, currentPointCloud->vertexInfo.data(), currentPointCloud->getPointCount() * kVertexSize, currentPointCloud->getPointCount() * kVertexSize);
	//ctx->UpdateSubresource(*currentPointCloud->currentBuffer_CS, 0, NULL, currentPointCloud->vertexInfo.data(), currentPointCloud->getPointCount() * kVertexSize, currentPointCloud->getPointCount() * kVertexSize);
#else
	for (size_t i = 0; i < currentPointCloud->getSearchOctree()->PointnsInSphere.size(); i++)
	{
		currentPointCloud->vertexInfo[currentPointCloud->getSearchOctree()->PointnsInSphere[i]].position[0] = DELETED_POINTS_COORDINATE;
		currentPointCloud->vertexInfo[currentPointCloud->getSearchOctree()->PointnsInSphere[i]].position[1] = DELETED_POINTS_COORDINATE;
		currentPointCloud->vertexInfo[currentPointCloud->getSearchOctree()->PointnsInSphere[i]].position[2] = DELETED_POINTS_COORDINATE;
	}
	currentPointCloud->getSearchOctree()->PointnsInSphere.clear();

	// Update GPU buffer.
	const int kVertexSize = 12 + 4;
	ID3D11DeviceContext* ctx = NULL;

	if (GPU.getDevice() != nullptr)
		GPU.getDevice()->GetImmediateContext(&ctx);

	ctx->UpdateSubresource(currentPointCloud->mainVB, 0, NULL, currentPointCloud->vertexInfo.data(), currentPointCloud->getPointCount() * kVertexSize, currentPointCloud->getPointCount() * kVertexSize);
	ctx->Release();

	currentPointCloud->getSearchOctree()->PointnsInSphere.clear();
#endif // USE_COMPUTE_SHADER
	currentPointCloud->lastOutliers.clear();
}

extern "C" bool UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API unLoad(char* pointCloudID)
{
	pointCloud* currentPointCloud = getPointCloud(pointCloudID);
	if (currentPointCloud == nullptr)
		return false;

	for (size_t i = 0; i < pointClouds.size(); i++)
	{
		if (pointClouds[i]->ID == currentPointCloud->ID)
		{
			UNDO_MANAGER.clear(pointClouds[i]);
			delete pointClouds[i];
			pointClouds.erase(pointClouds.begin() + i, pointClouds.begin() + i + 1);
			return true;
		}
	}

	return false;
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API setScreenIndex(int newScreenIndex)
{
	//screenIndex = newScreenIndex;
	//LOG.Add("newScreenIndex: " + std::to_string(newScreenIndex), "screens");
}

extern "C" int UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API RequestEPSGFromUnity(char* pointCloudID)
{
	pointCloud* currentPointCloud = getPointCloud(pointCloudID);
	if (currentPointCloud == nullptr)
		return 0;

	return currentPointCloud->EPSG;
}

extern "C" bool UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API IsLastAsyncSaveFinished()
{
	return SaveManager::getInstance().isSaveDone();
}

extern "C" bool UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API IsDebugLogFileOutActive()
{
	return LOG.IsFileOutputActive();
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API SetDebugLogFileOutput(bool NewValue)
{
	LOG.SetFileOutput(NewValue);
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API DisableTopicFileOutput(char* TopicName)
{
	LOG.DisableTopicFileOutput(TopicName);
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API EnableTopicFileOutput(char* TopicName)
{
	LOG.EnableTopicFileOutput(TopicName);
}