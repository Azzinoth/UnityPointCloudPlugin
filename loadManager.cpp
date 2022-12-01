#include "loadManager.h"

LoadManager* LoadManager::Instance = nullptr;

void LoadManager::copyLAZvlr(laszip_header* dest, laszip_vlr_struct source)
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

void LoadManager::copyLAZFileHeader(laszip_header* dest, laszip_header* source)
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

std::string LoadManager::GetWKT(std::string FileName, std::string VariableName)
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

std::vector<std::string> LoadManager::GetEPSG(std::string TotalString)
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

NumPyMinMax LoadManager::GetMinMax(std::string FileName, std::string VariableName)
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

	if (PointCloud == nullptr)
	{
		LOG.Add("PointCloud = nullptr", "File_Load_Log");
		return;
	}

	PointCloud->EPSG = 0;
	PointCloud->Metrics.Min = glm::dvec3(DBL_MAX);
	PointCloud->Metrics.Max = glm::dvec3(-DBL_MAX);

	if (Path.empty() || Path.size() <= 4 || !std::filesystem::exists(Path))
	{
		if (Path.size() <= 4)
			LOG.Add("FilePath lenght was less then 4", "ERRORS");

		if (!std::filesystem::exists(Path))
			LOG.Add("File can't be found in \"" + Path + "\"", "ERRORS");

		return;
	}

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

	// If file is in our own format.
	if (Path[Path.size() - 4] == '.' &&
		Path[Path.size() - 3] == 'c' &&
		Path[Path.size() - 2] == 'p' &&
		Path[Path.size() - 1] == 'c')
	{
		LoadOwnFormat(PointCloud, Path);
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
		LoadNPY(PointCloud, Path);
	}
	else
	{
		LoadLazLas(PointCloud, Path);
	}

	// Saving original data.
	PointCloud->originalData = PointCloud->vertexInfo;

	LOG.Add("EPSG: " + std::to_string(PointCloud->EPSG), "EPSG");

	LOG.Add("PointCloud->Metrics.InitialXShift: " + std::to_string(PointCloud->Metrics.InitialXShift), "File_Load_Log");
	LOG.Add("PointCloud->Metrics.InitialZShift: " + std::to_string(PointCloud->Metrics.InitialZShift), "File_Load_Log");

	LOG.Add("PointCloud->Metrics.Min.x: " + std::to_string(PointCloud->Metrics.Min.x), "File_Load_Log");
	LOG.Add("PointCloud->Metrics.Max.x: " + std::to_string(PointCloud->Metrics.Max.x), "File_Load_Log");
	LOG.Add("PointCloud->Metrics.Min.y: " + std::to_string(PointCloud->Metrics.Min.y), "File_Load_Log");
	LOG.Add("PointCloud->Metrics.Max.y: " + std::to_string(PointCloud->Metrics.Max.y), "File_Load_Log");
	LOG.Add("PointCloud->Metrics.Min.z: " + std::to_string(PointCloud->Metrics.Min.z), "File_Load_Log");
	LOG.Add("PointCloud->Metrics.Max.z: " + std::to_string(PointCloud->Metrics.Max.z), "File_Load_Log");

	LOG.Add("PointCloud->Metrics.RawMin.x: " + std::to_string(PointCloud->Metrics.RawMin.x), "File_Load_Log");
	LOG.Add("PointCloud->Metrics.RawMax.x: " + std::to_string(PointCloud->Metrics.RawMax.x), "File_Load_Log");
	LOG.Add("PointCloud->Metrics.RawMin.y: " + std::to_string(PointCloud->Metrics.RawMin.y), "File_Load_Log");
	LOG.Add("PointCloud->Metrics.RawMax.y: " + std::to_string(PointCloud->Metrics.RawMax.y), "File_Load_Log");
	LOG.Add("PointCloud->Metrics.RawMin.z: " + std::to_string(PointCloud->Metrics.RawMin.z), "File_Load_Log");
	LOG.Add("PointCloud->Metrics.RawMax.z: " + std::to_string(PointCloud->Metrics.RawMax.z), "File_Load_Log");

	LOG.Add("PointCloud->Metrics.Range.x: " + std::to_string(PointCloud->Metrics.Range.x), "File_Load_Log");
	LOG.Add("PointCloud->Metrics.Range.y: " + std::to_string(PointCloud->Metrics.Range.y), "File_Load_Log");
	LOG.Add("PointCloud->Metrics.Range.z: " + std::to_string(PointCloud->Metrics.Range.z), "File_Load_Log");

	LOG.Add("PointCloud->Metrics.Adjustment.x: " + std::to_string(PointCloud->Metrics.Adjustment.x), "File_Load_Log");
	LOG.Add("PointCloud->Metrics.Adjustment.y: " + std::to_string(PointCloud->Metrics.Adjustment.y), "File_Load_Log");
	LOG.Add("PointCloud->Metrics.Adjustment.z: " + std::to_string(PointCloud->Metrics.Adjustment.z), "File_Load_Log");

	LOG.Add("before initializeOctree", "OctreeEvents");
	LOG.Add("PointCloud->Metrics.Range.XYZ: " + vec3ToString(glm::vec3(PointCloud->Metrics.Range.x, PointCloud->Metrics.Range.y, PointCloud->Metrics.Range.z)), "OctreeEvents");
	PointCloud->initializeOctree(PointCloud->Metrics.Range.x, PointCloud->Metrics.Range.y, PointCloud->Metrics.Range.z, glm::vec3(PointCloud->Metrics.Min.x + PointCloud->Metrics.Range.x / 2.0f, PointCloud->Metrics.Min.y + PointCloud->Metrics.Range.y / 2.0f, PointCloud->Metrics.Min.z + PointCloud->Metrics.Range.z / 2.0f));
	LOG.Add("after initializeOctree", "OctreeEvents");
		
	LOG.Add("Points inserted: " + std::to_string(PointCloud->getSearchOctree()->getPointsInserted()), "OctreeEvents");
	LOG.Add("Total nodes created: " + std::to_string(PointCloud->getSearchOctree()->getDebugNodeCount()), "OctreeEvents");
	LOG.Add("Rootnode AABB size: " + std::to_string(PointCloud->getSearchOctree()->root->nodeAABB.size), "OctreeEvents");
	LOG.Add("Rootnode AABB min: " + vec3ToString(PointCloud->getSearchOctree()->root->nodeAABB.min), "OctreeEvents");
	LOG.Add("Rootnode AABB max: " + vec3ToString(PointCloud->getSearchOctree()->root->nodeAABB.max), "OctreeEvents");
	LOG.Add("Max depth: " + std::to_string(PointCloud->getSearchOctree()->getDebugMaxNodeDepth()), "OctreeEvents");

	PointCloud->calculateApproximateGroundLevel();
	LOG.Add("ApproximateGroundLevel: " + std::to_string(PointCloud->getApproximateGroundLevel()), "OctreeEvents");

	PointCloud->wasFullyLoaded = true;

	//double* adjustment = new double[13];
	//adjustment[0] = PointCloud->Metrics.Adjustment.x;
	//adjustment[1] = PointCloud->Metrics.Adjustment.y;
	//adjustment[2] = PointCloud->Metrics.Adjustment.z;

	//adjustment[3] = PointCloud->Metrics.InitialXShift;
	//adjustment[4] = PointCloud->Metrics.InitialZShift;

	//adjustment[5] = PointCloud->Metrics.RawMin.x;
	//adjustment[6] = PointCloud->Metrics.RawMin.y;
	//adjustment[7] = PointCloud->Metrics.RawMin.z;

	//adjustment[8] = PointCloud->Metrics.RawMax.x;
	//adjustment[9] = PointCloud->Metrics.RawMax.y;
	//adjustment[10] = PointCloud->Metrics.RawMax.z;

	//adjustment[11] = PointCloud->EPSG;
	//adjustment[12] = PointCloud->getApproximateGroundLevel();

	//std::string Result;
	//for (size_t i = 0; i < 13; i++)
	//{
	//	Result += std::to_string(i) + ": ";
	//	Result += std::to_string(adjustment[i]);
	//	Result += '\n';
	//}

	PointCloud->wasFullyLoaded = true;
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

