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
#include "thirdparty/FEBasicApplication/FEBasicApplication.h"

#include "Unity/IUnityGraphicsD3D11.h"
#include "Unity/IUnityInterface.h"
#include "Unity/IUnityGraphics.h"

#define GLM_FORCE_XYZW_ONLY
#include "glm/gtc/type_ptr.hpp"
#include "glm/mat4x4.hpp"
#include "glm/vec3.hpp"

std::string vec3ToString(glm::vec3 vector);
std::string mat4ToString(glm::mat4 matrix);

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