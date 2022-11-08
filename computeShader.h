#pragma once

#include "DX11GPU.h"

using namespace FocalEngine;

const BYTE g_CSMain_[] =
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

const BYTE g_CSOutLiers[] =
{
	 68,  88,  66,  67, 119, 187,
	 55,  90, 140,   1,   8, 148,
	 44,  16,  18, 185, 159, 212,
	 76,  92,   1,   0,   0,   0,
	 84,   5,   0,   0,   5,   0,
	  0,   0,  52,   0,   0,   0,
	 72,   2,   0,   0,  88,   2,
	  0,   0, 104,   2,   0,   0,
	184,   4,   0,   0,  82,  68,
	 69,  70,  12,   2,   0,   0,
	  2,   0,   0,   0, 144,   0,
	  0,   0,   2,   0,   0,   0,
	 60,   0,   0,   0,   0,   5,
	 83,  67,   0,   1,   0,   0,
	216,   1,   0,   0,  82,  68,
	 49,  49,  60,   0,   0,   0,
	 24,   0,   0,   0,  32,   0,
	  0,   0,  40,   0,   0,   0,
	 36,   0,   0,   0,  12,   0,
	  0,   0,   0,   0,   0,   0,
	124,   0,   0,   0,   5,   0,
	  0,   0,   6,   0,   0,   0,
	  1,   0,   0,   0,  16,   0,
	  0,   0,   0,   0,   0,   0,
	  1,   0,   0,   0,   1,   0,
	  0,   0, 132,   0,   0,   0,
	  9,   0,   0,   0,   6,   0,
	  0,   0,   1,   0,   0,   0,
	  4,   0,   0,   0,   0,   0,
	  0,   0,   1,   0,   0,   0,
	  1,   0,   0,   0,  66, 117,
	102, 102, 101, 114,  48,   0,
	 66, 117, 102, 102, 101, 114,
	 79, 117, 116,   0, 171, 171,
	124,   0,   0,   0,   1,   0,
	  0,   0, 192,   0,   0,   0,
	 16,   0,   0,   0,   0,   0,
	  0,   0,   3,   0,   0,   0,
	132,   0,   0,   0,   1,   0,
	  0,   0, 176,   1,   0,   0,
	  4,   0,   0,   0,   0,   0,
	  0,   0,   3,   0,   0,   0,
	232,   0,   0,   0,   0,   0,
	  0,   0,  16,   0,   0,   0,
	  2,   0,   0,   0, 140,   1,
	  0,   0,   0,   0,   0,   0,
	255, 255, 255, 255,   0,   0,
	  0,   0, 255, 255, 255, 255,
	  0,   0,   0,   0,  36,  69,
	108, 101, 109, 101, 110, 116,
	  0,  66, 117, 102,  84, 121,
	112, 101,   0, 120,   0, 102,
	108, 111,  97, 116,   0, 171,
	171, 171,   0,   0,   3,   0,
	  1,   0,   1,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0, 251,   0,
	  0,   0, 121,   0, 122,   0,
	 99, 111, 108, 111, 114,   0,
	105, 110, 116,   0, 171, 171,
	  0,   0,   2,   0,   1,   0,
	  1,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,  50,   1,   0,   0,
	249,   0,   0,   0,   4,   1,
	  0,   0,   0,   0,   0,   0,
	 40,   1,   0,   0,   4,   1,
	  0,   0,   4,   0,   0,   0,
	 42,   1,   0,   0,   4,   1,
	  0,   0,   8,   0,   0,   0,
	 44,   1,   0,   0,  56,   1,
	  0,   0,  12,   0,   0,   0,
	  5,   0,   0,   0,   1,   0,
	  4,   0,   0,   0,   4,   0,
	 92,   1,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0, 241,   0,   0,   0,
	232,   0,   0,   0,   0,   0,
	  0,   0,   4,   0,   0,   0,
	  2,   0,   0,   0,   4,   1,
	  0,   0,   0,   0,   0,   0,
	255, 255, 255, 255,   0,   0,
	  0,   0, 255, 255, 255, 255,
	  0,   0,   0,   0,  77, 105,
	 99, 114, 111, 115, 111, 102,
	116,  32,  40,  82,  41,  32,
	 72,  76,  83,  76,  32,  83,
	104,  97, 100, 101, 114,  32,
	 67, 111, 109, 112, 105, 108,
	101, 114,  32,  57,  46,  50,
	 57,  46,  57,  53,  50,  46,
	 51,  49,  49,  49,   0, 171,
	171, 171,  73,  83,  71,  78,
	  8,   0,   0,   0,   0,   0,
	  0,   0,   8,   0,   0,   0,
	 79,  83,  71,  78,   8,   0,
	  0,   0,   0,   0,   0,   0,
	  8,   0,   0,   0,  83,  72,
	 69,  88,  72,   2,   0,   0,
	 80,   0,   5,   0, 146,   0,
	  0,   0, 106,   8,   0,   1,
	162,   0,   0,   4,   0, 112,
	 16,   0,   0,   0,   0,   0,
	 16,   0,   0,   0, 158,   0,
	  0,   4,   0, 224,  17,   0,
	  0,   0,   0,   0,   4,   0,
	  0,   0,  95,   0,   0,   2,
	 18,   0,   2,   0, 104,   0,
	  0,   2,   3,   0,   0,   0,
	155,   0,   0,   4,   0,   4,
	  0,   0,   1,   0,   0,   0,
	  1,   0,   0,   0, 167,   0,
	  0, 138,   2, 131,   0, 128,
	131, 153,  25,   0, 114,   0,
	 16,   0,   0,   0,   0,   0,
	 10,   0,   2,   0,   1,  64,
	  0,   0,   0,   0,   0,   0,
	 70, 114,  16,   0,   0,   0,
	  0,   0,  30,   0,   0,   9,
	 50,   0,  16,   0,   1,   0,
	  0,   0,   6,   0,   2,   0,
	  2,  64,   0,   0,   1,   0,
	  0,   0,  16,  39,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,  54,   0,   0,   5,
	 66,   0,  16,   0,   1,   0,
	  0,   0,   1,  64,   0,   0,
	127, 150,  24,  75,  54,   0,
	  0,   5, 130,   0,  16,   0,
	  1,   0,   0,   0,  10,   0,
	 16,   0,   1,   0,   0,   0,
	 48,   0,   0,   1,  80,   0,
	  0,   7, 130,   0,  16,   0,
	  0,   0,   0,   0,  58,   0,
	 16,   0,   1,   0,   0,   0,
	 26,   0,  16,   0,   1,   0,
	  0,   0,   3,   0,   4,   3,
	 58,   0,  16,   0,   0,   0,
	  0,   0, 167,   0,   0, 139,
	  2, 131,   0, 128, 131, 153,
	 25,   0, 114,   0,  16,   0,
	  2,   0,   0,   0,  58,   0,
	 16,   0,   1,   0,   0,   0,
	  1,  64,   0,   0,   0,   0,
	  0,   0,  70, 114,  16,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   8, 114,   0,  16,   0,
	  2,   0,   0,   0,  70,   2,
	 16,   0,   0,   0,   0,   0,
	 70,   2,  16, 128,  65,   0,
	  0,   0,   2,   0,   0,   0,
	 16,   0,   0,   7, 130,   0,
	 16,   0,   0,   0,   0,   0,
	 70,   2,  16,   0,   2,   0,
	  0,   0,  70,   2,  16,   0,
	  2,   0,   0,   0,  75,   0,
	  0,   5, 130,   0,  16,   0,
	  0,   0,   0,   0,  58,   0,
	 16,   0,   0,   0,   0,   0,
	 49,   0,   0,   7,  18,   0,
	 16,   0,   2,   0,   0,   0,
	 58,   0,  16,   0,   0,   0,
	  0,   0,  42,   0,  16,   0,
	  1,   0,   0,   0,  31,   0,
	  4,   3,  10,   0,  16,   0,
	  2,   0,   0,   0,  49,   0,
	  0,   7,  18,   0,  16,   0,
	  2,   0,   0,   0,  58,   0,
	 16,   0,   0,   0,   0,   0,
	  1,  64,   0,   0,   0,   0,
	 32,  65,  31,   0,   4,   3,
	 10,   0,  16,   0,   2,   0,
	  0,   0,  54,   0,   0,   5,
	 66,   0,  16,   0,   1,   0,
	  0,   0,  58,   0,  16,   0,
	  0,   0,   0,   0,   2,   0,
	  0,   1,  21,   0,   0,   1,
	 54,   0,   0,   5,  66,   0,
	 16,   0,   1,   0,   0,   0,
	 58,   0,  16,   0,   0,   0,
	  0,   0,  21,   0,   0,   1,
	 30,   0,   0,   7, 130,   0,
	 16,   0,   1,   0,   0,   0,
	 58,   0,  16,   0,   1,   0,
	  0,   0,   1,  64,   0,   0,
	  1,   0,   0,   0,  22,   0,
	  0,   1, 178,   0,   0,   5,
	 18,   0,  16,   0,   0,   0,
	  0,   0,   0, 224,  17,   0,
	  0,   0,   0,   0, 168,   0,
	  0,   9,  18, 224,  17,   0,
	  0,   0,   0,   0,  10,   0,
	 16,   0,   0,   0,   0,   0,
	  1,  64,   0,   0,   0,   0,
	  0,   0,  42,   0,  16,   0,
	  1,   0,   0,   0,  62,   0,
	  0,   1,  83,  84,  65,  84,
	148,   0,   0,   0,  25,   0,
	  0,   0,   3,   0,   0,   0,
	  0,   0,   0,   0,   1,   0,
	  0,   0,   5,   0,   0,   0,
	  2,   0,   0,   0,   1,   0,
	  0,   0,   2,   0,   0,   0,
	  3,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,  17,   0,   0,   0,
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
	  0,   0,   0,   0,   0,   0,
	  0,   0
};

