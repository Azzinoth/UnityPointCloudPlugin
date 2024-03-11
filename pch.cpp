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

static std::string currentVersion = "version 2023.2.10.17535";

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

static bool localHighlightDeletedPoints = false;
const int kVertexSize = sizeof(VertexData);

static int screenIndex = -1;
static int internalScreenIndex = 0;

static std::unordered_map<int, float*> viewProjectionMatrices;
static std::unordered_map<int, ID3D11Buffer*> viewProjectionMatricesBuffers;

#define FLOAT_TEST

#ifdef FLOAT_TEST
	//float* InputData_CS;
#else
	VertexData* InputData_CS;
#endif // FLOAT_TEST

static float testLevel = -830.0f;

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
	LOG.Add("WorkOnRequests() got called", "WorkOnRequests");

	if (MainRequest.bFulfilled)
		return;

	ModificationRequest MainRequestLocalCopy;
	MainRequestLocalCopy = MainRequest;
	MainRequest.bFulfilled = true;

	LOG.Add("Before loop", "WorkOnRequests");

	for (size_t j = 0; j < pointClouds.size(); j++)
	{
		if (!pointClouds[j]->wasFullyLoaded)
			continue;

		glm::vec3 localPosition = glm::inverse(glm::transpose(pointClouds[j]->worldMatrix)) * glm::vec4(MainRequestLocalCopy.Center, 1.0f);
		float extractedScale = glm::length(glm::transpose(pointClouds[j]->worldMatrix)[0]);
		MainRequestLocalCopy.Size /= extractedScale;

		if (pointClouds[j]->getSearchOctree()->isInOctreeBound(localPosition, MainRequestLocalCopy.Size))
		{
			pointClouds[j]->getSearchOctree()->searchForObjects(localPosition, MainRequestLocalCopy.Size, pointClouds[j]->getSearchOctree()->PointnsInSphere);
		}

		if (pointClouds[j]->getSearchOctree()->PointnsInSphere.size() > 0)
		{
			bDeletionResultCountInProgress = true;
			LOG.Add("bDeletionResultCountInProgress = true", "deleteEvents");

			LastResults.push_back(0);

			if (MainRequestLocalCopy.TypeOfModification == 0)
			{
				LOG.Add("==============================================================", "deleteEvents");
				LOG.Add("Brush location: " + vec3ToString(localPosition), "deleteEvents");
				LOG.Add("Brush size: " + std::to_string(MainRequestLocalCopy.Size), "deleteEvents");
				LOG.Add("PointnsInSphere size: " + std::to_string(pointClouds[j]->getSearchOctree()->PointnsInSphere.size()), "deleteEvents");

				octree* currentOctree = pointClouds[j]->getSearchOctree();
				for (size_t k = 0; k < currentOctree->PointnsInSphere.size(); k++)
				{
					if (pointClouds[j]->vertexInfo[currentOctree->PointnsInSphere[k]].position[0] != DELETED_POINTS_COORDINATE &&
						pointClouds[j]->vertexInfo[currentOctree->PointnsInSphere[k]].position[1] != DELETED_POINTS_COORDINATE &&
						pointClouds[j]->vertexInfo[currentOctree->PointnsInSphere[k]].position[2] != DELETED_POINTS_COORDINATE)
					{
						MainRequestLocalCopy.Result++;
							
						LastResults.back()++;
					}
				}

				// Unity part counts undo based on LastResult
				if (LastResults.back() > 0)
				{
					LOG.Add("LastResult = " + std::to_string(LastResults.back()), "deleteEvents");
					LOG.Add("UNDO_MANAGER.addAction", "deleteEvents");

					UNDO_MANAGER.addAction(new deleteAction(localPosition, MainRequestLocalCopy.Size, pointClouds[j]));
				}
			}
			else if (MainRequestLocalCopy.TypeOfModification == 1)
			{
				octree* currentOctree = pointClouds[j]->getSearchOctree();
				for (size_t k = 0; k < currentOctree->PointnsInSphere.size(); k++)
				{
					if (pointClouds[j]->vertexInfo[currentOctree->PointnsInSphere[k]].classification != MainRequestLocalCopy.AdditionalData)
					{
						MainRequestLocalCopy.Result++;

						LastResults.back()++;
					}
				}

				// Unity part counts undo based on LastResult
				if (LastResults.back() > 0)
				{
					LOG.Add("LastResult = " + std::to_string(LastResults.back()), "deleteEvents");
					LOG.Add("UNDO_MANAGER.addAction", "deleteEvents");

					UNDO_MANAGER.addAction(new changeClassificationAction(localPosition, MainRequestLocalCopy.Size, pointClouds[j], MainRequestLocalCopy.AdditionalData));
				}
			}

			bDeletionResultCountInProgress = false;
			LOG.Add("bDeletionResultCountInProgress = false", "deleteEvents");

			ID3D11DeviceContext* ctx = NULL;
			GPU.getDevice()->GetImmediateContext(&ctx);
			ApplyPointModificationRequest(pointClouds[j], ctx, MainRequestLocalCopy);
			ctx->Release();
		}
	}
}

