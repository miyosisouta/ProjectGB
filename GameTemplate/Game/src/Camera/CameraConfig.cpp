/**
 * CameraConfig.cpp
 * カメラ設定のシングルトン実装
 */
#include "stdafx.h"
#include "CameraConfig.h"
#include "src/Core/ParameterManager.h"


CameraConfig* CameraConfig::instance_ = nullptr;


void CameraConfig::Initialize()
{
    const auto* param = ParameterManager::Get().GetParameter<MasterBattleCommonParameter>(0);

    // JSONの値をリセット先の初期値として保持（感度・距離）
    defaultSensitivity_ = param->cameraParam.sensitivity; // 感度の初期値
    defaultDistance_    = param->cameraParam.distance;    // 距離の初期値

    // 現在値も初期値で埋める（感度・距離・反転）
    sensitivity_ = defaultSensitivity_;             // 感度
    distance_    = defaultDistance_;                // 距離
    invert_      = param->cameraParam.invert;       // 反転設定
}
