#include "pch.h"
#define LOG debugLog::getInstance()

static ID3D11Buffer* m_CB;
static ID3D11VertexShader* m_VertexShader;
static ID3D11PixelShader* m_PixelShader;
static ID3D11InputLayout* m_InputLayout;
static ID3D11RasterizerState* m_RasterState;
static ID3D11BlendState* m_BlendState;
static ID3D11DepthStencilState* m_DepthState;
static float pointsWorldMatrix[16];
static float worldToViewMatrix[16];
static float projectionMatrix[16];

static float** frustum = nullptr;
static void updateFrustumPlanes();
static bool frustumCulling = false;

const BYTE kVertexShaderCode[] =
{
	 68,  88,  66,  67,  86, 189,
	 21,  50, 166, 106, 171,   1,
	 10,  62, 115,  48, 224, 137,
	163, 129,   1,   0,   0,   0,
	168,   2,   0,   0,   4,   0,
	  0,   0,  48,   0,   0,   0,
	  0,   1,   0,   0,   4,   2,
	  0,   0,  84,   2,   0,   0,
	 65, 111, 110,  57, 200,   0,
	  0,   0, 200,   0,   0,   0,
	  0,   2, 254, 255, 148,   0,
	  0,   0,  52,   0,   0,   0,
	  1,   0,  36,   0,   0,   0,
	 48,   0,   0,   0,  48,   0,
	  0,   0,  36,   0,   1,   0,
	 48,   0,   0,   0,   0,   0,
	  4,   0,   1,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  1,   2, 254, 255,  31,   0,
	  0,   2,   5,   0,   0, 128,
	  0,   0,  15, 144,  31,   0,
	  0,   2,   5,   0,   1, 128,
	  1,   0,  15, 144,   5,   0,
	  0,   3,   0,   0,  15, 128,
	  0,   0,  85, 144,   2,   0,
	228, 160,   4,   0,   0,   4,
	  0,   0,  15, 128,   1,   0,
	228, 160,   0,   0,   0, 144,
	  0,   0, 228, 128,   4,   0,
	  0,   4,   0,   0,  15, 128,
	  3,   0, 228, 160,   0,   0,
	170, 144,   0,   0, 228, 128,
	  2,   0,   0,   3,   0,   0,
	 15, 128,   0,   0, 228, 128,
	  4,   0, 228, 160,   4,   0,
	  0,   4,   0,   0,   3, 192,
	  0,   0, 255, 128,   0,   0,
	228, 160,   0,   0, 228, 128,
	  1,   0,   0,   2,   0,   0,
	 12, 192,   0,   0, 228, 128,
	  1,   0,   0,   2,   0,   0,
	 15, 224,   1,   0, 228, 144,
	255, 255,   0,   0,  83,  72,
	 68,  82, 252,   0,   0,   0,
	 64,   0,   1,   0,  63,   0,
	  0,   0,  89,   0,   0,   4,
	 70, 142,  32,   0,   0,   0,
	  0,   0,   4,   0,   0,   0,
	 95,   0,   0,   3, 114,  16,
	 16,   0,   0,   0,   0,   0,
	 95,   0,   0,   3, 242,  16,
	 16,   0,   1,   0,   0,   0,
	101,   0,   0,   3, 242,  32,
	 16,   0,   0,   0,   0,   0,
	103,   0,   0,   4, 242,  32,
	 16,   0,   1,   0,   0,   0,
	  1,   0,   0,   0, 104,   0,
	  0,   2,   1,   0,   0,   0,
	 54,   0,   0,   5, 242,  32,
	 16,   0,   0,   0,   0,   0,
	 70,  30,  16,   0,   1,   0,
	  0,   0,  56,   0,   0,   8,
	242,   0,  16,   0,   0,   0,
	  0,   0,  86,  21,  16,   0,
	  0,   0,   0,   0,  70, 142,
	 32,   0,   0,   0,   0,   0,
	  1,   0,   0,   0,  50,   0,
	  0,  10, 242,   0,  16,   0,
	  0,   0,   0,   0,  70, 142,
	 32,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   6,  16,
	 16,   0,   0,   0,   0,   0,
	 70,  14,  16,   0,   0,   0,
	  0,   0,  50,   0,   0,  10,
	242,   0,  16,   0,   0,   0,
	  0,   0,  70, 142,  32,   0,
	  0,   0,   0,   0,   2,   0,
	  0,   0, 166,  26,  16,   0,
	  0,   0,   0,   0,  70,  14,
	 16,   0,   0,   0,   0,   0,
	  0,   0,   0,   8, 242,  32,
	 16,   0,   1,   0,   0,   0,
	 70,  14,  16,   0,   0,   0,
	  0,   0,  70, 142,  32,   0,
	  0,   0,   0,   0,   3,   0,
	  0,   0,  62,   0,   0,   1,
	 73,  83,  71,  78,  72,   0,
	  0,   0,   2,   0,   0,   0,
	  8,   0,   0,   0,  56,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   3,   0,
	  0,   0,   0,   0,   0,   0,
	  7,   7,   0,   0,  65,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   3,   0,
	  0,   0,   1,   0,   0,   0,
	 15,  15,   0,   0,  80,  79,
	 83,  73,  84,  73,  79,  78,
	  0,  67,  79,  76,  79,  82,
	  0, 171,  79,  83,  71,  78,
	 76,   0,   0,   0,   2,   0,
	  0,   0,   8,   0,   0,   0,
	 56,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  3,   0,   0,   0,   0,   0,
	  0,   0,  15,   0,   0,   0,
	 62,   0,   0,   0,   0,   0,
	  0,   0,   1,   0,   0,   0,
	  3,   0,   0,   0,   1,   0,
	  0,   0,  15,   0,   0,   0,
	 67,  79,  76,  79,  82,   0,
	 83,  86,  95,  80, 111, 115,
	105, 116, 105, 111, 110,   0,
	171, 171
};
const BYTE kPixelShaderCode[] =
{
	 68,  88,  66,  67, 196,  65,
	213, 199,  14,  78,  29, 150,
	 87, 236, 231, 156, 203, 125,
	244, 112,   1,   0,   0,   0,
	 32,   1,   0,   0,   4,   0,
	  0,   0,  48,   0,   0,   0,
	124,   0,   0,   0, 188,   0,
	  0,   0, 236,   0,   0,   0,
	 65, 111, 110,  57,  68,   0,
	  0,   0,  68,   0,   0,   0,
	  0,   2, 255, 255,  32,   0,
	  0,   0,  36,   0,   0,   0,
	  0,   0,  36,   0,   0,   0,
	 36,   0,   0,   0,  36,   0,
	  0,   0,  36,   0,   0,   0,
	 36,   0,   1,   2, 255, 255,
	 31,   0,   0,   2,   0,   0,
	  0, 128,   0,   0,  15, 176,
	  1,   0,   0,   2,   0,   8,
	 15, 128,   0,   0, 228, 176,
	255, 255,   0,   0,  83,  72,
	 68,  82,  56,   0,   0,   0,
	 64,   0,   0,   0,  14,   0,
	  0,   0,  98,  16,   0,   3,
	242,  16,  16,   0,   0,   0,
	  0,   0, 101,   0,   0,   3,
	242,  32,  16,   0,   0,   0,
	  0,   0,  54,   0,   0,   5,
	242,  32,  16,   0,   0,   0,
	  0,   0,  70,  30,  16,   0,
	  0,   0,   0,   0,  62,   0,
	  0,   1,  73,  83,  71,  78,
	 40,   0,   0,   0,   1,   0,
	  0,   0,   8,   0,   0,   0,
	 32,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  3,   0,   0,   0,   0,   0,
	  0,   0,  15,  15,   0,   0,
	 67,  79,  76,  79,  82,   0,
	171, 171,  79,  83,  71,  78,
	 44,   0,   0,   0,   1,   0,
	  0,   0,   8,   0,   0,   0,
	 32,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  3,   0,   0,   0,   0,   0,
	  0,   0,  15,   0,   0,   0,
	 83,  86,  95,  84,  65,  82,
	 71,  69,  84,   0, 171, 171
};

