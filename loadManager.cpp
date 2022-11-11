#include "pch.h"
#include "loadManager.h"

LoadManager* LoadManager::Instance = nullptr;

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

std::string GetWKT(std::string FileName, std::string VariableName = "wkt")
{
	FILE* fp = fopen(FileName.c_str(), "rb");

	while (1) {
		std::vector<char> local_header(30);
		size_t header_res = fread(&local_header[0], sizeof(char), 30, fp);
		if (header_res != 30)
			throw std::runtime_error("npz_load: failed fread");

		//if we've reached the global header, stop reading
		if (local_header[2] != 0x03 || local_header[3] != 0x04) break;

		//read in the variable name
		uint16_t name_len = *(uint16_t*)&local_header[26];
		std::string vname(name_len, ' ');
		size_t vname_res = fread(&vname[0], sizeof(char), name_len, fp);
		if (vname_res != name_len)
			throw std::runtime_error("npz_load: failed fread");
		vname.erase(vname.end() - 4, vname.end()); //erase the lagging .npy

		//read in the extra field
		uint16_t extra_field_len = *(uint16_t*)&local_header[28];
		fseek(fp, extra_field_len, SEEK_CUR); //skip past the extra field

		uint16_t compr_method = *reinterpret_cast<uint16_t*>(&local_header[0] + 8);
		uint32_t compr_bytes = *reinterpret_cast<uint32_t*>(&local_header[0] + 18);
		uint32_t uncompr_bytes = *reinterpret_cast<uint32_t*>(&local_header[0] + 22);

		if (vname == VariableName) {
			std::vector<size_t> shape;
			cnpy::NpyArray arr(shape, compr_bytes, false);
			size_t nread = fread(arr.data<char>(), 1, arr.num_bytes(), fp);

			auto data = arr.data_holder.get();

			std::string WKTData;
			for (size_t i = 0; i < data->size(); i++)
			{
				if (data->operator[](i) != 0)
					WKTData.push_back(data->operator[](i));
			}

			fclose(fp);
			return WKTData;
		}
		else {
			//skip past the data
			uint32_t size = *(uint32_t*)&local_header[22];
			fseek(fp, size, SEEK_CUR);
		}
	}

	fclose(fp);
}

std::vector<std::string> GetEPSG(std::string TotalString)
{
	std::vector<std::string> result;
	static std::string LookingFor = "AUTHORITY[\"EPSG\",\"";

	while (TotalString.find(LookingFor) != std::string::npos)
	{
		size_t CurrentStartIndex = TotalString.find(LookingFor) + LookingFor.size();
		size_t CurrentEndIndex = 0;

		for (size_t i = CurrentStartIndex + 1; i < TotalString.size(); i++)
		{
			if (TotalString[i] == '\"')
			{
				CurrentEndIndex = i;
				break;
			}
		}

		if (CurrentEndIndex != 0)
		{
			std::string Temp = TotalString.substr(CurrentStartIndex, CurrentEndIndex - CurrentStartIndex);

			TotalString.erase(CurrentStartIndex - LookingFor.size(), CurrentEndIndex - (CurrentStartIndex - LookingFor.size()) + 1);

			result.push_back(Temp);
		}

	}

	return result;
}

NumPyMinMax GetMinMax(std::string FileName, std::string VariableName = "minmax")
{
	cnpy::NpyArray MinMaxNpy = cnpy::npz_load(FileName, "minmax");
	auto MinMaxNpyRawData = MinMaxNpy.data_holder.get();

	NumPyMinMax Result;

	char* Word = new char[8];
	size_t CurrentIndex = 0;
	for (size_t i = 0; i < 8; i++)
	{
		Word[i] = MinMaxNpyRawData->operator[](CurrentIndex + i);
	}
	Result.MinX = *reinterpret_cast<double*>(Word);

	CurrentIndex = 8;
	for (size_t i = 0; i < 8; i++)
	{
		Word[i] = MinMaxNpyRawData->operator[](CurrentIndex + i);
	}
	Result.MinY = *reinterpret_cast<double*>(Word);

	CurrentIndex = 16;
	for (size_t i = 0; i < 8; i++)
	{
		Word[i] = MinMaxNpyRawData->operator[](CurrentIndex + i);
	}
	Result.MinZ = *reinterpret_cast<double*>(Word);

	CurrentIndex = 24;
	for (size_t i = 0; i < 8; i++)
	{
		Word[i] = MinMaxNpyRawData->operator[](CurrentIndex + i);
	}
	Result.MinUncertainty = *reinterpret_cast<double*>(Word);

	CurrentIndex = 32;
	for (size_t i = 0; i < 8; i++)
	{
		Word[i] = MinMaxNpyRawData->operator[](CurrentIndex + i);
	}
	Result.MaxX = *reinterpret_cast<double*>(Word);

	CurrentIndex = 40;
	for (size_t i = 0; i < 8; i++)
	{
		Word[i] = MinMaxNpyRawData->operator[](CurrentIndex + i);
	}
	Result.MaxY = *reinterpret_cast<double*>(Word);

	CurrentIndex = 48;
	for (size_t i = 0; i < 8; i++)
	{
		Word[i] = MinMaxNpyRawData->operator[](CurrentIndex + i);
	}
	Result.MaxZ = *reinterpret_cast<double*>(Word);

	CurrentIndex = 56;
	for (size_t i = 0; i < 8; i++)
	{
		Word[i] = MinMaxNpyRawData->operator[](CurrentIndex + i);
	}
	Result.MaxUncertainty = *reinterpret_cast<double*>(Word);

	return Result;
}