HRESULT CreateBufferUAV(ID3D11Device* pDevice, ID3D11Buffer* pBuffer, ID3D11UnorderedAccessView** ppUAVOut);
HRESULT CreateBufferSRV(ID3D11Device* pDevice, ID3D11Buffer* pBuffer, ID3D11ShaderResourceView** ppSRVOut);
void RunComputeShader(ID3D11DeviceContext* pd3dImmediateContext,
					  ID3D11ComputeShader* pComputeShader,
					  UINT nNumViews, ID3D11ShaderResourceView** pShaderResourceViews,
					  ID3D11Buffer* pCBCS, void* pCSData, DWORD dwNumDataBytes,
					  ID3D11UnorderedAccessView* pUnorderedAccessView,
					  UINT X, UINT Y, UINT Z);

void compileAndCreateComputeShader(ID3D11Device* pDevice, std::string sourceFile, ID3D11ComputeShader** computeShader);
void compileAndCreateComputeShader(ID3D11Device* pDevice, BYTE* source, ID3D11ComputeShader** computeShader);
int getComputeShaderResultCounter(ID3D11Device* pDevice, ID3D11UnorderedAccessView* result_UAV);

ID3D11Buffer* CreateAndCopyToDebugBuf(ID3D11Device* pDevice, ID3D11DeviceContext* pd3dImmediateContext, ID3D11Buffer* pBuffer);

