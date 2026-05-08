#pragma once
struct DitherCBData
{
    Vector3 playerWorldPos = Vector3::Zero;  // 12バイト
    float   ditherNear = 100.0f;         // 4バイト
    Vector3 cameraWorldPos = Vector3::Zero;  // 12バイト
    float   ditherFar = 275.0f;         // 4バイト
    float   isEnable = 0.0f;           // 4バイト
    float   padding0 = 0.0f;           // 4バイト
    float   padding1 = 0.0f;           // 4バイト
    float   padding2 = 0.0f;           // 4バイト
};
extern DitherCBData g_ditherCBData;