void LoadManager::LoadNPY(pointCloud* PointCloud, std::string Path)
{
	PointCloud->NumPy = new NumPyInfo();
	PointCloud->NumPy->WKT = GetWKT(Path);
	PointCloud->NumPy->WKT.erase(PointCloud->NumPy->WKT.begin(),
		PointCloud->NumPy->WKT.begin() + PointCloud->NumPy->WKT.find("PROJCS"));

	PointCloud->NumPy->AllEPSG = GetEPSG(PointCloud->NumPy->WKT);
	PointCloud->NumPy->MinMax = GetMinMax(Path);

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

			if (PointCloud->Metrics.Min.x > tempValue)
				PointCloud->Metrics.Min.x = tempValue;

			if (PointCloud->Metrics.Max.x < tempValue)
				PointCloud->Metrics.Max.x = tempValue;
		}
		// Z and Y is flipped.
		else if (Iteration == 1)
		{
			PointCloud->NumPy->LoadedRawData.back().Y = tempValue;
			PointCloud->vertexInfo.back().position.z = tempValue;

			if (PointCloud->Metrics.Min.z > tempValue)
				PointCloud->Metrics.Min.z = tempValue;

			if (PointCloud->Metrics.Max.z < tempValue)
				PointCloud->Metrics.Max.z = tempValue;
		}
		else if (Iteration == 2)
		{
			PointCloud->NumPy->LoadedRawData.back().Z = tempValue;
			PointCloud->vertexInfo.back().position.y = tempValue;

			if (PointCloud->Metrics.Min.y > tempValue)
				PointCloud->Metrics.Min.y = tempValue;

			if (PointCloud->Metrics.Max.y < tempValue)
				PointCloud->Metrics.Max.y = tempValue;

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

	PointCloud->Metrics.Range.x = PointCloud->Metrics.Max.x - PointCloud->Metrics.Min.x;
	PointCloud->Metrics.Range.y = PointCloud->Metrics.Max.y - PointCloud->Metrics.Min.y;
	PointCloud->Metrics.Range.z = PointCloud->Metrics.Max.z - PointCloud->Metrics.Min.z;

	PointCloud->Metrics.Adjustment.x = PointCloud->Metrics.Min.x;
	PointCloud->Metrics.Adjustment.y = PointCloud->Metrics.Min.y;
	PointCloud->Metrics.Adjustment.z = PointCloud->Metrics.Min.z;

	PointCloud->Metrics.Min = glm::dvec3(DBL_MAX);
	PointCloud->Metrics.Max = glm::dvec3(-DBL_MAX);

	double UncertaintyRange = UncertaintyMax - UncertaintyMin;
	glm::vec3 CertainPointsColor = glm::vec3(1.0f);
	glm::vec3 UnCertainPointsColor = glm::vec3(1.0f, 0.0f, 0.0f);

	for (int i = 0; i < PointCloud->vertexInfo.size(); i++)
	{
		PointCloud->vertexInfo[i].position.x = PointCloud->vertexInfo[i].position.x - PointCloud->Metrics.Adjustment.x;
		PointCloud->vertexInfo[i].position.y = PointCloud->vertexInfo[i].position.y - PointCloud->Metrics.Adjustment.y;
		PointCloud->vertexInfo[i].position.z = PointCloud->vertexInfo[i].position.z - PointCloud->Metrics.Adjustment.z;

		PointCloud->Metrics.Min.x = glm::min(float(PointCloud->Metrics.Min.x), PointCloud->vertexInfo[i].position.x);
		PointCloud->Metrics.Min.y = glm::min(float(PointCloud->Metrics.Min.y), PointCloud->vertexInfo[i].position.y);
		PointCloud->Metrics.Min.z = glm::min(float(PointCloud->Metrics.Min.z), PointCloud->vertexInfo[i].position.z);

		PointCloud->Metrics.Max.x = glm::min(float(PointCloud->Metrics.Max.x), PointCloud->vertexInfo[i].position.x);
		PointCloud->Metrics.Max.y = glm::min(float(PointCloud->Metrics.Max.y), PointCloud->vertexInfo[i].position.y);
		PointCloud->Metrics.Max.z = glm::min(float(PointCloud->Metrics.Max.z), PointCloud->vertexInfo[i].position.z);

		double HowCertainPointIs = (UncertaintyMax - PointCloud->NumPy->LoadedRawData[i].Uncertainty) / UncertaintyRange;
		glm::vec3 ColorMixFactor = glm::vec3(HowCertainPointIs);

		glm::vec3 Color = CertainPointsColor * ColorMixFactor + UnCertainPointsColor * (glm::vec3(1.0f) - ColorMixFactor);

		PointCloud->vertexInfo[i].color[0] = unsigned char(Color.x * 255);
		PointCloud->vertexInfo[i].color[1] = unsigned char(Color.y * 255);
		PointCloud->vertexInfo[i].color[2] = unsigned char(Color.z * 255);
	}
}

