#include "pch.h"
#include "DX11GPU.h"

std::string vec3ToString(glm::vec3 vector)
{
	return "x: " + std::to_string(vector.x) + " y: " + std::to_string(vector.y) + " z: " + std::to_string(vector.z);
}

std::string mat4ToString(glm::mat4 matrix)
{
	std::string result;
	result = "\n[ " + std::to_string(matrix[0][0]) + ", " + std::to_string(matrix[0][1]) + ", " + std::to_string(matrix[0][2]) + ", " + std::to_string(matrix[0][3]) + " ]\n";
	result += "[ " + std::to_string(matrix[1][0]) + ", " + std::to_string(matrix[1][1]) + ", " + std::to_string(matrix[1][2]) + ", " + std::to_string(matrix[1][3]) + " ]\n";
	result += "[ " + std::to_string(matrix[2][0]) + ", " + std::to_string(matrix[2][1]) + ", " + std::to_string(matrix[2][2]) + ", " + std::to_string(matrix[2][3]) + " ]\n";
	result += "[ " + std::to_string(matrix[3][0]) + ", " + std::to_string(matrix[3][1]) + ", " + std::to_string(matrix[3][2]) + ", " + std::to_string(matrix[3][3]) + " ]";

	return result;
}

DX11GPU* DX11GPU::Instance = nullptr;

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