class computeShaderWrapper
{
	ID3D11Device* device;
	ID3D11ComputeShader* shader;

	computeShaderWrapper(ID3D11Device* pDevice, std::string sourceFile);
};

class CShader
{
public:
	ID3D11ComputeShader* shader;
	std::vector<ID3D11Buffer*> buffers;
	std::vector<ID3D11ShaderResourceView*> SRVs;
	std::vector<ID3D11UnorderedAccessView*> UAVs;

	CShader(std::string sourceFile)
	{
		compileAndCreateComputeShader(GPU.getDevice(), sourceFile, &shader);
	}

	void addBuffer(size_t byteSizeOfOneElement, size_t elementsCount, void* data)
	{
		buffers.resize(buffers.size() + 1);

		D3D11_BUFFER_DESC descBuffer = {};
		descBuffer.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		descBuffer.ByteWidth = UINT(byteSizeOfOneElement * elementsCount);
		descBuffer.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		descBuffer.StructureByteStride = UINT(byteSizeOfOneElement);
		if (data == nullptr)
		{
			GPU.getDevice()->CreateBuffer(&descBuffer, nullptr, &buffers[buffers.size() - 1]);
		}
		else
		{
			D3D11_SUBRESOURCE_DATA InitData;
			InitData.pSysMem = data;
			GPU.getDevice()->CreateBuffer(&descBuffer, &InitData, &buffers[buffers.size() - 1]);
		}
	}

