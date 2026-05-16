/**
 * GrassObject.cpp
 */

#include "stdafx.h"
#include <fstream>
#include "GrassObject.h"
#include "GrassBendManager.h"
#include "../../k2Engine/graphics/DitherCBData.h"

/* LODオブジェクトのファイルパス */
const char* const GrassObject::LOD_MODEL_PATHS[GrassObject::LOD_COUNT] = {
    "Assets/Objects/Stage/Forest/ObjectData/grass.tkm",     
    "Assets/Objects/Stage/Forest/ObjectData/grassLOD1.tkm",
    "Assets/Objects/Stage/Forest/ObjectData/grassLOD2.tkm",
};


void GrassObject::Init(const char* jsonPath, GrassBendManager* bendMgr)
{
    // jsonファイルの読み込み
    std::ifstream ifs(jsonPath);
    if (!ifs.is_open()) return;

    nlohmann::json root;
    try { ifs >> root; }
    catch (...) { return; }

    if (!root.contains("grass")) return;

    const auto& arr = root["grass"];
    const int count = (int)arr.size();
    if (count == 0) return;

    // LODの処理
    for (int lod = 0; lod < LOD_COUNT; lod++)
    {
        if (lod < 2 && bendMgr != nullptr)
        {
            // シェーダーの設定
            renderer_[lod].SetGBufferFxOverride("Assets/shader/grass.fx");
            // 頂点シェーダーのエントリーポイントの上書き
            renderer_[lod].SetVSEntryOverride("VSGrass");
            // SRVの設定
            renderer_[lod].SetExtraGBufferSRV(0, &bendMgr->GetStructuredBuffer());
        }
        // モデルを出力とディザの初期のアルファ値の設定
        renderer_[lod].Init(LOD_MODEL_PATHS[lod], nullptr, 0, enModelUpAxisZ, true, count);
        renderer_[lod].SetDitherAlpha(1.0f);
    }

    // モデルのメッシュを計算
    templateBounds_.Compute(renderer_[0].GetModel());
    // メモリの事前確保
    transforms_.reserve(count);

    // jsonファイルから取得したトランスフォームをインスタンシングデータに設定
    for (int i = 0; i < count; i++)
    {
        const auto& e = arr[i];
        GrassTransform t;

        const auto& p = e["position"];
        t.position = Vector3(p[0].get<float>(), p[1].get<float>(), p[2].get<float>());

        const auto& r = e["rotation"];
        t.rotation = Quaternion(r[0].get<float>(), r[1].get<float>(), r[2].get<float>(), r[3].get<float>());

        const auto& s = e["scale"];
        t.scale = Vector3(s[0].get<float>(), s[1].get<float>(), s[2].get<float>());

        transforms_.push_back(t);

        renderer_[0].UpdateInstancingData(i, t.position, t.rotation, t.scale);
        for (int lod = 1; lod < LOD_COUNT; lod++)
            renderer_[lod].UpdateInstancingData(i, t.position, t.rotation, Vector3::Zero);
    }
}


void GrassObject::Update()
{
    if (transforms_.empty()) return;

    // フラスタムカリング
    Frustum frustum;
    // 行列で形を作る
    frustum.BuildFromViewProjectionMatrix(g_camera3D->GetViewProjectionMatrix());
    const Vector3 camPos = g_camera3D->GetPosition();

    // 2条する
    const float dist0Sq = LOD0_MAX_DIST * LOD0_MAX_DIST;
    const float dist1Sq = LOD1_MAX_DIST * LOD1_MAX_DIST;

    // トランスフォームの数だけ
    for (int i = 0; i < (int)transforms_.size(); i++)
    {
        const auto& t = transforms_[i];

        // メッシュ
        Bounds wb;
        wb.minPoint = templateBounds_.minPoint + t.position;
        wb.maxPoint = templateBounds_.maxPoint + t.position;
        
        // フラスタム外にある場合
        if (!frustum.IsVisible(wb))
        {
            for (int lod = 0; lod < LOD_COUNT; lod++)
                renderer_[lod].UpdateInstancingData(i, t.position, t.rotation, Vector3::Zero);
            continue;
        }

        // Y成分を無視したXZ距離でLOD選択
        const float distSq = Vector3(t.position.x - camPos.x, 0.0f, t.position.z - camPos.z).LengthSq();

        int activeLod;
        if (!isLodEnabled_ || distSq < dist0Sq) { activeLod = 0; }
        else if (distSq < dist1Sq)              { activeLod = 1; }
        else                                    { activeLod = 2; }

        for (int lod = 0; lod < LOD_COUNT; lod++)
        {
            renderer_[lod].UpdateInstancingData(
                i, t.position, t.rotation,
                (lod == activeLod) ? t.scale : Vector3::Zero
            );
        }
    }
}


void GrassObject::Draw(RenderContext& rc)
{
    if (transforms_.empty()) return;

    // LOD0/1: ディザ有効時はCBをセット、無効時は明示的に0
    if (isDitherEnabled_)
    {
        g_ditherCBData.isGrassLodDither        = 1.0f;
        g_ditherCBData.grassLodDitherStartDist = DITHER_START_DIST;
        g_ditherCBData.grassLodDitherEndDist   = LOD1_MAX_DIST;
        g_ditherCBData.grassLodDitherAlphaMin  = DITHER_ALPHA_MIN;
    }
    else
    {
        g_ditherCBData.isGrassLodDither = 0.0f;
    }

    renderer_[0].Draw(rc);
    renderer_[1].Draw(rc);

    // LOD2: 常にディザなし（無条件でフラグを落とす）
    g_ditherCBData.isGrassLodDither = 0.0f;
    renderer_[2].Draw(rc);
}
