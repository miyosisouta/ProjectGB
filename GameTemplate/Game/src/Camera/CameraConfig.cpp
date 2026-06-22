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

    defaultSensitivity_ = param->cameraParam.sensitivity;
    defaultDistance_    = param->cameraParam.distance;

    sensitivity_ = defaultSensitivity_;
    distance_    = defaultDistance_;
    invert_      = param->cameraParam.invert;
}
