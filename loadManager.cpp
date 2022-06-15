#include "pch.h"
#include "loadManager.h"

LoadManager* LoadManager::_instance = nullptr;
#ifdef LOD_SYSTEM
std::vector<LODSetting> pointCloud::LODSettings = std::vector<LODSetting>();
#endif

void copyLAZvlr(laszip_header* dest, laszip_vlr_struct source)
{
	size_t i = 0;
	if (dest->vlrs)
	{
		// overwrite existing VLR ?
		for (i = 0; i < dest->number_of_variable_length_records; i++)
		{
			if ((strncmp(dest->vlrs[i].user_id, source.user_id, 16) == 0) && (dest->vlrs[i].record_id == source.record_id))
			{
				if (dest->vlrs[i].record_length_after_header)
				{
					dest->offset_to_point_data -= dest->vlrs[i].record_length_after_header;
					dest->vlrs[i].record_length_after_header = 0;
					delete[] dest->vlrs[i].data;
					dest->vlrs[i].data = 0;
				}
				break;
			}
		}

		// create new VLR
		if (i == dest->number_of_variable_length_records)
		{
			dest->number_of_variable_length_records++;
			dest->offset_to_point_data += 54;
			dest->vlrs = (laszip_vlr_struct*)realloc(dest->vlrs, sizeof(laszip_vlr_struct) * dest->number_of_variable_length_records);
			if (dest->vlrs == 0)
			{
				//sprintf(laszip_dll->error, "reallocating vlrs[%u] array", dest->number_of_variable_length_records);
				//return 1;
			}
		}
	}
	else
	{
		dest->number_of_variable_length_records = 1;
		dest->offset_to_point_data += 54;
		dest->vlrs = (laszip_vlr_struct*)malloc(sizeof(laszip_vlr_struct));
		if (dest->vlrs == 0)
		{
			//sprintf(laszip_dll->error, "allocating vlrs[1] array");
			//return 1;
		}
	}

	// zero the VLR
	memset(&(dest->vlrs[i]), 0, sizeof(laszip_vlr_struct));

	// copy the VLR
	dest->vlrs[i].reserved = 0x0;
	strncpy(dest->vlrs[i].user_id, source.user_id, 16);
	dest->vlrs[i].record_id = source.record_id;
	dest->vlrs[i].record_length_after_header = source.record_length_after_header;
	if (source.description)
	{
		strncpy(dest->vlrs[i].description, source.description, 32);
	}
	else
	{
		sprintf(dest->vlrs[i].description, "LASzip DLL");
	}

	if (source.record_length_after_header)
	{
		dest->offset_to_point_data += source.record_length_after_header;
		dest->vlrs[i].data = new unsigned char[source.record_length_after_header];
		memcpy(dest->vlrs[i].data, source.data, source.record_length_after_header);
	}
}

void copyLAZFileHeader(laszip_header* dest, laszip_header* source)
{
	dest->file_source_ID = source->file_source_ID;
	dest->global_encoding = source->global_encoding;
	dest->project_ID_GUID_data_1 = source->project_ID_GUID_data_1;
	dest->project_ID_GUID_data_2 = source->project_ID_GUID_data_2;
	dest->project_ID_GUID_data_3 = source->project_ID_GUID_data_3;
	memcpy(dest->project_ID_GUID_data_4, source->project_ID_GUID_data_4, 8);
	dest->version_major = source->version_major;
	dest->version_minor = source->version_minor;
	memcpy(dest->system_identifier, source->system_identifier, 32);
	memcpy(dest->generating_software, source->generating_software, 32);
	dest->file_creation_day = source->file_creation_day;
	dest->file_creation_year = source->file_creation_year;
	dest->header_size = source->header_size;
	dest->offset_to_point_data = source->header_size;
	dest->number_of_variable_length_records = source->number_of_variable_length_records;
	dest->point_data_format = source->point_data_format;
	dest->point_data_record_length = source->point_data_record_length;
	dest->number_of_point_records = source->number_of_point_records;
	for (int i = 0; i < 5; i++)
	{
		dest->number_of_points_by_return[i] = source->number_of_points_by_return[i];
	}
	dest->x_scale_factor = source->x_scale_factor;
	dest->y_scale_factor = source->y_scale_factor;
	dest->z_scale_factor = source->z_scale_factor;
	dest->x_offset = source->x_offset;
	dest->y_offset = source->y_offset;
	dest->z_offset = source->z_offset;
	dest->max_x = source->max_x;
	dest->min_x = source->min_x;
	dest->max_y = source->max_y;
	dest->min_y = source->min_y;
	dest->max_z = source->max_z;
	dest->min_z = source->min_z;

	// LAS 1.3 and higher only
	dest->start_of_waveform_data_packet_record = source->start_of_waveform_data_packet_record;

	// LAS 1.4 and higher only
	dest->start_of_first_extended_variable_length_record = source->start_of_first_extended_variable_length_record;
	dest->number_of_extended_variable_length_records = source->number_of_extended_variable_length_records;
	dest->extended_number_of_point_records = source->extended_number_of_point_records;
	for (int i = 0; i < 15; i++)
	{
		dest->extended_number_of_points_by_return[i] = source->extended_number_of_points_by_return[i];
	}

	// we may modify output because we omit any user defined data that may be ** the header
	if (source->user_data_in_header_size)
	{
		dest->header_size -= source->user_data_in_header_size;
		dest->offset_to_point_data -= source->user_data_in_header_size;
	}

	// add all the VLRs
	if (source->number_of_variable_length_records)
	{
		for (size_t i = 0; i < source->number_of_variable_length_records; i++)
		{
			copyLAZvlr(dest, source->vlrs[i]);
		}
	}

	// we may modify output because we omit any user defined data that may be *after* the header
	if (source->user_data_after_header_size)
	{
		//fprintf(stderr, "omitting %d bytes of user_data_after_header\n", source->user_data_after_header_size);
	}
}

