#include "pch.h"
#include "loadManager.h"

LoadManager* LoadManager::_instance = nullptr;
std::vector<LODSetting> pointCloud::LODSettings = std::vector<LODSetting>();

void LoadManager::loadFunc()
{
	while (true)
	{
		if (currentPath != "")
		{
			newJobReady = false;

			if (currentPath.size() <= 4)
			{
				debugLog::getInstance().addToLog("filePath lenght was less then 4", "ERRORS");
				return;
			}

			double rangeX = 0.0f;
			double rangeY = 0.0f;
			double rangeZ = 0.0f;

			double newMinX = DBL_MAX;
			double newMaxX = -DBL_MAX;
			double newMinY = DBL_MAX;
			double newMaxY = -DBL_MAX;
			double newMinZ = DBL_MAX;
			double newMaxZ = -DBL_MAX;

			// if file is in our own format
			if (currentPath[currentPath.size() - 4] == '.' &&
				currentPath[currentPath.size() - 3] == 'c' &&
				currentPath[currentPath.size() - 2] == 'p' &&
				currentPath[currentPath.size() - 1] == 'c')
			{
				
				std::fstream file;
				file.open(currentPath, std::ios::in | std::ios::binary);

				char* buffer = new char[sizeof(float)];

				// Read how many points does the file have.
				file.read(buffer, sizeof(int));
				int pointCount = *(int*)buffer;

				// Read initialXShift and initialZShift.
				file.read(buffer, sizeof(double));
				currentPointCloud->initialXShift = *(double*)buffer;
				file.read(buffer, sizeof(double));
				currentPointCloud->initialZShift = *(double*)buffer;

				// Read adjustment.
				file.read(buffer, sizeof(float));
				currentPointCloud->adjustment[0] = *(float*)buffer;
				file.read(buffer, sizeof(float));
				currentPointCloud->adjustment[1] = *(float*)buffer;
				file.read(buffer, sizeof(float));
				currentPointCloud->adjustment[2] = *(float*)buffer;

				// Read min and max.
				file.read(buffer, sizeof(float));
				currentPointCloud->min[0] = *(float*)buffer;
				file.read(buffer, sizeof(float));
				currentPointCloud->min[1] = *(float*)buffer;
				file.read(buffer, sizeof(float));
				currentPointCloud->min[2] = *(float*)buffer;

				file.read(buffer, sizeof(float));
				currentPointCloud->max[0] = *(float*)buffer;
				file.read(buffer, sizeof(float));
				currentPointCloud->max[1] = *(float*)buffer;
				file.read(buffer, sizeof(float));
				currentPointCloud->max[2] = *(float*)buffer;

				newMinX = currentPointCloud->min.x;
				newMaxX = currentPointCloud->max.x;
				newMinY = currentPointCloud->min.y;
				newMaxY = currentPointCloud->max.y;
				newMinZ = currentPointCloud->min.z;
				newMaxZ = currentPointCloud->max.z;

				rangeX = currentPointCloud->max.x - currentPointCloud->min.x;
				rangeY = currentPointCloud->max.y - currentPointCloud->min.y;
				rangeZ = currentPointCloud->max.z - currentPointCloud->min.z;

				debugLog::getInstance().addToLog("rangeX: " + std::to_string(rangeX), "File_Load_Log");
				debugLog::getInstance().addToLog("rangeY: " + std::to_string(rangeY), "File_Load_Log");
				debugLog::getInstance().addToLog("rangeZ: " + std::to_string(rangeZ), "File_Load_Log");

				debugLog::getInstance().addToLog("adjustment.x: " + std::to_string(currentPointCloud->adjustment.x), "File_Load_Log");
				debugLog::getInstance().addToLog("adjustment.y: " + std::to_string(currentPointCloud->adjustment.y), "File_Load_Log");
				debugLog::getInstance().addToLog("adjustment.z: " + std::to_string(currentPointCloud->adjustment.z), "File_Load_Log");

				debugLog::getInstance().addToLog("contains " + std::to_string(pointCount) + " points", "File_Load_Log");

				if (currentPointCloud == nullptr)
					debugLog::getInstance().addToLog("currentPointCloud = nullptr", "testThread");

				currentPointCloud->vertexInfo.resize(pointCount);
				currentPointCloud->LODs.resize(pointCloud::LODSettings.size());

				debugLog::getInstance().addToLog("before file.read(rawPointData, pointCount * sizeof(MeshVertex)); ", "File_Load_Log");
				char* rawPointData = new char[pointCount * sizeof(MeshVertex)];
				file.read(rawPointData, pointCount * sizeof(MeshVertex));
				debugLog::getInstance().addToLog("after file.read(rawPointData, pointCount * sizeof(MeshVertex)); ", "File_Load_Log");

				MeshVertex* temp = (MeshVertex*)rawPointData;
				for (size_t i = 0; i < pointCount; i++)
				{
					/*debugLog::getInstance().addToLog("(temp + i * sizeof(MeshVertex))->position.x: " + std::to_string((temp + i * sizeof(MeshVertex))->position.x), "testLoad");
					debugLog::getInstance().addToLog("(temp + i * sizeof(MeshVertex))->position.y: " + std::to_string((temp + i * sizeof(MeshVertex))->position.y), "testLoad");
					debugLog::getInstance().addToLog("(temp + i * sizeof(MeshVertex))->position.z: " + std::to_string((temp + i * sizeof(MeshVertex))->position.z), "testLoad");

					debugLog::getInstance().addToLog("(temp + i * sizeof(MeshVertex))->color[0]: " + std::to_string((temp + i * sizeof(MeshVertex))->color[0]), "testLoad");
					debugLog::getInstance().addToLog("(temp + i * sizeof(MeshVertex))->color[1]: " + std::to_string((temp + i * sizeof(MeshVertex))->color[1]), "testLoad");
					debugLog::getInstance().addToLog("(temp + i * sizeof(MeshVertex))->color[2]: " + std::to_string((temp + i * sizeof(MeshVertex))->color[2]), "testLoad");
					debugLog::getInstance().addToLog("(temp + i * sizeof(MeshVertex))->color[3]: " + std::to_string((temp + i * sizeof(MeshVertex))->color[3]), "testLoad");*/
				
					currentPointCloud->vertexInfo[i].position[0] = (temp + i)->position.x;
					currentPointCloud->vertexInfo[i].position[1] = (temp + i)->position.y;
					currentPointCloud->vertexInfo[i].position[2] = (temp + i)->position.z;
					currentPointCloud->vertexInfo[i].color[0] = (temp + i)->color[0];
					currentPointCloud->vertexInfo[i].color[1] = (temp + i)->color[1];
					currentPointCloud->vertexInfo[i].color[2] = (temp + i)->color[2];

					for (size_t j = 0; j < pointCloud::LODSettings.size(); j++)
					{
						if (i % pointCloud::LODSettings[j].takeEach_Nth_Point == 0)
						{
							currentPointCloud->LODs[j].vertexInfo.resize(currentPointCloud->LODs[j].vertexInfo.size() + 1);
							currentPointCloud->LODs[j].vertexInfo.back().position[0] = currentPointCloud->vertexInfo[i].position[0];
							currentPointCloud->LODs[j].vertexInfo.back().position[1] = currentPointCloud->vertexInfo[i].position[1];
							currentPointCloud->LODs[j].vertexInfo.back().position[2] = currentPointCloud->vertexInfo[i].position[2];
							currentPointCloud->LODs[j].vertexInfo.back().color[0] = currentPointCloud->vertexInfo[i].color[0];
							currentPointCloud->LODs[j].vertexInfo.back().color[1] = currentPointCloud->vertexInfo[i].color[1];
							currentPointCloud->LODs[j].vertexInfo.back().color[2] = currentPointCloud->vertexInfo[i].color[2];
						}
					}
				}
			}
			else
			{
				// create the reader
				laszip_POINTER laszip_reader;
				if (laszip_create(&laszip_reader))
				{
					debugLog::getInstance().addToLog("creating laszip reader failed", "DLL_ERRORS");
					return;
				}

				// open the reader
				laszip_BOOL is_compressed = 0;
				if (laszip_open_reader(laszip_reader, currentPath.c_str(), &is_compressed))
				{
					debugLog::getInstance().addToLog("opening laszip reader for " + currentPath + " failed", "DLL_ERRORS");
					return;
				}

				// get a pointer to the header of the reader that was just populated
				laszip_header* header;
				if (laszip_get_header_pointer(laszip_reader, &header))
				{
					debugLog::getInstance().addToLog("getting header pointer from laszip reader failed", "DLL_ERRORS");
					return;
				}

				//LAZFileInfo* fileInfo = new LAZFileInfo();
				//copyLAZFileHeader(&fileInfo->header, header);
				//fileInfo->compressed = is_compressed;

				debugLog::getInstance().addToLog("Compressed: " + std::string(is_compressed ? "true" : "false"), "File_Load_Log");
				debugLog::getInstance().addToLog("Signature: " + std::string(header->generating_software), "File_Load_Log");
				debugLog::getInstance().addToLog("Points count: " + std::to_string(header->number_of_point_records), "File_Load_Log");
				debugLog::getInstance().addToLog("X Min: " + std::to_string(header->min_x), "File_Load_Log");
				debugLog::getInstance().addToLog("X Max: " + std::to_string(header->max_x), "File_Load_Log");
				debugLog::getInstance().addToLog("Y Min: " + std::to_string(header->min_y), "File_Load_Log");
				debugLog::getInstance().addToLog("Y Max: " + std::to_string(header->max_y), "File_Load_Log");
				debugLog::getInstance().addToLog("Z Min: " + std::to_string(header->min_z), "File_Load_Log");
				debugLog::getInstance().addToLog("Z Max: " + std::to_string(header->max_z), "File_Load_Log");

				// how many points does the file have
				laszip_U64 npoints = (header->number_of_point_records ? header->number_of_point_records : header->extended_number_of_point_records);

				debugLog::getInstance().addToLog("contains " + std::to_string(npoints) + " points", "File_Load_Log");

				if (currentPointCloud == nullptr)
					debugLog::getInstance().addToLog("currentPointCloud = nullptr", "testThread");

				currentPointCloud->vertexInfo.resize(npoints);
				currentPointCloud->vertexIntensity.resize(npoints);

				currentPointCloud->min = glm::vec3(FLT_MAX);
				currentPointCloud->max = glm::vec3(-FLT_MAX);

				for (size_t i = 0; i < header->number_of_variable_length_records; i++)
				{
					if (header->vlrs[i].record_length_after_header)
					{
						std::string text = reinterpret_cast<char*>(header->vlrs[i].data);

						size_t position = text.find("NAD_1983_2011_UTM_Zone_");
						size_t position1 = text.find("UTM zone ");

						if (position != std::string::npos)
						{
							currentPointCloud->spatialInfo = text;
							debugLog::getInstance().addToLog("spatialInfo: " + currentPointCloud->spatialInfo, "File_Load_Log");
							currentPointCloud->UTMZone = text.substr(position + strlen("NAD_1983_2011_UTM_Zone_"), 3);
							debugLog::getInstance().addToLog("UTMZone: " + currentPointCloud->UTMZone, "File_Load_Log");
							break;
						}
						else if (position1 != std::string::npos)
						{
							currentPointCloud->spatialInfo = text;
							debugLog::getInstance().addToLog("spatialInfo: " + currentPointCloud->spatialInfo, "File_Load_Log");
							currentPointCloud->UTMZone = text.substr(position + strlen("UTM zone "), 3);
							debugLog::getInstance().addToLog("UTMZone: " + currentPointCloud->UTMZone, "File_Load_Log");
							break;
						}
					}
				}

				// get a pointer to the points that will be read
				laszip_point* point;
				if (laszip_get_point_pointer(laszip_reader, &point))
				{
					debugLog::getInstance().addToLog("getting point pointer from laszip reader failed", "DLL_ERRORS");
					return;
				}

				currentPointCloud->LODs.resize(pointCloud::LODSettings.size());

				// read the points
				laszip_U64 p_count = 0;
				std::vector<MeshVertex> points;
				float maxIntensity = -FLT_MAX;
				while (p_count < npoints)
				{
					// read a point
					if (laszip_read_point(laszip_reader))
					{
						debugLog::getInstance().addToLog("reading point " + std::to_string(p_count) + " failed", "DLL_ERRORS");
						return;
					}

					//debugLog::getInstance().addToLog("if (laszip_read_point(laszip_reader)): " + std::to_string(p_count), "testThread");
					//fileInfo->LAZpoints.push_back(laszip_point(*point));

					// point->X -> lonX, point->Y -> latY, point->Z -> depth
					float readX = float(point->X * header->x_scale_factor);
					float readY = float(point->Z * header->z_scale_factor);
					float readZ = float(point->Y * header->y_scale_factor);

					//debugLog::getInstance().addToLog("float readZ : " + std::to_string(p_count), "testThread");

					currentPointCloud->vertexInfo[p_count].position[0] = readX;
					currentPointCloud->vertexInfo[p_count].position[1] = readY;
					currentPointCloud->vertexInfo[p_count].position[2] = readZ;
					currentPointCloud->vertexInfo[p_count].color[0] = byte(point->rgb[0] / float(1 << 16) * 255);
					currentPointCloud->vertexInfo[p_count].color[1] = byte(point->rgb[1] / float(1 << 16) * 255);
					currentPointCloud->vertexInfo[p_count].color[2] = byte(point->rgb[2] / float(1 << 16) * 255);

					for (size_t i = 0; i < pointCloud::LODSettings.size(); i++)
					{
						if (p_count % pointCloud::LODSettings[i].takeEach_Nth_Point == 0)
						{
							currentPointCloud->LODs[i].vertexInfo.resize(currentPointCloud->LODs[i].vertexInfo.size() + 1);
							currentPointCloud->LODs[i].vertexInfo.back().position[0] = currentPointCloud->vertexInfo[p_count].position[0];
							currentPointCloud->LODs[i].vertexInfo.back().position[1] = currentPointCloud->vertexInfo[p_count].position[1];
							currentPointCloud->LODs[i].vertexInfo.back().position[2] = currentPointCloud->vertexInfo[p_count].position[2];
							currentPointCloud->LODs[i].vertexInfo.back().color[0] = currentPointCloud->vertexInfo[p_count].color[0];
							currentPointCloud->LODs[i].vertexInfo.back().color[1] = currentPointCloud->vertexInfo[p_count].color[1];
							currentPointCloud->LODs[i].vertexInfo.back().color[2] = currentPointCloud->vertexInfo[p_count].color[2];
						}
					}

					currentPointCloud->vertexIntensity[p_count] = point->intensity;
					if (maxIntensity < point->intensity)
						maxIntensity = point->intensity;

					if (currentPointCloud->min.x > readX)
						currentPointCloud->min.x = readX;

					if (currentPointCloud->max.x < readX)
						currentPointCloud->max.x = readX;

					if (currentPointCloud->min.y > readY)
						currentPointCloud->min.y = readY;

					if (currentPointCloud->max.y < readY)
						currentPointCloud->max.y = readY;

					if (currentPointCloud->min.z > readZ)
						currentPointCloud->min.z = readZ;

					if (currentPointCloud->max.z < readZ)
						currentPointCloud->max.z = readZ;

					p_count++;
				}

				debugLog::getInstance().addToLog("while (p_count < npoints)", "testThread");

				if (header->point_data_format == 1)
				{
					for (size_t i = 0; i < p_count; i++)
					{
						currentPointCloud->vertexInfo[i].color[0] = byte(currentPointCloud->vertexIntensity[i] / maxIntensity * 255);
						currentPointCloud->vertexInfo[i].color[1] = byte(currentPointCloud->vertexIntensity[i] / maxIntensity * 255);
						currentPointCloud->vertexInfo[i].color[2] = byte(currentPointCloud->vertexIntensity[i] / maxIntensity * 255);

						for (size_t j = 0; j < currentPointCloud->LODs.size(); j++)
						{
							if (i % pointCloud::LODSettings[j].takeEach_Nth_Point == 0)
							{
								currentPointCloud->LODs[j].vertexInfo[i / pointCloud::LODSettings[j].takeEach_Nth_Point].color[0] = currentPointCloud->vertexInfo[i].color[0];
								currentPointCloud->LODs[j].vertexInfo[i / pointCloud::LODSettings[j].takeEach_Nth_Point].color[1] = currentPointCloud->vertexInfo[i].color[1];
								currentPointCloud->LODs[j].vertexInfo[i / pointCloud::LODSettings[j].takeEach_Nth_Point].color[2] = currentPointCloud->vertexInfo[i].color[2];
							}
						}
					}
				}

				currentPointCloud->initialZShift = currentPointCloud->max.z < currentPointCloud->min.z ? currentPointCloud->max.z : currentPointCloud->min.z;
				currentPointCloud->initialZShift = header->min_y - currentPointCloud->initialZShift;

				if (header->x_offset == 0 && header->y_offset == 0 && header->z_offset == 0)
				{
					rangeX = currentPointCloud->max.x - currentPointCloud->min.x;
					rangeY = currentPointCloud->max.y - currentPointCloud->min.y;
					rangeZ = currentPointCloud->max.z - currentPointCloud->min.z;

					currentPointCloud->adjustment.x = -currentPointCloud->min.x;
					currentPointCloud->adjustment.y = -currentPointCloud->min.y;
					currentPointCloud->adjustment.z = -currentPointCloud->min.z;
				}
				else
				{
					rangeX = currentPointCloud->max.x - currentPointCloud->min.x;
					rangeY = currentPointCloud->max.y - currentPointCloud->min.y;
					rangeZ = currentPointCloud->max.z - currentPointCloud->min.z;

					currentPointCloud->adjustment.x = float(-header->x_offset);
					currentPointCloud->adjustment.y = float(-header->y_offset);
					currentPointCloud->adjustment.z = float(-header->z_offset);
				}

				debugLog::getInstance().addToLog("rangeX: " + std::to_string(rangeX), "File_Load_Log");
				debugLog::getInstance().addToLog("rangeY: " + std::to_string(rangeY), "File_Load_Log");
				debugLog::getInstance().addToLog("rangeZ: " + std::to_string(rangeZ), "File_Load_Log");

				debugLog::getInstance().addToLog("adjustment.x: " + std::to_string(currentPointCloud->adjustment.x), "File_Load_Log");
				debugLog::getInstance().addToLog("adjustment.y: " + std::to_string(currentPointCloud->adjustment.y), "File_Load_Log");
				debugLog::getInstance().addToLog("adjustment.z: " + std::to_string(currentPointCloud->adjustment.z), "File_Load_Log");

				for (int i = 0; i < npoints; i++)
				{
					if (header->x_offset == 0 && header->y_offset == 0 && header->z_offset == 0)
					{
						currentPointCloud->vertexInfo[i].position[0] = currentPointCloud->vertexInfo[i].position[0] + currentPointCloud->adjustment.x;
						currentPointCloud->vertexInfo[i].position[1] = currentPointCloud->vertexInfo[i].position[1] + currentPointCloud->adjustment.y;
						currentPointCloud->vertexInfo[i].position[2] = currentPointCloud->vertexInfo[i].position[2] + currentPointCloud->adjustment.z;
					}
					else
					{
						currentPointCloud->vertexInfo[i].position[0] = currentPointCloud->vertexInfo[i].position[0] + float(currentPointCloud->adjustment.x * header->x_scale_factor);
						currentPointCloud->vertexInfo[i].position[1] = currentPointCloud->vertexInfo[i].position[1] + float(currentPointCloud->adjustment.y * header->y_scale_factor);
						currentPointCloud->vertexInfo[i].position[2] = currentPointCloud->vertexInfo[i].position[2] + float(currentPointCloud->adjustment.z * header->z_scale_factor);
					}

					if (newMinX > currentPointCloud->vertexInfo[i].position[0])
						newMinX = currentPointCloud->vertexInfo[i].position[0];

					if (newMaxX < currentPointCloud->vertexInfo[i].position[0])
						newMaxX = currentPointCloud->vertexInfo[i].position[0];

					if (newMinY > currentPointCloud->vertexInfo[i].position[1])
						newMinY = currentPointCloud->vertexInfo[i].position[1];

					if (newMaxY < currentPointCloud->vertexInfo[i].position[1])
						newMaxY = currentPointCloud->vertexInfo[i].position[1];

					if (newMinZ > currentPointCloud->vertexInfo[i].position[2])
						newMinZ = currentPointCloud->vertexInfo[i].position[2];

					if (newMaxZ < currentPointCloud->vertexInfo[i].position[2])
						newMaxZ = currentPointCloud->vertexInfo[i].position[2];

					for (size_t j = 0; j < currentPointCloud->LODs.size(); j++)
					{
						if (i % pointCloud::LODSettings[j].takeEach_Nth_Point == 0)
						{
							currentPointCloud->LODs[j].vertexInfo[i / pointCloud::LODSettings[j].takeEach_Nth_Point].position[0] = currentPointCloud->vertexInfo[i].position[0];
							currentPointCloud->LODs[j].vertexInfo[i / pointCloud::LODSettings[j].takeEach_Nth_Point].position[1] = currentPointCloud->vertexInfo[i].position[1];
							currentPointCloud->LODs[j].vertexInfo[i / pointCloud::LODSettings[j].takeEach_Nth_Point].position[2] = currentPointCloud->vertexInfo[i].position[2];
						}
					}
				}

				currentPointCloud->min.x = newMinX;
				currentPointCloud->min.y = newMinY;
				currentPointCloud->min.z = newMinZ;

				currentPointCloud->max.x = newMaxX;
				currentPointCloud->max.y = newMaxY;
				currentPointCloud->max.z = newMaxZ;

				debugLog::getInstance().addToLog("newMinX: " + std::to_string(newMinX), "File_Load_Log");
				debugLog::getInstance().addToLog("newMaxX: " + std::to_string(newMaxX), "File_Load_Log");
				debugLog::getInstance().addToLog("newMinY: " + std::to_string(newMinY), "File_Load_Log");
				debugLog::getInstance().addToLog("newMaxY: " + std::to_string(newMaxY), "File_Load_Log");
				debugLog::getInstance().addToLog("newMinZ: " + std::to_string(newMinZ), "File_Load_Log");
				debugLog::getInstance().addToLog("newMaxZ: " + std::to_string(newMaxZ), "File_Load_Log");

				currentPointCloud->initialXShift = header->min_x - newMinX;
				if (header->x_offset == 0 && header->y_offset == 0 && header->z_offset == 0)
				{
					currentPointCloud->initialZShift = -currentPointCloud->adjustment.z;
				}

				debugLog::getInstance().addToLog("currentPointCloud->initialXShift: " + std::to_string(currentPointCloud->initialXShift), "File_Load_Log");
				debugLog::getInstance().addToLog("currentPointCloud->initialZShift: " + std::to_string(currentPointCloud->initialZShift), "File_Load_Log");

				if (header->x_offset == 0 && header->y_offset == 0 && header->z_offset == 0)
					std::swap(currentPointCloud->adjustment.y, currentPointCloud->adjustment.z);

				// close the reader
				if (laszip_close_reader(laszip_reader))
				{
					debugLog::getInstance().addToLog("closing laszip reader failed", "DLL_ERRORS");
				}

				// destroy the reader
				if (laszip_destroy(laszip_reader))
				{
					debugLog::getInstance().addToLog("destroying laszip reader failed", "DLL_ERRORS");
				}
			}

			// saving original colors
			currentPointCloud->pointsOriginalColor.resize(currentPointCloud->getPointCount());
			for (size_t i = 0; i < currentPointCloud->pointsOriginalColor.size(); i++)
			{
				currentPointCloud->pointsOriginalColor[i].r = currentPointCloud->vertexInfo[i].color[0];
				currentPointCloud->pointsOriginalColor[i].g = currentPointCloud->vertexInfo[i].color[1];
				currentPointCloud->pointsOriginalColor[i].b = currentPointCloud->vertexInfo[i].color[2];
			}

			debugLog::getInstance().addToLog("before initializeOctree", "testThread");
			debugLog::getInstance().addToLog("rangeXYZ: ", glm::vec3(rangeX, rangeY, rangeZ), "OctreeEvents");
			currentPointCloud->initializeOctree(rangeX, rangeY, rangeZ, glm::vec3(newMinX + rangeX / 2.0f, newMinY + rangeY / 2.0f, newMinZ + rangeZ / 2.0f));
			debugLog::getInstance().addToLog("after initializeOctree", "testThread");
			//currentPointCloud->loadedFrom = fileInfo;
			//currentPointCloud->loadedFrom->resultingPointCloud = pointClouds.back();

			debugLog::getInstance().addToLog("Total nodes created: " + std::to_string(currentPointCloud->getSearchOctree()->getDebugNodeCount()), "OctreeEvents");
			debugLog::getInstance().addToLog("Rootnode AABB size: " + std::to_string(currentPointCloud->getSearchOctree()->root->nodeAABB.size), "OctreeEvents");
			debugLog::getInstance().addToLog("Rootnode AABB min: ", currentPointCloud->getSearchOctree()->root->nodeAABB.min, "OctreeEvents");
			debugLog::getInstance().addToLog("Rootnode AABB max: ", currentPointCloud->getSearchOctree()->root->nodeAABB.max, "OctreeEvents");
			debugLog::getInstance().addToLog("Max depth: " + std::to_string(currentPointCloud->getSearchOctree()->getDebugMaxNodeDepth()), "OctreeEvents");

			currentPath = "";
			currentPointCloud->wasFullyLoaded = true;
			currentPointCloud = nullptr;
			debugLog::getInstance().addToLog("currentPointCloud->wasFullyLoaded = true;", "testThread");
		}

		loadingDone = true;
		while (true)
		{
			Sleep(5);
			if (newJobReady.load())
				break;
		}
	}
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
	// Firstly we check if we can process this request right now
	// if we are working on different request we should decline this one.
	bool expected = true;
	if (!loadingDone.compare_exchange_strong(expected, false))
	{
		debugLog::getInstance().addToLog("loadingDone was false !", "OctreeEvents");
		return false;
	}

	currentPath = path;
	currentPointCloud = PointCloud;

	newJobReady = true;
	return true;
}

bool LoadManager::isLoadingDone()
{
	return loadingDone.load();
}