/**
 * GrassBendManager.cpp
 */

#include "stdafx.h"
#include "GrassBendManager.h"

GrassBendManager* GrassBendManager::instance_ = nullptr;


void GrassBendManager::Init()
{
    ZeroMemory(gpuData_, sizeof(gpuData_));

    bendSourceSB_.Init(
        sizeof(GrassBendSourceGPU),
        MAX_BEND_SOURCES,
        gpuData_
    );
}


void GrassBendManager::Update(float dt)
{
    for (int i = 0; i < MAX_BEND_SOURCES; i++)
    {
        BendSource& src = sources_[i];
        if (!src.active)
            continue;

        src.elapsed += dt;
        if (src.elapsed >= src.duration)
        {
            src.active  = false;
            gpuData_[i] = {};   // duration=0 でシェーダーが無効と判断する
        }
        else
        {
            gpuData_[i].elapsed = src.elapsed;
        }
    }

    bendSourceSB_.Update(gpuData_);
}


void GrassBendManager::AddSource(const Vector3& pos, const AttackParams& params)
{
    for (int i = 0; i < MAX_BEND_SOURCES; i++)
    {
        if (sources_[i].active)
            continue;

        BendSource& src   = sources_[i];
        src.position      = pos;
        src.force         = params.force;
        src.radius        = params.radius;
        src.duration      = params.duration;
        src.recoverySpeed = params.recoverySpeed;
        src.elapsed       = 0.0f;
        src.active        = true;

        GrassBendSourceGPU& gpu = gpuData_[i];
        gpu.position[0] = pos.x;
        gpu.position[1] = pos.y;
        gpu.position[2] = pos.z;
        gpu.force         = params.force;
        gpu.radius        = params.radius;
        gpu.elapsed       = 0.0f;
        gpu.duration      = params.duration;
        gpu.recoverySpeed = params.recoverySpeed;

        bendSourceSB_.Update(gpuData_);  // 即座にGPUへ反映
        return;
    }
    // 全スロットが埋まっている場合は無視
}


void GrassBendManager::SetSource(int slot, const Vector3& pos, const AttackParams& params)
{
    if (slot < 0 || slot >= MAX_BEND_SOURCES) return;

    BendSource& src   = sources_[slot];
    src.position      = pos;
    src.force         = params.force;
    src.radius        = params.radius;
    src.elapsed       = 0.0f;   // 毎フレームリセット → 呼ぶ限り全力維持
    src.duration      = params.duration;
    src.recoverySpeed = params.recoverySpeed;
    src.active        = true;

    GrassBendSourceGPU& gpu = gpuData_[slot];
    gpu.position[0]  = pos.x;
    gpu.position[1]  = pos.y;
    gpu.position[2]  = pos.z;
    gpu.force        = params.force;
    gpu.radius       = params.radius;
    gpu.elapsed      = 0.0f;
    gpu.duration     = params.duration;
    gpu.recoverySpeed= params.recoverySpeed;

    bendSourceSB_.Update(gpuData_);
}
