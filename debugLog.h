#pragma once

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <system_error>

#include <d3d11.h>
#include <d3dcompiler.h>
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib") // WKPDID_D3DDebugObjectName

//#include "computeShader.h"

#include "Unity/IUnityGraphicsD3D11.h"
#include "Unity/IUnityInterface.h"
#include "Unity/IUnityGraphics.h"

#define GLM_FORCE_XYZW_ONLY
#include "glm/gtc/type_ptr.hpp"
#include "glm/mat4x4.hpp"
#include "glm/vec3.hpp"

#define SINGLETON_PUBLIC_PART(CLASS_NAME)  \
static CLASS_NAME& getInstance()           \
{										   \
	if (!_instance)                        \
		_instance = new CLASS_NAME();      \
	return *_instance;				       \
}                                          \
										   \
~CLASS_NAME();

#define SINGLETON_PRIVATE_PART(CLASS_NAME) \
static CLASS_NAME* _instance;              \
CLASS_NAME();                              \
CLASS_NAME(const CLASS_NAME &);            \
void operator= (const CLASS_NAME &);

class debugLog
{
public:
	SINGLETON_PUBLIC_PART(debugLog)
	void addToLog(std::string logEntry, std::string topic = "debugLog");
	void addToLog(std::string logEntry, glm::vec3 vector, std::string topic = "debugLog");
	void addToLog(std::string logEntry, glm::mat4 matrix, std::string topic = "debugLog");
	std::vector<std::string> log;

	void DisableTopicFileOutput(std::string TopicToDisable);
	void EnableTopicFileOutput(std::string TopicToDisable);

	bool IsFileOutputActive();
	void SetFileOutput(bool NewValue);
private:
	SINGLETON_PRIVATE_PART(debugLog)
	std::unordered_map<std::string, std::fstream*> openedFiles;
	std::string vec3ToString(glm::vec3 vector);
	std::string mat4ToString(glm::mat4 matrix);

	std::unordered_map<std::string, bool> DisabledTopics;
	bool bFileOutput = true;
};

#define LOG debugLog::getInstance()

class DX11GPU
{
public:
	SINGLETON_PUBLIC_PART(DX11GPU)

	void initialize(IUnityInterfaces* interfaces);
	ID3D11Device* getDevice();
private:
	SINGLETON_PRIVATE_PART(DX11GPU)

	ID3D11Device* m_Device = nullptr;
};

#define GPU DX11GPU::getInstance()