void LoadManager::LoadOwnFormat(pointCloud* PointCloud, std::string Path)
{
	std::fstream file;
	file.open(Path, std::ios::in | std::ios::binary);

	char* buffer = new char[sizeof(float)];

	// Read how many points does the file have.
	file.read(buffer, sizeof(int));
	int pointCount = *(int*)buffer;

	// Read initialXShift and initialZShift.
	file.read(buffer, sizeof(double));
	PointCloud->Metrics.InitialXShift = *(double*)buffer;
	file.read(buffer, sizeof(double));
	PointCloud->Metrics.InitialZShift = *(double*)buffer;

	// Read adjustment.
	file.read(buffer, sizeof(float));
	PointCloud->Metrics.Adjustment[0] = *(float*)buffer;
	file.read(buffer, sizeof(float));
	PointCloud->Metrics.Adjustment[1] = *(float*)buffer;
	file.read(buffer, sizeof(float));
	PointCloud->Metrics.Adjustment[2] = *(float*)buffer;

	// Read min and max.
	file.read(buffer, sizeof(float));
	PointCloud->Metrics.Min[0] = *(float*)buffer;
	file.read(buffer, sizeof(float));
	PointCloud->Metrics.Min[1] = *(float*)buffer;
	file.read(buffer, sizeof(float));
	PointCloud->Metrics.Min[2] = *(float*)buffer;

	file.read(buffer, sizeof(float));
	PointCloud->Metrics.Max[0] = *(float*)buffer;
	file.read(buffer, sizeof(float));
	PointCloud->Metrics.Max[1] = *(float*)buffer;
	file.read(buffer, sizeof(float));
	PointCloud->Metrics.Max[2] = *(float*)buffer;

	PointCloud->Metrics.Range.x = PointCloud->Metrics.Max.x - PointCloud->Metrics.Min.x;
	PointCloud->Metrics.Range.y = PointCloud->Metrics.Max.y - PointCloud->Metrics.Min.y;
	PointCloud->Metrics.Range.z = PointCloud->Metrics.Max.z - PointCloud->Metrics.Min.z;

	LOG.Add("contains " + std::to_string(pointCount) + " points", "File_Load_Log");

	PointCloud->vertexInfo.resize(pointCount);

	char* rawPointData = new char[pointCount * sizeof(VertexData)];
	file.read(rawPointData, pointCount * sizeof(VertexData));

	VertexData* temp = (VertexData*)rawPointData;
	for (size_t i = 0; i < pointCount; i++)
	{
		PointCloud->vertexInfo[i].position[0] = (temp + i)->position.x;
		PointCloud->vertexInfo[i].position[1] = (temp + i)->position.y;
		PointCloud->vertexInfo[i].position[2] = (temp + i)->position.z;
		PointCloud->vertexInfo[i].color[0] = (temp + i)->color[0];
		PointCloud->vertexInfo[i].color[1] = (temp + i)->color[1];
		PointCloud->vertexInfo[i].color[2] = (temp + i)->color[2];
	}
}

