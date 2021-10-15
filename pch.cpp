#include "pch.h"

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

ID3D11ComputeShader* computeShader = nullptr;

MeshVertex* allPointsData_CS;

#define FLOAT_TEST

#ifdef FLOAT_TEST
	float* InputData_CS;
#else
	MeshVertex* InputData_CS;
#endif // FLOAT_TEST

//#define USE_COMPUTE_SHADER

#ifdef USE_COMPUTE_SHADER
	ID3D11Buffer* allPointsDataBuffer_CS = nullptr;
	ID3D11Buffer* InputDataBuffer_CS = nullptr;
	ID3D11Buffer* resultBuffer_CS = nullptr;
	ID3D11Buffer* resultBufferSecond_CS = nullptr;
	ID3D11Buffer** currentBuffer_CS = nullptr;

	ID3D11ShaderResourceView* inputPoints_CS_SRV = nullptr;
	ID3D11ShaderResourceView** currentInputPoints_CS_SRV = nullptr;
	ID3D11ShaderResourceView* InputData_CS_SRV = nullptr;
	ID3D11UnorderedAccessView* result_CS_UAV = nullptr;
	ID3D11UnorderedAccessView* resultSecond_CS_UAV = nullptr;
	ID3D11UnorderedAccessView** current_CS_UAV = nullptr;

	ID3D11ShaderResourceView* result_CS_SRV = nullptr;
	ID3D11ShaderResourceView* resultSecond_CS_SRV = nullptr;
	ID3D11ShaderResourceView** current_CS_SRV = nullptr;

	void runMyDeleteComputeShader();
#endif

CShader* outliers_CS = nullptr;
//ID3D11ComputeShader* outliers_CS = nullptr;
//ID3D11Buffer* outliersBuffer_CS = nullptr;
//ID3D11ShaderResourceView* outliers_CS_SRV = nullptr;
//ID3D11UnorderedAccessView* outliers_result_CS_UAV = nullptr;

static float testLevel = -830.0f;
static float** testFrustum = nullptr;

static float** frustum = nullptr;
static void updateFrustumPlanes();
static bool frustumCulling = false;
static bool LODSystemActive = false;
static bool highlightDeletedPoints = false;
static glm::vec3 deletionSpherePosition = glm::vec3(0.0f);
static float deletionSphereSize = 0.0f;

static float LODTransitionDistance = 3500.0f;

#ifdef USE_COMPUTE_SHADER
const BYTE kVertexShaderCode[] =
{
	 68,  88,  66,  67, 151, 203,
	165,  23, 102,  69,  86,  29,
	 66, 255, 232,  30, 144, 208,
	226,  99,   1,   0,   0,   0,
	164,   5,   0,   0,   6,   0,
	  0,   0,  56,   0,   0,   0,
	216,   1,   0,   0,  76,   2,
	  0,   0, 160,   2,   0,   0,
	 24,   5,   0,   0,  40,   5,
	  0,   0,  82,  68,  69,  70,
	152,   1,   0,   0,   2,   0,
	  0,   0, 112,   0,   0,   0,
	  2,   0,   0,   0,  28,   0,
	  0,   0,   0,   4, 254, 255,
	  0,   1,   0,   0, 100,   1,
	  0,   0,  92,   0,   0,   0,
	  5,   0,   0,   0,   6,   0,
	  0,   0,   1,   0,   0,   0,
	 16,   0,   0,   0,   0,   0,
	  0,   0,   1,   0,   0,   0,
	  1,   0,   0,   0, 104,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   1,   0,
	  0,   0,   1,   0,   0,   0,
	112, 111, 105, 110, 116,  66,
	117, 102, 102, 101, 114,   0,
	 77, 121,  67,  66,   0, 171,
	171, 171, 104,   0,   0,   0,
	  1,   0,   0,   0, 160,   0,
	  0,   0,  64,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,  92,   0,   0,   0,
	  1,   0,   0,   0, 212,   0,
	  0,   0,  16,   0,   0,   0,
	  0,   0,   0,   0,   3,   0,
	  0,   0, 184,   0,   0,   0,
	  0,   0,   0,   0,  64,   0,
	  0,   0,   2,   0,   0,   0,
	196,   0,   0,   0,   0,   0,
	  0,   0, 119, 111, 114, 108,
	100,  77,  97, 116, 114, 105,
	120,   0,   3,   0,   3,   0,
	  4,   0,   4,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	236,   0,   0,   0,   0,   0,
	  0,   0,  16,   0,   0,   0,
	  2,   0,   0,   0,  84,   1,
	  0,   0,   0,   0,   0,   0,
	 36,  69, 108, 101, 109, 101,
	110, 116,   0, 120,   0, 171,
	  0,   0,   3,   0,   1,   0,
	  1,   0,   0,   0,   0,   0,
	  0,   0,   0,   0, 121,   0,
	122,   0,  99, 111, 108, 111,
	114,   0, 171, 171,   0,   0,
	  2,   0,   1,   0,   1,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0, 245,   0,   0,   0,
	248,   0,   0,   0,   0,   0,
	  0,   0,   8,   1,   0,   0,
	248,   0,   0,   0,   4,   0,
	  0,   0,  10,   1,   0,   0,
	248,   0,   0,   0,   8,   0,
	  0,   0,  12,   1,   0,   0,
	 20,   1,   0,   0,  12,   0,
	  0,   0,   5,   0,   0,   0,
	  1,   0,   4,   0,   0,   0,
	  4,   0,  36,   1,   0,   0,
	 77, 105,  99, 114, 111, 115,
	111, 102, 116,  32,  40,  82,
	 41,  32,  72,  76,  83,  76,
	 32,  83, 104,  97, 100, 101,
	114,  32,  67, 111, 109, 112,
	105, 108, 101, 114,  32,  57,
	 46,  50,  57,  46,  57,  53,
	 50,  46,  51,  49,  49,  49,
	  0, 171, 171, 171,  73,  83,
	 71,  78, 108,   0,   0,   0,
	  3,   0,   0,   0,   8,   0,
	  0,   0,  80,   0,   0,   0,
	  0,   0,   0,   0,   6,   0,
	  0,   0,   1,   0,   0,   0,
	  0,   0,   0,   0,   1,   1,
	  0,   0,  92,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   3,   0,   0,   0,
	  1,   0,   0,   0,   7,   0,
	  0,   0, 101,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   3,   0,   0,   0,
	  2,   0,   0,   0,  15,   0,
	  0,   0,  83,  86,  95,  86,
	101, 114, 116, 101, 120,  73,
	 68,   0,  80,  79,  83,  73,
	 84,  73,  79,  78,   0,  67,
	 79,  76,  79,  82,   0, 171,
	 79,  83,  71,  78,  76,   0,
	  0,   0,   2,   0,   0,   0,
	  8,   0,   0,   0,  56,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   3,   0,
	  0,   0,   0,   0,   0,   0,
	 15,   0,   0,   0,  62,   0,
	  0,   0,   0,   0,   0,   0,
	  1,   0,   0,   0,   3,   0,
	  0,   0,   1,   0,   0,   0,
	 15,   0,   0,   0,  67,  79,
	 76,  79,  82,   0,  83,  86,
	 95,  80, 111, 115, 105, 116,
	105, 111, 110,   0, 171, 171,
	 83,  72,  69,  88, 112,   2,
	  0,   0,  64,   0,   1,   0,
	156,   0,   0,   0, 106,  72,
	  0,   1,  89,   0,   0,   4,
	 70, 142,  32,   0,   0,   0,
	  0,   0,   4,   0,   0,   0,
	162,   0,   0,   4,   0, 112,
	 16,   0,   0,   0,   0,   0,
	 16,   0,   0,   0,  96,   0,
	  0,   4,  18,  16,  16,   0,
	  0,   0,   0,   0,   6,   0,
	  0,   0, 101,   0,   0,   3,
	242,  32,  16,   0,   0,   0,
	  0,   0, 103,   0,   0,   4,
	242,  32,  16,   0,   1,   0,
	  0,   0,   1,   0,   0,   0,
	104,   0,   0,   2,   2,   0,
	  0,   0, 167,   0,   0,   9,
	 18,   0,  16,   0,   0,   0,
	  0,   0,  10,  16,  16,   0,
	  0,   0,   0,   0,   1,  64,
	  0,   0,  12,   0,   0,   0,
	  6, 112,  16,   0,   0,   0,
	  0,   0,  85,   0,   0,   7,
	 34,   0,  16,   0,   0,   0,
	  0,   0,  10,   0,  16,   0,
	  0,   0,   0,   0,   1,  64,
	  0,   0,   8,   0,   0,   0,
	 85,   0,   0,   7,  66,   0,
	 16,   0,   0,   0,   0,   0,
	 10,   0,  16,   0,   0,   0,
	  0,   0,   1,  64,   0,   0,
	 16,   0,   0,   0,   1,   0,
	  0,   7,  18,   0,  16,   0,
	  0,   0,   0,   0,  10,   0,
	 16,   0,   0,   0,   0,   0,
	  1,  64,   0,   0, 255,   0,
	  0,   0,  86,   0,   0,   5,
	 18,   0,  16,   0,   0,   0,
	  0,   0,  10,   0,  16,   0,
	  0,   0,   0,   0,  56,   0,
	  0,   7,  18,  32,  16,   0,
	  0,   0,   0,   0,  10,   0,
	 16,   0,   0,   0,   0,   0,
	  1,  64,   0,   0, 129, 128,
	128,  59,   1,   0,   0,  10,
	 50,   0,  16,   0,   0,   0,
	  0,   0, 150,   5,  16,   0,
	  0,   0,   0,   0,   2,  64,
	  0,   0, 255,   0,   0,   0,
	255,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	 86,   0,   0,   5,  50,   0,
	 16,   0,   0,   0,   0,   0,
	 70,   0,  16,   0,   0,   0,
	  0,   0,  56,   0,   0,  10,
	 98,  32,  16,   0,   0,   0,
	  0,   0,   6,   1,  16,   0,
	  0,   0,   0,   0,   2,  64,
	  0,   0,   0,   0,   0,   0,
	129, 128, 128,  59, 129, 128,
	128,  59,   0,   0,   0,   0,
	167,   0,   0,   9, 242,   0,
	 16,   0,   0,   0,   0,   0,
	 10,  16,  16,   0,   0,   0,
	  0,   0,   1,  64,   0,   0,
	  0,   0,   0,   0,  70, 126,
	 16,   0,   0,   0,   0,   0,
	 42,   0,   0,   7, 130,   0,
	 16,   0,   0,   0,   0,   0,
	 58,   0,  16,   0,   0,   0,
	  0,   0,   1,  64,   0,   0,
	 24,   0,   0,   0,  43,   0,
	  0,   5, 130,   0,  16,   0,
	  0,   0,   0,   0,  58,   0,
	 16,   0,   0,   0,   0,   0,
	 56,   0,   0,   7, 130,  32,
	 16,   0,   0,   0,   0,   0,
	 58,   0,  16,   0,   0,   0,
	  0,   0,   1,  64,   0,   0,
	129, 128, 128,  59,  56,   0,
	  0,   8, 242,   0,  16,   0,
	  1,   0,   0,   0,  86,   5,
	 16,   0,   0,   0,   0,   0,
	 70, 142,  32,   0,   0,   0,
	  0,   0,   1,   0,   0,   0,
	 50,   0,   0,  10, 242,   0,
	 16,   0,   1,   0,   0,   0,
	 70, 142,  32,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  6,   0,  16,   0,   0,   0,
	  0,   0,  70,  14,  16,   0,
	  1,   0,   0,   0,  50,   0,
	  0,  10, 242,   0,  16,   0,
	  0,   0,   0,   0,  70, 142,
	 32,   0,   0,   0,   0,   0,
	  2,   0,   0,   0, 166,  10,
	 16,   0,   0,   0,   0,   0,
	 70,  14,  16,   0,   1,   0,
	  0,   0,   0,   0,   0,   8,
	242,  32,  16,   0,   1,   0,
	  0,   0,  70,  14,  16,   0,
	  0,   0,   0,   0,  70, 142,
	 32,   0,   0,   0,   0,   0,
	  3,   0,   0,   0,  62,   0,
	  0,   1,  83,  70,  73,  48,
	  8,   0,   0,   0,   2,   0,
	  0,   0,   0,   0,   0,   0,
	 83,  84,  65,  84, 116,   0,
	  0,   0,  18,   0,   0,   0,
	  2,   0,   0,   0,   0,   0,
	  0,   0,   3,   0,   0,   0,
	  5,   0,   0,   0,   1,   0,
	  0,   0,   4,   0,   0,   0,
	  1,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   3,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0
};
#else
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

#endif // USE_COMPUTE_SHADER

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

