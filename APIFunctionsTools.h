#include "undoManager.h"

#include "framework.h"
#include <iostream>

#include "comdef.h"

#include "thirdparty/laszip/include/laszip_api.h"
#include "thirdparty/cnpy/cnpy.h"

struct ModificationRequest
{
	glm::vec3 Center;
	float Size;
};

static std::vector<ModificationRequest> ModificationRequests;

void onDrawDeletePointsinGPUMem(pointCloud* pointCloud, ID3D11DeviceContext* ctx);

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