void LoadManager::LoadLazLas(pointCloud* PointCloud, std::string Path)
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

	LAZFileInfo* FileInfo = nullptr;
	FileInfo = new LAZFileInfo();
	copyLAZFileHeader(&FileInfo->header, header);
	FileInfo->compressed = is_compressed;

	LOG.Add("Compressed: " + std::string(is_compressed ? "true" : "false"), "File_Load_Log");
	LOG.Add("Signature: " + std::string(header->generating_software), "File_Load_Log");
	LOG.Add("header->number_of_point_records: " + std::to_string(header->number_of_point_records), "File_Load_Log");
	LOG.Add("header->min_x: " + std::to_string(header->min_x), "File_Load_Log");
	LOG.Add("header->max_x: " + std::to_string(header->max_x), "File_Load_Log");
	LOG.Add("header->min_y: " + std::to_string(header->min_y), "File_Load_Log");
	LOG.Add("header->max_y: " + std::to_string(header->max_y), "File_Load_Log");
	LOG.Add("header->min_z: " + std::to_string(header->min_z), "File_Load_Log");
	LOG.Add("header->max_z: " + std::to_string(header->max_z), "File_Load_Log");
	LOG.Add("header->x_scale_factor : " + std::to_string(header->x_scale_factor), "File_Load_Log");
	LOG.Add("header->z_scale_factor : " + std::to_string(header->z_scale_factor), "File_Load_Log");
	LOG.Add("header->y_scale_factor : " + std::to_string(header->y_scale_factor), "File_Load_Log");

	// How many points does the file have.
	laszip_U64 npoints = (header->number_of_point_records ? header->number_of_point_records : header->extended_number_of_point_records);
	LOG.Add("contains " + std::to_string(npoints) + " points", "File_Load_Log");

	PointCloud->vertexInfo.resize(npoints);
	std::vector<glm::dvec3> DoublePoints;
	DoublePoints.resize(npoints);
	PointCloud->vertexIntensity.resize(npoints);

	ExtractLazLasHeaderInfo(PointCloud, header);

	// get a pointer to the points that will be read
	laszip_point* point;
	if (laszip_get_point_pointer(laszip_reader, &point))
	{
		LOG.Add("getting point pointer from laszip reader failed", "DLL_ERRORS");
		return;
	}

	// read the points
	laszip_U64 p_count = 0;
	std::vector<VertexData> points;
	float maxIntensity = -FLT_MAX;
	int MinClassification = INT_MAX;
	int MaxClassification = -INT_MAX;
	while (p_count < npoints)
	{
		// read a point
		if (laszip_read_point(laszip_reader))
		{
			LOG.Add("reading point " + std::to_string(p_count) + " failed", "DLL_ERRORS");
			return;
		}

		FileInfo->LAZpoints.push_back(laszip_point(*point));

		// point->X -> lonX, point->Y -> latY, point->Z -> depth
		double readX = point->X;
		double readY = point->Z;
		double readZ = point->Y;

		if (point->num_extra_bytes == 1)
		{
			PointCloud->vertexInfo[p_count].classification = int(*(unsigned char*)point->extra_bytes);

			MinClassification = glm::min(MinClassification, PointCloud->vertexInfo[p_count].classification);
			MaxClassification = glm::max(MaxClassification, PointCloud->vertexInfo[p_count].classification);
		}
		else if (point->num_extra_bytes == 2)
		{
			PointCloud->vertexInfo[p_count].classification = int(*(unsigned short*)point->extra_bytes);

			MinClassification = glm::min(MinClassification, PointCloud->vertexInfo[p_count].classification);
			MaxClassification = glm::max(MaxClassification, PointCloud->vertexInfo[p_count].classification);
		}

		DoublePoints[p_count].x = readX * header->x_scale_factor;
		DoublePoints[p_count].y = readY * header->z_scale_factor;
		DoublePoints[p_count].z = readZ * header->y_scale_factor;

		PointCloud->vertexInfo[p_count].color[0] = unsigned char(point->rgb[0] / float(1 << 16) * 255);
		PointCloud->vertexInfo[p_count].color[1] = unsigned char(point->rgb[1] / float(1 << 16) * 255);
		PointCloud->vertexInfo[p_count].color[2] = unsigned char(point->rgb[2] / float(1 << 16) * 255);

		PointCloud->vertexIntensity[p_count] = point->intensity;
		maxIntensity = std::max(maxIntensity, float(point->intensity));

		PointCloud->Metrics.RawMin.x = std::min(PointCloud->Metrics.RawMin.x, readX);
		PointCloud->Metrics.RawMin.y = std::min(PointCloud->Metrics.RawMin.y, readY);
		PointCloud->Metrics.RawMin.z = std::min(PointCloud->Metrics.RawMin.z, readZ);

		PointCloud->Metrics.RawMax.x = std::max(PointCloud->Metrics.RawMax.x, readX);
		PointCloud->Metrics.RawMax.y = std::max(PointCloud->Metrics.RawMax.y, readY);
		PointCloud->Metrics.RawMax.z = std::max(PointCloud->Metrics.RawMax.z, readZ);

		PointCloud->Metrics.Min.x = std::min(PointCloud->Metrics.Min.x, DoublePoints[p_count].x);
		PointCloud->Metrics.Min.y = std::min(PointCloud->Metrics.Min.y, DoublePoints[p_count].y);
		PointCloud->Metrics.Min.z = std::min(PointCloud->Metrics.Min.z, DoublePoints[p_count].z);

		PointCloud->Metrics.Max.x = std::max(PointCloud->Metrics.Max.x, DoublePoints[p_count].x);
		PointCloud->Metrics.Max.y = std::max(PointCloud->Metrics.Max.y, DoublePoints[p_count].y);
		PointCloud->Metrics.Max.z = std::max(PointCloud->Metrics.Max.z, DoublePoints[p_count].z);

		p_count++;
	}

	LOG.Add("MinClassification : " + std::to_string(MinClassification), "File_Load_Log");
	LOG.Add("MaxClassification : " + std::to_string(MaxClassification), "File_Load_Log");

	if (header->point_data_format == 1)
	{
		for (size_t i = 0; i < p_count; i++)
		{
			PointCloud->vertexInfo[i].color[0] = unsigned char(PointCloud->vertexIntensity[i] / maxIntensity * 255);
			PointCloud->vertexInfo[i].color[1] = unsigned char(PointCloud->vertexIntensity[i] / maxIntensity * 255);
			PointCloud->vertexInfo[i].color[2] = unsigned char(PointCloud->vertexIntensity[i] / maxIntensity * 255);
		}
	}

	PointCloud->Metrics.InitialZShift = PointCloud->Metrics.Max.z < PointCloud->Metrics.Min.z ? PointCloud->Metrics.Max.z : PointCloud->Metrics.Min.z;
	PointCloud->Metrics.InitialZShift = header->min_y - PointCloud->Metrics.InitialZShift;

	if (header->x_offset == 0 && header->y_offset == 0 && header->z_offset == 0)
	{
		LOG.Add("header does not contain offset", "File_Load_Log");

		PointCloud->Metrics.Range.x = PointCloud->Metrics.Max.x - PointCloud->Metrics.Min.x;
		PointCloud->Metrics.Range.y = PointCloud->Metrics.Max.y - PointCloud->Metrics.Min.y;
		PointCloud->Metrics.Range.z = PointCloud->Metrics.Max.z - PointCloud->Metrics.Min.z;

		PointCloud->Metrics.Adjustment.x = -PointCloud->Metrics.Min.x;
		PointCloud->Metrics.Adjustment.y = -PointCloud->Metrics.Min.y;
		PointCloud->Metrics.Adjustment.z = -PointCloud->Metrics.Min.z;
	}
	else
	{
		LOG.Add("header contain offset", "File_Load_Log");
		LOG.Add("header->x_offset : " + std::to_string(header->x_offset), "File_Load_Log");
		LOG.Add("header->y_offset : " + std::to_string(header->y_offset), "File_Load_Log");
		LOG.Add("header->z_offset : " + std::to_string(header->z_offset), "File_Load_Log");

		PointCloud->Metrics.Range.x = PointCloud->Metrics.Max.x - PointCloud->Metrics.Min.x;
		PointCloud->Metrics.Range.y = PointCloud->Metrics.Max.y - PointCloud->Metrics.Min.y;
		PointCloud->Metrics.Range.z = PointCloud->Metrics.Max.z - PointCloud->Metrics.Min.z;

		PointCloud->Metrics.Adjustment.x = -header->x_offset;
		PointCloud->Metrics.Adjustment.y = -header->y_offset;
		PointCloud->Metrics.Adjustment.z = -header->z_offset;
	}

	PointCloud->Metrics.Min = glm::dvec3(DBL_MAX);
	PointCloud->Metrics.Max = glm::dvec3(-DBL_MAX);

	for (int i = 0; i < npoints; i++)
	{
		if (header->x_offset == 0 && header->y_offset == 0 && header->z_offset == 0)
		{
			DoublePoints[i].x = DoublePoints[i].x + PointCloud->Metrics.Adjustment.x;
			DoublePoints[i].y = DoublePoints[i].y + PointCloud->Metrics.Adjustment.y;
			DoublePoints[i].z = DoublePoints[i].z + PointCloud->Metrics.Adjustment.z;
		}
		else
		{
			DoublePoints[i].x = DoublePoints[i].x + PointCloud->Metrics.Adjustment.x * header->x_scale_factor;
			DoublePoints[i].y = DoublePoints[i].y + PointCloud->Metrics.Adjustment.y * header->y_scale_factor;
			DoublePoints[i].z = DoublePoints[i].z + PointCloud->Metrics.Adjustment.z * header->z_scale_factor;
		}

		PointCloud->Metrics.Min.x = std::min(PointCloud->Metrics.Min.x, DoublePoints[i].x);
		PointCloud->Metrics.Min.y = std::min(PointCloud->Metrics.Min.y, DoublePoints[i].y);
		PointCloud->Metrics.Min.z = std::min(PointCloud->Metrics.Min.z, DoublePoints[i].z);

		PointCloud->Metrics.Max.x = std::max(PointCloud->Metrics.Max.x, DoublePoints[i].x);
		PointCloud->Metrics.Max.y = std::max(PointCloud->Metrics.Max.y, DoublePoints[i].y);
		PointCloud->Metrics.Max.z = std::max(PointCloud->Metrics.Max.z, DoublePoints[i].z);

		PointCloud->vertexInfo[i].position[0] = float(DoublePoints[i].x);
		PointCloud->vertexInfo[i].position[1] = float(DoublePoints[i].y);
		PointCloud->vertexInfo[i].position[2] = float(DoublePoints[i].z);
	}

	PointCloud->Metrics.InitialXShift = header->min_x - PointCloud->Metrics.Min.x;
	if (header->x_offset == 0 && header->y_offset == 0 && header->z_offset == 0)
	{
		PointCloud->Metrics.InitialZShift = -PointCloud->Metrics.Adjustment.z;
	}

	if (header->x_offset == 0 && header->y_offset == 0 && header->z_offset == 0)
		std::swap(PointCloud->Metrics.Adjustment.y, PointCloud->Metrics.Adjustment.z);

	// Close the reader.
	if (laszip_close_reader(laszip_reader))
		LOG.Add("closing laszip reader failed", "DLL_ERRORS");

	// Destroy the reader.
	if (laszip_destroy(laszip_reader))
		LOG.Add("destroying laszip reader failed", "DLL_ERRORS");

	PointCloud->loadedFrom = FileInfo;
	if (PointCloud->loadedFrom != nullptr)
		PointCloud->loadedFrom->resultingPointCloud = PointCloud;
}

