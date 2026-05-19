/*!
 * @brief 地面スプラットマップGBufferシェーダー
 *
 * 3種類の地面テクスチャをRGBスプラットマップでブレンドして出力する。
 *
 * テクスチャバインド:
 *   t0  : モデルマテリアルのアルベド (未使用。スプラットブレンドで置換)
 *   t1  : モデルマテリアルの法線マップ
 *   t2  : モデルマテリアルのスペキュラマップ
 *   t11 : スプラットマップ (R=草, G=岩土, B=腐葉土)  ← SetExtraGBufferTextureSRV(0, ...)
 *   t12 : 草テクスチャ (kusa_ground)                  ← SetExtraGBufferTextureSRV(1, ...)
 *   t13 : 岩土テクスチャ (tuthi_ground)               ← SetExtraGBufferTextureSRV(2, ...)
 *   t14 : 腐葉土テクスチャ (fuyoudo_ground)           ← SetExtraGBufferTextureSRV(3, ...)
 *
 * 既存 grass.fx (t11=StructuredBuffer) とは別シェーダーなので衝突しない。
 * PSエントリーは ModelRender のデフォルト名 (PSMain / PSMainShadowReciever) に合わせる。
 */

////////////////////////////////////////////////
// 構造体
////////////////////////////////////////////////

struct SPSIn
{
    float4 pos      : SV_POSITION;
    float3 normal   : NORMAL;
    float3 tangent  : TANGENT;
    float3 biNormal : BINORMAL;
    float2 uv       : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
};

struct SPSOut
{
    float4 albedo              : SV_Target0;
    float4 normal              : SV_Target1;
    float4 metaricShadowSmooth : SV_Target2;
};

///////////////////////////////////////
// 頂点シェーダー共通処理 (SVSIn, mView, mProj 等を提供)
// SPSIn を先に定義してから include する必要がある。
///////////////////////////////////////
#include "ModelVSCommon.h"

///////////////////////////////////////
// ディザリング用定数バッファ (b1)
// ModelRender が m_localDitherCBData を expandConstantBuffer として渡す。
// 地面は ditherAlpha=1.0 のため実質無効だが、構造を合わせるために宣言する。
///////////////////////////////////////
cbuffer DitherCB : register(b1)
{
    float3 g_playerPos;               // プレイヤー座標
    float  g_ditherNear;              // ディザ開始距離
    float3 g_cameraPos;               // カメラ座標
    float  g_ditherFar;               // ディザ終了距離
    float  g_ditherdValue;            // ディザ有効フラグ
    float  g_ditherAlpha;             // 透明度 (地面=1.0)
    float  g_grassLodDitherStartDist; // 草LODディザ開始距離 (未使用)
    float  g_grassLodDitherAlphaMin;  // 草LODディザ最小α (未使用)
    float  g_isGrassLodDither;        // 草LODディザ有効フラグ (未使用)
    float  g_grassLodDitherEndDist;   // 草LODディザ終了距離 (未使用)
    float2 g_pad2;
};

// スプラット専用カラーパラメータ (b2 = SplatColorCBData)
// ModelRender::SetSplatColorParams() で C++ 側から制御する
cbuffer SplatColorCB : register(b2)
{
    float g_splatParam0;    // テクスチャ0 (草) 明るさ倍率
    float g_splatParam1;    // テクスチャ1 (岩土) 明るさ倍率
    float g_splatParam2;    // テクスチャ2 (腐葉土) 明るさ倍率
    float g_splatParam3;    // グローバルHSV明るさ倍率
    float g_splatSaturation;// 彩度倍率 (1.0=変化なし, 1.5=鮮やか, 0.5=くすみ)
    float3 _pad;
};

///////////////////////////////////////
// テクスチャ / サンプラー
///////////////////////////////////////

// モデルマテリアルテクスチャ (TKMマテリアルから自動バインド)
Texture2D<float4> g_albedo   : register(t0); // アルベド (スプラットブレンドに置換するため未使用)
Texture2D<float4> g_normal   : register(t1); // 法線マップ (地面メッシュのノーマルマップ)
Texture2D<float4> g_spacular : register(t2); // スペキュラマップ
sampler g_sampler : register(s0);

// スプラット専用テクスチャ (ModelRender::SetExtraGBufferTextureSRV で渡す)
// index=0 → m_expandShaderResoruceView[1] → t11
// index=1 → m_expandShaderResoruceView[2] → t12  ...以下同様
Texture2D<float4> g_splatMap   : register(t11); // スプラットマップ
Texture2D<float4> g_kusaTex    : register(t12); // 草テクスチャ
Texture2D<float4> g_tuthiTex   : register(t13); // 岩土テクスチャ
Texture2D<float4> g_fuyoudoTex : register(t14); // 腐葉土テクスチャ

// UV タイリング係数 (マクロで変更可能。デフォルト 10.0)
#define UV_TILE 1.0f

