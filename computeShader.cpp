#include "computeShader.h"

HRESULT CreateBufferUAV(ID3D11Device* pDevice, ID3D11Buffer* pBuffer, ID3D11UnorderedAccessView** ppUAVOut)
{
	D3D11_BUFFER_DESC descBuf = {};
	pBuffer->GetDesc(&descBuf);

	/*D3D11_BUFFER_UAV UAV_desc = {};
	UAV_desc.FirstElement = 0;
	UAV_desc.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;
	UAV_desc.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;*/

	D3D11_UNORDERED_ACCESS_VIEW_DESC desc = {};
	desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	desc.Buffer.FirstElement = 0;

	desc.Format = DXGI_FORMAT_UNKNOWN;      // Format must be must be DXGI_FORMAT_UNKNOWN, when creating a View of a Structured Buffer
	desc.Buffer.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;
	desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;

	return pDevice->CreateUnorderedAccessView(pBuffer, &desc, ppUAVOut);
}

HRESULT CreateBufferSRV(ID3D11Device* pDevice, ID3D11Buffer* pBuffer, ID3D11ShaderResourceView** ppSRVOut)
{
	D3D11_BUFFER_DESC descBuf = {};
	pBuffer->GetDesc(&descBuf);

	D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
	desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	desc.BufferEx.FirstElement = 0;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.BufferEx.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;

	return pDevice->CreateShaderResourceView(pBuffer, &desc, ppSRVOut);
}

static DWORD timeLastTimeCall = GetTickCount();
void RunComputeShader(ID3D11DeviceContext* pd3dImmediateContext,
	ID3D11ComputeShader* pComputeShader,
	UINT nNumViews, ID3D11ShaderResourceView** pShaderResourceViews,
	ID3D11Buffer* pCBCS, void* pCSData, DWORD dwNumDataBytes,
	ID3D11UnorderedAccessView* pUnorderedAccessView,
	UINT X, UINT Y, UINT Z)
{
	if (GetTickCount() - timeLastTimeCall < 20)
	{
		//LOG.addToLog("denial RequestToDeleteFromUnity", "deleteEvents");
		//LOG.addToLog("time: " + std::to_string(GetTickCount() - timeLastTimeCall), "deleteEvents");
		return;
	}
	timeLastTimeCall = GetTickCount();

	pd3dImmediateContext->CSSetShader(pComputeShader, nullptr, 0);
	pd3dImmediateContext->CSSetShaderResources(0, nNumViews, pShaderResourceViews);
	const UINT zero = 0;
	pd3dImmediateContext->CSSetUnorderedAccessViews(0, 1, &pUnorderedAccessView, &zero);
	if (pCBCS && pCSData)
	{
		D3D11_MAPPED_SUBRESOURCE MappedResource;
		pd3dImmediateContext->Map(pCBCS, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
		memcpy(MappedResource.pData, pCSData, dwNumDataBytes);
		pd3dImmediateContext->Unmap(pCBCS, 0);
		ID3D11Buffer* ppCB[1] = { pCBCS };
		pd3dImmediateContext->CSSetConstantBuffers(0, 1, ppCB);
	}

	pd3dImmediateContext->Dispatch(X, Y, Z);

	pd3dImmediateContext->CSSetShader(nullptr, nullptr, 0);

	ID3D11UnorderedAccessView* ppUAViewnullptr[1] = { nullptr };
	pd3dImmediateContext->CSSetUnorderedAccessViews(0, 1, ppUAViewnullptr, nullptr);

	ID3D11ShaderResourceView* ppSRVnullptr[2] = { nullptr, nullptr };
	pd3dImmediateContext->CSSetShaderResources(0, 2, ppSRVnullptr);

	ID3D11Buffer* ppCBnullptr[1] = { nullptr };
	pd3dImmediateContext->CSSetConstantBuffers(0, 1, ppCBnullptr);
}

void compileAndCreateComputeShader(ID3D11Device* pDevice, BYTE* source, ID3D11ComputeShader** computeShader)
{
	HRESULT shaderResult = pDevice->CreateComputeShader(g_CSMain_, sizeof(g_CSMain_), nullptr, computeShader);
	
	if (FAILED(shaderResult))
	{
		LOG.addToLog("shaderResult: " + std::system_category().message(shaderResult), "computeShader");
	}
}

void compileAndCreateComputeShader(ID3D11Device* pDevice, std::string sourceFile, ID3D11ComputeShader** computeShader)
{
	ID3DBlob* shaderBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	const D3D_SHADER_MACRO defines[] =
	{
		"EXAMPLE_DEFINE", "1",
		NULL, NULL
	};

	
	HRESULT shaderResult = D3DCompileFromFile(std::wstring(sourceFile.begin(), sourceFile.end()).c_str(), defines,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"CSMain", "cs_5_0",
		D3DCOMPILE_ENABLE_STRICTNESS, 0, &shaderBlob, &errorBlob);

	if (FAILED(shaderResult))
	{
		LOG.addToLog("shaderResult: " + std::system_category().message(shaderResult), "computeShader");
		if (errorBlob)
		{
			LOG.addToLog("shaderResult: " + std::string((char*)errorBlob->GetBufferPointer()), "computeShader");
			errorBlob->Release();
		}

		if (shaderBlob)
			shaderBlob->Release();
	}

	shaderResult = pDevice->CreateComputeShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &*computeShader);
	LOG.addToLog("pDevice->CreateComputeShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &computeShader): " + std::system_category().message(shaderResult), "computeShader");
	shaderBlob->Release();
}

