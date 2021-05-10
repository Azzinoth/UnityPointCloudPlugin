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
	std::fstream* file = nullptr;
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
	file->flush();
}

void debugLog::addToLog(std::string logEntry, glm::vec3 vector, std::string topic)
{
	logEntry += vec3ToString(vector);
	addToLog(logEntry, topic);
}