////////////////////////////////////////////////
// VSMainCore の定義
// ModelVSCommon.h が前方宣言しているため定義が必要。
// 地面は標準変換のみ。
////////////////////////////////////////////////
SPSIn VSMainCore(SVSIn vsIn, float4x4 mWorldLocal, uniform bool isUsePreComputedVertexBuffer)
{
    SPSIn psIn;
    float4 worldPos = CalcVertexPositionInWorldSpace(vsIn.pos, mWorldLocal, isUsePreComputedVertexBuffer);
    psIn.worldPos = worldPos.xyz;
    psIn.pos      = mul(mView, worldPos);
    psIn.pos      = mul(mProj, psIn.pos);
    CalcVertexNormalTangentBiNormalInWorldSpace(
        psIn.normal, psIn.tangent, psIn.biNormal,
        mWorldLocal, vsIn.normal, vsIn.tangent, vsIn.biNormal,
        isUsePreComputedVertexBuffer
    );
    psIn.uv = vsIn.uv;
    return psIn;
}

////////////////////////////////////////////////
// カラー補正ユーティリティ (分岐なし)
////////////////////////////////////////////////

// 彩度調整: 輝度との線形補間で実現。if/else 不要でGPU friendlyな実装。
float3 AdjustSaturation(float3 color, float saturation)
{
    // BT.709 輝度係数
    float luma = dot(color, float3(0.2126f, 0.7152f, 0.0722f));
    return lerp(float3(luma, luma, luma), color, saturation);
}

////////////////////////////////////////////////
// ピクセルシェーダー
////////////////////////////////////////////////

float3 GetNormalFromNormalMap(float3 normal, float3 tangent, float3 biNormal, float2 uv)
{
    float3 binSpaceNormal = g_normal.SampleLevel(g_sampler, uv, 0.0f).xyz;
    binSpaceNormal        = (binSpaceNormal * 2.0f) - 1.0f;
    return tangent * binSpaceNormal.x + biNormal * binSpaceNormal.y + normal * binSpaceNormal.z;
}

SPSOut PSMainCore(SPSIn psIn, int isShadowReciever)
{
    static const float BayerMatrix[16] =
    {
         0.0f / 16.0f,  8.0f / 16.0f,  2.0f / 16.0f, 10.0f / 16.0f,
        12.0f / 16.0f,  4.0f / 16.0f, 14.0f / 16.0f,  6.0f / 16.0f,
         3.0f / 16.0f, 11.0f / 16.0f,  1.0f / 16.0f,  9.0f / 16.0f,
        15.0f / 16.0f,  7.0f / 16.0f, 13.0f / 16.0f,  5.0f / 16.0f
    };

    // ディザリング (地面は ditherAlpha=1.0 なので ditherStrength=0 になり実質無効)
    if (g_ditherdValue > 0.5f)
    {
        float dist         = length(g_cameraPos - psIn.worldPos);
        float distStrength = 1.0f - saturate(
            (dist - g_ditherNear) / max(g_ditherFar - g_ditherNear, 0.001f)
        );
        float ditherStrength = distStrength * (1.0f - g_ditherAlpha);

        uint px = (uint)psIn.pos.x % 4;
        uint py = (uint)psIn.pos.y % 4;
        if (BayerMatrix[py * 4 + px] < ditherStrength)
            discard;
    }

    // スプラットマップでテクスチャをブレンド
    // splat.r = 草の重み, splat.g = 岩土の重み, splat.b = 腐葉土の重み
    float3 splat  = g_splatMap.Sample(g_sampler, psIn.uv).rgb;
    float  total  = splat.r + splat.g + splat.b + 0.0001f; // ゼロ除算防止

    float2 tiledUV  = psIn.uv * UV_TILE;
    // 各テクスチャをサンプリングし、splatParam0..2 で個別の明るさを調整
    float3 kusa     = g_kusaTex.Sample(g_sampler, tiledUV).rgb    * g_splatParam0;
    float3 tuthi    = g_tuthiTex.Sample(g_sampler, tiledUV).rgb   * g_splatParam1;
    float3 fuyoudo  = g_fuyoudoTex.Sample(g_sampler, tiledUV).rgb * g_splatParam2;

    float3 blended = (kusa * splat.r + tuthi * splat.g + fuyoudo * splat.b) / total;

    // 彩度補正 (輝度lerp, 分岐なし)
    blended = AdjustSaturation(blended, g_splatSaturation);
    // 明るさ補正 (乗算, 分岐なし)
    blended = saturate(blended * g_splatParam3);

    SPSOut psOut;
    psOut.albedo.rgb           = blended;
    psOut.albedo.a             = psIn.pos.z; // 深度値 (GBuffer規約)
    psOut.normal.xyz           = normalize(psIn.normal);
    psOut.normal.w             = 1.0f;
    // 地面は非金属なので metallic(r)=0 固定。smoothness(b) は微量だけ残して明るさを保つ。
    // Specular.DDS の値は使わない（高値だと鏡面反射が発生するため）。
    psOut.metaricShadowSmooth   = float4(0.0f, 0.0f, 0.15f, 0.0f);
    psOut.metaricShadowSmooth.g = 255.0f * isShadowReciever;
    return psOut;
}

// ModelRender のデフォルト PS エントリー名に合わせる
// (SetPSEntryOverride は ModelRender に存在しないため名前を固定)
SPSOut PSMain(SPSIn psIn)
{
    return PSMainCore(psIn, 0);
}
SPSOut PSMainShadowReciever(SPSIn psIn)
{
    return PSMainCore(psIn, 1);
}
