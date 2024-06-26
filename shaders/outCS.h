#if 0
//
// Generated by Microsoft (R) HLSL Shader Compiler 9.29.952.3111
//
//
//   fxc /T cs_5_0 /Fh outCS.h /ECSMain computeShader_DELETE.hlsl
//
//
// Buffer Definitions: 
//
// Resource bind info for Buffer0
// {
//
//   struct BufType
//   {
//       
//       float x;                       // Offset:    0
//       float y;                       // Offset:    4
//       float z;                       // Offset:    8
//       int color;                     // Offset:   12
//
//   } $Element;                        // Offset:    0 Size:    16
//
// }
//
// Resource bind info for Buffer1
// {
//
//   float $Element;                    // Offset:    0 Size:     4
//
// }
//
// Resource bind info for BufferOut
// {
//
//   struct BufType
//   {
//       
//       float x;                       // Offset:    0
//       float y;                       // Offset:    4
//       float z;                       // Offset:    8
//       int color;                     // Offset:   12
//
//   } $Element;                        // Offset:    0 Size:    16
//
// }
//
//
// Resource Bindings:
//
// Name                                 Type  Format         Dim Slot Elements
// ------------------------------ ---------- ------- ----------- ---- --------
// Buffer0                           texture  struct         r/o    0        1
// Buffer1                           texture  struct         r/o    1        1
// BufferOut                             UAV  struct      append    0        1
//
//
//
// Input signature:
//
// Name                 Index   Mask Register SysValue Format   Used
// -------------------- ----- ------ -------- -------- ------ ------
// no Input
//
// Output signature:
//
// Name                 Index   Mask Register SysValue Format   Used
// -------------------- ----- ------ -------- -------- ------ ------
// no Output
cs_5_0
dcl_globalFlags refactoringAllowed
dcl_resource_structured t0, 16 
dcl_resource_structured t1, 4 
dcl_uav_structured u0, 16
dcl_input vThreadID.x
dcl_temps 2
dcl_thread_group 64, 1, 1
ld_structured_indexable(structured_buffer, stride=16)(mixed,mixed,mixed,mixed) r0.xyz, vThreadID.x, l(0), t0.xyzx
ld_structured_indexable(structured_buffer, stride=4)(mixed,mixed,mixed,mixed) r1.x, l(0), l(0), t1.xxxx
ld_structured_indexable(structured_buffer, stride=4)(mixed,mixed,mixed,mixed) r1.y, l(1), l(0), t1.xxxx
ld_structured_indexable(structured_buffer, stride=4)(mixed,mixed,mixed,mixed) r1.z, l(2), l(0), t1.xxxx
add r0.xyz, r0.xyzx, -r1.xyzx
dp3 r0.x, r0.xyzx, r0.xyzx
sqrt r0.x, r0.x
ld_structured_indexable(structured_buffer, stride=4)(mixed,mixed,mixed,mixed) r0.y, l(3), l(0), t1.xxxx
lt r0.x, r0.y, r0.x
if_nz r0.x
  ld_structured_indexable(structured_buffer, stride=16)(mixed,mixed,mixed,mixed) r0.xyzw, vThreadID.x, l(0), t0.xyzw
  imm_atomic_alloc r1.x, u0
  store_structured u0.xyzw, r1.x, l(0), r0.xyzw
endif 
ret 
// Approximately 15 instruction slots used
#endif

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