void LoadManager::ExtractLazLasHeaderInfo(pointCloud* PointCloud, laszip_header* Header)
{
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

	for (size_t i = 0; i < Header->number_of_variable_length_records; i++)
	{
		if (Header->vlrs[i].record_length_after_header)
		{
			std::string text = reinterpret_cast<char*>(Header->vlrs[i].data);

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

		if (Header->vlrs[i].record_id == 34735) // GeoKeyDirectoryTag
		{
			LASvlr_geo_keys* read = new LASvlr_geo_keys[1];
			memcpy_s(read, sizeof(unsigned short) * 4, Header->vlrs[i].data, sizeof(unsigned short) * 4);

			GeoProjectionGeoKeys* readKeys = new GeoProjectionGeoKeys[read->number_of_keys];
			memcpy_s(readKeys, read->number_of_keys * sizeof(GeoProjectionGeoKeys),
				Header->vlrs[i].data + sizeof(unsigned short) * 4, read->number_of_keys * sizeof(unsigned short) * 4);

			for (int j = 0; j < read->number_of_keys; j++)
			{
				if (readKeys[j].key_id != 3072)
					continue;

				PointCloud->EPSG = readKeys[j].value_offset;
			}
		}
	}
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
			laszip_POINTER laszip_writer;
			if (laszip_create(&laszip_writer))
			{
				LOG.Add("creating laszip writer failed", "DLL_ERRORS");
				return;
			}

			int pointsToWrite = 0;
			for (size_t j = 0; j < PointCloud->getPointCount(); j++)
			{
				if (PointCloud->vertexInfo[j].position[0] != DELETED_POINTS_COORDINATE &&
					PointCloud->vertexInfo[j].position[1] != DELETED_POINTS_COORDINATE &&
					PointCloud->vertexInfo[j].position[2] != DELETED_POINTS_COORDINATE)
					pointsToWrite++;
			}

			PointCloud->loadedFrom->header.number_of_point_records = pointsToWrite;
			if (laszip_set_header(laszip_writer, &PointCloud->loadedFrom->header))
			{
				LOG.Add("setting header for laszip writer failed", "DLL_ERRORS");
			}

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

			for (size_t j = 0; j < PointCloud->getPointCount(); j++)
			{
				if (PointCloud->vertexInfo[j].position[0] != DELETED_POINTS_COORDINATE &&
					PointCloud->vertexInfo[j].position[1] != DELETED_POINTS_COORDINATE &&
					PointCloud->vertexInfo[j].position[2] != DELETED_POINTS_COORDINATE)
				{
					if (PointCloud->vertexInfo[j].classification != int(*(unsigned char*)PointCloud->loadedFrom->LAZpoints[j].extra_bytes))
					{
						static unsigned char* Old = new unsigned char[PointCloud->loadedFrom->LAZpoints[0].num_extra_bytes];
						//std::copy(PointCloud->loadedFrom->LAZpoints[j].extra_bytes, PointCloud->loadedFrom->LAZpoints[j].extra_bytes + 2, Old)
						*Old = *PointCloud->loadedFrom->LAZpoints[j].extra_bytes;

						*PointCloud->loadedFrom->LAZpoints[j].extra_bytes = PointCloud->vertexInfo[j].classification;
						if (laszip_set_point(laszip_writer, &PointCloud->loadedFrom->LAZpoints[j]))
						{
							LOG.Add("setting point " + std::to_string(j) + " failed", "DLL_ERRORS");
							return;
						}

						PointCloud->loadedFrom->LAZpoints[j].classification = *Old;
					}
					else
					{
						if (laszip_set_point(laszip_writer, &PointCloud->loadedFrom->LAZpoints[j]))
						{
							LOG.Add("setting point " + std::to_string(j) + " failed", "DLL_ERRORS");
							return;
						}
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