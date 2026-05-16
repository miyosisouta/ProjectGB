#pragma once
struct DitherCBData
{
    Vector3 playerWorldPos = Vector3::Zero;      // 12バイト
    float   ditherNear = 100.0f;                 //  4バイト  →合計16
    Vector3 cameraWorldPos = Vector3::Zero;      // 12バイト
    float   ditherFar = 275.0f;                  //  4バイト  →合計32
    float   isEnable = 0.0f;                    //  4バイト
    float   ditherAlpha = 1.0f;                 //  4バイト  per-objectの透明度ディザ値(0.0=完全透明, 1.0=不透明)
    float   grassLodDitherStartDist = 0.0f;     //  4バイト  草LODディザ開始距離
    float   grassLodDitherAlphaMin  = 0.4f;     //  4バイト  草LODディザ最小アルファ  →合計48
    // 草LODディザ用 追加ブロック
    float   isGrassLodDither        = 0.0f;     //  4バイト  草LODディザ有効フラグ
    float   grassLodDitherEndDist   = 0.0f;     //  4バイト  草LODディザ終了距離
    float   padding1 = 0.0f;                    //  4バイト
    float   padding2 = 0.0f;                    //  4バイト  →合計64
};
extern DitherCBData g_ditherCBData;