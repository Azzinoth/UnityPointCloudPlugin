#include "pch.h"
#include "PointCloud.h"

#ifdef LOD_SYSTEM
std::vector<LODSetting> pointCloud::LODSettings = std::vector<LODSetting>();
#endif

pointCloud::pointCloud()
{
	mainVB = nullptr;
	intermediateVB = nullptr;
	worldMatrix = glm::identity<glm::mat4>();
	searchOctree = new octree();
}

pointCloud::~pointCloud()
{
	LOG.Add("start of ~pointCloud", "OctreeMemory");

	delete searchOctree;
	if (mainVB != nullptr)
		mainVB->Release();
	if (intermediateVB != nullptr)
		intermediateVB->Release();
	delete loadedFrom;

	LOG.Add("end of ~pointCloud", "OctreeMemory");
}

void pointCloud::initializeOctree(double rangeX, double rangeY, double rangeZ, glm::vec3 worldCenter)
{
	double AABBsize = rangeX > rangeY ? rangeX : rangeY;
	AABBsize = AABBsize > rangeZ ? AABBsize : rangeZ;

#ifdef USE_QUADS_NOT_POINTS
	searchOctree->initialize(float(AABBsize), worldCenter);
#else
	searchOctree->initialize(float(AABBsize), worldCenter);

	for (int i = 0; i < vertexInfo.size(); i++)
	{
		bool accepted = searchOctree->addObject(vertexInfo[i], i);
		if (!accepted)
		{
			LOG.Add("point was: rejected", "OctreeEvents");
			LOG.Add("point:" + vec3ToString(vertexInfo[i].position), "OctreeEvents");
		}
	}
#endif
}

int pointCloud::getPointCount()
{
	return int(vertexInfo.size());
}

octree* pointCloud::getSearchOctree()
{
	return searchOctree;
}

void pointCloud::highlightOutliers(float discardDistance, int minNeighborsInRange)
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

float pointCloud::getApproximateGroundLevel()
{
	return approximateGroundLevel;
}

void pointCloud::calculateApproximateGroundLevel()
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