static string NextTextToSend = "";
extern "C" int UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API GetNextTextLengthFromDLL()
{
	return int(NextTextToSend.size());
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API FillTextFromDLL(int* Data)
{
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

bool AsynOpenLAZFileFromUnity(char* filePath, char* ID)
{
	LOG.Add(std::string("File name: ") + filePath, "File_Load_Log");

	if (strlen(filePath) < 4)
	{
		LOG.Add(std::string("Call of AsynOpenLAZFileFromUnity can't be executed because file name is incorrect: ") + filePath, "ERRORS");
		return false;
	}

	// if file is in our own format we will not need dll functionality
	if (filePath[strlen(filePath) - 4] != '.' && filePath[strlen(filePath) - 3] != 'c' && filePath[strlen(filePath) - 2] != 'p' && filePath[strlen(filePath) - 1] != 'c')
	{
		if (!DLLWasLoadedCorrectly)
		{
			LOG.Add("Call of AsynOpenLAZFileFromUnity can't be executed because DLL was not loaded correctly", "ERRORS");
			return false;
		}
	}

	if (!SaveManager::getInstance().isSaveDone())
	{
		LOG.Add("Call of AsynOpenLAZFileFromUnity can't be executed because save job is running.", "ERRORS");
		return false;
	}

	pointCloud* temp = new pointCloud();
	temp->ID = ID;
	LoadManager::getInstance().loadPointCloudAsync(std::string(filePath), projectPath, temp);
	pointClouds.push_back(temp);

	return true;
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

	return AsynOpenLAZFileFromUnity(filePath, ID);
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

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API OnSceneStartFromUnity(char* projectFilePath)
{
	//LOG.SetFileOutput(true);

	LOG.DisableTopicFileOutput("camera");
	LOG.DisableTopicFileOutput("precision");
	LOG.DisableTopicFileOutput("screens");
	LOG.DisableTopicFileOutput("Threads");

	if (THREAD_POOL.GetThreadCount() == 0)
		THREAD_POOL.SetConcurrentThreadCount(2);

	ClassificationCount = 0;

	if (FloatsToSync.size() == 0)
	{
		FloatsToSync["VisualizeClassification"] = 0.0f;
	}

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

	if (!LoadManager::getInstance().isLoadingDone())
	{
		LOG.Add("Call of SaveToLAZFileFromUnity can't be executed because loading job is running.", "ERRORS");
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

void ApplyPointModificationRequest(pointCloud* pointCloud, ID3D11DeviceContext* ctx, ModificationRequest Request)
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

	LOG.Add("RequestToDeleteFromUnity was called", "LOOK HERE");
	LOG.Add("NewRequest.Center: " + vec3ToString(NewRequest.Center), "LOOK HERE");
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

	LOG.Add("RequestToModifyPointsFromUnity was called", "LOOK HERE");
	LOG.Add("NewRequest.Center: " + vec3ToString(NewRequest.Center), "LOOK HERE");
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UpdateModificationParameters(float* Center, float Size, int TypeOfModification, float AdditionalData)
{
	LOG.Add("UpdateModificationParameters was called", "UpdateModificationParameters");

	bool expected = false;
	if (!bDeletionResultCountInProgress.compare_exchange_strong(expected, false))
	{
		LOG.Add("bDeletionResultCountInProgress is true", "UpdateModificationParameters");
	}
	else
	{
		MainRequest.Center = glm::vec3(Center[0], Center[1], Center[2]);
		MainRequest.Size = Size;
		MainRequest.TypeOfModification = TypeOfModification;
		MainRequest.AdditionalData = AdditionalData;
		MainRequest.bFulfilled = false;

		LOG.Add("bDeletionResultCountInProgress is false", "UpdateModificationParameters");
	}
}

extern "C" int UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API GetRequestResult(/*char* RequestID*/)
{
	bool expected = false;
	if (!bDeletionResultCountInProgress.compare_exchange_strong(expected, false))
	{
		LOG.Add("bDeletionResultCountInProgress is true", "GetRequestResult");
		return -1;
	}
	else
	{
		if (LastResults.size() > 0)
		{
			int Temp = LastResults[0];
			LastResults.erase(LastResults.begin());
			return Temp;
		}
		else
		{
			return 0;
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

		adjustment[7] *= currentPointCloud->loadedFrom->header.y_scale_factor;
		adjustment[7] += currentPointCloud->loadedFrom->header.y_offset;

		adjustment[8] = currentPointCloud->Metrics.RawMax.x;

		adjustment[8] *= currentPointCloud->loadedFrom->header.x_scale_factor;
		adjustment[8] += currentPointCloud->loadedFrom->header.x_offset;

		adjustment[9] = currentPointCloud->Metrics.Max.y;

		adjustment[10] = currentPointCloud->Metrics.RawMax.z;
		adjustment[10] *= currentPointCloud->loadedFrom->header.y_scale_factor;
		adjustment[10] += currentPointCloud->loadedFrom->header.y_offset;
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

	long VSLen = 0;
	hr = D3DCompile(VertexShaderSource.c_str(),
		VertexShaderSource.length(),
		nullptr, nullptr, nullptr, "VS", "vs_5_0",
		D3DCOMPILE_ENABLE_STRICTNESS, 0, &pVSBlob, &errorBlob);

	LOG.Add("shaderResult: " + std::system_category().message(hr), "computeShader");
	if (errorBlob)
	{
		LOG.Add("shaderResult: " + std::string((char*)errorBlob->GetBufferPointer()), "computeShader");
		errorBlob->Release();
	}

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
}



static int drawCount = 0;
static void DrawPointCloud(pointCloud* pointCloudToRender, bool HighlightDeletedPoints)
{
	if (!pointCloudToRender->wasFullyLoaded)
		return;

	if (pointCloudToRender->getSearchOctree()->PointnsInSphere.size() != 0)
	{
		LOG.Add("DrawPointCloud begin with PointnsInSphere size: " + std::to_string(pointCloudToRender->getSearchOctree()->PointnsInSphere.size()), "deleteEvents");
	}

	glm::mat4 glmWorldMatrix = pointCloudToRender->worldMatrix;
	glm::mat4 glmViewMatrix;
	glm::mat4 glmProjectionMatrix; 

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

	ID3D11DeviceContext* ctx = NULL;
	GPU.getDevice()->GetImmediateContext(&ctx);

	ctx->OMSetDepthStencilState(m_DepthState, 0);
	ctx->RSSetState(m_RasterState);
	ctx->OMSetBlendState(m_BlendState, NULL, 0xFFFFFFFF);

	glm::mat4 finalMatrix = glmProjectionMatrix * glmViewMatrix * glmWorldMatrix;
	LOG.Add("finalMatrix :" + mat4ToString(finalMatrix), "camera");
	LOG.Add("glmViewMatrix :" + mat4ToString(glmViewMatrix), "camera");
	LOG.Add("glmWorldMatrix :" + mat4ToString(glmWorldMatrix), "camera");

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

#endif

	// Set shaders
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
	
	if (!pointCloudToRender->wasInitialized)
	{
		D3D11_BUFFER_DESC desc;
		memset(&desc, 0, sizeof(desc));

		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = UINT(pointCloudToRender->vertexInfo.size() * kVertexSize);
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		GPU.getDevice()->CreateBuffer(&desc, NULL, &pointCloudToRender->mainVB);

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

#ifdef LOD_SYSTEM
		for (size_t i = 0; i < pointCloudToRender->LODs.size(); i++)
		{
			ctx->UpdateSubresource(pointCloudToRender->LODs[i].VB, 0, nullptr, pointCloudToRender->LODs[i].vertexInfo.data(), UINT(pointCloudToRender->LODs[i].vertexInfo.size() * kVertexSize), UINT(pointCloudToRender->LODs[i].vertexInfo.size() * kVertexSize));
		}
#endif
		pointCloudToRender->wasInitialized = true;
	}

	//Sleep(5);
	//if (localHighlightDeletedPoints)
		highlightDeletedPointsFunction(pointCloudToRender, ctx);
	//onDrawDeletePointsinGPUMem(pointCloudToRender, ctx, HighlightDeletedPoints);

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

	ctx->Release();
}

static void ProcessDeviceEvent(UnityGfxDeviceEventType type, IUnityInterfaces* interfaces)
{
	switch (type)
	{
		case kUnityGfxDeviceEventInitialize:
		{
			GPU.initialize(interfaces);
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

static UnityGfxRenderer s_DeviceType = kUnityGfxRendererNull;

static void UNITY_INTERFACE_API OnGraphicsDeviceEvent(UnityGfxDeviceEventType eventType)
{
	// Create graphics API implementation upon initialization
	if (eventType == kUnityGfxDeviceEventInitialize)
	{
		s_DeviceType = s_Graphics->GetRenderer();
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

	Render();
}

extern "C" UnityRenderingEventAndData UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API GetRenderEventAndDataFunc()
{
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
	DWORD totalTime = GetTickCount();

	if (pointClouds.size() == 0)
		return;

	glm::vec3 referencePoint = glm::vec3(center[0], center[1], center[2]);

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

	if (minDistance == FLT_MAX)
		return;

	time = GetTickCount();

	// Multiplying by 4 to decrease chance that we will not "caught" any points.
	float workingRange = minDistance * 4.0f;
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

	center[0] = closestPoint.x;
	center[1] = closestPoint.y;
	center[2] = closestPoint.z;
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