int getComputeShaderResultCounter(ID3D11Device* pDevice, ID3D11UnorderedAccessView* result_UAV)
{
	ID3D11DeviceContext* ctx = NULL;
	pDevice->GetImmediateContext(&ctx);

	ID3D11Buffer* buffer;
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));

	bufferDesc.Usage = D3D11_USAGE_STAGING;
	bufferDesc.BindFlags = 0;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	bufferDesc.MiscFlags = 0;
	bufferDesc.ByteWidth = 4;

	pDevice->CreateBuffer(&bufferDesc, NULL, &buffer);

	ctx->CopyStructureCount(buffer, 0, result_UAV);
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	unsigned int* counter = nullptr;

	ctx->Map(buffer, 0, D3D11_MAP_READ, 0, &MappedResource);
	counter = (unsigned int*)MappedResource.pData;
	//int result = *counter;
	std::string c_ = std::to_string(*counter);
	int result = atoi(c_.c_str());

	//LOG.addToLog("counter_: " + std::to_string(test), "computeShader");
	ctx->Unmap(buffer, 0);

	ctx->Release();

	return result;
}

computeShaderWrapper::computeShaderWrapper(ID3D11Device* pDevice, std::string sourceFile)
{
	ID3DBlob* shaderBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	const D3D_SHADER_MACRO defines[] =
	{
		"EXAMPLE_DEFINE", "1",
		NULL, NULL
	};

	HRESULT shaderResult = D3DCompileFromFile(std::wstring(sourceFile.begin(), sourceFile.end()).c_str(), defines,
											  D3D_COMPILE_STANDARD_FILE_INCLUDE,
											  "CSMain", "cs_5_0",
											  D3DCOMPILE_ENABLE_STRICTNESS, 0, &shaderBlob, &errorBlob);

	if (FAILED(shaderResult))
	{
		LOG.addToLog("D3DCompileFromFile result: " + std::system_category().message(shaderResult), "computeShaderWrapper");
		if (errorBlob)
		{
			LOG.addToLog("D3DCompileFromFile msg: " + std::string((char*)errorBlob->GetBufferPointer()), "computeShaderWrapper");
			errorBlob->Release();
		}

		if (shaderBlob)
			shaderBlob->Release();
	}

	shaderResult = pDevice->CreateComputeShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &shader);
	LOG.addToLog("CreateComputeShader result: " + std::system_category().message(shaderResult), "computeShaderWrapper");
	shaderBlob->Release();
}

ID3D11Buffer* CreateAndCopyToDebugBuf(ID3D11Device* pDevice, ID3D11DeviceContext* pd3dImmediateContext, ID3D11Buffer* pBuffer)
{
	ID3D11Buffer* debugbuf = nullptr;

	D3D11_BUFFER_DESC desc = {};
	pBuffer->GetDesc(&desc);
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc.Usage = D3D11_USAGE_STAGING;
	desc.BindFlags = 0;
	desc.MiscFlags = 0;
	if (SUCCEEDED(pDevice->CreateBuffer(&desc, nullptr, &debugbuf)))
	{
		debugbuf->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof("Debug") - 1, "Debug");
		pd3dImmediateContext->CopyResource(debugbuf, pBuffer);
	}

	return debugbuf;
}