void LoadManager::LoadFunc(void* InputData, void* OutputData)
{
	InfoForLoading* Info = reinterpret_cast<InfoForLoading*>(InputData);
	pointCloud* PointCloud = Info->currentPointCloud;
	std::string Path = Info->currentPath;

	if (Path != "" && Path.size() > 4 && std::filesystem::exists(Path))
	{
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

		LOG.Add("Coping file from original location: " + Path, "File_Load_Log");
		LOG.Add("To new location: " + Info->currentProjectPath + "/Resources/CCOM/PointCloud/Data/", "File_Load_Log");
			
		std::filesystem::path newLocation = Info->currentProjectPath + "/Resources/CCOM/PointCloud/Data/" + std::filesystem::path(Path).filename().string();
		if (!std::filesystem::exists(newLocation) && std::filesystem::exists(Info->currentProjectPath + "/Resources/CCOM/PointCloud/Data/"))
		{
			std::filesystem::copy(Path, Info->currentProjectPath + "/Resources/CCOM/PointCloud/Data/" + std::filesystem::path(Path).filename().string());
		}
		else
		{
			newLocation = Path;
		}

		PointCloud->filePath = newLocation.string();

		// if file is in our own format
		if (Path[Path.size() - 4] == '.' &&
			Path[Path.size() - 3] == 'c' &&
			Path[Path.size() - 2] == 'p' &&
			Path[Path.size() - 1] == 'c')
		{
			std::fstream file;
			file.open(Path, std::ios::in | std::ios::binary);

			char* buffer = new char[sizeof(float)];

			// Read how many points does the file have.
			file.read(buffer, sizeof(int));
			int pointCount = *(int*)buffer;

			// Read initialXShift and initialZShift.
			file.read(buffer, sizeof(double));
			PointCloud->initialXShift = *(double*)buffer;
			file.read(buffer, sizeof(double));
			PointCloud->initialZShift = *(double*)buffer;

			// Read adjustment.
			file.read(buffer, sizeof(float));
			PointCloud->adjustment[0] = *(float*)buffer;
			file.read(buffer, sizeof(float));
			PointCloud->adjustment[1] = *(float*)buffer;
			file.read(buffer, sizeof(float));
			PointCloud->adjustment[2] = *(float*)buffer;

			// Read min and max.
			file.read(buffer, sizeof(float));
			PointCloud->min[0] = *(float*)buffer;
			file.read(buffer, sizeof(float));
			PointCloud->min[1] = *(float*)buffer;
			file.read(buffer, sizeof(float));
			PointCloud->min[2] = *(float*)buffer;

			file.read(buffer, sizeof(float));
			PointCloud->max[0] = *(float*)buffer;
			file.read(buffer, sizeof(float));
			PointCloud->max[1] = *(float*)buffer;
			file.read(buffer, sizeof(float));
			PointCloud->max[2] = *(float*)buffer;

			newMinX = PointCloud->min.x;
			newMaxX = PointCloud->max.x;
			newMinY = PointCloud->min.y;
			newMaxY = PointCloud->max.y;
			newMinZ = PointCloud->min.z;
			newMaxZ = PointCloud->max.z;

			rangeX = PointCloud->max.x - PointCloud->min.x;
			rangeY = PointCloud->max.y - PointCloud->min.y;
			rangeZ = PointCloud->max.z - PointCloud->min.z;

			LOG.Add("rangeX: " + std::to_string(rangeX), "File_Load_Log");
			LOG.Add("rangeY: " + std::to_string(rangeY), "File_Load_Log");
			LOG.Add("rangeZ: " + std::to_string(rangeZ), "File_Load_Log");

			LOG.Add("adjustment.x: " + std::to_string(PointCloud->adjustment.x), "File_Load_Log");
			LOG.Add("adjustment.y: " + std::to_string(PointCloud->adjustment.y), "File_Load_Log");
			LOG.Add("adjustment.z: " + std::to_string(PointCloud->adjustment.z), "File_Load_Log");

			LOG.Add("contains " + std::to_string(pointCount) + " points", "File_Load_Log");

			if (PointCloud == nullptr)
				LOG.Add("PointCloud = nullptr", "File_Load_Log");

			PointCloud->vertexInfo.resize(pointCount);
#ifdef LOD_SYSTEM
			PointCloud->LODs.resize(pointCloud::LODSettings.size());
#endif

			char* rawPointData = new char[pointCount * sizeof(MeshVertex)];
			file.read(rawPointData, pointCount * sizeof(MeshVertex));

			MeshVertex* temp = (MeshVertex*)rawPointData;
			for (size_t i = 0; i < pointCount; i++)
			{
				/*LOG.Add("(temp + i * sizeof(MeshVertex))->position.x: " + std::to_string((temp + i * sizeof(MeshVertex))->position.x), "testLoad");
				LOG.Add("(temp + i * sizeof(MeshVertex))->position.y: " + std::to_string((temp + i * sizeof(MeshVertex))->position.y), "testLoad");
				LOG.Add("(temp + i * sizeof(MeshVertex))->position.z: " + std::to_string((temp + i * sizeof(MeshVertex))->position.z), "testLoad");

				LOG.Add("(temp + i * sizeof(MeshVertex))->color[0]: " + std::to_string((temp + i * sizeof(MeshVertex))->color[0]), "testLoad");
				LOG.Add("(temp + i * sizeof(MeshVertex))->color[1]: " + std::to_string((temp + i * sizeof(MeshVertex))->color[1]), "testLoad");
				LOG.Add("(temp + i * sizeof(MeshVertex))->color[2]: " + std::to_string((temp + i * sizeof(MeshVertex))->color[2]), "testLoad");
				LOG.Add("(temp + i * sizeof(MeshVertex))->color[3]: " + std::to_string((temp + i * sizeof(MeshVertex))->color[3]), "testLoad");*/
				
				PointCloud->vertexInfo[i].position[0] = (temp + i)->position.x;
				PointCloud->vertexInfo[i].position[1] = (temp + i)->position.y;
				PointCloud->vertexInfo[i].position[2] = (temp + i)->position.z;
				PointCloud->vertexInfo[i].color[0] = (temp + i)->color[0];
				PointCloud->vertexInfo[i].color[1] = (temp + i)->color[1];
				PointCloud->vertexInfo[i].color[2] = (temp + i)->color[2];

#ifdef LOD_SYSTEM
				for (size_t j = 0; j < pointCloud::LODSettings.size(); j++)
				{
					if (i % pointCloud::LODSettings[j].takeEach_Nth_Point == 0)
					{
						PointCloud->LODs[j].vertexInfo.resize(PointCloud->LODs[j].vertexInfo.size() + 1);
						PointCloud->LODs[j].vertexInfo.back().position[0] = PointCloud->vertexInfo[i].position[0];
						PointCloud->LODs[j].vertexInfo.back().position[1] = PointCloud->vertexInfo[i].position[1];
						PointCloud->LODs[j].vertexInfo.back().position[2] = PointCloud->vertexInfo[i].position[2];
						PointCloud->LODs[j].vertexInfo.back().color[0] = PointCloud->vertexInfo[i].color[0];
						PointCloud->LODs[j].vertexInfo.back().color[1] = PointCloud->vertexInfo[i].color[1];
						PointCloud->LODs[j].vertexInfo.back().color[2] = PointCloud->vertexInfo[i].color[2];
					}
				}
#endif
			}
		}
		else if ((Path[Path.size() - 4] == '.' &&
				  Path[Path.size() - 3] == 'n' &&
				  Path[Path.size() - 2] == 'p' &&
				  Path[Path.size() - 1] == 'y')
				  
				  ||

				 (Path[Path.size() - 4] == '.' &&
				  Path[Path.size() - 3] == 'n' &&
				  Path[Path.size() - 2] == 'p' &&
				  Path[Path.size() - 1] == 'z'))
		{
			PointCloud->NumPy = new NumPyInfo();
			PointCloud->NumPy->WKT = GetWKT(Path);
			PointCloud->NumPy->WKT.erase(PointCloud->NumPy->WKT.begin(),
			PointCloud->NumPy->WKT.begin() + PointCloud->NumPy->WKT.find("PROJCS"));

			PointCloud->NumPy->AllEPSG = GetEPSG(PointCloud->NumPy->WKT);
			PointCloud->NumPy->MinMax = GetMinMax(Path);

			LOG.Add("EPSG: " + PointCloud->NumPy->AllEPSG.back(), "EPSG");
			PointCloud->EPSG = atoi(PointCloud->NumPy->AllEPSG.back().c_str());

			cnpy::NpyArray DataVar = cnpy::npz_load(Path, "data");

			auto FinalData = DataVar.data_holder.get();
			size_t Count = DataVar.num_vals;
			size_t SizePerValue = FinalData->size() / Count;

			double UncertaintyMin = DBL_MAX;
			double UncertaintyMax = -DBL_MAX;

			int Iteration = 0;
			char* word = new char[8];
			for (size_t i = 0; i < FinalData->size() / 8; i++)
			{
				for (size_t j = 0; j < 8; j++)
				{
				    word[j] = FinalData->operator[](i * 8 + j);
				}

				double tempValue = *reinterpret_cast<double*>(word);
				    
				if (Iteration == 0)
				{
					PointCloud->NumPy->LoadedRawData.resize(PointCloud->NumPy->LoadedRawData.size() + 1);
					PointCloud->NumPy->LoadedRawData.back().X = tempValue;

					PointCloud->vertexInfo.resize(PointCloud->vertexInfo.size() + 1);
					PointCloud->vertexInfo.back().position.x = tempValue;

					if (newMinX > tempValue)
						newMinX = tempValue;

					if (newMaxX < tempValue)
						newMaxX = tempValue;
				}
				// Z and Y is flipped.
				else if (Iteration == 1)
				{
					PointCloud->NumPy->LoadedRawData.back().Y = tempValue;
					PointCloud->vertexInfo.back().position.z = tempValue;

					if (newMinZ > tempValue)
						newMinZ = tempValue;

					if (newMaxZ < tempValue)
						newMaxZ = tempValue;
				}
				else if (Iteration == 2)
				{
					PointCloud->NumPy->LoadedRawData.back().Z = tempValue;
					PointCloud->vertexInfo.back().position.y = tempValue;

					if (newMinY > tempValue)
						newMinY = tempValue;

					if (newMaxY < tempValue)
						newMaxY = tempValue;
						
				}
				else if (Iteration == 3)
				{
					PointCloud->NumPy->LoadedRawData.back().Uncertainty = tempValue;

					if (UncertaintyMin > tempValue)
						UncertaintyMin = tempValue;

					if (UncertaintyMax < tempValue)
						UncertaintyMax = tempValue;
				}

				Iteration++;
				if (Iteration > 3)
				    Iteration = 0;
			}

			rangeX = newMaxX - newMinX;
			rangeY = newMaxY - newMinY;
			rangeZ = newMaxZ - newMinZ;

			PointCloud->adjustment.x = newMinX;
			PointCloud->adjustment.y = newMinY;
			PointCloud->adjustment.z = newMinZ;

			newMinX = DBL_MAX;
			newMaxX = -DBL_MAX;
			newMinY = DBL_MAX;
			newMaxY = -DBL_MAX;
			newMinZ = DBL_MAX;
			newMaxZ = -DBL_MAX;

			double UncertaintyRange = UncertaintyMax - UncertaintyMin;
			glm::vec3 CertainPointsColor = glm::vec3(1.0f);
			glm::vec3 UnCertainPointsColor = glm::vec3(1.0f, 0.0f, 0.0f);

			for (int i = 0; i < PointCloud->vertexInfo.size(); i++)
			{
				PointCloud->vertexInfo[i].position.x = PointCloud->vertexInfo[i].position.x - PointCloud->adjustment.x;
				PointCloud->vertexInfo[i].position.y = PointCloud->vertexInfo[i].position.y - PointCloud->adjustment.y;
				PointCloud->vertexInfo[i].position.z = PointCloud->vertexInfo[i].position.z - PointCloud->adjustment.z;
					
				if (newMinX > PointCloud->vertexInfo[i].position.x)
					newMinX = PointCloud->vertexInfo[i].position.x;

				if (newMaxX < PointCloud->vertexInfo[i].position.x)
					newMaxX = PointCloud->vertexInfo[i].position.x;

				if (newMinY > PointCloud->vertexInfo[i].position.y)
					newMinY = PointCloud->vertexInfo[i].position.y;

				if (newMaxY < PointCloud->vertexInfo[i].position.y)
					newMaxY = PointCloud->vertexInfo[i].position.y;

				if (newMinZ > PointCloud->vertexInfo[i].position.z)
					newMinZ = PointCloud->vertexInfo[i].position.z;

				if (newMaxZ < PointCloud->vertexInfo[i].position.z)
					newMaxZ = PointCloud->vertexInfo[i].position.z;

				double HowCertainPointIs = (UncertaintyMax - PointCloud->NumPy->LoadedRawData[i].Uncertainty) / UncertaintyRange;
				glm::vec3 ColorMixFactor = glm::vec3(HowCertainPointIs);

				glm::vec3 Color = CertainPointsColor * ColorMixFactor + UnCertainPointsColor * (glm::vec3(1.0f) - ColorMixFactor);

				PointCloud->vertexInfo[i].color[0] = unsigned char(Color.x * 255);
				PointCloud->vertexInfo[i].color[1] = unsigned char(Color.y * 255);
				PointCloud->vertexInfo[i].color[2] = unsigned char(Color.z * 255);
			}

			LOG.Add("newMinX: " + std::to_string(newMinX), "File_Load_Log");
			LOG.Add("newMinY: " + std::to_string(newMinY), "File_Load_Log");
			LOG.Add("newMinZ: " + std::to_string(newMinZ), "File_Load_Log");
		}
		else
		{
			// create the reader
			laszip_POINTER laszip_reader;

			laszip_I32 error = laszip_create(&laszip_reader);
			if (error)
			{
				LOG.Add("Creating laszip reader failed", "DLL_ERRORS");
				LOG.Add("Error: " + std::to_string(error), "DLL_ERRORS");
				return;
			}

			// open the reader
			laszip_BOOL is_compressed = 0;
			if (laszip_open_reader(laszip_reader, Path.c_str(), &is_compressed))
			{
				LOG.Add("opening laszip reader for " + Path + " failed", "DLL_ERRORS");
				return;
			}

			// get a pointer to the header of the reader that was just populated
			laszip_header* header;
			if (laszip_get_header_pointer(laszip_reader, &header))
			{
				LOG.Add("getting header pointer from laszip reader failed", "DLL_ERRORS");
				return;
			}

			fileInfo = new LAZFileInfo();
			copyLAZFileHeader(&fileInfo->header, header);
			fileInfo->compressed = is_compressed;

			LOG.Add("Compressed: " + std::string(is_compressed ? "true" : "false"), "File_Load_Log");
			LOG.Add("Signature: " + std::string(header->generating_software), "File_Load_Log");
			LOG.Add("Points count: " + std::to_string(header->number_of_point_records), "File_Load_Log");
			LOG.Add("X Min: " + std::to_string(header->min_x), "File_Load_Log");
			LOG.Add("X Max: " + std::to_string(header->max_x), "File_Load_Log");
			LOG.Add("Y Min: " + std::to_string(header->min_y), "File_Load_Log");
			LOG.Add("Y Max: " + std::to_string(header->max_y), "File_Load_Log");
			LOG.Add("Z Min: " + std::to_string(header->min_z), "File_Load_Log");
			LOG.Add("Z Max: " + std::to_string(header->max_z), "File_Load_Log");

			// how many points does the file have
			laszip_U64 npoints = (header->number_of_point_records ? header->number_of_point_records : header->extended_number_of_point_records);

			LOG.Add("contains " + std::to_string(npoints) + " points", "File_Load_Log");

			if (PointCloud == nullptr)
				LOG.Add("PointCloud = nullptr", "File_Load_Log");

#ifdef USE_QUADS_NOT_POINTS
			PointCloud->vertexInfo.resize(npoints * 6);
#else
			PointCloud->vertexInfo.resize(npoints);
			std::vector<glm::dvec3> tempVertex;
			tempVertex.resize(npoints);
#endif
			PointCloud->vertexIntensity.resize(npoints);

			PointCloud->min = glm::dvec3(DBL_MAX);
			PointCloud->max = glm::dvec3(-DBL_MAX);

			for (size_t i = 0; i < header->number_of_variable_length_records; i++)
			{
				if (header->vlrs[i].record_length_after_header)
				{
					std::string text = reinterpret_cast<char*>(header->vlrs[i].data);

					size_t position = text.find("NAD_1983_2011_UTM_Zone_");
					size_t position1 = text.find("UTM zone ");

					if (position != std::string::npos)
					{
						PointCloud->spatialInfo = text;
						LOG.Add("spatialInfo: " + PointCloud->spatialInfo, "File_Load_Log");
						PointCloud->UTMZone = text.substr(position + strlen("NAD_1983_2011_UTM_Zone_"), 3);
						LOG.Add("UTMZone: " + PointCloud->UTMZone, "File_Load_Log");
						break;
					}
					else if (position1 != std::string::npos)
					{
						PointCloud->spatialInfo = text;
						LOG.Add("spatialInfo: " + PointCloud->spatialInfo, "File_Load_Log");
						PointCloud->UTMZone = text.substr(position + strlen("UTM zone "), 3);
						LOG.Add("UTMZone: " + PointCloud->UTMZone, "File_Load_Log");
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

			PointCloud->EPSG = 0;
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

						LOG.Add("EPSG: " + std::to_string(readKeys[j].value_offset), "EPSG");
						PointCloud->EPSG = readKeys[j].value_offset;
					}
				}
			}

			// get a pointer to the points that will be read
			laszip_point* point;
			if (laszip_get_point_pointer(laszip_reader, &point))
			{
				LOG.Add("getting point pointer from laszip reader failed", "DLL_ERRORS");
				return;
			}

#ifdef LOD_SYSTEM
			PointCloud->LODs.resize(pointCloud::LODSettings.size());
#endif

			LOG.Add("header->x_scale_factor : " + std::to_string(header->x_scale_factor), "File_Load_Log");
			LOG.Add("header->z_scale_factor : " + std::to_string(header->z_scale_factor), "File_Load_Log");
			LOG.Add("header->y_scale_factor : " + std::to_string(header->y_scale_factor), "File_Load_Log");

			// read the points
			laszip_U64 p_count = 0;
			std::vector<MeshVertex> points;
			float maxIntensity = -FLT_MAX;
			while (p_count < npoints)
			{
				// read a point
				if (laszip_read_point(laszip_reader))
				{
					LOG.Add("reading point " + std::to_string(p_count) + " failed", "DLL_ERRORS");
					return;
				}

				//LOG.Add("if (laszip_read_point(laszip_reader)): " + std::to_string(p_count), "File_Load_Log");
				fileInfo->LAZpoints.push_back(laszip_point(*point));

				// point->X -> lonX, point->Y -> latY, point->Z -> depth
				double readX = point->X * header->x_scale_factor;
				double readY = point->Z * header->z_scale_factor;
				double readZ = point->Y * header->y_scale_factor;

				//LOG.Add("float readZ : " + std::to_string(p_count), "File_Load_Log");

#ifdef USE_QUADS_NOT_POINTS
				//PointCloud->vertexInfo[p_count].position[0] = readX;
				//PointCloud->vertexInfo[p_count].position[1] = readY;
				//PointCloud->vertexInfo[p_count].position[2] = readZ;

				double quad_size = 2.0;
				int tempCount = p_count * 6;
				if (tempCount < PointCloud->vertexInfo.size())
				{
					PointCloud->vertexInfo[tempCount].position[0] = readX + quad_size;
					PointCloud->vertexInfo[tempCount].position[1] = readY + quad_size;
					PointCloud->vertexInfo[tempCount].position[2] = readZ + 0.0f;
					PointCloud->vertexInfo[tempCount].color[0] = unsigned char(point->rgb[0] / float(1 << 16) * 255);
					PointCloud->vertexInfo[tempCount].color[1] = unsigned char(point->rgb[1] / float(1 << 16) * 255);
					PointCloud->vertexInfo[tempCount].color[2] = unsigned char(point->rgb[2] / float(1 << 16) * 255);

					PointCloud->vertexInfo[tempCount + 1].position[0] = readX + -quad_size;
					PointCloud->vertexInfo[tempCount + 1].position[1] = readY + quad_size;
					PointCloud->vertexInfo[tempCount + 1].position[2] = readZ + 0.0f;
					PointCloud->vertexInfo[tempCount + 1].color[0] = unsigned char(point->rgb[0] / float(1 << 16) * 255);
					PointCloud->vertexInfo[tempCount + 1].color[1] = unsigned char(point->rgb[1] / float(1 << 16) * 255);
					PointCloud->vertexInfo[tempCount + 1].color[2] = unsigned char(point->rgb[2] / float(1 << 16) * 255);

					PointCloud->vertexInfo[tempCount + 2].position[0] = readX + -quad_size;
					PointCloud->vertexInfo[tempCount + 2].position[1] = readY + -quad_size;
					PointCloud->vertexInfo[tempCount + 2].position[2] = readZ + 0.0f;
					PointCloud->vertexInfo[tempCount + 2].color[0] = unsigned char(point->rgb[0] / float(1 << 16) * 255);
					PointCloud->vertexInfo[tempCount + 2].color[1] = unsigned char(point->rgb[1] / float(1 << 16) * 255);
					PointCloud->vertexInfo[tempCount + 2].color[2] = unsigned char(point->rgb[2] / float(1 << 16) * 255);

					PointCloud->vertexInfo[tempCount + 3].position[0] = readX + quad_size;
					PointCloud->vertexInfo[tempCount + 3].position[1] = readY + -quad_size;
					PointCloud->vertexInfo[tempCount + 3].position[2] = readZ + 0.0f;
					PointCloud->vertexInfo[tempCount + 3].color[0] = unsigned char(point->rgb[0] / float(1 << 16) * 255);
					PointCloud->vertexInfo[tempCount + 3].color[1] = unsigned char(point->rgb[1] / float(1 << 16) * 255);
					PointCloud->vertexInfo[tempCount + 3].color[2] = unsigned char(point->rgb[2] / float(1 << 16) * 255);

					PointCloud->vertexInfo[tempCount + 4].position[0] = readX + -quad_size;
					PointCloud->vertexInfo[tempCount + 4].position[1] = readY + -quad_size;
					PointCloud->vertexInfo[tempCount + 4].position[2] = readZ + 0.0f;
					PointCloud->vertexInfo[tempCount + 4].color[0] = unsigned char(point->rgb[0] / float(1 << 16) * 255);
					PointCloud->vertexInfo[tempCount + 4].color[1] = unsigned char(point->rgb[1] / float(1 << 16) * 255);
					PointCloud->vertexInfo[tempCount + 4].color[2] = unsigned char(point->rgb[2] / float(1 << 16) * 255);

					PointCloud->vertexInfo[tempCount + 5].position[0] = readX + quad_size;
					PointCloud->vertexInfo[tempCount + 5].position[1] = readY + quad_size;
					PointCloud->vertexInfo[tempCount + 5].position[2] = readZ + 0.0f;
					PointCloud->vertexInfo[tempCount + 5].color[0] = unsigned char(point->rgb[0] / float(1 << 16) * 255);
					PointCloud->vertexInfo[tempCount + 5].color[1] = unsigned char(point->rgb[1] / float(1 << 16) * 255);
					PointCloud->vertexInfo[tempCount + 5].color[2] = unsigned char(point->rgb[2] / float(1 << 16) * 255);
				}

#else
				//PointCloud->vertexInfo[p_count].position[0] = readX;
				//PointCloud->vertexInfo[p_count].position[1] = readY;
				//PointCloud->vertexInfo[p_count].position[2] = readZ;

				tempVertex[p_count].x = readX;
				tempVertex[p_count].y = readY;
				tempVertex[p_count].z = readZ;

				/*if (p_count < 1000)
				{
					LOG.Add("readX : " + std::to_string(readX), "precision");
					LOG.Add("readY : " + std::to_string(readY), "precision");
					LOG.Add("readZ : " + std::to_string(readZ), "precision");

					LOG.Add("PointCloud->vertexInfo[p_count].position[0] : " + std::to_string(PointCloud->vertexInfo[p_count].position[0]), "precision");
					LOG.Add("PointCloud->vertexInfo[p_count].position[1] : " + std::to_string(PointCloud->vertexInfo[p_count].position[1]), "precision");
					LOG.Add("PointCloud->vertexInfo[p_count].position[2] : " + std::to_string(PointCloud->vertexInfo[p_count].position[2]), "precision");
				}*/

				PointCloud->vertexInfo[p_count].color[0] = unsigned char(point->rgb[0] / float(1 << 16) * 255);
				PointCloud->vertexInfo[p_count].color[1] = unsigned char(point->rgb[1] / float(1 << 16) * 255);
				PointCloud->vertexInfo[p_count].color[2] = unsigned char(point->rgb[2] / float(1 << 16) * 255);
#endif


#ifdef LOD_SYSTEM
				for (size_t i = 0; i < pointCloud::LODSettings.size(); i++)
				{
					if (p_count % pointCloud::LODSettings[i].takeEach_Nth_Point == 0)
					{
						PointCloud->LODs[i].vertexInfo.resize(PointCloud->LODs[i].vertexInfo.size() + 1);
						PointCloud->LODs[i].vertexInfo.back().position[0] = PointCloud->vertexInfo[p_count].position[0];
						PointCloud->LODs[i].vertexInfo.back().position[1] = PointCloud->vertexInfo[p_count].position[1];
						PointCloud->LODs[i].vertexInfo.back().position[2] = PointCloud->vertexInfo[p_count].position[2];
						PointCloud->LODs[i].vertexInfo.back().color[0] = PointCloud->vertexInfo[p_count].color[0];
						PointCloud->LODs[i].vertexInfo.back().color[1] = PointCloud->vertexInfo[p_count].color[1];
						PointCloud->LODs[i].vertexInfo.back().color[2] = PointCloud->vertexInfo[p_count].color[2];
					}
				}
#endif

				PointCloud->vertexIntensity[p_count] = point->intensity;
				if (maxIntensity < point->intensity)
					maxIntensity = point->intensity;

				if (PointCloud->min.x > readX)
					PointCloud->min.x = readX;

				if (PointCloud->max.x < readX)
					PointCloud->max.x = readX;

				if (PointCloud->min.y > readY)
					PointCloud->min.y = readY;

				if (PointCloud->max.y < readY)
					PointCloud->max.y = readY;

				if (PointCloud->min.z > readZ)
					PointCloud->min.z = readZ;

				if (PointCloud->max.z < readZ)
					PointCloud->max.z = readZ;

				p_count++;
			}

			PointCloud->RawMin = PointCloud->min;
			PointCloud->RawMax = PointCloud->max;

			LOG.Add("PointCloud->RawMin.x: " + std::to_string(PointCloud->RawMin.x), "File_Load_Log");

			//LOG.Add("while (p_count < npoints)", "File_Load_Log");

			if (header->point_data_format == 1)
			{
				for (size_t i = 0; i < p_count; i++)
				{
					PointCloud->vertexInfo[i].color[0] = unsigned char(PointCloud->vertexIntensity[i] / maxIntensity * 255);
					PointCloud->vertexInfo[i].color[1] = unsigned char(PointCloud->vertexIntensity[i] / maxIntensity * 255);
					PointCloud->vertexInfo[i].color[2] = unsigned char(PointCloud->vertexIntensity[i] / maxIntensity * 255);

#ifdef LOD_SYSTEM
					for (size_t j = 0; j < PointCloud->LODs.size(); j++)
					{
						if (i % pointCloud::LODSettings[j].takeEach_Nth_Point == 0)
						{
							PointCloud->LODs[j].vertexInfo[i / pointCloud::LODSettings[j].takeEach_Nth_Point].color[0] = PointCloud->vertexInfo[i].color[0];
							PointCloud->LODs[j].vertexInfo[i / pointCloud::LODSettings[j].takeEach_Nth_Point].color[1] = PointCloud->vertexInfo[i].color[1];
							PointCloud->LODs[j].vertexInfo[i / pointCloud::LODSettings[j].takeEach_Nth_Point].color[2] = PointCloud->vertexInfo[i].color[2];
						}
					}
#endif
						
				}
			}

			PointCloud->initialZShift = PointCloud->max.z < PointCloud->min.z ? PointCloud->max.z : PointCloud->min.z;
			PointCloud->initialZShift = header->min_y - PointCloud->initialZShift;

			if (header->x_offset == 0 && header->y_offset == 0 && header->z_offset == 0)
			{
				LOG.Add("header does not contain offset", "File_Load_Log");

				rangeX = PointCloud->max.x - PointCloud->min.x;
				rangeY = PointCloud->max.y - PointCloud->min.y;
				rangeZ = PointCloud->max.z - PointCloud->min.z;

				PointCloud->adjustment.x = -PointCloud->min.x;
				PointCloud->adjustment.y = -PointCloud->min.y;
				PointCloud->adjustment.z = -PointCloud->min.z;
			}
			else
			{
				LOG.Add("header contain offset", "File_Load_Log");
				LOG.Add("header->x_offset : " + std::to_string(header->x_offset), "File_Load_Log");
				LOG.Add("header->y_offset : " + std::to_string(header->y_offset), "File_Load_Log");
				LOG.Add("header->z_offset : " + std::to_string(header->z_offset), "File_Load_Log");

				rangeX = PointCloud->max.x - PointCloud->min.x;
				rangeY = PointCloud->max.y - PointCloud->min.y;
				rangeZ = PointCloud->max.z - PointCloud->min.z;

				PointCloud->adjustment.x = -header->x_offset;
				PointCloud->adjustment.y = -header->y_offset;
				PointCloud->adjustment.z = -header->z_offset;
			}

			LOG.Add("rangeX: " + std::to_string(rangeX), "File_Load_Log");
			LOG.Add("rangeY: " + std::to_string(rangeY), "File_Load_Log");
			LOG.Add("rangeZ: " + std::to_string(rangeZ), "File_Load_Log");

			LOG.Add("adjustment.x: " + std::to_string(PointCloud->adjustment.x), "File_Load_Log");
			LOG.Add("adjustment.y: " + std::to_string(PointCloud->adjustment.y), "File_Load_Log");
			LOG.Add("adjustment.z: " + std::to_string(PointCloud->adjustment.z), "File_Load_Log");

			for (int i = 0; i < npoints; i++)
			{
				if (header->x_offset == 0 && header->y_offset == 0 && header->z_offset == 0)
				{
					//PointCloud->vertexInfo[i].position[0] = PointCloud->vertexInfo[i].position[0] + PointCloud->adjustment.x;
					//PointCloud->vertexInfo[i].position[1] = PointCloud->vertexInfo[i].position[1] + PointCloud->adjustment.y;
					//PointCloud->vertexInfo[i].position[2] = PointCloud->vertexInfo[i].position[2] + PointCloud->adjustment.z;
						
					tempVertex[i].x = tempVertex[i].x + PointCloud->adjustment.x;
					tempVertex[i].y = tempVertex[i].y + PointCloud->adjustment.y;
					tempVertex[i].z = tempVertex[i].z + PointCloud->adjustment.z;
				}
				else
				{
					//PointCloud->vertexInfo[i].position[0] = PointCloud->vertexInfo[i].position[0] + float(PointCloud->adjustment.x * header->x_scale_factor);
					//PointCloud->vertexInfo[i].position[1] = PointCloud->vertexInfo[i].position[1] + float(PointCloud->adjustment.y * header->y_scale_factor);
					//PointCloud->vertexInfo[i].position[2] = PointCloud->vertexInfo[i].position[2] + float(PointCloud->adjustment.z * header->z_scale_factor);
					
					tempVertex[i].x = tempVertex[i].x + PointCloud->adjustment.x * header->x_scale_factor;
					tempVertex[i].y = tempVertex[i].y + PointCloud->adjustment.y * header->y_scale_factor;
					tempVertex[i].z = tempVertex[i].z + PointCloud->adjustment.z * header->z_scale_factor;
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


				PointCloud->vertexInfo[i].position[0] = float(tempVertex[i].x);
				PointCloud->vertexInfo[i].position[1] = float(tempVertex[i].y);
				PointCloud->vertexInfo[i].position[2] = float(tempVertex[i].z);

				if (i < 1000)
				{
					LOG.Add("2) PointCloud->vertexInfo[i].position[0] : " + std::to_string(PointCloud->vertexInfo[i].position[0]), "precision");
					LOG.Add("2) PointCloud->vertexInfo[i].position[1] : " + std::to_string(PointCloud->vertexInfo[i].position[1]), "precision");
					LOG.Add("2) PointCloud->vertexInfo[i].position[2] : " + std::to_string(PointCloud->vertexInfo[i].position[2]), "precision");
				}

#ifdef LOD_SYSTEM
				for (size_t j = 0; j < PointCloud->LODs.size(); j++)
				{
					if (i % pointCloud::LODSettings[j].takeEach_Nth_Point == 0)
					{
						PointCloud->LODs[j].vertexInfo[i / pointCloud::LODSettings[j].takeEach_Nth_Point].position[0] = PointCloud->vertexInfo[i].position[0];
						PointCloud->LODs[j].vertexInfo[i / pointCloud::LODSettings[j].takeEach_Nth_Point].position[1] = PointCloud->vertexInfo[i].position[1];
						PointCloud->LODs[j].vertexInfo[i / pointCloud::LODSettings[j].takeEach_Nth_Point].position[2] = PointCloud->vertexInfo[i].position[2];
					}
				}
#endif
					
			}

			PointCloud->min = glm::vec3(newMinX, newMinY, newMinZ);
			PointCloud->max = glm::vec3(newMaxX, newMaxY, newMaxZ);

			LOG.Add("newMinX: " + std::to_string(newMinX), "File_Load_Log");
			LOG.Add("newMaxX: " + std::to_string(newMaxX), "File_Load_Log");
			LOG.Add("newMinY: " + std::to_string(newMinY), "File_Load_Log");
			LOG.Add("newMaxY: " + std::to_string(newMaxY), "File_Load_Log");
			LOG.Add("newMinZ: " + std::to_string(newMinZ), "File_Load_Log");
			LOG.Add("newMaxZ: " + std::to_string(newMaxZ), "File_Load_Log");

			PointCloud->initialXShift = header->min_x - newMinX;
			if (header->x_offset == 0 && header->y_offset == 0 && header->z_offset == 0)
			{
				PointCloud->initialZShift = -PointCloud->adjustment.z;
			}

			LOG.Add("PointCloud->initialXShift: " + std::to_string(PointCloud->initialXShift), "File_Load_Log");
			LOG.Add("PointCloud->initialZShift: " + std::to_string(PointCloud->initialZShift), "File_Load_Log");

			if (header->x_offset == 0 && header->y_offset == 0 && header->z_offset == 0)
				std::swap(PointCloud->adjustment.y, PointCloud->adjustment.z);

			// close the reader
			if (laszip_close_reader(laszip_reader))
			{
				LOG.Add("closing laszip reader failed", "DLL_ERRORS");
			}

			// destroy the reader
			if (laszip_destroy(laszip_reader))
			{
				LOG.Add("destroying laszip reader failed", "DLL_ERRORS");
			}
		}

//#ifdef USE_QUADS_NOT_POINTS
//			PointCloud->vertexInfo[0].position[0] = 10.0f;
//			PointCloud->vertexInfo[0].position[1] = 10.0f;
//			PointCloud->vertexInfo[0].position[2] = 0.0f;
//
//			PointCloud->vertexInfo[1].position[0] = -10.0f;
//			PointCloud->vertexInfo[1].position[1] = 10.0f;
//			PointCloud->vertexInfo[1].position[2] = 0.0f;
//
//			PointCloud->vertexInfo[2].position[0] = -10.0f;
//			PointCloud->vertexInfo[2].position[1] = -10.0f;
//			PointCloud->vertexInfo[2].position[2] = 0.0f;
//
//			PointCloud->vertexInfo[3].position[0] = 10.0f;
//			PointCloud->vertexInfo[3].position[1] = -10.0f;
//			PointCloud->vertexInfo[3].position[2] = 0.0f;
//
//			PointCloud->vertexInfo[4].position[0] = -10.0f;
//			PointCloud->vertexInfo[4].position[1] = -10.0f;
//			PointCloud->vertexInfo[4].position[2] = 0.0f;
//
//			PointCloud->vertexInfo[5].position[0] = 10.0f;
//			PointCloud->vertexInfo[5].position[1] = 10.0f;
//			PointCloud->vertexInfo[5].position[2] = 0.0f;
//#endif

		// saving original data
		PointCloud->originalData = PointCloud->vertexInfo;
		//PointCloud->originalData.resize(PointCloud->getPointCount());
		//for (size_t i = 0; i < PointCloud->originalData.size(); i++)
		//{
		//	PointCloud->originalData[i] = PointCloud->vertexInfo[i];
		//	/*PointCloud->originalData[i].color[1] = PointCloud->vertexInfo[i].color[1];
		//	PointCloud->originalData[i].color[2] = PointCloud->vertexInfo[i].color[2];

		//	PointCloud->originalData[i].color[0] = PointCloud->vertexInfo[i].color[0];
		//	PointCloud->originalData[i].color[1] = PointCloud->vertexInfo[i].color[1];
		//	PointCloud->originalData[i].color[2] = PointCloud->vertexInfo[i].color[2];*/
		//}

		LOG.Add("before initializeOctree", "OctreeEvents");
		LOG.Add("rangeXYZ: " + vec3ToString(glm::vec3(rangeX, rangeY, rangeZ)), "OctreeEvents");
		PointCloud->initializeOctree(rangeX, rangeY, rangeZ, glm::vec3(newMinX + rangeX / 2.0f, newMinY + rangeY / 2.0f, newMinZ + rangeZ / 2.0f));
		LOG.Add("after initializeOctree", "OctreeEvents");
			
		LOG.Add("Points inserted: " + std::to_string(PointCloud->getSearchOctree()->getPointsInserted()), "OctreeEvents");
		LOG.Add("Total nodes created: " + std::to_string(PointCloud->getSearchOctree()->getDebugNodeCount()), "OctreeEvents");
		LOG.Add("Rootnode AABB size: " + std::to_string(PointCloud->getSearchOctree()->root->nodeAABB.size), "OctreeEvents");
		LOG.Add("Rootnode AABB min: " + vec3ToString(PointCloud->getSearchOctree()->root->nodeAABB.min), "OctreeEvents");
		LOG.Add("Rootnode AABB max: " + vec3ToString(PointCloud->getSearchOctree()->root->nodeAABB.max), "OctreeEvents");
		LOG.Add("Max depth: " + std::to_string(PointCloud->getSearchOctree()->getDebugMaxNodeDepth()), "OctreeEvents");

		PointCloud->calculateApproximateGroundLevel();
		LOG.Add("ApproximateGroundLevel: " + std::to_string(PointCloud->getApproximateGroundLevel()), "OctreeEvents");

		PointCloud->loadedFrom = fileInfo;
		if (PointCloud->loadedFrom != nullptr)
			PointCloud->loadedFrom->resultingPointCloud = PointCloud;
			
		PointCloud->wasFullyLoaded = true;
		LOG.Add("PointCloud->wasFullyLoaded = true;", "File_Load_Log");
	}
	else
	{
		if (Path.size() <= 4)
			LOG.Add("FilePath lenght was less then 4", "ERRORS");

		if (!std::filesystem::exists(Path))
			LOG.Add("File can't be found in \"" + Path + "\"", "ERRORS");
	}
}

LoadManager::LoadManager() {}
LoadManager::~LoadManager() {}

void LoadManager::loadPointCloudAsync(std::string path, std::string projectPath, pointCloud* PointCloud)
{
	InfoForLoading* InputData = new InfoForLoading();
	InputData->currentPath = path;
	InputData->currentProjectPath = projectPath;
	InputData->currentPointCloud = PointCloud;

	THREAD_POOL.Execute(LoadFunc, InputData, nullptr);
}

bool LoadManager::isLoadingDone()
{
	return !THREAD_POOL.IsAnyThreadHaveActiveJob();
}

SaveManager* SaveManager::Instance = nullptr;

void SaveManager::SaveFunc(void* InputData, void* OutputData)
{
	InfoForSaving* Info = reinterpret_cast<InfoForSaving*>(InputData);
	pointCloud* PointCloud = Info->currentPointCloud;
	std::string Path = Info->currentPath;

	if (Path != "" && Path.size() > 4)
	{
		if (PointCloud->NumPy != nullptr || 
			(Path[Path.size() - 4] == '.' &&
			Path[Path.size() - 3] == 'n' &&
			Path[Path.size() - 2] == 'p' &&
			Path[Path.size() - 1] == 'y') ||
			(Path[Path.size() - 4] == '.' &&
			Path[Path.size() - 3] == 'n' &&
			Path[Path.size() - 2] == 'p' &&
			Path[Path.size() - 1] == 'z'))
		{
			/*std::vector<char> RawData;
			int test = sizeof(double);
			char* TempArray = new char[sizeof(double)];
			for (size_t i = 0; i < PointCloud->NumPy->LoadedRawData.size(); i++)
			{
				if (PointCloud->vertexInfo[i].position[0] == DELETED_POINTS_COORDINATE &&
					PointCloud->vertexInfo[i].position[1] == DELETED_POINTS_COORDINATE &&
					PointCloud->vertexInfo[i].position[2] == DELETED_POINTS_COORDINATE)
				{
					continue;
				}

				TempArray = reinterpret_cast<char*>(&PointCloud->NumPy->LoadedRawData[i].X);
				for (size_t j = 0; j < sizeof(double); j++)
				{
					RawData.push_back(TempArray[j]);
				}

				TempArray = reinterpret_cast<char*>(&PointCloud->NumPy->LoadedRawData[i].Y);
				for (size_t j = 0; j < sizeof(double); j++)
				{
					RawData.push_back(TempArray[j]);
				}

				TempArray = reinterpret_cast<char*>(&PointCloud->NumPy->LoadedRawData[i].Z);
				for (size_t j = 0; j < sizeof(double); j++)
				{
					RawData.push_back(TempArray[j]);
				}

				TempArray = reinterpret_cast<char*>(&PointCloud->NumPy->LoadedRawData[i].Uncertainty);
				for (size_t j = 0; j < sizeof(double); j++)
				{
					RawData.push_back(TempArray[j]);
				}
			}*/

			std::vector<double> FinalData;

			for (size_t i = 0; i < PointCloud->NumPy->LoadedRawData.size(); i++)
			{
				if (PointCloud->vertexInfo[i].position[0] == DELETED_POINTS_COORDINATE &&
					PointCloud->vertexInfo[i].position[1] == DELETED_POINTS_COORDINATE &&
					PointCloud->vertexInfo[i].position[2] == DELETED_POINTS_COORDINATE)
				{
					continue;
				}

				FinalData.push_back(PointCloud->NumPy->LoadedRawData[i].X);
				FinalData.push_back(PointCloud->NumPy->LoadedRawData[i].Y);
				FinalData.push_back(PointCloud->NumPy->LoadedRawData[i].Z);
				FinalData.push_back(PointCloud->NumPy->LoadedRawData[i].Uncertainty);
			}

			cnpy::npz_save(Path, "wkt", &PointCloud->NumPy->WKT, { PointCloud->NumPy->WKT.size() }, "w");
			cnpy::npz_save(Path, "data", FinalData.data(), { FinalData.size() / 4, 4 }, "a");
			cnpy::npz_save(Path, "minmax", reinterpret_cast<double*>(&PointCloud->NumPy->MinMax), { 2, 4 }, "a");
		}
		// if file is in our own format
		else if (Path[Path.size() - 4] == '.' &&
				 Path[Path.size() - 3] == 'c' &&
				 Path[Path.size() - 2] == 'p' &&
				 Path[Path.size() - 1] == 'c')
		{

		}
		else
		{
			/*pointCloud* PointCloud = getPointCloud(pointCloudID);
			if (PointCloud == nullptr)
				return;*/

			//LOG.Add("flag 0", "writeTest");
			laszip_POINTER laszip_writer;
			if (laszip_create(&laszip_writer))
			{
				LOG.Add("creating laszip writer failed", "DLL_ERRORS");
				return;
			}
			//LOG.Add("flag 1", "writeTest");

			int pointsToWrite = 0;
			for (size_t j = 0; j < PointCloud->getPointCount(); j++)
			{
				if (PointCloud->vertexInfo[j].position[0] != DELETED_POINTS_COORDINATE &&
					PointCloud->vertexInfo[j].position[1] != DELETED_POINTS_COORDINATE &&
					PointCloud->vertexInfo[j].position[2] != DELETED_POINTS_COORDINATE)
					pointsToWrite++;
			}
			//LOG.Add("flag 2", "writeTest");

			PointCloud->loadedFrom->header.number_of_point_records = pointsToWrite;
			if (laszip_set_header(laszip_writer, &PointCloud->loadedFrom->header))
			{
				LOG.Add("setting header for laszip writer failed", "DLL_ERRORS");
			}
			//LOG.Add("flag 3", "writeTest");

			std::string fileName = Path;
			if (laszip_open_writer(laszip_writer, fileName.c_str(), true))
			{
				LOG.Add("opening laszip writer for " + fileName + " failed", "DLL_ERRORS");

				laszip_CHAR* error;
				if (laszip_get_error(laszip_writer, &error))
				{
					LOG.Add("getting error messages", "DLL_ERRORS");
				}
				LOG.Add("MESSAGE: " + std::string(error), "DLL_ERRORS");
				return;
			}

			//LOG.Add("flag 4", "writeTest");
			for (size_t j = 0; j < PointCloud->getPointCount(); j++)
			{
				//LOG.Add("iteration of (size_t j = 0; j < pointClouds[pointCloudIndex]->getPointCount(); j++)", "TEST");
				if (PointCloud->vertexInfo[j].position[0] != DELETED_POINTS_COORDINATE &&
					PointCloud->vertexInfo[j].position[1] != DELETED_POINTS_COORDINATE &&
					PointCloud->vertexInfo[j].position[2] != DELETED_POINTS_COORDINATE)
				{
					if (laszip_set_point(laszip_writer, &PointCloud->loadedFrom->LAZpoints[j]))
					{
						LOG.Add("setting point " + std::to_string(j) + " failed", "DLL_ERRORS");
						return;
					}

					if (laszip_write_point(laszip_writer))
					{
						LOG.Add("writing point " + std::to_string(j) + " failed", "DLL_ERRORS");
						return;
					}
				}
			}

			// close the writer
			if (laszip_close_writer(laszip_writer))
			{
				LOG.Add("closing laszip writer failed", "DLL_ERRORS");
				return;
			}

			// destroy the writer
			if (laszip_destroy(laszip_writer))
			{
				LOG.Add("destroying laszip writer failed", "DLL_ERRORS");
				return;
			}
		}

		PointCloud->wasFullyLoaded = true;
	}
	else
	{
		if (Path.size() <= 4)
			LOG.Add("FilePath lenght was less then 4 in SaveManager::saveFunc()", "ERRORS");
	}
}

SaveManager::SaveManager() {}

SaveManager::~SaveManager() {}

bool SaveManager::SavePointCloudAsync(std::string path, pointCloud* PointCloud)
{
	//// Firstly we check if we can process this request right now
	//// if we are working on different request we should decline this one.
	//bool expected = true;
	//if (!savingDone.compare_exchange_strong(expected, false))
	//{
	//	LOG.Add("Saving thread was in work !", "OctreeEvents");
	//	return false;
	//}

	//currentPath = path;
	//currentPointCloud = PointCloud;

	//newJobReady = true;

	InfoForSaving* InputData = new InfoForSaving();
	InputData->currentPath = path;
	InputData->currentPointCloud = PointCloud;

	THREAD_POOL.Execute(SaveFunc, InputData, nullptr);

	return true;
}

bool SaveManager::isSaveDone()
{
	return !THREAD_POOL.IsAnyThreadHaveActiveJob();
}