void LoadManager::loadFunc()
{
	while (true)
	{
		if (currentPath != "" && currentPath.size() > 4 && std::filesystem::exists(currentPath))
		{
			newJobReady = false;

			double rangeX = 0.0f;
			double rangeY = 0.0f;
			double rangeZ = 0.0f;

			double newMinX = DBL_MAX;
			double newMaxX = -DBL_MAX;
			double newMinY = DBL_MAX;
			double newMaxY = -DBL_MAX;
			double newMinZ = DBL_MAX;
			double newMaxZ = -DBL_MAX;

			LAZFileInfo* fileInfo = nullptr;

			debugLog::getInstance().addToLog("Coping file from original location: " + currentPath, "File_Load_Log");
			debugLog::getInstance().addToLog("To new location: " + currentProjectPath + "/Resources/CCOM/PointCloud/Data/", "File_Load_Log");
			
			std::filesystem::path newLocation = currentProjectPath + "/Resources/CCOM/PointCloud/Data/" + std::filesystem::path(currentPath).filename().string();
			if (!std::filesystem::exists(newLocation) && std::filesystem::exists(currentProjectPath + "/Resources/CCOM/PointCloud/Data/"))
			{
				std::filesystem::copy(currentPath, currentProjectPath + "/Resources/CCOM/PointCloud/Data/" + std::filesystem::path(currentPath).filename().string());
			}
			else
			{
				newLocation = currentPath;
			}

			currentPointCloud->filePath = newLocation.string();

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
#ifdef LOD_SYSTEM
				currentPointCloud->LODs.resize(pointCloud::LODSettings.size());
#endif

				char* rawPointData = new char[pointCount * sizeof(MeshVertex)];
				file.read(rawPointData, pointCount * sizeof(MeshVertex));

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

#ifdef LOD_SYSTEM
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
#endif
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

				fileInfo = new LAZFileInfo();
				copyLAZFileHeader(&fileInfo->header, header);
				fileInfo->compressed = is_compressed;

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

#ifdef USE_QUADS_NOT_POINTS
				currentPointCloud->vertexInfo.resize(npoints * 6);
#else
				currentPointCloud->vertexInfo.resize(npoints);
				std::vector<glm::dvec3> tempVertex;
				tempVertex.resize(npoints);
#endif
				currentPointCloud->vertexIntensity.resize(npoints);

				currentPointCloud->min = glm::dvec3(DBL_MAX);
				currentPointCloud->max = glm::dvec3(-DBL_MAX);

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

				class LASvlr_geo_keys
				{
				public:
					unsigned short key_directory_version;
					unsigned short key_revision;
					unsigned short minor_revision;
					unsigned short number_of_keys;
				};

				struct GeoProjectionGeoKeys
				{
					unsigned short key_id;
					unsigned short tiff_tag_location;
					unsigned short count;
					unsigned short value_offset;
				};

				for (size_t i = 0; i < header->number_of_variable_length_records; i++)
				{
					if (header->vlrs[i].record_id == 34735) // GeoKeyDirectoryTag
					{
						LASvlr_geo_keys* read = new LASvlr_geo_keys[1];
						memcpy_s(read, sizeof(unsigned short) * 4, header->vlrs[i].data, sizeof(unsigned short) * 4);

						GeoProjectionGeoKeys* readKeys = new GeoProjectionGeoKeys[read->number_of_keys];
						memcpy_s(readKeys, read->number_of_keys * sizeof(GeoProjectionGeoKeys),
										   header->vlrs[i].data + sizeof(unsigned short) * 4, read->number_of_keys * sizeof(unsigned short) * 4);

						for (int j = 0; j < read->number_of_keys; j++)
						{
							if (readKeys[j].key_id != 3072)
								continue;

							//std::cout << "index: " << j << std::endl;
							//std::cout << "key_id: " << readKeys[j].key_id << std::endl;
							//std::cout << "tiff_tag_location: " << readKeys[j].tiff_tag_location << std::endl;
							//std::cout << "count: " << readKeys[j].count << std::endl;
							//std::cout << "value_offset: " << readKeys[j].value_offset << std::endl;

							debugLog::getInstance().addToLog("EPSG: " + std::to_string(readKeys[j].value_offset), "EPSG");
							currentPointCloud->EPSG = readKeys[j].value_offset;
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

#ifdef LOD_SYSTEM
				currentPointCloud->LODs.resize(pointCloud::LODSettings.size());
#endif

				debugLog::getInstance().addToLog("header->x_scale_factor : " + std::to_string(header->x_scale_factor), "File_Load_Log");
				debugLog::getInstance().addToLog("header->z_scale_factor : " + std::to_string(header->z_scale_factor), "File_Load_Log");
				debugLog::getInstance().addToLog("header->y_scale_factor : " + std::to_string(header->y_scale_factor), "File_Load_Log");

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
					fileInfo->LAZpoints.push_back(laszip_point(*point));

					// point->X -> lonX, point->Y -> latY, point->Z -> depth
					double readX = point->X * header->x_scale_factor;
					double readY = point->Z * header->z_scale_factor;
					double readZ = point->Y * header->y_scale_factor;

					//debugLog::getInstance().addToLog("float readZ : " + std::to_string(p_count), "testThread");

#ifdef USE_QUADS_NOT_POINTS
					//currentPointCloud->vertexInfo[p_count].position[0] = readX;
					//currentPointCloud->vertexInfo[p_count].position[1] = readY;
					//currentPointCloud->vertexInfo[p_count].position[2] = readZ;

					double quad_size = 2.0;
					int tempCount = p_count * 6;
					if (tempCount < currentPointCloud->vertexInfo.size())
					{
						currentPointCloud->vertexInfo[tempCount].position[0] = readX + quad_size;
						currentPointCloud->vertexInfo[tempCount].position[1] = readY + quad_size;
						currentPointCloud->vertexInfo[tempCount].position[2] = readZ + 0.0f;
						currentPointCloud->vertexInfo[tempCount].color[0] = unsigned char(point->rgb[0] / float(1 << 16) * 255);
						currentPointCloud->vertexInfo[tempCount].color[1] = unsigned char(point->rgb[1] / float(1 << 16) * 255);
						currentPointCloud->vertexInfo[tempCount].color[2] = unsigned char(point->rgb[2] / float(1 << 16) * 255);

						currentPointCloud->vertexInfo[tempCount + 1].position[0] = readX + -quad_size;
						currentPointCloud->vertexInfo[tempCount + 1].position[1] = readY + quad_size;
						currentPointCloud->vertexInfo[tempCount + 1].position[2] = readZ + 0.0f;
						currentPointCloud->vertexInfo[tempCount + 1].color[0] = unsigned char(point->rgb[0] / float(1 << 16) * 255);
						currentPointCloud->vertexInfo[tempCount + 1].color[1] = unsigned char(point->rgb[1] / float(1 << 16) * 255);
						currentPointCloud->vertexInfo[tempCount + 1].color[2] = unsigned char(point->rgb[2] / float(1 << 16) * 255);

						currentPointCloud->vertexInfo[tempCount + 2].position[0] = readX + -quad_size;
						currentPointCloud->vertexInfo[tempCount + 2].position[1] = readY + -quad_size;
						currentPointCloud->vertexInfo[tempCount + 2].position[2] = readZ + 0.0f;
						currentPointCloud->vertexInfo[tempCount + 2].color[0] = unsigned char(point->rgb[0] / float(1 << 16) * 255);
						currentPointCloud->vertexInfo[tempCount + 2].color[1] = unsigned char(point->rgb[1] / float(1 << 16) * 255);
						currentPointCloud->vertexInfo[tempCount + 2].color[2] = unsigned char(point->rgb[2] / float(1 << 16) * 255);

						currentPointCloud->vertexInfo[tempCount + 3].position[0] = readX + quad_size;
						currentPointCloud->vertexInfo[tempCount + 3].position[1] = readY + -quad_size;
						currentPointCloud->vertexInfo[tempCount + 3].position[2] = readZ + 0.0f;
						currentPointCloud->vertexInfo[tempCount + 3].color[0] = unsigned char(point->rgb[0] / float(1 << 16) * 255);
						currentPointCloud->vertexInfo[tempCount + 3].color[1] = unsigned char(point->rgb[1] / float(1 << 16) * 255);
						currentPointCloud->vertexInfo[tempCount + 3].color[2] = unsigned char(point->rgb[2] / float(1 << 16) * 255);

						currentPointCloud->vertexInfo[tempCount + 4].position[0] = readX + -quad_size;
						currentPointCloud->vertexInfo[tempCount + 4].position[1] = readY + -quad_size;
						currentPointCloud->vertexInfo[tempCount + 4].position[2] = readZ + 0.0f;
						currentPointCloud->vertexInfo[tempCount + 4].color[0] = unsigned char(point->rgb[0] / float(1 << 16) * 255);
						currentPointCloud->vertexInfo[tempCount + 4].color[1] = unsigned char(point->rgb[1] / float(1 << 16) * 255);
						currentPointCloud->vertexInfo[tempCount + 4].color[2] = unsigned char(point->rgb[2] / float(1 << 16) * 255);

						currentPointCloud->vertexInfo[tempCount + 5].position[0] = readX + quad_size;
						currentPointCloud->vertexInfo[tempCount + 5].position[1] = readY + quad_size;
						currentPointCloud->vertexInfo[tempCount + 5].position[2] = readZ + 0.0f;
						currentPointCloud->vertexInfo[tempCount + 5].color[0] = unsigned char(point->rgb[0] / float(1 << 16) * 255);
						currentPointCloud->vertexInfo[tempCount + 5].color[1] = unsigned char(point->rgb[1] / float(1 << 16) * 255);
						currentPointCloud->vertexInfo[tempCount + 5].color[2] = unsigned char(point->rgb[2] / float(1 << 16) * 255);
					}

#else
					//currentPointCloud->vertexInfo[p_count].position[0] = readX;
					//currentPointCloud->vertexInfo[p_count].position[1] = readY;
					//currentPointCloud->vertexInfo[p_count].position[2] = readZ;

					tempVertex[p_count].x = readX;
					tempVertex[p_count].y = readY;
					tempVertex[p_count].z = readZ;

					if (p_count < 1000)
					{
						debugLog::getInstance().addToLog("readX : " + std::to_string(readX), "precision");
						debugLog::getInstance().addToLog("readY : " + std::to_string(readY), "precision");
						debugLog::getInstance().addToLog("readZ : " + std::to_string(readZ), "precision");

						debugLog::getInstance().addToLog("currentPointCloud->vertexInfo[p_count].position[0] : " + std::to_string(currentPointCloud->vertexInfo[p_count].position[0]), "precision");
						debugLog::getInstance().addToLog("currentPointCloud->vertexInfo[p_count].position[1] : " + std::to_string(currentPointCloud->vertexInfo[p_count].position[1]), "precision");
						debugLog::getInstance().addToLog("currentPointCloud->vertexInfo[p_count].position[2] : " + std::to_string(currentPointCloud->vertexInfo[p_count].position[2]), "precision");
					}

					currentPointCloud->vertexInfo[p_count].color[0] = unsigned char(point->rgb[0] / float(1 << 16) * 255);
					currentPointCloud->vertexInfo[p_count].color[1] = unsigned char(point->rgb[1] / float(1 << 16) * 255);
					currentPointCloud->vertexInfo[p_count].color[2] = unsigned char(point->rgb[2] / float(1 << 16) * 255);
#endif


#ifdef LOD_SYSTEM
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
#endif

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

				//debugLog::getInstance().addToLog("while (p_count < npoints)", "testThread");

				if (header->point_data_format == 1)
				{
					for (size_t i = 0; i < p_count; i++)
					{
						currentPointCloud->vertexInfo[i].color[0] = unsigned char(currentPointCloud->vertexIntensity[i] / maxIntensity * 255);
						currentPointCloud->vertexInfo[i].color[1] = unsigned char(currentPointCloud->vertexIntensity[i] / maxIntensity * 255);
						currentPointCloud->vertexInfo[i].color[2] = unsigned char(currentPointCloud->vertexIntensity[i] / maxIntensity * 255);

#ifdef LOD_SYSTEM
						for (size_t j = 0; j < currentPointCloud->LODs.size(); j++)
						{
							if (i % pointCloud::LODSettings[j].takeEach_Nth_Point == 0)
							{
								currentPointCloud->LODs[j].vertexInfo[i / pointCloud::LODSettings[j].takeEach_Nth_Point].color[0] = currentPointCloud->vertexInfo[i].color[0];
								currentPointCloud->LODs[j].vertexInfo[i / pointCloud::LODSettings[j].takeEach_Nth_Point].color[1] = currentPointCloud->vertexInfo[i].color[1];
								currentPointCloud->LODs[j].vertexInfo[i / pointCloud::LODSettings[j].takeEach_Nth_Point].color[2] = currentPointCloud->vertexInfo[i].color[2];
							}
						}
#endif
						
					}
				}

				currentPointCloud->initialZShift = currentPointCloud->max.z < currentPointCloud->min.z ? currentPointCloud->max.z : currentPointCloud->min.z;
				currentPointCloud->initialZShift = header->min_y - currentPointCloud->initialZShift;

				if (header->x_offset == 0 && header->y_offset == 0 && header->z_offset == 0)
				{
					debugLog::getInstance().addToLog("header does not contain offset", "File_Load_Log");

					rangeX = currentPointCloud->max.x - currentPointCloud->min.x;
					rangeY = currentPointCloud->max.y - currentPointCloud->min.y;
					rangeZ = currentPointCloud->max.z - currentPointCloud->min.z;

					currentPointCloud->adjustment.x = -currentPointCloud->min.x;
					currentPointCloud->adjustment.y = -currentPointCloud->min.y;
					currentPointCloud->adjustment.z = -currentPointCloud->min.z;
				}
				else
				{
					debugLog::getInstance().addToLog("header contain offset", "File_Load_Log");
					debugLog::getInstance().addToLog("header->x_offset : " + std::to_string(header->x_offset), "File_Load_Log");
					debugLog::getInstance().addToLog("header->y_offset : " + std::to_string(header->y_offset), "File_Load_Log");
					debugLog::getInstance().addToLog("header->z_offset : " + std::to_string(header->z_offset), "File_Load_Log");

					rangeX = currentPointCloud->max.x - currentPointCloud->min.x;
					rangeY = currentPointCloud->max.y - currentPointCloud->min.y;
					rangeZ = currentPointCloud->max.z - currentPointCloud->min.z;

					currentPointCloud->adjustment.x = -header->x_offset;
					currentPointCloud->adjustment.y = -header->y_offset;
					currentPointCloud->adjustment.z = -header->z_offset;
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
						//currentPointCloud->vertexInfo[i].position[0] = currentPointCloud->vertexInfo[i].position[0] + currentPointCloud->adjustment.x;
						//currentPointCloud->vertexInfo[i].position[1] = currentPointCloud->vertexInfo[i].position[1] + currentPointCloud->adjustment.y;
						//currentPointCloud->vertexInfo[i].position[2] = currentPointCloud->vertexInfo[i].position[2] + currentPointCloud->adjustment.z;
						
						tempVertex[i].x = tempVertex[i].x + currentPointCloud->adjustment.x;
						tempVertex[i].y = tempVertex[i].y + currentPointCloud->adjustment.y;
						tempVertex[i].z = tempVertex[i].z + currentPointCloud->adjustment.z;
					}
					else
					{
						//currentPointCloud->vertexInfo[i].position[0] = currentPointCloud->vertexInfo[i].position[0] + float(currentPointCloud->adjustment.x * header->x_scale_factor);
						//currentPointCloud->vertexInfo[i].position[1] = currentPointCloud->vertexInfo[i].position[1] + float(currentPointCloud->adjustment.y * header->y_scale_factor);
						//currentPointCloud->vertexInfo[i].position[2] = currentPointCloud->vertexInfo[i].position[2] + float(currentPointCloud->adjustment.z * header->z_scale_factor);
					
						tempVertex[i].x = tempVertex[i].x + currentPointCloud->adjustment.x * header->x_scale_factor;
						tempVertex[i].y = tempVertex[i].y + currentPointCloud->adjustment.y * header->y_scale_factor;
						tempVertex[i].z = tempVertex[i].z + currentPointCloud->adjustment.z * header->z_scale_factor;
					}

					if (newMinX > tempVertex[i].x)
						newMinX = tempVertex[i].x;

					if (newMaxX < tempVertex[i].x)
						newMaxX = tempVertex[i].x;

					if (newMinY > tempVertex[i].y)
						newMinY = tempVertex[i].y;

					if (newMaxY < tempVertex[i].y)
						newMaxY = tempVertex[i].y;

					if (newMinZ > tempVertex[i].z)
						newMinZ = tempVertex[i].z;

					if (newMaxZ < tempVertex[i].z)
						newMaxZ = tempVertex[i].z;


					currentPointCloud->vertexInfo[i].position[0] = float(tempVertex[i].x);
					currentPointCloud->vertexInfo[i].position[1] = float(tempVertex[i].y);
					currentPointCloud->vertexInfo[i].position[2] = float(tempVertex[i].z);

					if (i < 1000)
					{
						debugLog::getInstance().addToLog("2) currentPointCloud->vertexInfo[i].position[0] : " + std::to_string(currentPointCloud->vertexInfo[i].position[0]), "precision");
						debugLog::getInstance().addToLog("2) currentPointCloud->vertexInfo[i].position[1] : " + std::to_string(currentPointCloud->vertexInfo[i].position[1]), "precision");
						debugLog::getInstance().addToLog("2) currentPointCloud->vertexInfo[i].position[2] : " + std::to_string(currentPointCloud->vertexInfo[i].position[2]), "precision");
					}

#ifdef LOD_SYSTEM
					for (size_t j = 0; j < currentPointCloud->LODs.size(); j++)
					{
						if (i % pointCloud::LODSettings[j].takeEach_Nth_Point == 0)
						{
							currentPointCloud->LODs[j].vertexInfo[i / pointCloud::LODSettings[j].takeEach_Nth_Point].position[0] = currentPointCloud->vertexInfo[i].position[0];
							currentPointCloud->LODs[j].vertexInfo[i / pointCloud::LODSettings[j].takeEach_Nth_Point].position[1] = currentPointCloud->vertexInfo[i].position[1];
							currentPointCloud->LODs[j].vertexInfo[i / pointCloud::LODSettings[j].takeEach_Nth_Point].position[2] = currentPointCloud->vertexInfo[i].position[2];
						}
					}
#endif
					
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

//#ifdef USE_QUADS_NOT_POINTS
//			currentPointCloud->vertexInfo[0].position[0] = 10.0f;
//			currentPointCloud->vertexInfo[0].position[1] = 10.0f;
//			currentPointCloud->vertexInfo[0].position[2] = 0.0f;
//
//			currentPointCloud->vertexInfo[1].position[0] = -10.0f;
//			currentPointCloud->vertexInfo[1].position[1] = 10.0f;
//			currentPointCloud->vertexInfo[1].position[2] = 0.0f;
//
//			currentPointCloud->vertexInfo[2].position[0] = -10.0f;
//			currentPointCloud->vertexInfo[2].position[1] = -10.0f;
//			currentPointCloud->vertexInfo[2].position[2] = 0.0f;
//
//			currentPointCloud->vertexInfo[3].position[0] = 10.0f;
//			currentPointCloud->vertexInfo[3].position[1] = -10.0f;
//			currentPointCloud->vertexInfo[3].position[2] = 0.0f;
//
//			currentPointCloud->vertexInfo[4].position[0] = -10.0f;
//			currentPointCloud->vertexInfo[4].position[1] = -10.0f;
//			currentPointCloud->vertexInfo[4].position[2] = 0.0f;
//
//			currentPointCloud->vertexInfo[5].position[0] = 10.0f;
//			currentPointCloud->vertexInfo[5].position[1] = 10.0f;
//			currentPointCloud->vertexInfo[5].position[2] = 0.0f;
//#endif

			// saving original data
			currentPointCloud->originalData = currentPointCloud->vertexInfo;
			//currentPointCloud->originalData.resize(currentPointCloud->getPointCount());
			//for (size_t i = 0; i < currentPointCloud->originalData.size(); i++)
			//{
			//	currentPointCloud->originalData[i] = currentPointCloud->vertexInfo[i];
			//	/*currentPointCloud->originalData[i].color[1] = currentPointCloud->vertexInfo[i].color[1];
			//	currentPointCloud->originalData[i].color[2] = currentPointCloud->vertexInfo[i].color[2];

			//	currentPointCloud->originalData[i].color[0] = currentPointCloud->vertexInfo[i].color[0];
			//	currentPointCloud->originalData[i].color[1] = currentPointCloud->vertexInfo[i].color[1];
			//	currentPointCloud->originalData[i].color[2] = currentPointCloud->vertexInfo[i].color[2];*/
			//}

			debugLog::getInstance().addToLog("before initializeOctree", "testThread");
			debugLog::getInstance().addToLog("rangeXYZ: ", glm::vec3(rangeX, rangeY, rangeZ), "OctreeEvents");
			currentPointCloud->initializeOctree(rangeX, rangeY, rangeZ, glm::vec3(newMinX + rangeX / 2.0f, newMinY + rangeY / 2.0f, newMinZ + rangeZ / 2.0f));
			debugLog::getInstance().addToLog("after initializeOctree", "testThread");
			
			debugLog::getInstance().addToLog("Points inserted: " + std::to_string(currentPointCloud->getSearchOctree()->getPointsInserted()), "OctreeEvents");
			debugLog::getInstance().addToLog("Total nodes created: " + std::to_string(currentPointCloud->getSearchOctree()->getDebugNodeCount()), "OctreeEvents");
			debugLog::getInstance().addToLog("Rootnode AABB size: " + std::to_string(currentPointCloud->getSearchOctree()->root->nodeAABB.size), "OctreeEvents");
			debugLog::getInstance().addToLog("Rootnode AABB min: ", currentPointCloud->getSearchOctree()->root->nodeAABB.min, "OctreeEvents");
			debugLog::getInstance().addToLog("Rootnode AABB max: ", currentPointCloud->getSearchOctree()->root->nodeAABB.max, "OctreeEvents");
			debugLog::getInstance().addToLog("Max depth: " + std::to_string(currentPointCloud->getSearchOctree()->getDebugMaxNodeDepth()), "OctreeEvents");

			currentPointCloud->calculateApproximateGroundLevel();
			debugLog::getInstance().addToLog("ApproximateGroundLevel: " + std::to_string(currentPointCloud->getApproximateGroundLevel()), "OctreeEvents");

			currentPointCloud->loadedFrom = fileInfo;
			if (currentPointCloud->loadedFrom != nullptr)
				currentPointCloud->loadedFrom->resultingPointCloud = currentPointCloud;
			
			currentPath = "";
			currentPointCloud->wasFullyLoaded = true;
			currentPointCloud = nullptr;
			debugLog::getInstance().addToLog("currentPointCloud->wasFullyLoaded = true;", "testThread");
		}
		else
		{
			if (currentPath.size() <= 4)
				debugLog::getInstance().addToLog("filePath lenght was less then 4", "ERRORS");

			if (!std::filesystem::exists(currentPath))
				debugLog::getInstance().addToLog("File can't be found!", "ERRORS");
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

bool LoadManager::tryLoadPointCloudAsync(std::string path, std::string projectPath, pointCloud* PointCloud)
{
	// Firstly we check if we can process this request right now
	// if we are working on different request we should decline this one.
	bool expected = true;
	if (!loadingDone.compare_exchange_strong(expected, false))
	{
		debugLog::getInstance().addToLog("Loading thread was in work !", "OctreeEvents");
		return false;
	}

	currentPath = path;
	currentProjectPath = projectPath;
	currentPointCloud = PointCloud;

	newJobReady = true;
	return true;
}

void LoadManager::loadPointCloudAsync(std::string path, std::string projectPath, pointCloud* PointCloud)
{
	//debugLog::getInstance().addToLog("loadPointCloudAsync path: " + path, "LoadManager");
	bool expected = true;
	if (!loadingDone.compare_exchange_strong(expected, false))
	{
		pointCloudsToLoad.push_back(make_pair(path, PointCloud));
		//debugLog::getInstance().addToLog("pointCloudsToLoad.size(): " + std::to_string(pointCloudsToLoad.size()), "LoadManager");
		return;
	}

	//debugLog::getInstance().addToLog("loadPointCloudAsync loading " + path, "LoadManager");

	currentPath = path;
	currentProjectPath = projectPath;
	currentPointCloud = PointCloud;

	newJobReady = true;
}

void LoadManager::update()
{
	//debugLog::getInstance().addToLog("pointCloudsToLoad.size(): " + std::to_string(pointCloudsToLoad.size()), "LoadManager");
	if (pointCloudsToLoad.size() > 0)
	{
		bool expected = true;
		if (!loadingDone.compare_exchange_strong(expected, false))
			return;
			
		currentPath = pointCloudsToLoad[0].first;
		currentPointCloud = pointCloudsToLoad[0].second;

		newJobReady = true;
		pointCloudsToLoad.erase(pointCloudsToLoad.begin());
	}
}

bool LoadManager::isLoadingDone()
{
	return loadingDone.load();
}



// =============================================================================================

SaveManager* SaveManager::_instance = nullptr;

void SaveManager::saveFunc()
{
	while (true)
	{
		if (currentPath != "" && currentPath.size() > 4)
		{
			newJobReady = false;

			// if file is in our own format
			if (currentPath[currentPath.size() - 4] == '.' &&
				currentPath[currentPath.size() - 3] == 'c' &&
				currentPath[currentPath.size() - 2] == 'p' &&
				currentPath[currentPath.size() - 1] == 'c')
			{

			}
			else
			{
				/*pointCloud* currentPointCloud = getPointCloud(pointCloudID);
				if (currentPointCloud == nullptr)
					return;*/

				//LOG.addToLog("flag 0", "writeTest");
				laszip_POINTER laszip_writer;
				if (laszip_create(&laszip_writer))
				{
					LOG.addToLog("creating laszip writer failed", "DLL_ERRORS");
					return;
				}
				//LOG.addToLog("flag 1", "writeTest");

				int pointsToWrite = 0;
				for (size_t j = 0; j < currentPointCloud->getPointCount(); j++)
				{
					if (currentPointCloud->vertexInfo[j].position[0] != DELETED_POINTS_COORDINATE &&
						currentPointCloud->vertexInfo[j].position[1] != DELETED_POINTS_COORDINATE &&
						currentPointCloud->vertexInfo[j].position[2] != DELETED_POINTS_COORDINATE)
						pointsToWrite++;
				}
				//LOG.addToLog("flag 2", "writeTest");

				currentPointCloud->loadedFrom->header.number_of_point_records = pointsToWrite;
				if (laszip_set_header(laszip_writer, &currentPointCloud->loadedFrom->header))
				{
					LOG.addToLog("setting header for laszip writer failed", "DLL_ERRORS");
				}
				//LOG.addToLog("flag 3", "writeTest");

				std::string fileName = currentPath;
				if (laszip_open_writer(laszip_writer, fileName.c_str(), true))
				{
					LOG.addToLog("opening laszip writer for " + fileName + " failed", "DLL_ERRORS");

					laszip_CHAR* error;
					if (laszip_get_error(laszip_writer, &error))
					{
						LOG.addToLog("getting error messages", "DLL_ERRORS");
					}
					LOG.addToLog("MESSAGE: " + std::string(error), "DLL_ERRORS");
					return;
				}

				//LOG.addToLog("flag 4", "writeTest");
				for (size_t j = 0; j < currentPointCloud->getPointCount(); j++)
				{
					//LOG.addToLog("iteration of (size_t j = 0; j < pointClouds[pointCloudIndex]->getPointCount(); j++)", "TEST");
					if (currentPointCloud->vertexInfo[j].position[0] != DELETED_POINTS_COORDINATE &&
						currentPointCloud->vertexInfo[j].position[1] != DELETED_POINTS_COORDINATE &&
						currentPointCloud->vertexInfo[j].position[2] != DELETED_POINTS_COORDINATE)
					{
						if (laszip_set_point(laszip_writer, &currentPointCloud->loadedFrom->LAZpoints[j]))
						{
							LOG.addToLog("setting point " + std::to_string(j) + " failed", "DLL_ERRORS");
							return;
						}

						if (laszip_write_point(laszip_writer))
						{
							LOG.addToLog("writing point " + std::to_string(j) + " failed", "DLL_ERRORS");
							return;
						}
					}
				}

				// close the writer
				if (laszip_close_writer(laszip_writer))
				{
					LOG.addToLog("closing laszip writer failed", "DLL_ERRORS");
					return;
				}

				// destroy the writer
				if (laszip_destroy(laszip_writer))
				{
					LOG.addToLog("destroying laszip writer failed", "DLL_ERRORS");
					return;
				}
			}

			currentPath = "";
			currentPointCloud->wasFullyLoaded = true;
			currentPointCloud = nullptr;
		}
		else
		{
			if (currentPath.size() <= 4)
				debugLog::getInstance().addToLog("FilePath lenght was less then 4 in SaveManager::saveFunc()", "ERRORS");
		}

		savingDone = true;
		while (true)
		{
			Sleep(5);
			if (newJobReady.load())
				break;
		}
	}
}

SaveManager::SaveManager()
{
	savingDone = false;
	newJobReady = false;
	threadHandler = std::thread(&SaveManager::saveFunc, this);
	threadHandler.detach();
}

SaveManager::~SaveManager()
{
}

bool SaveManager::trySavePointCloudAsync(std::string path, pointCloud* PointCloud)
{
	// Firstly we check if we can process this request right now
	// if we are working on different request we should decline this one.
	bool expected = true;
	if (!savingDone.compare_exchange_strong(expected, false))
	{
		debugLog::getInstance().addToLog("Saving thread was in work !", "OctreeEvents");
		return false;
	}

	currentPath = path;
	currentPointCloud = PointCloud;

	newJobReady = true;
	return true;
}

bool SaveManager::isSaveDone()
{
	return savingDone.load();
}