	size_t addBufferSRV(ID3D11Buffer* buffer)
	{
		SRVs.resize(SRVs.size() + 1);

		D3D11_BUFFER_DESC descBuf = {};
		buffer->GetDesc(&descBuf);

		D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
		desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		desc.BufferEx.FirstElement = 0;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.BufferEx.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;

		GPU.getDevice()->CreateShaderResourceView(buffer, &desc, &SRVs[SRVs.size() - 1]);

		return SRVs.size() - 1;
	}

	size_t addBufferUAV(ID3D11Buffer* buffer)
	{
		UAVs.resize(UAVs.size() + 1);

		D3D11_BUFFER_DESC descBuf = {};
		buffer->GetDesc(&descBuf);

		D3D11_UNORDERED_ACCESS_VIEW_DESC desc = {};
		desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		desc.Buffer.FirstElement = 0;

		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.Buffer.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;
		desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;

		GPU.getDevice()->CreateUnorderedAccessView(buffer, &desc, &UAVs[UAVs.size() - 1]);

		return UAVs.size() - 1;
	}

	void run(UINT X, UINT Y, UINT Z)
	{
		ID3D11DeviceContext* ctx = NULL;
		GPU.getDevice()->GetImmediateContext(&ctx);

		ctx->CSSetShader(shader, nullptr, 0);
		ctx->CSSetShaderResources(0, UINT(SRVs.size()), SRVs.data());
		const UINT zero = 0;
		ctx->CSSetUnorderedAccessViews(0, UINT(UAVs.size()), UAVs.data(), &zero);
		
		ctx->Dispatch(X, Y, Z);

		ctx->CSSetShader(nullptr, nullptr, 0);

		ID3D11UnorderedAccessView* ppUAViewnullptr[1] = { nullptr };
		ctx->CSSetUnorderedAccessViews(0, 1, ppUAViewnullptr, nullptr);

		ID3D11ShaderResourceView* ppSRVnullptr[2] = { nullptr, nullptr };
		ctx->CSSetShaderResources(0, 2, ppSRVnullptr);
	}

	void* beginReadingBufferData(ID3D11Buffer* buffer)
	{
		ID3D11DeviceContext* ctx = NULL;
		GPU.getDevice()->GetImmediateContext(&ctx);

		ID3D11Buffer* debugbuf = CreateAndCopyToDebugBuf(GPU.getDevice(), ctx, buffer);
		D3D11_MAPPED_SUBRESOURCE MappedResource;
		ctx->Map(debugbuf, 0, D3D11_MAP_READ, 0, &MappedResource);

		return MappedResource.pData;
	}

	void endReadingBufferData(ID3D11Buffer* buffer)
	{
		ID3D11DeviceContext* ctx = NULL;
		GPU.getDevice()->GetImmediateContext(&ctx);

		ctx->Unmap(buffer, 0);
	}
};