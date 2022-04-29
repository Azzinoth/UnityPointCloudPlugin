#include "pch.h"
#include "debugLog.h"

debugLog* debugLog::_instance = nullptr;

debugLog::debugLog()
{
}

debugLog::~debugLog()
{
	auto it = openedFiles.begin();
	while (it != openedFiles.end())
	{
		it->second->close();
		it++;
	}
}

std::string debugLog::vec3ToString(glm::vec3 vector)
{
	return "x: " + std::to_string(vector.x) + " y: " + std::to_string(vector.y) + " z: " + std::to_string(vector.z);
}

void debugLog::addToLog(std::string logEntry, std::string topic)
{
	/*std::fstream* file = nullptr;
	if (openedFiles.find(topic) != openedFiles.end())
	{
		file = openedFiles[topic];
	}
	else
	{
		file = new std::fstream;
		file->open(topic + ".txt", std::ios::out);
		openedFiles[topic] = file;
	}

	logEntry += '\n';
	file->write(logEntry.c_str(), logEntry.size());
	file->flush();*/
}

void debugLog::addToLog(std::string logEntry, glm::vec3 vector, std::string topic)
{
	logEntry += vec3ToString(vector);
	addToLog(logEntry, topic);
}

std::string debugLog::mat4ToString(glm::mat4 matrix)
{
	string result;
	result = "\n[ " + std::to_string(matrix[0][0]) + ", " + std::to_string(matrix[0][1]) + ", " + std::to_string(matrix[0][2]) + ", " + std::to_string(matrix[0][3]) + " ]\n";
	result += "[ " + std::to_string(matrix[1][0]) + ", " + std::to_string(matrix[1][1]) + ", " + std::to_string(matrix[1][2]) + ", " + std::to_string(matrix[1][3]) + " ]\n";
	result += "[ " + std::to_string(matrix[2][0]) + ", " + std::to_string(matrix[2][1]) + ", " + std::to_string(matrix[2][2]) + ", " + std::to_string(matrix[2][3]) + " ]\n";
	result += "[ " + std::to_string(matrix[3][0]) + ", " + std::to_string(matrix[3][1]) + ", " + std::to_string(matrix[3][2]) + ", " + std::to_string(matrix[3][3]) + " ]";

	return result;
}

void debugLog::addToLog(std::string logEntry, glm::mat4 matrix, std::string topic)
{
	logEntry += mat4ToString(matrix);
	addToLog(logEntry, topic);
}

DX11GPU* DX11GPU::_instance = nullptr;

DX11GPU::DX11GPU() {};
DX11GPU::~DX11GPU() {};

ID3D11Device* DX11GPU::getDevice()
{
	return m_Device;
}

void DX11GPU::initialize(IUnityInterfaces* interfaces)
{
	IUnityGraphicsD3D11* d3d = interfaces->Get<IUnityGraphicsD3D11>();
	m_Device = d3d->GetDevice();
}