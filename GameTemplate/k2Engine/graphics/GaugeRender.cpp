#include "k2EnginePreCompile.h"
#include "GaugeRender.h"
#include "RenderingEngine.h"

namespace nsK2Engine {

namespace {
    // 12時スタートになるオフセット（-90度 = -π/2）
    constexpr float DEFAULT_ROTATION_ANGLE = Math::PI / 2.0f;
}

void GaugeRender::Init(float w, float h)
{
    // -------------------------------------------------------
    // ゲージ用定数バッファのデフォルト値
    // -------------------------------------------------------
    m_gaugeCb.startProgress  = 0.0f;                            // 12時から
    m_gaugeCb.endProgress    = 1.0f;                            // 一周
    m_gaugeCb.innerRadius    = 0.35f;                           // 内径
    m_gaugeCb.outerRadius    = 0.45f;                           // 外径（太さ = 0.45 - 0.35 = 0.10）
    m_gaugeCb.rotationAngle  = DEFAULT_ROTATION_ANGLE;          // 12時スタート
    m_gaugeCb.padding0       = 0.0f;
    m_gaugeCb.padding1       = 0.0f;
    m_gaugeCb.padding2       = 0.0f;
    m_gaugeCb.gaugeColor     = Vector4(0.2f, 0.8f, 0.2f, 1.0f); // 緑
    m_gaugeCb.bgColor        = Vector4(0.2f, 0.2f, 0.2f, 0.8f); // 暗いグレー

    // -------------------------------------------------------
    // SpriteInitData の構築
    // -------------------------------------------------------
    SpriteInitData initData;
    initData.m_ddsFilePath[0]           = "Assets/modelData/preset/miniRed.png";
    initData.m_fxFilePath               = "Assets/shader/gauge.fx";
    initData.m_width                    = static_cast<UINT>(w);
    initData.m_height                   = static_cast<UINT>(h);
    initData.m_alphaBlendMode           = AlphaBlendMode_Trans;

    // ゲージ用定数バッファを b1 スロットに渡す
    initData.m_expandConstantBuffer     = &m_gaugeCb;
    initData.m_expandConstantBufferSize = sizeof(GaugeConstantBuffer);

    m_sprite.Init(initData);
}

void GaugeRender::Update()
{
    // スプライトのワールド行列を更新
    m_sprite.Update(m_position, m_rotation, m_scale, m_pivot);
}

void GaugeRender::Draw(RenderContext& rc)
{
    g_renderingEngine->AddRenderObject(this);
}

} // namespace nsK2Engine