const BYTE g_CSMain[] =
{
	 68,  88,  66,  67, 211, 222,
	194, 146, 173,   7,  60, 211,
	 47, 242, 108, 180, 169, 113,
	149, 208,   1,   0,   0,   0,
	136,   5,   0,   0,   5,   0,
	  0,   0,  52,   0,   0,   0,
	176,   2,   0,   0, 192,   2,
	  0,   0, 208,   2,   0,   0,
	236,   4,   0,   0,  82,  68,
	 69,  70, 116,   2,   0,   0,
	  3,   0,   0,   0, 184,   0,
	  0,   0,   3,   0,   0,   0,
	 60,   0,   0,   0,   0,   5,
	 83,  67,   0,   1,   0,   0,
	 64,   2,   0,   0,  82,  68,
	 49,  49,  60,   0,   0,   0,
	 24,   0,   0,   0,  32,   0,
	  0,   0,  40,   0,   0,   0,
	 36,   0,   0,   0,  12,   0,
	  0,   0,   0,   0,   0,   0,
	156,   0,   0,   0,   5,   0,
	  0,   0,   6,   0,   0,   0,
	  1,   0,   0,   0,  16,   0,
	  0,   0,   0,   0,   0,   0,
	  1,   0,   0,   0,   1,   0,
	  0,   0, 164,   0,   0,   0,
	  5,   0,   0,   0,   6,   0,
	  0,   0,   1,   0,   0,   0,
	  4,   0,   0,   0,   1,   0,
	  0,   0,   1,   0,   0,   0,
	  1,   0,   0,   0, 172,   0,
	  0,   0,   9,   0,   0,   0,
	  6,   0,   0,   0,   1,   0,
	  0,   0,  16,   0,   0,   0,
	  0,   0,   0,   0,   1,   0,
	  0,   0,   1,   0,   0,   0,
	 66, 117, 102, 102, 101, 114,
	 48,   0,  66, 117, 102, 102,
	101, 114,  49,   0,  66, 117,
	102, 102, 101, 114,  79, 117,
	116,   0, 171, 171, 156,   0,
	  0,   0,   1,   0,   0,   0,
	  0,   1,   0,   0,  16,   0,
	  0,   0,   0,   0,   0,   0,
	  3,   0,   0,   0, 164,   0,
	  0,   0,   1,   0,   0,   0,
	240,   1,   0,   0,   4,   0,
	  0,   0,   0,   0,   0,   0,
	  3,   0,   0,   0, 172,   0,
	  0,   0,   1,   0,   0,   0,
	 24,   2,   0,   0,  16,   0,
	  0,   0,   0,   0,   0,   0,
	  3,   0,   0,   0,  40,   1,
	  0,   0,   0,   0,   0,   0,
	 16,   0,   0,   0,   2,   0,
	  0,   0, 204,   1,   0,   0,
	  0,   0,   0,   0, 255, 255,
	255, 255,   0,   0,   0,   0,
	255, 255, 255, 255,   0,   0,
	  0,   0,  36,  69, 108, 101,
	109, 101, 110, 116,   0,  66,
	117, 102,  84, 121, 112, 101,
	  0, 120,   0, 102, 108, 111,
	 97, 116,   0, 171, 171, 171,
	  0,   0,   3,   0,   1,   0,
	  1,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,  59,   1,   0,   0,
	121,   0, 122,   0,  99, 111,
	108, 111, 114,   0, 105, 110,
	116,   0, 171, 171,   0,   0,
	  2,   0,   1,   0,   1,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	114,   1,   0,   0,  57,   1,
	  0,   0,  68,   1,   0,   0,
	  0,   0,   0,   0, 104,   1,
	  0,   0,  68,   1,   0,   0,
	  4,   0,   0,   0, 106,   1,
	  0,   0,  68,   1,   0,   0,
	  8,   0,   0,   0, 108,   1,
	  0,   0, 120,   1,   0,   0,
	 12,   0,   0,   0,   5,   0,
	  0,   0,   1,   0,   4,   0,
	  0,   0,   4,   0, 156,   1,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	 49,   1,   0,   0,  40,   1,
	  0,   0,   0,   0,   0,   0,
	  4,   0,   0,   0,   2,   0,
	  0,   0,  68,   1,   0,   0,
	  0,   0,   0,   0, 255, 255,
	255, 255,   0,   0,   0,   0,
	255, 255, 255, 255,   0,   0,
	  0,   0,  40,   1,   0,   0,
	  0,   0,   0,   0,  16,   0,
	  0,   0,   2,   0,   0,   0,
	204,   1,   0,   0,   0,   0,
	  0,   0, 255, 255, 255, 255,
	  0,   0,   0,   0, 255, 255,
	255, 255,   0,   0,   0,   0,
	 77, 105,  99, 114, 111, 115,
	111, 102, 116,  32,  40,  82,
	 41,  32,  72,  76,  83,  76,
	 32,  83, 104,  97, 100, 101,
	114,  32,  67, 111, 109, 112,
	105, 108, 101, 114,  32,  57,
	 46,  50,  57,  46,  57,  53,
	 50,  46,  51,  49,  49,  49,
	  0, 171, 171, 171,  73,  83,
	 71,  78,   8,   0,   0,   0,
	  0,   0,   0,   0,   8,   0,
	  0,   0,  79,  83,  71,  78,
	  8,   0,   0,   0,   0,   0,
	  0,   0,   8,   0,   0,   0,
	 83,  72,  69,  88,  20,   2,
	  0,   0,  80,   0,   5,   0,
	133,   0,   0,   0, 106,   8,
	  0,   1, 162,   0,   0,   4,
	  0, 112,  16,   0,   0,   0,
	  0,   0,  16,   0,   0,   0,
	162,   0,   0,   4,   0, 112,
	 16,   0,   1,   0,   0,   0,
	  4,   0,   0,   0, 158,   0,
	  0,   4,   0, 224,  17,   0,
	  0,   0,   0,   0,  16,   0,
	  0,   0,  95,   0,   0,   2,
	 18,   0,   2,   0, 104,   0,
	  0,   2,   2,   0,   0,   0,
	155,   0,   0,   4,  64,   0,
	  0,   0,   1,   0,   0,   0,
	  1,   0,   0,   0, 167,   0,
	  0, 138,   2, 131,   0, 128,
	131, 153,  25,   0, 114,   0,
	 16,   0,   0,   0,   0,   0,
	 10,   0,   2,   0,   1,  64,
	  0,   0,   0,   0,   0,   0,
	 70, 114,  16,   0,   0,   0,
	  0,   0, 167,   0,   0, 139,
	  2,  35,   0, 128, 131, 153,
	 25,   0,  18,   0,  16,   0,
	  1,   0,   0,   0,   1,  64,
	  0,   0,   0,   0,   0,   0,
	  1,  64,   0,   0,   0,   0,
	  0,   0,   6, 112,  16,   0,
	  1,   0,   0,   0, 167,   0,
	  0, 139,   2,  35,   0, 128,
	131, 153,  25,   0,  34,   0,
	 16,   0,   1,   0,   0,   0,
	  1,  64,   0,   0,   1,   0,
	  0,   0,   1,  64,   0,   0,
	  0,   0,   0,   0,   6, 112,
	 16,   0,   1,   0,   0,   0,
	167,   0,   0, 139,   2,  35,
	  0, 128, 131, 153,  25,   0,
	 66,   0,  16,   0,   1,   0,
	  0,   0,   1,  64,   0,   0,
	  2,   0,   0,   0,   1,  64,
	  0,   0,   0,   0,   0,   0,
	  6, 112,  16,   0,   1,   0,
	  0,   0,   0,   0,   0,   8,
	114,   0,  16,   0,   0,   0,
	  0,   0,  70,   2,  16,   0,
	  0,   0,   0,   0,  70,   2,
	 16, 128,  65,   0,   0,   0,
	  1,   0,   0,   0,  16,   0,
	  0,   7,  18,   0,  16,   0,
	  0,   0,   0,   0,  70,   2,
	 16,   0,   0,   0,   0,   0,
	 70,   2,  16,   0,   0,   0,
	  0,   0,  75,   0,   0,   5,
	 18,   0,  16,   0,   0,   0,
	  0,   0,  10,   0,  16,   0,
	  0,   0,   0,   0, 167,   0,
	  0, 139,   2,  35,   0, 128,
	131, 153,  25,   0,  34,   0,
	 16,   0,   0,   0,   0,   0,
	  1,  64,   0,   0,   3,   0,
	  0,   0,   1,  64,   0,   0,
	  0,   0,   0,   0,   6, 112,
	 16,   0,   1,   0,   0,   0,
	 49,   0,   0,   7,  18,   0,
	 16,   0,   0,   0,   0,   0,
	 26,   0,  16,   0,   0,   0,
	  0,   0,  10,   0,  16,   0,
	  0,   0,   0,   0,  31,   0,
	  4,   3,  10,   0,  16,   0,
	  0,   0,   0,   0, 167,   0,
	  0, 138,   2, 131,   0, 128,
	131, 153,  25,   0, 242,   0,
	 16,   0,   0,   0,   0,   0,
	 10,   0,   2,   0,   1,  64,
	  0,   0,   0,   0,   0,   0,
	 70, 126,  16,   0,   0,   0,
	  0,   0, 178,   0,   0,   5,
	 18,   0,  16,   0,   1,   0,
	  0,   0,   0, 224,  17,   0,
	  0,   0,   0,   0, 168,   0,
	  0,   9, 242, 224,  17,   0,
	  0,   0,   0,   0,  10,   0,
	 16,   0,   1,   0,   0,   0,
	  1,  64,   0,   0,   0,   0,
	  0,   0,  70,  14,  16,   0,
	  0,   0,   0,   0,  21,   0,
	  0,   1,  62,   0,   0,   1,
	 83,  84,  65,  84, 148,   0,
	  0,   0,  15,   0,   0,   0,
	  2,   0,   0,   0,   0,   0,
	  0,   0,   1,   0,   0,   0,
	  4,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  1,   0,   0,   0,   1,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  2,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0
};

static bool DLLWasLoadedCorrectly = false;
static std::string resultString;

std::vector<pointCloud*> pointClouds;
void runMyComputeShader();

extern "C" bool UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API IsLastAsyncLoadFinished()
{
	return LoadManager::getInstance().isLoadingDone();
}