static bool DLLWasLoadedCorrectly = false;
static std::string resultString;

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

std::vector<pointCloud*> pointClouds;

extern "C" bool UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API OpenLAZFileFromUnity(char* filePath)
{
	if (!DLLWasLoadedCorrectly)
	{
		LOG.addToLog("Call of OpenLAZFileFromUnity can't be executed because DLL was not loaded correctly", "ERRORS");
		return false;
	}

	pointCloud* temp = new pointCloud();
	bool willBeLoaded = false;
	willBeLoaded = LoadManager::getInstance().tryLoadPointCloudAsync(std::string(filePath), temp);
	if (willBeLoaded)
	{
		pointClouds.push_back(temp);
	}
	else
	{
		delete temp;
	}
	
	return willBeLoaded;
			
	//if (strlen(filePath) <= 1)
	//{
	//	LOG.addToLog("filePath lenght was less then 1", "ERRORS");
	//	return false;
	//}

	//// create the reader
	//laszip_POINTER laszip_reader;
	//if (laszip_create(&laszip_reader))
	//{
	//	LOG.addToLog("creating laszip reader failed", "DLL_ERRORS");
	//	return false;
	//}

	//// open the reader
	//laszip_BOOL is_compressed = 0;
	//std::string fileName = filePath;
	//if (laszip_open_reader(laszip_reader, fileName.c_str(), &is_compressed))
	//{
	//	LOG.addToLog("opening laszip reader for " + fileName + " failed", "DLL_ERRORS");
	//	return false;
	//}

	//// get a pointer to the header of the reader that was just populated
	//laszip_header* header;
	//if (laszip_get_header_pointer(laszip_reader, &header))
	//{
	//	LOG.addToLog("getting header pointer from laszip reader failed", "DLL_ERRORS");
	//	return false;
	//}

	////LAZFileInfo* fileInfo = new LAZFileInfo();
	////copyLAZFileHeader(&fileInfo->header, header);
	////fileInfo->compressed = is_compressed;
	//
	//LOG.addToLog("Compressed: " + std::string(is_compressed ? "true" : "false"), "File_Load_Log");
	//LOG.addToLog("Signature: " + std::string(header->generating_software), "File_Load_Log");
	//LOG.addToLog("Points count: " + std::to_string(header->number_of_point_records), "File_Load_Log");
	//LOG.addToLog("X Min: " + std::to_string(header->min_x), "File_Load_Log");
	//LOG.addToLog("X Max: " + std::to_string(header->max_x), "File_Load_Log");
	//LOG.addToLog("Y Min: " + std::to_string(header->min_y), "File_Load_Log");
	//LOG.addToLog("Y Max: " + std::to_string(header->max_y), "File_Load_Log");
	//LOG.addToLog("Z Min: " + std::to_string(header->min_z), "File_Load_Log");
	//LOG.addToLog("Z Max: " + std::to_string(header->max_z), "File_Load_Log");

	//// how many points does the file have
	//laszip_U64 npoints = (header->number_of_point_records ? header->number_of_point_records : header->extended_number_of_point_records);

	//LOG.addToLog("contains " + std::to_string(npoints) + " points", "File_Load_Log");

	//pointClouds.push_back(new pointCloud());
	//pointClouds.back()->vertexInfo.resize(npoints);
	//pointClouds.back()->vertexIntensity.resize(npoints);

	//pointClouds.back()->min = glm::vec3(FLT_MAX);
	//pointClouds.back()->max = glm::vec3(-FLT_MAX);

	//for (size_t i = 0; i < header->number_of_variable_length_records; i++)
	//{
	//	if (header->vlrs[i].record_length_after_header)
	//	{
	//		std::string text = reinterpret_cast<char*>(header->vlrs[i].data);

	//		size_t position = text.find("NAD_1983_2011_UTM_Zone_");
	//		size_t position1 = text.find("UTM zone ");

	//		if (position != std::string::npos)
	//		{
	//			pointClouds.back()->spatialInfo = text;
	//			LOG.addToLog("spatialInfo: " + pointClouds.back()->spatialInfo, "File_Load_Log");
	//			pointClouds.back()->UTMZone = text.substr(position + strlen("NAD_1983_2011_UTM_Zone_"), 3);
	//			LOG.addToLog("UTMZone: " + pointClouds.back()->UTMZone, "File_Load_Log");
	//			break;
	//		}
	//		else if (position1 != std::string::npos)
	//		{
	//			pointClouds.back()->spatialInfo = text;
	//			LOG.addToLog("spatialInfo: " + pointClouds.back()->spatialInfo, "File_Load_Log");
	//			pointClouds.back()->UTMZone = text.substr(position + strlen("UTM zone "), 3);
	//			LOG.addToLog("UTMZone: " + pointClouds.back()->UTMZone, "File_Load_Log");
	//			break;
	//		}
	//	}
	//}

	//// get a pointer to the points that will be read
	//laszip_point* point;
	//if (laszip_get_point_pointer(laszip_reader, &point))
	//{
	//	LOG.addToLog("getting point pointer from laszip reader failed", "DLL_ERRORS");
	//	return false;
	//}

	//// read the points
	//laszip_U64 p_count = 0;
	//std::vector<MeshVertex> points;
	//float maxIntensity = -FLT_MAX;
	//while (p_count < npoints)
	//{
	//	// read a point
	//	if (laszip_read_point(laszip_reader))
	//	{
	//		LOG.addToLog("reading point " + std::to_string(p_count) + " failed", "DLL_ERRORS");
	//		return false;
	//	}
	//	//fileInfo->LAZpoints.push_back(laszip_point(*point));

	//	// point->X -> lonX, point->Y -> latY, point->Z -> depth
	//	float readX = float(point->X * header->x_scale_factor);
	//	float readY = float(point->Z * header->z_scale_factor);
	//	float readZ = float(point->Y * header->y_scale_factor);

	//	pointClouds.back()->vertexInfo[p_count].position[0] = readX;
	//	pointClouds.back()->vertexInfo[p_count].position[1] = readY;
	//	pointClouds.back()->vertexInfo[p_count].position[2] = readZ;
	//	pointClouds.back()->vertexInfo[p_count].color[0] = byte(point->rgb[0] / float(1 << 16) * 255);
	//	pointClouds.back()->vertexInfo[p_count].color[1] = byte(point->rgb[1] / float(1 << 16) * 255);
	//	pointClouds.back()->vertexInfo[p_count].color[2] = byte(point->rgb[2] / float(1 << 16) * 255);
	//	pointClouds.back()->vertexIntensity[p_count] = point->intensity;
	//	if (maxIntensity < point->intensity)
	//		maxIntensity = point->intensity;

	//	if (pointClouds.back()->min.x > readX)
	//		pointClouds.back()->min.x = readX;

	//	if (pointClouds.back()->max.x < readX)
	//		pointClouds.back()->max.x = readX;

	//	if (pointClouds.back()->min.y > readY)
	//		pointClouds.back()->min.y = readY;

	//	if (pointClouds.back()->max.y < readY)
	//		pointClouds.back()->max.y = readY;

	//	if (pointClouds.back()->min.z > readZ)
	//		pointClouds.back()->min.z = readZ;

	//	if (pointClouds.back()->max.z < readZ)
	//		pointClouds.back()->max.z = readZ;

	//	p_count++;
	//}

	//if (header->point_data_format == 1)
	//{
	//	for (size_t i = 0; i < p_count; i++)
	//	{
	//		pointClouds.back()->vertexInfo[i].color[0] = byte(pointClouds.back()->vertexIntensity[i] / maxIntensity * 255);
	//		pointClouds.back()->vertexInfo[i].color[1] = byte(pointClouds.back()->vertexIntensity[i] / maxIntensity * 255);
	//		pointClouds.back()->vertexInfo[i].color[2] = byte(pointClouds.back()->vertexIntensity[i] / maxIntensity * 255);
	//	}
	//}

	//double rangeX = 0.0f;
	//double rangeY = 0.0f;
	//double rangeZ = 0.0f;

	//pointClouds.back()->initialZShift = pointClouds.back()->max.z < pointClouds.back()->min.z ? pointClouds.back()->max.z : pointClouds.back()->min.z;
	//pointClouds.back()->initialZShift = header->min_y - pointClouds.back()->initialZShift;

	//if (header->x_offset == 0 && header->y_offset == 0 && header->z_offset == 0)
	//{
	//	rangeX = pointClouds.back()->max.x - pointClouds.back()->min.x;
	//	rangeY = pointClouds.back()->max.y - pointClouds.back()->min.y;
	//	rangeZ = pointClouds.back()->max.z - pointClouds.back()->min.z;

	//	pointClouds.back()->adjustment.x = -pointClouds.back()->min.x /*- rangeX * 0.25*/;
	//	pointClouds.back()->adjustment.y = -pointClouds.back()->min.y /*- rangeY * 0.25*/;
	//	pointClouds.back()->adjustment.z = -pointClouds.back()->min.z /*- rangeZ * 0.25*/;
	//}
	//else
	//{
	//	pointClouds.back()->adjustment.x = float(-header->x_offset);
	//	pointClouds.back()->adjustment.y = float(-header->y_offset);
	//	pointClouds.back()->adjustment.z = float(-header->z_offset);
	//}

	//LOG.addToLog("rangeX: " + std::to_string(rangeX), "File_Load_Log");
	//LOG.addToLog("rangeY: " + std::to_string(rangeY), "File_Load_Log");
	//LOG.addToLog("rangeZ: " + std::to_string(rangeZ), "File_Load_Log");

	//LOG.addToLog("adjustment.x: " + std::to_string(pointClouds.back()->adjustment.x), "File_Load_Log");
	//LOG.addToLog("adjustment.y: " + std::to_string(pointClouds.back()->adjustment.y), "File_Load_Log");
	//LOG.addToLog("adjustment.z: " + std::to_string(pointClouds.back()->adjustment.z), "File_Load_Log");

	//double newMinX = DBL_MAX;
	//double newMaxX = -DBL_MAX;
	//double newMinY = DBL_MAX;
	//double newMaxY = -DBL_MAX;
	//double newMinZ = DBL_MAX;
	//double newMaxZ = -DBL_MAX;

	//for (int i = 0; i < npoints; i++)
	//{
	//	if (header->x_offset == 0 && header->y_offset == 0 && header->z_offset == 0)
	//	{
	//		pointClouds.back()->vertexInfo[i].position[0] = pointClouds.back()->vertexInfo[i].position[0] + pointClouds.back()->adjustment.x;
	//		pointClouds.back()->vertexInfo[i].position[1] = pointClouds.back()->vertexInfo[i].position[1] + pointClouds.back()->adjustment.y;
	//		pointClouds.back()->vertexInfo[i].position[2] = pointClouds.back()->vertexInfo[i].position[2] + pointClouds.back()->adjustment.z;
	//	}
	//	else
	//	{
	//		pointClouds.back()->vertexInfo[i].position[0] = pointClouds.back()->vertexInfo[i].position[0] + float(pointClouds.back()->adjustment.x * header->x_scale_factor);
	//		pointClouds.back()->vertexInfo[i].position[1] = pointClouds.back()->vertexInfo[i].position[1] + float(pointClouds.back()->adjustment.y * header->y_scale_factor);
	//		pointClouds.back()->vertexInfo[i].position[2] = pointClouds.back()->vertexInfo[i].position[2] + float(pointClouds.back()->adjustment.z * header->z_scale_factor);
	//	}

	//	if (newMinX > pointClouds.back()->vertexInfo[i].position[0])
	//		newMinX = pointClouds.back()->vertexInfo[i].position[0];

	//	if (newMaxX < pointClouds.back()->vertexInfo[i].position[0])
	//		newMaxX = pointClouds.back()->vertexInfo[i].position[0];

	//	if (newMinY > pointClouds.back()->vertexInfo[i].position[1])
	//		newMinY = pointClouds.back()->vertexInfo[i].position[1];

	//	if (newMaxY < pointClouds.back()->vertexInfo[i].position[1])
	//		newMaxY = pointClouds.back()->vertexInfo[i].position[1];

	//	if (newMinZ > pointClouds.back()->vertexInfo[i].position[2])
	//		newMinZ = pointClouds.back()->vertexInfo[i].position[2];

	//	if (newMaxZ < pointClouds.back()->vertexInfo[i].position[2])
	//		newMaxZ = pointClouds.back()->vertexInfo[i].position[2];
	//}

	//LOG.addToLog("newMinX: " + std::to_string(newMinX), "File_Load_Log");
	//LOG.addToLog("newMaxX: " + std::to_string(newMaxX), "File_Load_Log");
	//LOG.addToLog("newMinY: " + std::to_string(newMinY), "File_Load_Log");
	//LOG.addToLog("newMaxY: " + std::to_string(newMaxY), "File_Load_Log");
	//LOG.addToLog("newMinZ: " + std::to_string(newMinZ), "File_Load_Log");
	//LOG.addToLog("newMaxZ: " + std::to_string(newMaxZ), "File_Load_Log");

	//pointClouds.back()->initialXShift = header->min_x - newMinX;
	//if (header->x_offset == 0 && header->y_offset == 0 && header->z_offset == 0)
	//{
	//	pointClouds.back()->initialZShift = -pointClouds.back()->adjustment.z;
	//}

	//LOG.addToLog("pointClouds.back()->initialXShift: " + std::to_string(pointClouds.back()->initialXShift), "File_Load_Log");
	//LOG.addToLog("pointClouds.back()->initialZShift: " + std::to_string(pointClouds.back()->initialZShift), "File_Load_Log");

	//if (header->x_offset == 0 && header->y_offset == 0 && header->z_offset == 0)
	// std::swap(pointClouds.back()->adjustment.y, pointClouds.back()->adjustment.z);

	//// close the reader
	//if (laszip_close_reader(laszip_reader))
	//{
	//	LOG.addToLog("closing laszip reader failed", "DLL_ERRORS");
	//}

	//// destroy the reader
	//if (laszip_destroy(laszip_reader))
	//{
	//	LOG.addToLog("destroying laszip reader failed", "DLL_ERRORS");
	//}

	//pointClouds.back()->initializeOctree(rangeX, rangeY, rangeZ);
	////pointClouds.back()->loadedFrom = fileInfo;
	////pointClouds.back()->loadedFrom->resultingPointCloud = pointClouds.back();
	//LOG.addToLog("Total nodes created: " + std::to_string(pointClouds.back()->getSearchOctree()->getDebugNodeCount()), "OctreeEvents");
	//LOG.addToLog("Rootnode AABB size: " + std::to_string(pointClouds.back()->getSearchOctree()->root->nodeAABB.size), "OctreeEvents");
	//LOG.addToLog("Rootnode AABB min: ", pointClouds.back()->getSearchOctree()->root->nodeAABB.min, "OctreeEvents");
	//LOG.addToLog("Rootnode AABB max: ", pointClouds.back()->getSearchOctree()->root->nodeAABB.max, "OctreeEvents");

	//return true;
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API OnSceneStartFromUnity(char* projectFilePath)
{
	// Call for thread initialization.
	LoadManager::getInstance();

	for (size_t i = 0; i < pointClouds.size(); i++)
	{
		delete pointClouds[i];
	}
	pointClouds.clear();

	if (!DLLWasLoadedCorrectly)
	{
		if (laszip_load_dll(projectFilePath))
		{
			DLLWasLoadedCorrectly = false;
			LOG.addToLog("project path: " + std::string(projectFilePath), "DLL_ERRORS");
			LOG.addToLog("loading LASzip DLL failed", "DLL_ERRORS");
			return;
		}

		DLLWasLoadedCorrectly = true;

		laszip_U8 version_major;
		laszip_U8 version_minor;
		laszip_U16 version_revision;
		laszip_U32 version_build;
		if (laszip_get_version(&version_major, &version_minor, &version_revision, &version_build))
		{
			LOG.addToLog("getting LASzip DLL version number failed", "DLL_ERRORS");
		}
		LOG.addToLog("LASzip DLL v" + std::to_string((int)version_major) + "." + std::to_string((int)version_minor) + "r" + std::to_string((int)version_revision) + " (build " + std::to_string((int)version_build) + ")", "DLL_START");
	}
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API SetMeshBuffersFromUnity(void* vertexBufferHandle, int vertexCount, float* sourceVertices, float* sourceColor)
{
	/*std::vector<glm::vec3> rawPositions;
	rawPositions.resize(vertexCount);
	g_VertexSource.resize(vertexCount);

	for (int i = 0; i < vertexCount; ++i)
	{
		MeshVertex& v = g_VertexSource[i];
		v.position[0] = sourceVertices[0];
		v.position[1] = sourceVertices[1];
		v.position[2] = sourceVertices[2];

		v.color[0] = sourceColor[0] * 255;
		v.color[1] = sourceColor[1] * 255;
		v.color[2] = sourceColor[2] * 255;
		v.color[3] = sourceColor[3] * 255;

		rawPositions[i].x = sourceVertices[0];
		rawPositions[i].y = sourceVertices[1];
		rawPositions[i].z = sourceVertices[2];

		sourceVertices += 3;
		sourceColor += 4;
	}
	
	DWORD time = GetTickCount();*/



	/*if (mainOctree != nullptr)
		delete mainOctree;
	
	mainOctree = new octree(2000.0f, glm::vec3(-259.8f, 0.4f, 38.3f), rawPositions);
	for (int i = 0; i < vertexCount; i++)
	{
		mainOctree->addObject(i);
	}

	DWORD timeSpent = GetTickCount() - time;
	std::fstream testFile;
	std::string text = std::to_string(timeSpent);
	testFile.open("timeOctree.txt", std::ios::out);
	testFile.write("total time: ", strlen("total time: "));
	testFile.write(text.c_str(), text.size());
	testFile.write("\n", strlen("\n"));
	testFile.write("total nodes created: ", strlen("total nodes created: "));
	testFile.write(std::to_string(mainOctree->getDebugNodeCount()).c_str(), std::to_string(mainOctree->getDebugNodeCount()).size());
	testFile.close();*/
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API SaveToLAZFileFromUnity(char* filePath, int pointCloudIndex)
{
	if (pointCloudIndex >= pointClouds.size() || pointCloudIndex < 0)
		return;

	//LOG.addToLog("flag 0", "writeTest");
	laszip_POINTER laszip_writer;
	if (laszip_create(&laszip_writer))
	{
		LOG.addToLog("creating laszip writer failed", "DLL_ERRORS");
		return;
	}

	int pointsToWrite = 0;
	for (size_t j = 0; j < pointClouds[pointCloudIndex]->getPointCount(); j++)
	{
		if (pointClouds[pointCloudIndex]->vertexInfo[j].position[0] != -10000.0f)
			pointsToWrite++;
	}

	pointClouds[pointCloudIndex]->loadedFrom->header.number_of_point_records = pointsToWrite;
	if (laszip_set_header(laszip_writer, &pointClouds[pointCloudIndex]->loadedFrom->header))
	{
		LOG.addToLog("setting header for laszip writer failed", "DLL_ERRORS");
	}

	std::string fileName = filePath;
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

	for (size_t j = 0; j < pointClouds[pointCloudIndex]->getPointCount(); j++)
	{
		if (pointClouds[pointCloudIndex]->vertexInfo[j].position[0] != -10000.0f)
		{
			if (laszip_set_point(laszip_writer, &pointClouds[pointCloudIndex]->loadedFrom->LAZpoints[j]))
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

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API RequestToDeleteFromUnity(float* center, float size)
{
	//DWORD time = GetTickCount();

	glm::vec3 centerOfBrush = glm::vec3(center[0], center[1], center[2]);

	for (size_t i = 0; i < pointClouds.size(); i++)
	{
		glm::vec3 localPosition = glm::inverse(glm::transpose(pointClouds[i]->worldMatrix)) * glm::vec4(centerOfBrush, 1.0f);

		if (pointClouds[i]->getSearchOctree()->isInOctreeBound(localPosition, size))
		{
			pointClouds[i]->getSearchOctree()->deleteObjects(localPosition, size);
		}

		LOG.addToLog("pointsToDelete size: " + std::to_string(pointClouds[i]->getSearchOctree()->pointsToDelete.size()), "deleteEvents");
	}
	
	/*DWORD timeSpent = GetTickCount() - time;
	std::fstream testFile;
	std::string text = std::to_string(timeSpent);
	testFile.open("timeDelete.txt", std::ios::out);
	testFile.write("total time: ", strlen("total time: "));
	testFile.write(text.c_str(), text.size());
	testFile.write("\n", strlen("\n"));
	testFile.close();*/
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

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API RequestPointCloudAdjustmentFromUnity(float* adjustment, int pointCloudIndex)
{
	if (pointCloudIndex >= pointClouds.size() || pointCloudIndex < 0)
		return;

	adjustment[0] = float(pointClouds[pointCloudIndex]->adjustment.x);
	adjustment[1] = float(pointClouds[pointCloudIndex]->adjustment.y);
	adjustment[2] = float(pointClouds[pointCloudIndex]->adjustment.z);

	adjustment[3] = float(pointClouds[pointCloudIndex]->initialXShift);
	adjustment[4] = float(pointClouds[pointCloudIndex]->initialZShift);
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API RequestPointCloudUTMZoneFromUnity(int* UTMZone, int* North, int pointCloudIndex)
{
	if (pointCloudIndex >= pointClouds.size() || pointCloudIndex < 0)
		return;

	UTMZone[0] = 0;
	North[0] = 0;

	if (pointClouds[pointCloudIndex]->UTMZone.size() == 0 || pointClouds[pointCloudIndex]->UTMZone.size() == 0)
	{
		LOG.addToLog("RequestPointCloudUTMZoneFromUnity was called but \"UTMZone\" or/and \"North\" was empty!", "DLL_ERRORS");
	}
	else if (pointClouds[pointCloudIndex]->UTMZone.size() == 1)
	{
		UTMZone[0] = atoi(pointClouds[pointCloudIndex]->UTMZone.substr(0, 1).c_str());
	}
	else if (pointClouds[pointCloudIndex]->UTMZone.size() > 2)
	{
		UTMZone[0] = atoi(pointClouds[pointCloudIndex]->UTMZone.substr(0, 2).c_str());
		North[0] = pointClouds[pointCloudIndex]->UTMZone.substr(2, 1) == "N" ? 1 : 0;
	}

	//LOG.addToLog("1", "DLL_ERRORS");
	//resultString = "";
	//if (pointCloudIndex >= pointClouds.size() || pointCloudIndex < 0)
	//	return;

	////resultString = pointClouds[pointCloudIndex]->spatialInfo;
	//output = const_cast<char*>(pointClouds[pointCloudIndex]->UTMZone.c_str());
	//LOG.addToLog("2", "DLL_ERRORS");
	////return const_cast<char*>(resultString.c_str());
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
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginUnload()
{
	s_Graphics->UnregisterDeviceEventCallback(OnGraphicsDeviceEvent);
}

static void CreateResources()
{
	D3D11_BUFFER_DESC desc;
	memset(&desc, 0, sizeof(desc));
	
	// constant buffer
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = 64;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = 0;
	m_Device->CreateBuffer(&desc, NULL, &m_CB);

	// shaders
	HRESULT hr;
	hr = m_Device->CreateVertexShader(kVertexShaderCode, sizeof(kVertexShaderCode), nullptr, &m_VertexShader);
	if (FAILED(hr))
		OutputDebugStringA("Failed to create vertex shader.\n");
	hr = m_Device->CreatePixelShader(kPixelShaderCode, sizeof(kPixelShaderCode), nullptr, &m_PixelShader);
	if (FAILED(hr))
		OutputDebugStringA("Failed to create pixel shader.\n");

	// input layout
	if (m_VertexShader)
	{
		D3D11_INPUT_ELEMENT_DESC s_DX11InputElementDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		m_Device->CreateInputLayout(s_DX11InputElementDesc, 2, kVertexShaderCode, sizeof(kVertexShaderCode), &m_InputLayout);
	}

	// render states
	D3D11_RASTERIZER_DESC rsdesc;
	memset(&rsdesc, 0, sizeof(rsdesc));
	rsdesc.FillMode = D3D11_FILL_SOLID;
	rsdesc.CullMode = D3D11_CULL_NONE;
	rsdesc.DepthClipEnable = TRUE;
	m_Device->CreateRasterizerState(&rsdesc, &m_RasterState);

	D3D11_DEPTH_STENCIL_DESC dsdesc;
	memset(&dsdesc, 0, sizeof(dsdesc));
	dsdesc.DepthEnable = TRUE;
	dsdesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsdesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
	m_Device->CreateDepthStencilState(&dsdesc, &m_DepthState);

	D3D11_BLEND_DESC bdesc;
	memset(&bdesc, 0, sizeof(bdesc));
	bdesc.RenderTarget[0].BlendEnable = FALSE;
	bdesc.RenderTarget[0].RenderTargetWriteMask = 0xF;
	m_Device->CreateBlendState(&bdesc, &m_BlendState);
}

static void DrawPointCloud(pointCloud* pointCloudToRender)
{
	if (!pointCloudToRender->wasFullyLoaded)
		return;

	glm::mat4 glmWorldMatrix = pointCloudToRender->worldMatrix;
	glm::mat4 glmViewMatrix = glm::make_mat4(worldToViewMatrix);
	glm::mat4 glmProjectionMatrix = glm::make_mat4(projectionMatrix);

	glmWorldMatrix = glm::transpose(glmWorldMatrix);
	glmViewMatrix = glm::transpose(glmViewMatrix);
	glmProjectionMatrix = glm::transpose(glmProjectionMatrix);

	if (frustumCulling)
	{
		float distance;
		for (int p = 0; p < 6; p++)
		{
			distance = frustum[p][0] * glmWorldMatrix[3][0] + frustum[p][1] * glmWorldMatrix[3][1] + frustum[p][2] * glmWorldMatrix[3][2] + frustum[p][3];
			if (distance <= -pointCloudToRender->getSearchOctree()->root->nodeAABB.size)
			{
				/*LOG.addToLog("distance: " + std::to_string(distance), "renderTest");
				LOG.addToLog("nodeAABB.size: " + std::to_string(pointCloudToRender->getSearchOctree()->root->nodeAABB.size), "renderTest");

				LOG.addToLog("glmWorldMatrix[3][0]: " + std::to_string(glmWorldMatrix[3][0]), "renderTest");
				LOG.addToLog("glmWorldMatrix[3][1]: " + std::to_string(glmWorldMatrix[3][1]), "renderTest");
				LOG.addToLog("glmWorldMatrix[3][2]: " + std::to_string(glmWorldMatrix[3][2]), "renderTest");*/
				return;
			}
		}
	}

	ID3D11DeviceContext* ctx = NULL;
	m_Device->GetImmediateContext(&ctx);

	ctx->OMSetDepthStencilState(m_DepthState, 0);
	ctx->RSSetState(m_RasterState);
	ctx->OMSetBlendState(m_BlendState, NULL, 0xFFFFFFFF);

	glm::mat4 finalMatrix = glmProjectionMatrix * glmViewMatrix * glmWorldMatrix;
	ctx->UpdateSubresource(m_CB, 0, NULL, glm::value_ptr(finalMatrix), 64, 0);

	// Set shaders
	ctx->VSSetConstantBuffers(0, 1, &m_CB);
	ctx->VSSetShader(m_VertexShader, NULL, 0);
	ctx->PSSetShader(m_PixelShader, NULL, 0);

	// Update vertex buffer
	const int kVertexSize = 12 + 4;

	if (!pointCloudToRender->wasInitialized || pointCloudToRender->getSearchOctree()->pointsToDelete.size() != 0)
	{
		if (!pointCloudToRender->wasInitialized)
		{
			D3D11_BUFFER_DESC desc;
			memset(&desc, 0, sizeof(desc));

			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.ByteWidth = UINT(pointCloudToRender->vertexInfo.size() * 16);
			desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			m_Device->CreateBuffer(&desc, NULL, &pointCloudToRender->mainVB);

			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.ByteWidth = UINT(pointCloudToRender->vertexInfo.size() * 16);
			desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			m_Device->CreateBuffer(&desc, NULL, &pointCloudToRender->intermediateVB);
		}

		D3D11_BOX box{};
		box.left = 0;
		box.right = 0 + pointCloudToRender->getPointCount() * kVertexSize;
		box.top = 0;
		box.bottom = 1;
		box.front = 0;
		box.back = 1;

		int min = INT_MAX;
		int max = INT_MIN;

		if (pointCloudToRender->getSearchOctree()->pointsToDelete.size() != 0)
		{
			LOG.addToLog("DrawPointCloud with pointsToDelete size: " + std::to_string(pointCloudToRender->getSearchOctree()->pointsToDelete.size()), "deleteEvents");
			LOG.addToLog("DrawPointCloud with pointsToDelete first element: " + std::to_string(pointCloudToRender->getSearchOctree()->pointsToDelete[0]), "deleteEvents");
			
			for (size_t i = 0; i < pointCloudToRender->getSearchOctree()->pointsToDelete.size(); i++)
			{
				if (pointCloudToRender->getSearchOctree()->pointsToDelete[i] > max)
					max = pointCloudToRender->getSearchOctree()->pointsToDelete[i];

				if (pointCloudToRender->getSearchOctree()->pointsToDelete[i] < min)
					min = pointCloudToRender->getSearchOctree()->pointsToDelete[i];

				pointCloudToRender->vertexInfo[pointCloudToRender->getSearchOctree()->pointsToDelete[i]].position[0] = -10000.0f;
			}

			pointCloudToRender->getSearchOctree()->pointsToDelete.clear();
			box.left = min * kVertexSize;
			box.right = min * kVertexSize + (max - min) * kVertexSize;
		}

		//DWORD time = GetTickCount();
		if (!pointCloudToRender->wasInitialized)
		{
			ctx->UpdateSubresource(pointCloudToRender->mainVB, 0, nullptr, pointCloudToRender->vertexInfo.data(), pointCloudToRender->getPointCount() * kVertexSize, pointCloudToRender->getPointCount() * kVertexSize);
			LOG.addToLog("copy data to vertex buffer, vertexInfo size: " + std::to_string(pointCloudToRender->vertexInfo.size()), "OctreeEvents");
		}
		else
		{
			LOG.addToLog("DrawPointCloud else block entered, vertex count: " + std::to_string(pointCloudToRender->vertexInfo.size()), "deleteEvents");

			D3D11_BOX dbox{};
			dbox.left = 0;
			dbox.right = (max - min) * kVertexSize;
			dbox.top = 0;
			dbox.bottom = 1;
			dbox.front = 0;
			dbox.back = 1;

			ctx->UpdateSubresource(pointCloudToRender->intermediateVB, 0, &dbox, pointCloudToRender->vertexInfo.data() + min, pointCloudToRender->getPointCount() * kVertexSize, pointCloudToRender->getPointCount() * kVertexSize);
			ctx->CopySubresourceRegion(pointCloudToRender->mainVB, 0, box.left, 0, 0, pointCloudToRender->intermediateVB, 0, &dbox);
		}

		pointCloudToRender->wasInitialized = true;

		//DWORD timeSpent = GetTickCount() - time;
		//std::fstream testFile;
		//std::string text = std::to_string(timeSpent);
		//testFile.open("time.txt", std::ios::out);
		//testFile.write("time: ", strlen("time: "));
		//testFile.write(text.c_str(), text.size());
		//testFile.write(" ms", strlen(" ms"));
		//testFile.write("\n", strlen("\n"));

		//testFile.write("points: ", strlen("points: "));
		//testFile.write(std::to_string(pointsCount).c_str(), std::to_string(pointsCount).size());
		//testFile.write("\n", strlen("\n"));

		//testFile.write("min: ", strlen("min: "));
		//testFile.write(std::to_string(min).c_str(), std::to_string(min).size());
		//testFile.write("\n", strlen("\n"));

		//testFile.write("lenght: ", strlen("lenght: "));
		//testFile.write(std::to_string((max - min)).c_str(), std::to_string((max - min)).size());
		//testFile.write("\n", strlen("\n"));

		//testFile.close();
	}

	ctx->IASetInputLayout(m_InputLayout);
	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	UINT stride = kVertexSize;
	UINT offset = 0;
	ctx->IASetVertexBuffers(0, 1, &pointCloudToRender->mainVB, &stride, &offset);
	ctx->Draw(pointCloudToRender->getPointCount(), 0);

	ctx->Release();
}

static void ProcessDeviceEvent(UnityGfxDeviceEventType type, IUnityInterfaces* interfaces)
{
	switch (type)
	{
		case kUnityGfxDeviceEventInitialize:
		{
			IUnityGraphicsD3D11* d3d = interfaces->Get<IUnityGraphicsD3D11>();
			m_Device = d3d->GetDevice();
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

static void Render()
{
	for (size_t i = 0; i < pointClouds.size(); i++)
	{
		DrawPointCloud(pointClouds[i]);
	}
}

extern "C" bool UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API updateWorldMatrix(void* worldMatrix, int pointCloudIndex)
{
	if (pointCloudIndex >= pointClouds.size())
		return false;
	
	for (size_t i = 0; i < 16; i++)
	{
		pointsWorldMatrix[i] = ((float*)(worldMatrix))[i];
	}
	pointClouds[pointCloudIndex]->worldMatrix = glm::make_mat4(pointsWorldMatrix);

	return true;
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API updateCamera(void* cameraWorldMatrix, void* cameraProjectionMatrix)
{
	for (size_t i = 0; i < 16; i++)
	{
		worldToViewMatrix[i] = ((float*)(cameraWorldMatrix))[i];
	}

	for (size_t i = 0; i < 16; i++)
	{
		projectionMatrix[i] = ((float*)(cameraProjectionMatrix))[i];
	}

	updateFrustumPlanes();
}

static void UNITY_INTERFACE_API OnRenderEvent(int eventID)
{
	Render();
}

extern "C" UnityRenderingEvent UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API GetRenderEventFunc()
{
	return OnRenderEvent;
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
	//frustumCulling = *(int*)active ? true : false;
	frustumCulling = active;
}