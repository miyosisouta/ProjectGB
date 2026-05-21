#include "stdafx.h"
#include "CameraManager.h"


CameraManager* CameraManager::instance_ = nullptr;


CameraManager::CameraManager()
{
}


CameraManager::~CameraManager()
{
}


void CameraManager::Setup(nsK2EngineLow::Camera* engineCamera)
{
    engineCamera_ = engineCamera;
}


void CameraManager::Register(const uint32_t nameHash, RefCameraController controller)
{
    if (controllers_.find(nameHash) != controllers_.end()) {
        // すでに登録されている場合は無視
        return;
    }
    controllers_[nameHash] = controller;
}


void CameraManager::Unregister(const uint32_t nameHash)
{
    if (controllers_.find(nameHash) == controllers_.end()) {
        // 登録されていない場合は無視
        return;
    }
    controllers_.erase(nameHash);
}


void CameraManager::SwitchCamera(const uint32_t nameHash, const float blendTime)
{
    auto it = controllers_.find(nameHash);
    if (it == controllers_.end()) return;
    SwitchCamera(it->second, blendTime);
}


void CameraManager::SwitchCamera(RefCameraController controller, const float blendTime)
{
#if defined(_DEBUG)
    prev_ = current_;
#endif
    if (!current_ || blendTime <= 0.0f) {
        // 即時切り替え
        current_ = controller;
        current_->OnEnter();
        next_ = nullptr;
        isBlending_ = false;
    }
    else {
        // ブレンド開始
        if (current_ != controller) {
            next_ = controller;
            next_->OnEnter();

            // 現在のエンジンカメラの状態をブレンド開始地点として保存
            blendStartData_.position = engineCamera_->GetPosition();
            blendStartData_.target = engineCamera_->GetTarget();
            blendStartData_.up = engineCamera_->GetUp();
            blendStartData_.fov = engineCamera_->GetViewAngle();

            blendDuration_ = blendTime;
            blendTimer_ = 0.0f;
            isBlending_ = true;
        }
    }
}


void CameraManager::Update(const float deltaTime)
{
    if (engineCamera_ == nullptr) {
        return;
    }

    CameraData applyData;

    // 各コントローラーのUpdate
    if (current_) {
        current_->Update();
    }
    if (next_) {
        next_->Update();
    }

    // 情報の決定
    if (isBlending_ && next_) {
        blendTimer_ += deltaTime;
        const float t = blendTimer_ / blendDuration_;
        // 保管終了
        if (t >= 1.0f) {
            isBlending_ = false;
            current_ = next_;
            next_ = nullptr;
            applyData = current_->GetCameraData();
        }
        else {
            applyData = CameraData::Lerp(t, blendStartData_, next_->GetCameraData());
        }
    }
    else if (current_) {
        applyData = current_->GetCameraData();
    }

    // 3. エンジンカメラへの反映
    engineCamera_->SetPosition(applyData.position);
    engineCamera_->SetTarget(applyData.target);
}