extern "C" bool UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API OpenLAZFileFromUnity(char* filePath)
{
	if (strlen(filePath) < 4)
		return false;

	// if file is in our own format we will not need dll functionality
	if (filePath[strlen(filePath) - 4] != '.' && filePath[strlen(filePath) - 3] != 'c' && filePath[strlen(filePath) - 2] != 'p' && filePath[strlen(filePath) - 1] != 'c')
	{
		if (!DLLWasLoadedCorrectly)
		{
			LOG.addToLog("Call of OpenLAZFileFromUnity can't be executed because DLL was not loaded correctly", "ERRORS");
			return false;
		}
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
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API OnSceneStartFromUnity(char* projectFilePath)
{
	// Call for thread initialization.
	LoadManager::getInstance();

	UNDO_MANAGER.clear();

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

	pointCloud::LODSettings.resize(4);
	pointCloud::LODSettings[0].targetPercentOFPoints = 50.0f;
	pointCloud::LODSettings[0].maxDistance = 1000.0f;
	pointCloud::LODSettings[1].targetPercentOFPoints = 33.0f;
	pointCloud::LODSettings[1].maxDistance = 3500.0f;
	pointCloud::LODSettings[2].targetPercentOFPoints = 25.0f;
	pointCloud::LODSettings[2].maxDistance = 5000.0f;
	pointCloud::LODSettings[3].targetPercentOFPoints = 12.5f;
	pointCloud::LODSettings[3].maxDistance = 21000.0f;

	for (size_t i = 0; i < 4; i++)
	{
		pointCloud::LODSettings[i].takeEach_Nth_Point = (int)(100.0f / pointCloud::LODSettings[i].targetPercentOFPoints);
	}
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
	//LOG.addToLog("flag 1", "writeTest");

	int pointsToWrite = 0;
	for (size_t j = 0; j < pointClouds[pointCloudIndex]->getPointCount(); j++)
	{
		if (pointClouds[pointCloudIndex]->vertexInfo[j].position[0] != DELETED_POINTS_COORDINATE &&
			pointClouds[pointCloudIndex]->vertexInfo[j].position[1] != DELETED_POINTS_COORDINATE &&
			pointClouds[pointCloudIndex]->vertexInfo[j].position[2] != DELETED_POINTS_COORDINATE)
			pointsToWrite++;
	}
	//LOG.addToLog("flag 2", "writeTest");

	pointClouds[pointCloudIndex]->loadedFrom->header.number_of_point_records = pointsToWrite;
	if (laszip_set_header(laszip_writer, &pointClouds[pointCloudIndex]->loadedFrom->header))
	{
		LOG.addToLog("setting header for laszip writer failed", "DLL_ERRORS");
	}
	//LOG.addToLog("flag 3", "writeTest");

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

	//LOG.addToLog("flag 4", "writeTest");
	for (size_t j = 0; j < pointClouds[pointCloudIndex]->getPointCount(); j++)
	{
		//LOG.addToLog("iteration of (size_t j = 0; j < pointClouds[pointCloudIndex]->getPointCount(); j++)", "TEST");
		if (pointClouds[pointCloudIndex]->vertexInfo[j].position[0] != DELETED_POINTS_COORDINATE &&
			pointClouds[pointCloudIndex]->vertexInfo[j].position[1] != DELETED_POINTS_COORDINATE &&
			pointClouds[pointCloudIndex]->vertexInfo[j].position[2] != DELETED_POINTS_COORDINATE)
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

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API SaveToOwnFormatFileFromUnity(char* filePath, int pointCloudIndex)
{
	if (pointCloudIndex >= pointClouds.size() || pointCloudIndex < 0)
		return;

	int pointsToWrite = 0;
	// Count how many points left.
	for (size_t j = 0; j < pointClouds[pointCloudIndex]->getPointCount(); j++)
	{
		if (pointClouds[pointCloudIndex]->vertexInfo[j].position[0] != DELETED_POINTS_COORDINATE &&
			pointClouds[pointCloudIndex]->vertexInfo[j].position[1] != DELETED_POINTS_COORDINATE &&
			pointClouds[pointCloudIndex]->vertexInfo[j].position[2] != DELETED_POINTS_COORDINATE)
			pointsToWrite++;
	}

	std::string fileName = filePath;
	std::fstream file;
	file.open(fileName, std::ios::out | std::ios::binary);

	// Write number of points.
	file.write((char*)&pointsToWrite, sizeof(int));

	// Write initialXShift and initialZShift.
	file.write((char*)&pointClouds[pointCloudIndex]->initialXShift, sizeof(double));
	file.write((char*)&pointClouds[pointCloudIndex]->initialZShift, sizeof(double));

	// Write adjustment.
	file.write((char*)&pointClouds[pointCloudIndex]->adjustment[0], sizeof(float));
	file.write((char*)&pointClouds[pointCloudIndex]->adjustment[1], sizeof(float));
	file.write((char*)&pointClouds[pointCloudIndex]->adjustment[2], sizeof(float));

	// Write min and max.
	file.write((char*)&pointClouds[pointCloudIndex]->min[0], sizeof(float));
	file.write((char*)&pointClouds[pointCloudIndex]->min[1], sizeof(float));
	file.write((char*)&pointClouds[pointCloudIndex]->min[2], sizeof(float));

	file.write((char*)&pointClouds[pointCloudIndex]->max[0], sizeof(float));
	file.write((char*)&pointClouds[pointCloudIndex]->max[1], sizeof(float));
	file.write((char*)&pointClouds[pointCloudIndex]->max[2], sizeof(float));

	// Write root node size.
	//file.write((char*)&pointClouds[pointCloudIndex]->getSearchOctree()->root->size, sizeof(float));

	for (size_t j = 0; j < pointClouds[pointCloudIndex]->getPointCount(); j++)
	{
		// Write point only if it is not "deleted".
		if (pointClouds[pointCloudIndex]->vertexInfo[j].position[0] != DELETED_POINTS_COORDINATE &&
			pointClouds[pointCloudIndex]->vertexInfo[j].position[1] != DELETED_POINTS_COORDINATE &&
			pointClouds[pointCloudIndex]->vertexInfo[j].position[2] != DELETED_POINTS_COORDINATE)
		{
			// Write position of point.
			file.write((char*)&pointClouds[pointCloudIndex]->vertexInfo[j].position[0], sizeof(float));
			file.write((char*)&pointClouds[pointCloudIndex]->vertexInfo[j].position[1], sizeof(float));
			file.write((char*)&pointClouds[pointCloudIndex]->vertexInfo[j].position[2], sizeof(float));

			// Write color of point.
			file.write((char*)&pointClouds[pointCloudIndex]->vertexInfo[j].color[0], sizeof(byte));
			file.write((char*)&pointClouds[pointCloudIndex]->vertexInfo[j].color[1], sizeof(byte));
			file.write((char*)&pointClouds[pointCloudIndex]->vertexInfo[j].color[2], sizeof(byte));
			file.write((char*)&pointClouds[pointCloudIndex]->vertexInfo[j].color[3], sizeof(byte));
		}
	}

	file.close();
}

static bool deletionOccuredThisFrame = false;
extern "C" bool UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API RequestToDeleteFromUnity(float* center, float size)
{
#ifdef USE_COMPUTE_SHADER
	deletionSpherePosition = glm::vec3(center[0], center[1], center[2]);
	deletionSphereSize = size / 2.0f;
	deletionOccuredThisFrame = true;

	runMyDeleteComputeShader();
#else
	//DWORD time = GetTickCount();

	bool anyPointWasDeleted = false;
	glm::vec3 centerOfBrush = glm::vec3(center[0], center[1], center[2]);

	for (size_t i = 0; i < pointClouds.size(); i++)
	{
		if (!pointClouds[i]->wasFullyLoaded)
			continue;

		glm::vec3 localPosition = glm::inverse(glm::transpose(pointClouds[i]->worldMatrix)) * glm::vec4(centerOfBrush, 1.0f);
		float extractedScale = glm::length(glm::transpose(pointClouds[i]->worldMatrix)[0]);
		size /= extractedScale;

		if (pointClouds[i]->getSearchOctree()->isInOctreeBound(localPosition, size))
		{
			//UNDO_MANAGER.setPointCloud(pointClouds[i]);
			//UNDO_MANAGER.addDeleteAction(localPosition, size);
			pointClouds[i]->getSearchOctree()->deleteObjects(localPosition, size);
		}

		if (pointClouds[i]->getSearchOctree()->pointsToDelete.size() > 0)
		{
			UNDO_MANAGER.setPointCloud(pointClouds[i]);
			UNDO_MANAGER.addDeleteAction(localPosition, size);

			LOG.addToLog("==============================================================", "deleteEvents");
			LOG.addToLog("Brush location: ", localPosition, "deleteEvents");
			LOG.addToLog("Brush size: " + std::to_string(size), "deleteEvents");
			LOG.addToLog("pointsToDelete size: " + std::to_string(pointClouds[i]->getSearchOctree()->pointsToDelete.size()), "deleteEvents");

			anyPointWasDeleted = true;
		}
	}

	return anyPointWasDeleted;
	/*DWORD timeSpent = GetTickCount() - time;
	std::fstream testFile;
	std::string text = std::to_string(timeSpent);
	testFile.open("timeDelete.txt", std::ios::out);
	testFile.write("total time: ", strlen("total time: "));
	testFile.write(text.c_str(), text.size());
	testFile.write("\n", strlen("\n"));
	testFile.close();*/
#endif // USE_COMPUTE_SHADER
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
	GPU.getDevice()->CreateBuffer(&desc, NULL, &m_CB);

	// shaders
	ID3DBlob* pVSBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	HRESULT hr;
	// C:/Users/kberegovyi/Downloads/ARNav2_compute_08.16.2021/Assets/Plugins/PointCloudPlugin/
	//hr = D3DCompileFromFile(L"C:/Users/kandr/OneDrive/University/ocean_lab/PointCloudPlugin/shaders/VS.hlsl", nullptr,
	//						D3D_COMPILE_STANDARD_FILE_INCLUDE,
	//						"VS", "vs_5_0",
	//						D3DCOMPILE_ENABLE_STRICTNESS, 0, &pVSBlob, &errorBlob);

	//LOG.addToLog("shaderResult: " + std::system_category().message(hr), "computeShader");
	//if (errorBlob)
	//{
	//	LOG.addToLog("shaderResult: " + std::string((char*)errorBlob->GetBufferPointer()), "computeShader");
	//	//OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	//	errorBlob->Release();
	//}

	hr = GPU.getDevice()->CreateVertexShader(kVertexShaderCode, sizeof(kVertexShaderCode), nullptr, &m_VertexShader);
	//hr = GPU.getDevice()->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_VertexShader);

	if (FAILED(hr))
		LOG.addToLog("Failed to create vertex shader.", "computeShader");
	hr = GPU.getDevice()->CreatePixelShader(kPixelShaderCode, sizeof(kPixelShaderCode), nullptr, &m_PixelShader);
	if (FAILED(hr))
		LOG.addToLog("Failed to create pixel shader.", "computeShader");

	// input layout
	if (m_VertexShader)
	{
		D3D11_INPUT_ELEMENT_DESC s_DX11InputElementDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		GPU.getDevice()->CreateInputLayout(s_DX11InputElementDesc, 2, kVertexShaderCode, sizeof(kVertexShaderCode), &m_InputLayout);
		//GPU.getDevice()->CreateInputLayout(s_DX11InputElementDesc, 2, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_InputLayout);
	}

	// render states
	D3D11_RASTERIZER_DESC rsdesc;
	memset(&rsdesc, 0, sizeof(rsdesc));
	rsdesc.FillMode = D3D11_FILL_SOLID;
	rsdesc.CullMode = D3D11_CULL_NONE;
	rsdesc.DepthClipEnable = TRUE;
	GPU.getDevice()->CreateRasterizerState(&rsdesc, &m_RasterState);

	D3D11_DEPTH_STENCIL_DESC dsdesc;
	memset(&dsdesc, 0, sizeof(dsdesc));
	dsdesc.DepthEnable = TRUE;
	dsdesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsdesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
	GPU.getDevice()->CreateDepthStencilState(&dsdesc, &m_DepthState);

	D3D11_BLEND_DESC bdesc;
	memset(&bdesc, 0, sizeof(bdesc));
	bdesc.RenderTarget[0].BlendEnable = FALSE;
	bdesc.RenderTarget[0].RenderTargetWriteMask = 0xF;
	GPU.getDevice()->CreateBlendState(&bdesc, &m_BlendState);

	// Compute shader.
	// C:/Users/kberegovyi/Downloads/ARNav2_compute_08.16.2021/Assets/Plugins/PointCloudPlugin/computeShader.hlsl
	// C:/Users/kandr/OneDrive/University/ocean_lab/PointCloudPlugin/shaders/computeShader.hlsl

	//"C:/Users/kberegovyi/Downloads/ARNav2_compute_08.16.2021/Assets/Plugins/PointCloudPlugin/computeShader_DELETE.hlsl"
	
	compileAndCreateComputeShader(GPU.getDevice(), (BYTE*)g_CSMain, &computeShader);

	outliers_CS = new CShader("C:/Users/kandr/OneDrive/University/ocean_lab/PointCloudPlugin/shaders/computeShader_Outliers.hlsl");
	//compileAndCreateComputeShader(GPU.getDevice(), "C:/Users/kberegovyi/Downloads/ARNav2_compute_08.16.2021/Assets/Plugins/PointCloudPlugin/computeShader_Outliers.hlsl", &outliers_CS);
}

const int kVertexSize = 12 + 4;
void onDrawDeletePointsinGPUMem(pointCloud* pointCloud, ID3D11DeviceContext* ctx, bool HighlightDeletedPoints)
{
	std::vector<int> pointsToHighlight;

	if (HighlightDeletedPoints)
	{
		glm::vec3 localDeletionSpherePosition = glm::inverse(glm::transpose(pointCloud->worldMatrix)) * glm::vec4(deletionSpherePosition, 1.0f);
		if (pointCloud->getSearchOctree()->isInOctreeBound(localDeletionSpherePosition, deletionSphereSize))
		{
			pointCloud->getSearchOctree()->searchForObjects(localDeletionSpherePosition, deletionSphereSize, pointsToHighlight);
		}
	}

	int minIndex = INT_MAX;
	int maxIndex = INT_MIN;

	// In case user switched off highlighting of points we need to clean up color data.
	if (!highlightDeletedPoints && pointCloud->lastHighlightedPoints.size() != 0)
	{
		//LOG.addToLog("pointCloud->lastHighlightedPoints.size(): " + std::to_string(pointCloud->lastHighlightedPoints.size()), "highlightCleanUp");

		minIndex = 0;
		maxIndex = int(pointCloud->vertexInfo.size());

		for (size_t i = 0; i < pointCloud->vertexInfo.size(); i++)
		{
			pointCloud->vertexInfo[i].color[0] = pointCloud->originalData[i].color[0];
			pointCloud->vertexInfo[i].color[1] = pointCloud->originalData[i].color[1];
			pointCloud->vertexInfo[i].color[2] = pointCloud->originalData[i].color[2];
		}

		//LOG.addToLog("pointCloud->pointsOriginalColor[0].r: " + std::to_string(pointCloud->pointsOriginalColor[0].r), "highlightCleanUp");
		//LOG.addToLog("pointCloud->pointsOriginalColor[0].g: " + std::to_string(pointCloud->pointsOriginalColor[0].g), "highlightCleanUp");
		//LOG.addToLog("pointCloud->pointsOriginalColor[0].b: " + std::to_string(pointCloud->pointsOriginalColor[0].b), "highlightCleanUp");

		//LOG.addToLog("maxIndex: " + std::to_string(maxIndex), "highlightCleanUp");
		//LOG.addToLog("minIndex: " + std::to_string(minIndex), "highlightCleanUp");

		pointCloud->lastHighlightedPoints.clear();
	}

	if (pointCloud->getSearchOctree()->pointsToDelete.size() != 0 || HighlightDeletedPoints || (minIndex != INT_MAX && maxIndex != INT_MIN))
	{
		pointCloud->highlightStep += 10;
		if (pointCloud->highlightStep > 100)
			pointCloud->highlightStep = 0;

		D3D11_BOX box{};
		box.left = 0;
		box.right = 0 + pointCloud->getPointCount() * kVertexSize;
		box.top = 0;
		box.bottom = 1;
		box.front = 0;
		box.back = 1;

		if (pointCloud->getSearchOctree()->pointsToDelete.size() != 0)
			LOG.addToLog("DrawPointCloud with pointsToDelete first element: " + std::to_string(pointCloud->getSearchOctree()->pointsToDelete[0]), "deleteEvents");

		for (size_t i = 0; i < pointCloud->getSearchOctree()->pointsToDelete.size(); i++)
		{
			if (pointCloud->getSearchOctree()->pointsToDelete[i] > maxIndex)
				maxIndex = pointCloud->getSearchOctree()->pointsToDelete[i];

			if (pointCloud->getSearchOctree()->pointsToDelete[i] < minIndex)
				minIndex = pointCloud->getSearchOctree()->pointsToDelete[i];

			pointCloud->vertexInfo[pointCloud->getSearchOctree()->pointsToDelete[i]].position[0] = DELETED_POINTS_COORDINATE;
			pointCloud->vertexInfo[pointCloud->getSearchOctree()->pointsToDelete[i]].position[1] = DELETED_POINTS_COORDINATE;
			pointCloud->vertexInfo[pointCloud->getSearchOctree()->pointsToDelete[i]].position[2] = DELETED_POINTS_COORDINATE;
		}
		pointCloud->getSearchOctree()->pointsToDelete.clear();

		LOG.addToLog("maxIndex: " + std::to_string(maxIndex), "deleteEvents");
		LOG.addToLog("minIndex: " + std::to_string(minIndex), "deleteEvents");

		for (size_t i = 0; i < pointCloud->lastHighlightedPoints.size(); i++)
		{
			if (pointCloud->lastHighlightedPoints[i] > maxIndex)
				maxIndex = pointCloud->lastHighlightedPoints[i];

			if (pointCloud->lastHighlightedPoints[i] < minIndex)
				minIndex = pointCloud->lastHighlightedPoints[i];

			pointCloud->vertexInfo[pointCloud->lastHighlightedPoints[i]].color[0] = pointCloud->originalData[pointCloud->lastHighlightedPoints[i]].color[0];
			pointCloud->vertexInfo[pointCloud->lastHighlightedPoints[i]].color[1] = pointCloud->originalData[pointCloud->lastHighlightedPoints[i]].color[1];
			pointCloud->vertexInfo[pointCloud->lastHighlightedPoints[i]].color[2] = pointCloud->originalData[pointCloud->lastHighlightedPoints[i]].color[2];
		}
		pointCloud->lastHighlightedPoints = pointsToHighlight;

		for (size_t i = 0; i < pointsToHighlight.size(); i++)
		{
			if (pointsToHighlight[i] > maxIndex)
				maxIndex = pointsToHighlight[i];

			if (pointsToHighlight[i] < minIndex)
				minIndex = pointsToHighlight[i];

			//byte rColor = pointCloud->pointsOriginalColor[pointsToHighlight[i]].r;
			//byte gColor = pointCloud->pointsOriginalColor[pointsToHighlight[i]].g;
			//byte bColor = pointCloud->pointsOriginalColor[pointsToHighlight[i]].b;

			//float highlightDelta = 0.033f / 0.250f; /*Time.deltaTime / BLINK_RATE_SECONDS*/
			//static float highlightRatio = 0.0f;
			//highlightRatio = fmodf(highlightRatio + highlightDelta, 1.0f);

			//glm::vec3 newPointColor = (1.0f / glm::vec3(rColor / 255.0f, gColor / 255.0f, bColor / 255.0f)) * highlightRatio;

			//rColor = newPointColor.x * 255;
			//gColor = newPointColor.y * 255;
			//bColor = newPointColor.z * 255;

			///*debugLog::getInstance().addToLog("highlightDelta: " + std::to_string(highlightDelta), "colorData");
			//debugLog::getInstance().addToLog("highlightRatio: " + std::to_string(highlightRatio), "colorData");
			//debugLog::getInstance().addToLog("newPointColor: ", newPointColor, "colorData");
			//debugLog::getInstance().addToLog("rColor " + std::to_string(rColor), "colorData");
			//debugLog::getInstance().addToLog("gColor " + std::to_string(gColor), "colorData");
			//debugLog::getInstance().addToLog("bColor " + std::to_string(bColor), "colorData");*/

			//pointCloud->vertexInfo[pointsToHighlight[i]].color[0] = rColor;
			//pointCloud->vertexInfo[pointsToHighlight[i]].color[1] = gColor;
			//pointCloud->vertexInfo[pointsToHighlight[i]].color[2] = bColor;

			byte originalR = pointCloud->originalData[pointsToHighlight[i]].color[0];
			byte originalG = pointCloud->originalData[pointsToHighlight[i]].color[1];
			byte originalB = pointCloud->originalData[pointsToHighlight[i]].color[2];

			byte invertedR = 255 - originalR;
			byte invertedG = 255 - originalG;
			byte invertedB = 255 - originalB;

			pointCloud->vertexInfo[pointsToHighlight[i]].color[0] = originalR + byte((invertedR - originalR) * (pointCloud->highlightStep / 100.0f));
			pointCloud->vertexInfo[pointsToHighlight[i]].color[1] = originalG + byte((invertedG - originalG) * (pointCloud->highlightStep / 100.0f));
			pointCloud->vertexInfo[pointsToHighlight[i]].color[2] = originalB + byte((invertedB - originalB) * (pointCloud->highlightStep / 100.0f));
		}

		if (minIndex == INT_MAX || maxIndex == INT_MIN)
			return;

		//LOG.addToLog("DrawPointCloud after \"for (size_t i = 0; i < pointCloudToRender->getSearchOctree()->pointsToDelete.size(); i++)\"", "deleteEvents");
		
		box.left = minIndex * kVertexSize;
		box.right = minIndex * kVertexSize + (maxIndex - minIndex + 1) * kVertexSize;

		//LOG.addToLog("DrawPointCloud, vertex count: " + std::to_string(pointCloud->vertexInfo.size()), "deleteEvents");

		D3D11_BOX dbox{};
		dbox.left = 0;
		dbox.right = (maxIndex - minIndex + 1) * kVertexSize;
		dbox.top = 0;
		dbox.bottom = 1;
		dbox.front = 0;
		dbox.back = 1;

		/*LOG.addToLog("min: " + std::to_string(minIndex), "deleteEvents");
		LOG.addToLog("dbox.right: " + std::to_string(dbox.right), "deleteEvents");

		LOG.addToLog("box.left: " + std::to_string(box.left), "deleteEvents");
		LOG.addToLog("box.right: " + std::to_string(box.right), "deleteEvents");*/

		if (box.right / kVertexSize > pointCloud->vertexInfo.size())
		{
			LOG.addToLog("Error ! box.right / kVertexSize > pointCloudToRender->vertexInfo.size()", "deleteEvents");
			return;
		}

		ctx->UpdateSubresource(pointCloud->intermediateVB, 0, &dbox, pointCloud->vertexInfo.data() + minIndex, pointCloud->getPointCount() * kVertexSize, pointCloud->getPointCount() * kVertexSize);
		ctx->CopySubresourceRegion(pointCloud->mainVB, 0, box.left, 0, 0, pointCloud->intermediateVB, 0, &dbox);
	}
}

static void DrawPointCloud(pointCloud* pointCloudToRender, bool HighlightDeletedPoints)
{
	if (!pointCloudToRender->wasFullyLoaded)
		return;

	//LOG.addToLog("DrawPointCloud()_START: " + std::system_category().message(GPU.getDevice()->GetDeviceRemovedReason()), "computeShader");

	static int pointsToDraw = 0;
#ifdef USE_COMPUTE_SHADER
	//getComputeShaderResultCounter(GPU.getDevice(), *current_CS_UAV/*result_CS_UAV*/);
	//runMyComputeShader();

	if (inputPoints_CS_SRV == nullptr)
	{
		deletionSpherePosition = glm::vec3(0.0f);
		deletionSphereSize = 0.0f;
		runMyDeleteComputeShader();
		LOG.addToLog("allPointsData_CS_SRV == nullptr updated", "computeShader");
		if (current_CS_UAV == nullptr)
			LOG.addToLog("current_CS_UAV == nullptr", "computeShader");
		pointsToDraw = getComputeShaderResultCounter(GPU.getDevice(), *current_CS_UAV/*result_CS_UAV*/);
		LOG.addToLog("getComputeShaderResultCounter(GPU.getDevice(), *current_CS_UAV/*result_CS_UAV*/);", "computeShader");
		currentInputPoints_CS_SRV = &result_CS_SRV;
		current_CS_UAV = &resultSecond_CS_UAV;
	}
#endif

	if (pointCloudToRender->getSearchOctree()->pointsToDelete.size() != 0)
	{
		LOG.addToLog("DrawPointCloud begin with pointsToDelete size: " + std::to_string(pointCloudToRender->getSearchOctree()->pointsToDelete.size()), "deleteEvents");
	}

	glm::mat4 glmWorldMatrix = pointCloudToRender->worldMatrix;
	glm::mat4 glmViewMatrix = glm::make_mat4(worldToViewMatrix);
	glm::mat4 glmProjectionMatrix = glm::make_mat4(projectionMatrix);

	glmWorldMatrix = glm::transpose(glmWorldMatrix);
	glmViewMatrix = glm::transpose(glmViewMatrix);
	glmProjectionMatrix = glm::transpose(glmProjectionMatrix);

	// WHY ??))
	//if (frustumCulling)
	//{
		//float distance;
		//for (int p = 0; p < 6; p++)
		//{
		//	distance = frustum[p][0] * glmWorldMatrix[3][0] + frustum[p][1] * glmWorldMatrix[3][1] + frustum[p][2] * glmWorldMatrix[3][2] + frustum[p][3];
		//	if (distance <= -pointCloudToRender->getSearchOctree()->root->nodeAABB.size)
		//	{
		//		/*LOG.addToLog("distance: " + std::to_string(distance), "renderTest");
		//		LOG.addToLog("nodeAABB.size: " + std::to_string(pointCloudToRender->getSearchOctree()->root->nodeAABB.size), "renderTest");

		//		LOG.addToLog("glmWorldMatrix[3][0]: " + std::to_string(glmWorldMatrix[3][0]), "renderTest");
		//		LOG.addToLog("glmWorldMatrix[3][1]: " + std::to_string(glmWorldMatrix[3][1]), "renderTest");
		//		LOG.addToLog("glmWorldMatrix[3][2]: " + std::to_string(glmWorldMatrix[3][2]), "renderTest");*/
		//		return;
		//	}
		//}
	//}

	ID3D11DeviceContext* ctx = NULL;
	GPU.getDevice()->GetImmediateContext(&ctx);

	ctx->OMSetDepthStencilState(m_DepthState, 0);
	ctx->RSSetState(m_RasterState);
	ctx->OMSetBlendState(m_BlendState, NULL, 0xFFFFFFFF);

	glm::mat4 finalMatrix = glmProjectionMatrix * glmViewMatrix * glmWorldMatrix;
	ctx->UpdateSubresource(m_CB, 0, NULL, glm::value_ptr(finalMatrix), 64, 0);

	// Set shaders
	ctx->VSSetConstantBuffers(0, 1, &m_CB);
	ctx->VSSetShader(m_VertexShader, NULL, 0);
	ctx->PSSetShader(m_PixelShader, NULL, 0);

	if (!pointCloudToRender->wasInitialized)
	{
		D3D11_BUFFER_DESC desc;
		memset(&desc, 0, sizeof(desc));

		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = UINT(pointCloudToRender->vertexInfo.size() * 16);
#ifdef USE_COMPUTE_SHADER
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
#else
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
#endif
		GPU.getDevice()->CreateBuffer(&desc, NULL, &pointCloudToRender->mainVB);

		/*descBuffer.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		descBuffer.ByteWidth = sizeof(MeshVertex) * NUM_ELEMENTS;
		descBuffer.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		descBuffer.StructureByteStride = sizeof(MeshVertex);*/

		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = UINT(pointCloudToRender->vertexInfo.size() * 16);
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		GPU.getDevice()->CreateBuffer(&desc, NULL, &pointCloudToRender->intermediateVB);

		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		for (size_t i = 0; i < pointCloudToRender->LODs.size(); i++)
		{
			desc.ByteWidth = UINT(pointCloudToRender->LODs[i].vertexInfo.size() * 16);
			GPU.getDevice()->CreateBuffer(&desc, NULL, &pointCloudToRender->LODs[i].VB);
		}

		ctx->UpdateSubresource(pointCloudToRender->mainVB, 0, nullptr, pointCloudToRender->vertexInfo.data(), pointCloudToRender->getPointCount() * kVertexSize, pointCloudToRender->getPointCount() * kVertexSize);
		LOG.addToLog("copy data to vertex buffer, vertexInfo size: " + std::to_string(pointCloudToRender->vertexInfo.size()), "OctreeEvents");

		for (size_t i = 0; i < pointCloudToRender->LODs.size(); i++)
		{
			ctx->UpdateSubresource(pointCloudToRender->LODs[i].VB, 0, nullptr, pointCloudToRender->LODs[i].vertexInfo.data(), UINT(pointCloudToRender->LODs[i].vertexInfo.size() * kVertexSize), UINT(pointCloudToRender->LODs[i].vertexInfo.size() * kVertexSize));
		}

		pointCloudToRender->wasInitialized = true;
	}

	onDrawDeletePointsinGPUMem(pointCloudToRender, ctx, HighlightDeletedPoints);

#ifdef USE_COMPUTE_SHADER
	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	ctx->VSSetShaderResources(0, 1, current_CS_SRV/*&result_CS_SRV*/);
	//int pointsToDraw = getComputeShaderResultCounter(GPU.getDevice(), *current_CS_UAV/*result_CS_UAV*/);

	if (deletionOccuredThisFrame)
	{
		pointsToDraw = getComputeShaderResultCounter(GPU.getDevice(), *current_CS_UAV);

		currentInputPoints_CS_SRV = current_CS_SRV == &result_CS_SRV ? &resultSecond_CS_SRV : &result_CS_SRV;

		current_CS_SRV = current_CS_SRV == &result_CS_SRV ? &resultSecond_CS_SRV : &result_CS_SRV;
		current_CS_UAV = current_CS_UAV == &result_CS_UAV ? &resultSecond_CS_UAV : &result_CS_UAV;

		deletionOccuredThisFrame = false;
	}

	LOG.addToLog("pointsToDraw: " + std::to_string(pointsToDraw), "computeShader");

	if (pointsToDraw != 0 && pointsToDraw < pointCloudToRender->getPointCount())
	{
		float distanceToCamera = glm::distance(glm::vec3(glmWorldMatrix[3][0], glmWorldMatrix[3][1], glmWorldMatrix[3][2]), glm::vec3(glmViewMatrix[3][0], glmViewMatrix[3][1], glmViewMatrix[3][2]));
		if (!LODSystemActive)
		{
			ctx->Draw(pointsToDraw, 0);
		}
		else
		{
			for (size_t i = 0; i < pointCloud::LODSettings.size(); i++)
			{
				if (distanceToCamera <= pointCloud::LODSettings[i].maxDistance)
				{
					ctx->Draw(UINT(pointCloudToRender->LODs[i].vertexInfo.size()), 0);
					break;
				}
			}
		}
	}

#else
	ctx->IASetInputLayout(m_InputLayout);
	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	UINT stride = kVertexSize;
	UINT offset = 0;

	float distanceToCamera = glm::distance(glm::vec3(glmWorldMatrix[3][0], glmWorldMatrix[3][1], glmWorldMatrix[3][2]), glm::vec3(glmViewMatrix[3][0], glmViewMatrix[3][1], glmViewMatrix[3][2]));
	if (!LODSystemActive)
	{
		ctx->IASetVertexBuffers(0, 1, &pointCloudToRender->mainVB, &stride, &offset);
		ctx->Draw(pointCloudToRender->getPointCount(), 0);
	}
	else
	{
		for (size_t i = 0; i < pointCloud::LODSettings.size(); i++)
		{
			if (distanceToCamera <= pointCloud::LODSettings[i].maxDistance)
			{
				ctx->IASetVertexBuffers(0, 1, &pointCloudToRender->LODs[i].VB, &stride, &offset);
				ctx->Draw(UINT(pointCloudToRender->LODs[i].vertexInfo.size()), 0);
				break;
			}
		}
	}
#endif

	//LOG.addToLog("distanceToCamera: " + std::to_string(distanceToCamera), "camera");
	ctx->Release();
}

static void ProcessDeviceEvent(UnityGfxDeviceEventType type, IUnityInterfaces* interfaces)
{
	switch (type)
	{
		case kUnityGfxDeviceEventInitialize:
		{
			GPU.initialize(interfaces);
			//IUnityGraphicsD3D11* d3d = interfaces->Get<IUnityGraphicsD3D11>();
			//m_Device = d3d->GetDevice();
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
	bool localHighlightDeletedPoints = false;

	static DWORD timeLastTimeMemoryModification = GetTickCount();
	if (highlightDeletedPoints && GetTickCount() - timeLastTimeMemoryModification > 33)
	{
		timeLastTimeMemoryModification = GetTickCount();
		localHighlightDeletedPoints = true;
	}

	for (size_t i = 0; i < pointClouds.size(); i++)
	{
		DrawPointCloud(pointClouds[i], localHighlightDeletedPoints);
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
		//LOG.addToLog("worldToViewMatrix[" + std::to_string(i) + "]: " + std::to_string(worldToViewMatrix[i]), "camera");
	}

	for (size_t i = 0; i < 16; i++)
	{
		projectionMatrix[i] = ((float*)(cameraProjectionMatrix))[i];
	}

	/*glm::mat4 glmViewMatrix = glm::make_mat4(worldToViewMatrix);
	glmViewMatrix = glm::transpose(glmViewMatrix);
	for (size_t i = 0; i < 4; i++)
	{
		for (size_t j = 0; j < 4; j++)
		{
			LOG.addToLog("glmViewMatrix[" + std::to_string(i) + "][" + std::to_string(j) + "]: " + std::to_string(glmViewMatrix[i][j]), "camera");
		}
	}*/

	//LOG.addToLog("glmWorldMatrix[0][3]: " + std::to_string(glmViewMatrix[0][3]), "camera");
	//LOG.addToLog("glmWorldMatrix[1][3]: " + std::to_string(glmViewMatrix[1][3]), "camera");
	//LOG.addToLog("glmWorldMatrix[2][3]: " + std::to_string(glmViewMatrix[2][3]), "camera");

	updateFrustumPlanes();
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API updateTestCamera(void* cameraWorldMatrix, void* cameraProjectionMatrix)
{
	//float testWorldToViewMatrix[16];
	//for (size_t i = 0; i < 16; i++)
	//{
	//	testWorldToViewMatrix[i] = ((float*)(cameraWorldMatrix))[i];
	//}

	//float testProjectionMatrix[16];
	//for (size_t i = 0; i < 16; i++)
	//{
	//	testProjectionMatrix[i] = ((float*)(cameraProjectionMatrix))[i];
	//}

	//if (testFrustum == nullptr)
	//{
	//	testFrustum = new float* [6];
	//	for (size_t i = 0; i < 6; i++)
	//	{
	//		testFrustum[i] = new float[4];
	//	}
	//}

	//float   clip[16];
	//float   t;

	//glm::mat4 glmViewMatrix = glm::make_mat4(testWorldToViewMatrix);
	//glm::mat4 glmProjectionMatrix = glm::make_mat4(testProjectionMatrix);

	//glmViewMatrix = glm::transpose(glmViewMatrix);
	//glmProjectionMatrix = glm::transpose(glmProjectionMatrix);

	//glm::mat4 cliping = glmProjectionMatrix * glmViewMatrix;
	//// This huge and ambiguous type of i is just to get rid of warning.
	//for (glm::mat<4, 4, float, glm::packed_highp>::length_type i = 0; i < 4; i++)
	//{
	//	clip[i * 4] = cliping[i][0];
	//	clip[i * 4 + 1] = cliping[i][1];
	//	clip[i * 4 + 2] = cliping[i][2];
	//	clip[i * 4 + 3] = cliping[i][3];
	//}

	///* Extract the numbers for the RIGHT plane */
	//testFrustum[0][0] = clip[3] - clip[0];
	//testFrustum[0][1] = clip[7] - clip[4];
	//testFrustum[0][2] = clip[11] - clip[8];
	//testFrustum[0][3] = clip[15] - clip[12];

	///* Normalize the result */
	//t = sqrt(testFrustum[0][0] * testFrustum[0][0] + testFrustum[0][1] * testFrustum[0][1] + testFrustum[0][2] * testFrustum[0][2]);
	//testFrustum[0][0] /= t;
	//testFrustum[0][1] /= t;
	//testFrustum[0][2] /= t;
	//testFrustum[0][3] /= t;

	///* Extract the numbers for the LEFT plane */
	//testFrustum[1][0] = clip[3] + clip[0];
	//testFrustum[1][1] = clip[7] + clip[4];
	//testFrustum[1][2] = clip[11] + clip[8];
	//testFrustum[1][3] = clip[15] + clip[12];

	///* Normalize the result */
	//t = sqrt(testFrustum[1][0] * testFrustum[1][0] + testFrustum[1][1] * testFrustum[1][1] + testFrustum[1][2] * testFrustum[1][2]);
	//testFrustum[1][0] /= t;
	//testFrustum[1][1] /= t;
	//testFrustum[1][2] /= t;
	//testFrustum[1][3] /= t;

	///* Extract the BOTTOM plane */
	//testFrustum[2][0] = clip[3] + clip[1];
	//testFrustum[2][1] = clip[7] + clip[5];
	//testFrustum[2][2] = clip[11] + clip[9];
	//testFrustum[2][3] = clip[15] + clip[13];

	///* Normalize the result */
	//t = sqrt(testFrustum[2][0] * testFrustum[2][0] + testFrustum[2][1] * testFrustum[2][1] + testFrustum[2][2] * testFrustum[2][2]);
	//testFrustum[2][0] /= t;
	//testFrustum[2][1] /= t;
	//testFrustum[2][2] /= t;
	//testFrustum[2][3] /= t;

	///* Extract the TOP plane */
	//testFrustum[3][0] = clip[3] - clip[1];
	//testFrustum[3][1] = clip[7] - clip[5];
	//testFrustum[3][2] = clip[11] - clip[9];
	//testFrustum[3][3] = clip[15] - clip[13];

	///* Normalize the result */
	//t = sqrt(testFrustum[3][0] * testFrustum[3][0] + testFrustum[3][1] * testFrustum[3][1] + testFrustum[3][2] * testFrustum[3][2]);
	//testFrustum[3][0] /= t;
	//testFrustum[3][1] /= t;
	//testFrustum[3][2] /= t;
	//testFrustum[3][3] /= t;

	///* Extract the FAR plane */
	//testFrustum[4][0] = clip[3] - clip[2];
	//testFrustum[4][1] = clip[7] - clip[6];
	//testFrustum[4][2] = clip[11] - clip[10];
	//testFrustum[4][3] = clip[15] - clip[14];

	///* Normalize the result */
	//t = sqrt(testFrustum[4][0] * testFrustum[4][0] + testFrustum[4][1] * testFrustum[4][1] + testFrustum[4][2] * testFrustum[4][2]);
	//testFrustum[4][0] /= t;
	//testFrustum[4][1] /= t;
	//testFrustum[4][2] /= t;
	//testFrustum[4][3] /= t;

	///* Extract the NEAR plane */
	//testFrustum[5][0] = clip[3] + clip[2];
	//testFrustum[5][1] = clip[7] + clip[6];
	//testFrustum[5][2] = clip[11] + clip[10];
	//testFrustum[5][3] = clip[15] + clip[14];

	///* Normalize the result */
	//t = sqrt(testFrustum[5][0] * testFrustum[5][0] + testFrustum[5][1] * testFrustum[5][1] + testFrustum[5][2] * testFrustum[5][2]);
	//testFrustum[5][0] /= t;
	//testFrustum[5][1] /= t;
	//testFrustum[5][2] /= t;
	//testFrustum[5][3] /= t;

	///*glm::mat4 glmViewMatrix = glm::make_mat4(worldToViewMatrix);
	//glmViewMatrix = glm::transpose(glmViewMatrix);
	//for (size_t i = 0; i < 4; i++)
	//{
	//	for (size_t j = 0; j < 4; j++)
	//	{
	//		LOG.addToLog("glmViewMatrix[" + std::to_string(i) + "][" + std::to_string(j) + "]: " + std::to_string(glmViewMatrix[i][j]), "camera");
	//	}
	//}*/

	////LOG.addToLog("glmWorldMatrix[0][3]: " + std::to_string(glmViewMatrix[0][3]), "camera");
	////LOG.addToLog("glmWorldMatrix[1][3]: " + std::to_string(glmViewMatrix[1][3]), "camera");
	////LOG.addToLog("glmWorldMatrix[2][3]: " + std::to_string(glmViewMatrix[2][3]), "camera");
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
	frustumCulling = active;
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API setLODSystemActive(bool active)
{
	LODSystemActive = active;
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API setLODInfo(float* values, int LODIndex, int pointCloudIndex)
{
	if (pointCloudIndex >= pointClouds.size() || pointCloudIndex < 0)
		return;

	if (LODIndex >= pointCloud::LODSettings.size() || LODIndex < 0)
		return;

	pointCloud::LODSettings[LODIndex].maxDistance = ((float*)(values))[0];
	pointCloud::LODSettings[LODIndex].targetPercentOFPoints = ((float*)(values))[1];
	pointCloud::LODSettings[LODIndex].takeEach_Nth_Point = (int)(100.0f / pointCloud::LODSettings[LODIndex].targetPercentOFPoints);
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API RequestLODInfoFromUnity(float* maxDistance, float* targetPercentOFPoints, int LODIndex, int pointCloudIndex)
{
	if (pointCloudIndex >= pointClouds.size() || pointCloudIndex < 0)
		return;

	if (LODIndex >= pointCloud::LODSettings.size() || LODIndex < 0)
		return;

	//LOG.addToLog("pointCloud::LODSettings[LODIndex].maxDistance: " + std::to_string(pointCloud::LODSettings[LODIndex].maxDistance), "TEST");
	//LOG.addToLog("pointCloud::LODSettings[LODIndex].targetPercentOFPoints: " + std::to_string(pointCloud::LODSettings[LODIndex].targetPercentOFPoints), "TEST");

	maxDistance[0] = pointCloud::LODSettings[LODIndex].maxDistance;
	targetPercentOFPoints[0] = pointCloud::LODSettings[LODIndex].targetPercentOFPoints;
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API RequestClosestPointToPointFromUnity(float* initialPointPosition)
{
	glm::vec3 initialPoint = glm::vec3(0.0f);
	initialPoint.x = initialPointPosition[0];
	initialPoint.y = initialPointPosition[1];
	initialPoint.z = initialPointPosition[2];

	glm::vec3 closestPoint = glm::vec3(0.0f);
	int pointCloudIndex = -1;

	float minDistance = FLT_MAX;
	for (size_t i = 0; i < pointClouds.size(); i++)
	{
		if (!pointClouds[i]->wasFullyLoaded)
			continue;

		glm::mat4 glmWorldMatrix = pointClouds[i]->worldMatrix;
		glmWorldMatrix = glm::transpose(glmWorldMatrix);
		glm::vec3 pointCloudTranslation = glm::vec3(glmWorldMatrix[3].x, glmWorldMatrix[3].y, glmWorldMatrix[3].z);

		for (size_t j = 0; j < pointClouds[i]->vertexInfo.size(); j++)
		{
			float distance = glm::distance(pointClouds[i]->vertexInfo[j].position + pointCloudTranslation, initialPoint);
			if (distance < minDistance)
			{
				pointCloudIndex = int(i);
				minDistance = distance;
				closestPoint = pointClouds[i]->vertexInfo[j].position + pointCloudTranslation;
			}
		}
	}

	initialPointPosition[0] = closestPoint.x;
	initialPointPosition[1] = closestPoint.y;
	initialPointPosition[2] = closestPoint.z;
}

bool isAtleastOnePointInSphere(int pointCloudIndex, float* center, float size)
{
	if (pointCloudIndex >= pointClouds.size() || pointCloudIndex < 0)
		return false;

	if (!pointClouds[pointCloudIndex]->wasFullyLoaded)
		return false;

	glm::vec3 centerOfBrush = glm::vec3(center[0], center[1], center[2]);
	glm::vec3 localPosition = glm::inverse(glm::transpose(pointClouds[pointCloudIndex]->worldMatrix)) * glm::vec4(centerOfBrush, 1.0f);
	debugLog::getInstance().addToLog("centerOfBrush: ", centerOfBrush, "isAtleastOnePointInSphere");
	debugLog::getInstance().addToLog("localPosition: ", localPosition, "isAtleastOnePointInSphere");

	if (pointClouds[pointCloudIndex]->getSearchOctree()->isInOctreeBound(localPosition, size))
	{
		if (pointClouds[pointCloudIndex]->getSearchOctree()->isAtleastOnePointInSphere(localPosition, size))
			return true;
	}

	return false;
}

extern "C" bool UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API RequestIsAtleastOnePointInSphereFromUnity(float* center, float size)
{
	glm::vec3 centerOfBrush = glm::vec3(center[0], center[1], center[2]);

	for (size_t i = 0; i < pointClouds.size(); i++)
	{
		if (!pointClouds[i]->wasFullyLoaded)
			continue;

		glm::vec3 localPosition = glm::inverse(glm::transpose(pointClouds[i]->worldMatrix)) * glm::vec4(centerOfBrush, 1.0f);

		if (pointClouds[i]->getSearchOctree()->isInOctreeBound(localPosition, size))
		{
			if (pointClouds[i]->getSearchOctree()->isAtleastOnePointInSphere(localPosition, size))
				return true;
		}
	}

	return false;
}

//#define CLOSEST_POINT_FAST_SEARCH
glm::vec3 getClosestPoint(int pointCloudIndex, glm::vec3 referencePoint)
{
	if (pointCloudIndex >= pointClouds.size() || pointCloudIndex < 0)
		return glm::vec3(FLT_MAX);

	if (!pointClouds[pointCloudIndex]->wasFullyLoaded)
		return glm::vec3(FLT_MAX);

	glm::vec3 closestPoint = glm::vec3(0.0f);

#ifdef CLOSEST_POINT_FAST_SEARCH

	debugLog::getInstance().addToLog("referencePoint: ", referencePoint, "getClosestPoint");

	glm::mat4 glmWorldMatrix = pointClouds[pointCloudIndex]->worldMatrix;
	glmWorldMatrix = glm::transpose(glmWorldMatrix);
	glm::vec3 pointCloudTranslation = glm::vec3(glmWorldMatrix[3].x, glmWorldMatrix[3].y, glmWorldMatrix[3].z);

	float distanceToPointCloud = glm::distance(referencePoint, pointCloudTranslation);
	debugLog::getInstance().addToLog("distanceToPointCloud: " + std::to_string(distanceToPointCloud), "getClosestPoint");

	// Multiplying by 4 to decrease chance that we will not "caught" any points.
	float workingRange = distanceToPointCloud * 4.0f;
	bool downScale = true;
	float sizeDifference = workingRange / 2.0f;
	bool pointInRange = isAtleastOnePointInSphere(pointCloudIndex, glm::value_ptr(referencePoint), workingRange);
	float lastScaleWithPoints = 0.0f;
	if (pointInRange)
		lastScaleWithPoints = workingRange;

	bool pointInRangeLastStep = pointInRange;
	for (int i = 0; i < 20; i++)
	{
		debugLog::getInstance().addToLog("step: " + std::to_string(i), "getClosestPoint");
		debugLog::getInstance().addToLog("sizeDifference: " + std::to_string(sizeDifference), "getClosestPoint");

		debugLog::getInstance().addToLog("workingRange_BEFORE: " + std::to_string(workingRange), "getClosestPoint");
		if (downScale)
		{
			workingRange += -sizeDifference;
		}
		else
		{
			workingRange += sizeDifference;
		}
		sizeDifference = sizeDifference / 2.0f;
		
		debugLog::getInstance().addToLog("workingRange_AFTER: " + std::to_string(workingRange), "getClosestPoint");

		pointInRange = isAtleastOnePointInSphere(pointCloudIndex, glm::value_ptr(referencePoint), workingRange);

		
		debugLog::getInstance().addToLog("pointInRange: " + std::to_string(pointInRange), "getClosestPoint");
		
		if (pointInRange)
			lastScaleWithPoints = workingRange;

		if (pointInRange != pointInRangeLastStep)
		{
			debugLog::getInstance().addToLog("pointInRange != pointInRangeLastStep", "getClosestPoint");
			pointInRangeLastStep = pointInRange;
			downScale = !downScale;
			//sizeDifference = sizeDifference / 2.0f;
		}
	}

	float size = lastScaleWithPoints;
	debugLog::getInstance().addToLog("lastScaleWithPoints: " + std::to_string(lastScaleWithPoints), "getClosestPoint");

	float minDistance = FLT_MAX;
	int totalCountOFPoints = 0;

	// With final size of sphere we are looking closest point in it is bounds.
	glm::vec3 localPosition = glm::inverse(glm::transpose(pointClouds[pointCloudIndex]->worldMatrix)) * glm::vec4(referencePoint, 1.0f);
	pointClouds[pointCloudIndex]->getSearchOctree()->deleteObjects(localPosition, size);
	totalCountOFPoints += int(pointClouds[pointCloudIndex]->getSearchOctree()->pointsToDelete.size());

	for (size_t j = 0; j < pointClouds[pointCloudIndex]->getSearchOctree()->pointsToDelete.size(); j++)
	{
		float distance = glm::distance(pointClouds[pointCloudIndex]->vertexInfo[pointClouds[pointCloudIndex]->getSearchOctree()->pointsToDelete[j]].position + pointCloudTranslation, referencePoint);
		if (distance < minDistance)
		{
			minDistance = distance;
			closestPoint = pointClouds[pointCloudIndex]->vertexInfo[pointClouds[pointCloudIndex]->getSearchOctree()->pointsToDelete[j]].position + pointCloudTranslation;
		}
	}
	pointClouds[pointCloudIndex]->getSearchOctree()->pointsToDelete.clear();
#else
	float minDistance = FLT_MAX;
	for (size_t i = 0; i < pointClouds[pointCloudIndex]->getPointCount(); i++)
	{
		if (pointClouds[pointCloudIndex]->vertexInfo[i].position == referencePoint)
			continue;

		float distanceToClosest = glm::length(pointClouds[pointCloudIndex]->vertexInfo[i].position - referencePoint);
		//debugLog::getInstance().addToLog("First point: ", pointClouds[pointCloudIndex]->vertexInfo[i].position, "getClosestPoint");
		//debugLog::getInstance().addToLog("ReferencePoint point: ", referencePoint, "getClosestPoint");
		//debugLog::getInstance().addToLog("distanceToClosest: " + std::to_string(distanceToClosest), "getClosestPoint");

		if (minDistance > distanceToClosest)
		{
			closestPoint = pointClouds[pointCloudIndex]->vertexInfo[i].position;
			minDistance = distanceToClosest;
		}
	}

#endif

	return closestPoint;
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API DeleteOutliers_OLD(int pointCloudIndex, float outliersRange)
{
	if (pointCloudIndex >= pointClouds.size() || pointCloudIndex < 0)
		return;

	DWORD totalTime = GetTickCount();

	for (size_t i = 0; i < pointClouds[pointCloudIndex]->getPointCount(); i++)
	{
		if (i > 2000)
			break;

		glm::vec3 point = getClosestPoint(pointCloudIndex, pointClouds[pointCloudIndex]->vertexInfo[i].position);
		//debugLog::getInstance().addToLog("referencePoint: ", pointClouds[pointCloudIndex]->vertexInfo[i].position, "DeleteOutliers");
		//debugLog::getInstance().addToLog("closestPoint: ", point, "DeleteOutliers");
		if (point != glm::vec3(FLT_MAX))
		{
			float distanceToClosest = glm::length(point - pointClouds[pointCloudIndex]->vertexInfo[i].position);
			if (distanceToClosest > outliersRange)
			{
				debugLog::getInstance().addToLog("distanceToClosest: " + std::to_string(distanceToClosest), "DeleteOutliers");
			}
		}
	}

	debugLog::getInstance().addToLog("Time spent in DeleteOutliers: " + std::to_string(GetTickCount() - totalTime) + " ms", "DeleteOutliers");
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API RequestClosestPointInSphereFromUnity(float* center, float size)
{
	//debugLog::getInstance().addToLog("===============================", "RequestClosestPointInSphereFromUnity");

	DWORD totalTime = GetTickCount();

	if (pointClouds.size() == 0)
		return;

	glm::vec3 referencePoint = glm::vec3(center[0], center[1], center[2]);
	//debugLog::getInstance().addToLog("point", centerOfBrush, "RequestClosestPointInSphereFromUnity");
	//debugLog::getInstance().addToLog("pointClouds.size(): " + std::to_string(pointClouds.size()), "RequestClosestPointInSphereFromUnity");

	DWORD time = GetTickCount();
	float minDistance = FLT_MAX;
	for (int i = 0; i < pointClouds.size(); i++)
	{
		if (!pointClouds[i]->wasFullyLoaded)
			continue;

		glm::mat4 glmWorldMatrix = pointClouds[i]->worldMatrix;
		glmWorldMatrix = glm::transpose(glmWorldMatrix);
		glm::vec3 pointCloudTranslation = glm::vec3(glmWorldMatrix[3].x, glmWorldMatrix[3].y, glmWorldMatrix[3].z);

		float currentDistance = glm::distance(referencePoint, pointCloudTranslation);
		if (currentDistance < minDistance)
		{
			minDistance = currentDistance;
		}
	}

	//debugLog::getInstance().addToLog("minDistance: " + std::to_string(minDistance), "RequestClosestPointInSphereFromUnity");
	//debugLog::getInstance().addToLog("Time spent on looking for closest point cloud: " + std::to_string(GetTickCount() - time) + " ms", "RequestClosestPointInSphereFromUnity");

	if (minDistance == FLT_MAX)
		return;

	time = GetTickCount();

	// Multiplying by 4 to decrease chance that we will not "caught" any points.
	float workingRange = minDistance * 4.0f;
	//pointCloudManager.getTestSphereGameObject().transform.localScale = new Vector3(workingRange, workingRange, workingRange);
	bool downScale = true;
	float sizeDifference = workingRange / 2.0f;
	bool pointInRange = RequestIsAtleastOnePointInSphereFromUnity(glm::value_ptr(referencePoint), workingRange);
	float lastScaleWithPoints = 0.0f;
	if (pointInRange)
		lastScaleWithPoints = workingRange;

	bool pointInRangeLastStep = pointInRange;

	for (int i = 0; i < 20; i++)
	{
		workingRange += downScale ? -sizeDifference : sizeDifference;
		//pointCloudManager.getTestSphereGameObject().transform.localScale = new Vector3(workingRange, workingRange, workingRange);
		pointInRange = RequestIsAtleastOnePointInSphereFromUnity(glm::value_ptr(referencePoint), workingRange);
		if (pointInRange)
			lastScaleWithPoints = workingRange;

		if (pointInRange != pointInRangeLastStep)
		{
			pointInRangeLastStep = pointInRange;
			downScale = !downScale;
			sizeDifference = sizeDifference / 2.0f;
		}
	}

	//debugLog::getInstance().addToLog("Time spent on binary tightening of search sphere: " + std::to_string(GetTickCount() - time) + " ms", "RequestClosestPointInSphereFromUnity");

	size = lastScaleWithPoints;
	glm::vec3 closestPoint = glm::vec3(0.0f);

	minDistance = FLT_MAX;
	time = GetTickCount();
	int totalCountOFPoints = 0;

	for (size_t i = 0; i < pointClouds.size(); i++)
	{
		if (!pointClouds[i]->wasFullyLoaded)
			continue;

		glm::mat4 glmWorldMatrix = pointClouds[i]->worldMatrix;
		glmWorldMatrix = glm::transpose(glmWorldMatrix);
		glm::vec3 pointCloudTranslation = glm::vec3(glmWorldMatrix[3].x, glmWorldMatrix[3].y, glmWorldMatrix[3].z);

		glm::vec3 localPosition = glm::inverse(glm::transpose(pointClouds[i]->worldMatrix)) * glm::vec4(referencePoint, 1.0f);

		auto timeDeleteObjects = GetTickCount();
		pointClouds[i]->getSearchOctree()->deleteObjects(localPosition, size);
		//debugLog::getInstance().addToLog("timeDeleteObjects: " + std::to_string(GetTickCount() - timeDeleteObjects), "RequestClosestPointInSphereFromUnity");

		//LOG.addToLog("pointClouds[i]->getSearchOctree()->pointsToDelete.size(): " + std::to_string(pointClouds[i]->getSearchOctree()->pointsToDelete.size()), "TEST");
		totalCountOFPoints += int(pointClouds[i]->getSearchOctree()->pointsToDelete.size());

		for (size_t j = 0; j < pointClouds[i]->getSearchOctree()->pointsToDelete.size(); j++)
		{
			float distance = glm::distance(pointClouds[i]->vertexInfo[pointClouds[i]->getSearchOctree()->pointsToDelete[j]].position + pointCloudTranslation, referencePoint);
			if (distance < minDistance)
			{
				minDistance = distance;
				closestPoint = pointClouds[i]->vertexInfo[pointClouds[i]->getSearchOctree()->pointsToDelete[j]].position + pointCloudTranslation;
			}
		}

		pointClouds[i]->getSearchOctree()->pointsToDelete.clear();
	}

	//if (GetTickCount() - time > 0)
	//	debugLog::getInstance().addToLog("totalCountOFPoints: " + std::to_string(totalCountOFPoints), "RequestClosestPointInSphereFromUnity");
	
	//debugLog::getInstance().addToLog("Time spent on search of closest point in search sphere: " + std::to_string(GetTickCount() - time) + " ms", "RequestClosestPointInSphereFromUnity");

	center[0] = closestPoint.x;
	center[1] = closestPoint.y;
	center[2] = closestPoint.z;

	//debugLog::getInstance().addToLog("totalTime: " + std::to_string(GetTickCount() - totalTime) + " ms", "RequestClosestPointInSphereFromUnity");
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API setHighlightDeletedPointsActive(bool active)
{
	highlightDeletedPoints = active;
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UpdateDeletionSpherePositionFromUnity(float* center, float size)
{
	deletionSpherePosition = glm::vec3(center[0], center[1], center[2]);
	deletionSphereSize = size;
}

#ifdef USE_COMPUTE_SHADER
void runMyComputeShader()
{
	static DWORD startTime = GetTickCount();

	if (pointClouds.size() == 0 || !pointClouds[0]->wasFullyLoaded || testFrustum == nullptr || (GetTickCount() - startTime < 1000))
		return;

	//LOG.addToLog("runMyComputeShader()_START: " + std::system_category().message(GPU.getDevice()->GetDeviceRemovedReason()), "computeShader");

	ID3D11DeviceContext* ctx = NULL;
	GPU.getDevice()->GetImmediateContext(&ctx);

	if (inputPoints_CS_SRV == nullptr)
	{
		allPointsData_CS = new MeshVertex[pointClouds[0]->vertexInfo.size()];
#ifdef FLOAT_TEST
		InputData_CS = new float[24];
#else
		InputData_CS = new MeshVertex[24];
#endif // FLOAT_TEST

		for (int i = 0; i < pointClouds[0]->vertexInfo.size(); ++i)
		{
			allPointsData_CS[i] = pointClouds[0]->vertexInfo[i];
		}

		D3D11_BUFFER_DESC descBuffer = {};
		descBuffer.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		descBuffer.ByteWidth = sizeof(MeshVertex) * pointClouds[0]->vertexInfo.size();
		descBuffer.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		descBuffer.StructureByteStride = sizeof(MeshVertex);
		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = &allPointsData_CS[0];
		auto result = GPU.getDevice()->CreateBuffer(&descBuffer, &InitData, &allPointsDataBuffer_CS);
		//HRESULT result = 0;
		//LOG.addToLog("pointClouds[0]->vertexInfo.size(): " + std::to_string(pointClouds[0]->vertexInfo.size()), "computeShader");
		//LOG.addToLog("GPU.getDevice()->CreateBuffer(&descBuffer, &InitData, &g_pBuf0);: " + std::system_category().message(result), "computeShader");
		//LOG.addToLog("GPU.getDevice()->CreateBuffer(&descBuffer, &InitData, &g_pBuf0);: " + std::system_category().message(GPU.getDevice()->GetDeviceRemovedReason()), "computeShader");

		InitData.pSysMem = &InputData_CS[0];
#ifdef FLOAT_TEST
		descBuffer.ByteWidth = sizeof(float) * 24;
		descBuffer.StructureByteStride = sizeof(float);
#else
		descBuffer.ByteWidth = sizeof(MeshVertex) * 24;
#endif // FLOAT_TEST

		result = GPU.getDevice()->CreateBuffer(&descBuffer, &InitData, &InputDataBuffer_CS);
		//LOG.addToLog("GPU.getDevice()->CreateBuffer(&descBuffer, &InitData, &g_pBuf1);: " + std::system_category().message(result), "computeShader");
		//LOG.addToLog("GPU.getDevice()->CreateBuffer(&descBuffer, &InitData, &g_pBuf1);: " + std::system_category().message(GPU.getDevice()->GetDeviceRemovedReason()), "computeShader");
		descBuffer.ByteWidth = sizeof(MeshVertex) * pointClouds[0]->vertexInfo.size();
		descBuffer.StructureByteStride = sizeof(MeshVertex);
		result = GPU.getDevice()->CreateBuffer(&descBuffer, nullptr, &resultBuffer_CS);
		result = GPU.getDevice()->CreateBuffer(&descBuffer, nullptr, &resultBufferSecond_CS);
		currentBuffer_CS = &resultBuffer_CS;
		//LOG.addToLog("GPU.getDevice()->CreateBuffer(&descBuffer, nullptr, &g_pBufResult);: " + std::system_category().message(result), "computeShader");
		//LOG.addToLog("GPU.getDevice()->CreateBuffer(&descBuffer, nullptr, &g_pBufResult);: " + std::system_category().message(GPU.getDevice()->GetDeviceRemovedReason()), "computeShader");

		// DEBUG
		result = allPointsDataBuffer_CS->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof("Buffer0") - 1, "Buffer0");
		//LOG.addToLog("g_pBuf0->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(\"Buffer0\") - 1, \"Buffer0\");: " + std::system_category().message(result), "computeShader");
		//LOG.addToLog("g_pBuf0->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(\"Buffer0\") - 1, \"Buffer0\");: " + std::system_category().message(GPU.getDevice()->GetDeviceRemovedReason()), "computeShader");
		InputDataBuffer_CS->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof("Buffer1") - 1, "Buffer1");
		resultBuffer_CS->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof("Result") - 1, "Result");
		// DEBUG

		CreateBufferSRV(GPU.getDevice(), allPointsDataBuffer_CS, &inputPoints_CS_SRV);
		CreateBufferSRV(GPU.getDevice(), InputDataBuffer_CS, &InputData_CS_SRV);
		CreateBufferUAV(GPU.getDevice(), resultBuffer_CS, &result_CS_UAV);
		CreateBufferSRV(GPU.getDevice(), resultBuffer_CS, &result_CS_SRV);
		CreateBufferUAV(GPU.getDevice(), resultBufferSecond_CS, &resultSecond_CS_UAV);
		CreateBufferSRV(GPU.getDevice(), resultBufferSecond_CS, &resultSecond_CS_SRV);
		current_CS_UAV = &result_CS_UAV;
		current_CS_SRV = &result_CS_SRV;

		// DEBUG
		inputPoints_CS_SRV->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof("Buffer0 SRV") - 1, "Buffer0 SRV");
		InputData_CS_SRV->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof("Buffer1 SRV") - 1, "Buffer1 SRV");
		result_CS_UAV->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof("Result UAV") - 1, "Result UAV");
		// DEBUG

		currentInputPoints_CS_SRV = &inputPoints_CS_SRV;
	}

	int linearIndex = 0;
	for (size_t i = 0; i < 6; i++)
	{
		for (size_t j = 0; j < 4; j++)
		{
#ifdef FLOAT_TEST
			InputData_CS[linearIndex++] = testFrustum[i][j];
#else
			InputData_CS[linearIndex++].position.x = testFrustum[i][j];
#endif // FLOAT_TEST
		}
	}

	glm::vec3 localDeletionSpherePosition = glm::inverse(glm::transpose(pointClouds[0]->worldMatrix)) * glm::vec4(deletionSpherePosition, 1.0f);
	InputData_CS[0] = localDeletionSpherePosition.x;
	InputData_CS[1] = localDeletionSpherePosition.y;
	InputData_CS[2] = localDeletionSpherePosition.z;
	InputData_CS[3] = deletionSphereSize * 2.0f;

#ifdef FLOAT_TEST
	ctx->UpdateSubresource(InputDataBuffer_CS, 0, nullptr, InputData_CS, sizeof(float) * 24, sizeof(float) * 24);
#else
	ctx->UpdateSubresource(InputDataBuffer_CS, 0, nullptr, InputData_CS, sizeof(MeshVertex) * 24, sizeof(MeshVertex) * 24);
#endif // FLOAT_TEST

	ID3D11ShaderResourceView* aRViews[2] = { inputPoints_CS_SRV, InputData_CS_SRV };
	RunComputeShader(ctx, computeShader, 2, aRViews, nullptr, nullptr, 0, result_CS_UAV, ceil(pointClouds[0]->vertexInfo.size() / 64), 1, 1);

	if (false)
	{
		ID3D11Buffer* debugbuf = CreateAndCopyToDebugBuf(GPU.getDevice(), ctx, resultBuffer_CS);
		D3D11_MAPPED_SUBRESOURCE MappedResource;
		MeshVertex* p;
		ctx->Map(debugbuf, 0, D3D11_MAP_READ, 0, &MappedResource);

		// Set a break point here and put down the expression "p, 1024" in your watch window to see what has been written out by our CS
		// This is also a common trick to debug CS programs.
		p = (MeshVertex*)MappedResource.pData;

		/*LOG.addToLog("resultBufferSize: " + std::to_string(MappedResource.RowPitch / sizeof(MeshVertex)), "computeShader");*/

		//for (int i = 0; i < MappedResource.RowPitch / sizeof(MeshVertex); i++)
		//{
		//	LOG.addToLog("result[" + std::to_string(i) + "] - X: " + std::to_string(p[i].position.x) + " Y : " + std::to_string(p[i].position.y) + " Z : " + std::to_string(p[i].position.z), "computeShader");
		//	//LOG.addToLog("result[" + std::to_string(i) + "] - R: " + std::to_string(p[i].color[0]) + " G : " + std::to_string(p[i].color[1]) + " B : " + std::to_string(p[i].color[2]), "computeShader");
		//}

		DWORD beforeTime = GetTickCount();

		D3D11_BOX dbox{};
		dbox.left = 0;
		dbox.right = MappedResource.RowPitch;
		dbox.top = 0;
		dbox.bottom = 1;
		dbox.front = 0;
		dbox.back = 1;

		ctx->UpdateSubresource(pointClouds[0]->mainVB, 0, &dbox, MappedResource.pData, pointClouds[0]->getPointCount() * kVertexSize, pointClouds[0]->getPointCount() * kVertexSize);
		//ctx->UpdateSubresource(pointClouds[0]->intermediateVB, 0, &dbox, MappedResource.pData, pointClouds[0]->getPointCount() * kVertexSize, pointClouds[0]->getPointCount() * kVertexSize);
		//ctx->CopySubresourceRegion(pointClouds[0]->mainVB, 0, dbox.left, 0, 0, pointClouds[0]->intermediateVB, 0, &dbox);

		/*LOG.addToLog("ctx->CopySubresourceRegion was called", "computeShader");
		LOG.addToLog("CPU time :" + std::to_string(GetTickCount() - beforeTime), "computeShader");
		LOG.addToLog("testLevel" + std::to_string(testLevel), "computeShader");*/

		//for (int i = 0; i < pointClouds[0]->vertexInfo.size(); i++)
		//{
		//	float distance = sqrt(pow(pointClouds[0]->vertexInfo[i].position.x, 2) + pow(pointClouds[0]->vertexInfo[i].position.y, 2) + pow(pointClouds[0]->vertexInfo[i].position.z, 2));
		//	LOG.addToLog("p[" + std::to_string(i) + "].x :" + std::to_string(p[i].position.x) + "CPU: " + std::to_string(distance), "computeShader");
		//	
		//	//LOG.addToLog("p[" + std::to_string(i) + "].x :" + std::to_string(p[i].position.x) + "CPU: " + std::to_string(pointClouds[0]->vertexInfo[i].position.x), "computeShader");
		//}




		/*DWORD beforeTime = GetTickCount();
		float smallestDistance = FLT_MAX;
		int index = -1;

		for (int i = 0; i < pointClouds[0]->vertexInfo.size(); ++i)
		{
			float distance = sqrt(pow(pointClouds[0]->vertexInfo[i].position.x, 2) + pow(pointClouds[0]->vertexInfo[i].position.y, 2) + pow(pointClouds[0]->vertexInfo[i].position.z, 2));
			if (distance < smallestDistance)
			{
				smallestDistance = distance;
				index = i;
			}
		}

		LOG.addToLog("CPU time :" + std::to_string(GetTickCount() - beforeTime), "computeShader");

		LOG.addToLog("CPU min distance :" + std::to_string(smallestDistance), "computeShader");
		LOG.addToLog("CPU point index :" + std::to_string(index), "computeShader");*/

		//LOG.addToLog("min distance :" + std::to_string(p[0].position.x), "computeShader");
		//LOG.addToLog("point index :" + std::to_string(p[1].position.x), "computeShader");

		ctx->Unmap(debugbuf, 0);
	}

	//LOG.addToLog("runMyComputeShader()_END: " + std::system_category().message(GPU.getDevice()->GetDeviceRemovedReason()), "computeShader");
}

void runMyDeleteComputeShader()
{
	static DWORD startTime = GetTickCount();

	if (pointClouds.size() == 0 || !pointClouds[0]->wasFullyLoaded)
		return;

	//LOG.addToLog("runMyDeleteComputeShader()_START: " + std::system_category().message(GPU.getDevice()->GetDeviceRemovedReason()), "computeShader");

	ID3D11DeviceContext* ctx = NULL;
	GPU.getDevice()->GetImmediateContext(&ctx);

	if (inputPoints_CS_SRV == nullptr)
	{
		allPointsData_CS = new MeshVertex[pointClouds[0]->vertexInfo.size()];
		InputData_CS = new float[24];

		for (int i = 0; i < pointClouds[0]->vertexInfo.size(); ++i)
		{
			allPointsData_CS[i] = pointClouds[0]->vertexInfo[i];
		}

		D3D11_BUFFER_DESC descBuffer = {};
		descBuffer.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		descBuffer.ByteWidth = sizeof(MeshVertex) * pointClouds[0]->vertexInfo.size();
		descBuffer.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		descBuffer.StructureByteStride = sizeof(MeshVertex);
		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = &allPointsData_CS[0];
		auto result = GPU.getDevice()->CreateBuffer(&descBuffer, &InitData, &allPointsDataBuffer_CS);

		InitData.pSysMem = &InputData_CS[0];
		descBuffer.ByteWidth = sizeof(float) * 24;
		descBuffer.StructureByteStride = sizeof(float);

		result = GPU.getDevice()->CreateBuffer(&descBuffer, &InitData, &InputDataBuffer_CS);
		descBuffer.ByteWidth = sizeof(MeshVertex) * pointClouds[0]->vertexInfo.size();
		descBuffer.StructureByteStride = sizeof(MeshVertex);
		result = GPU.getDevice()->CreateBuffer(&descBuffer, nullptr, &resultBuffer_CS);
		result = GPU.getDevice()->CreateBuffer(&descBuffer, nullptr, &resultBufferSecond_CS);
		currentBuffer_CS = &resultBuffer_CS;

		CreateBufferSRV(GPU.getDevice(), allPointsDataBuffer_CS, &inputPoints_CS_SRV);
		CreateBufferSRV(GPU.getDevice(), InputDataBuffer_CS, &InputData_CS_SRV);
		CreateBufferUAV(GPU.getDevice(), resultBuffer_CS, &result_CS_UAV);
		CreateBufferSRV(GPU.getDevice(), resultBuffer_CS, &result_CS_SRV);
		CreateBufferUAV(GPU.getDevice(), resultBufferSecond_CS, &resultSecond_CS_UAV);
		CreateBufferSRV(GPU.getDevice(), resultBufferSecond_CS, &resultSecond_CS_SRV);
		current_CS_UAV = &result_CS_UAV;
		current_CS_SRV = &result_CS_SRV;

		currentInputPoints_CS_SRV = &inputPoints_CS_SRV;
	}

	glm::vec3 localDeletionSpherePosition = glm::inverse(glm::transpose(pointClouds[0]->worldMatrix)) * glm::vec4(deletionSpherePosition, 1.0f);
	InputData_CS[0] = localDeletionSpherePosition.x;
	InputData_CS[1] = localDeletionSpherePosition.y;
	InputData_CS[2] = localDeletionSpherePosition.z;
	InputData_CS[3] = deletionSphereSize * 2.0f;

	ctx->UpdateSubresource(InputDataBuffer_CS, 0, nullptr, InputData_CS, sizeof(float) * 24, sizeof(float) * 24);

	//ID3D11ShaderResourceView* aRViews[2] = { allPointsData_CS_SRV, InputData_CS_SRV };
	ID3D11ShaderResourceView* aRViews[2] = { *currentInputPoints_CS_SRV, InputData_CS_SRV };
	RunComputeShader(ctx, computeShader, 2, aRViews, nullptr, nullptr, 0, *current_CS_UAV/*result_CS_UAV*/, ceil(pointClouds[0]->vertexInfo.size() / 64), 1, 1);
}
#endif

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API setTestLevel(float unityTestLevel)
{
	testLevel = unityTestLevel;
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API highlightOutliers(float discardDistance, int minNeighborsInRange)
{
	if (pointClouds.size() == 0 || !pointClouds[0]->wasFullyLoaded)
		return;


	//if (outliers_CS->buffers.size() == 0)
	//{
	//	outliers_CS->addBuffer(sizeof(MeshVertex), pointClouds[0]->getPointCount(), pointClouds[0]->vertexInfo.data());
	//	outliers_CS->addBuffer(sizeof(unsigned int), pointClouds[0]->getPointCount(), nullptr);

	//	outliers_CS->addBufferSRV(outliers_CS->buffers[0]);
	//	outliers_CS->addBufferUAV(outliers_CS->buffers[1]);
	//}

	//outliers_CS->run(UINT(ceil(pointClouds[0]->getPointCount() / 64.0f)), 1, 1);

	//unsigned int* result = (unsigned int*)outliers_CS->beginReadingBufferData(outliers_CS->buffers[1]);

	//for (int i = 0; i < 100; i++)
	//{
	//	LOG.addToLog("result[" + std::to_string(i) + "] - X: " + std::to_string(result[i]), "computeShader");
	//	//LOG.addToLog("result[" + std::to_string(i) + "] - R: " + std::to_string(p[i].color[0]) + " G : " + std::to_string(p[i].color[1]) + " B : " + std::to_string(p[i].color[2]), "computeShader");
	//}

	//outliers_CS->endReadingBufferData(outliers_CS->buffers[1]);

	pointClouds[0]->lastOutliers.clear();
	for (size_t i = 0; i < pointClouds[0]->vertexInfo.size(); i++)
	{
		float distance = FLT_MAX;
		// We relay on fact that points are somewhat sorted by their position in an array.
		int localNeighbors = 0;
		for (size_t j = i - 10; j < i + 10; j++)
		{
			if (j < 0 || j >= pointClouds[0]->vertexInfo.size() || j == i)
				continue;

			distance = glm::length(pointClouds[0]->vertexInfo[i].position - pointClouds[0]->vertexInfo[j].position);
			if (distance < discardDistance)
			{
				localNeighbors++;
				if (localNeighbors >= minNeighborsInRange)
					break;
			}
		}

		if (localNeighbors < minNeighborsInRange)
		{
			distance = pointClouds[0]->getSearchOctree()->closestPointDistance(pointClouds[0]->vertexInfo[i].position, discardDistance, minNeighborsInRange);

			if (distance > discardDistance)
			{
				pointClouds[0]->vertexInfo[i].color[0] = 255;
				pointClouds[0]->vertexInfo[i].color[1] = 0;
				pointClouds[0]->vertexInfo[i].color[2] = 0;
				pointClouds[0]->vertexInfo[i].color[3] = 255;

				pointClouds[0]->lastOutliers.push_back(i);
			}
			else
			{
				pointClouds[0]->vertexInfo[i].color[0] = pointClouds[0]->originalData[i].color[0];
				pointClouds[0]->vertexInfo[i].color[1] = pointClouds[0]->originalData[i].color[1];
				pointClouds[0]->vertexInfo[i].color[2] = pointClouds[0]->originalData[i].color[2];
				pointClouds[0]->vertexInfo[i].color[3] = pointClouds[0]->originalData[i].color[3];
			}
		}
		else
		{
			pointClouds[0]->vertexInfo[i].color[0] = pointClouds[0]->originalData[i].color[0];
			pointClouds[0]->vertexInfo[i].color[1] = pointClouds[0]->originalData[i].color[1];
			pointClouds[0]->vertexInfo[i].color[2] = pointClouds[0]->originalData[i].color[2];
			pointClouds[0]->vertexInfo[i].color[3] = pointClouds[0]->originalData[i].color[3];
		}
	}

	// Update GPU buffer.
	const int kVertexSize = 12 + 4;
	ID3D11DeviceContext* ctx = NULL;

	if (GPU.getDevice() != nullptr)
		GPU.getDevice()->GetImmediateContext(&ctx);

	//ctx->UpdateSubresource(pointClouds[0]->mainVB, 0, NULL, pointClouds[0]->originalData.data(), pointClouds[0]->getPointCount() * kVertexSize, pointClouds[0]->getPointCount() * kVertexSize);
	ctx->UpdateSubresource(pointClouds[0]->mainVB, 0, NULL, pointClouds[0]->vertexInfo.data(), pointClouds[0]->getPointCount() * kVertexSize, pointClouds[0]->getPointCount() * kVertexSize);
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API deleteOutliers()
{
	if (pointClouds.size() == 0 || !pointClouds[0]->wasFullyLoaded)
		return;

	pointClouds[0]->getSearchOctree()->pointsToDelete = pointClouds[0]->lastOutliers;
	pointClouds[0]->lastOutliers.clear();
}
