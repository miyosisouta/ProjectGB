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

        // CPU側のベンドソースに登録する（座標・曲げ量・半径・持続時間・回復速度・経過時間・有効フラグ）
        BendSource& src   = sources_[i];
        src.position      = pos;                  // 座標
        src.force         = params.force;         // 最大曲げ量
        src.radius        = params.radius;        // 影響半径
        src.duration      = params.duration;      // 持続時間
        src.recoverySpeed = params.recoverySpeed; // 回復速度
        src.elapsed       = 0.0f;                 // 経過時間をリセット
        src.active        = true;                 // 有効化

        // GPUへ送るデータにも同じ内容をミラーする
        GrassBendSourceGPU& gpu = gpuData_[i];
        gpu.position[0]   = pos.x;                // 座標X
        gpu.position[1]   = pos.y;                // 座標Y
        gpu.position[2]   = pos.z;                // 座標Z
        gpu.force         = params.force;         // 最大曲げ量
        gpu.radius        = params.radius;        // 影響半径
        gpu.elapsed       = 0.0f;                 // 経過時間
        gpu.duration      = params.duration;      // 持続時間
        gpu.recoverySpeed = params.recoverySpeed; // 回復速度

        bendSourceSB_.Update(gpuData_);  // 即座にGPUへ反映
        return;
    }
    // 全スロットが埋まっている場合は無視
}


void GrassBendManager::SetSource(int slot, const Vector3& pos, const AttackParams& params)
{
    if (slot < 0 || slot >= MAX_BEND_SOURCES) return;

    // CPU側のベンドソースを指定スロットへ直接書き込む（座標・曲げ量・半径・持続時間・回復速度・経過時間・有効フラグ）
    BendSource& src   = sources_[slot];
    src.position      = pos;                  // 座標
    src.force         = params.force;         // 最大曲げ量
    src.radius        = params.radius;        // 影響半径
    src.elapsed       = 0.0f;   // 毎フレームリセット → 呼ぶ限り全力維持
    src.duration      = params.duration;      // 持続時間
    src.recoverySpeed = params.recoverySpeed; // 回復速度
    src.active        = true;                 // 有効化

    // GPUへ送るデータにも同じ内容をミラーする
    GrassBendSourceGPU& gpu = gpuData_[slot];
    gpu.position[0]   = pos.x;                // 座標X
    gpu.position[1]   = pos.y;                // 座標Y
    gpu.position[2]   = pos.z;                // 座標Z
    gpu.force         = params.force;         // 最大曲げ量
    gpu.radius        = params.radius;        // 影響半径
    gpu.elapsed       = 0.0f;                 // 経過時間
    gpu.duration      = params.duration;      // 持続時間
    gpu.recoverySpeed = params.recoverySpeed; // 回復速度

    bendSourceSB_.Update(gpuData_);
}
