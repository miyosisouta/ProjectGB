/**
 * UIAnimation.cpp
 * UIAnimationをアニメーションさせる機能群
 */
#include "stdafx.h"
#include "UIAnimation.h"
#include "UIBase.h"


UIColorAnimation::UIColorAnimation()
{
	SetFunc([&](Vector4 v) {
		ui_->color = v;
		});
}




/*******************************************************/


UIScaleAnimation::UIScaleAnimation()
{
	SetFunc([&](Vector3 s) {
		ui_->transform.localScale = s;
		//m_ui->m_transform.UpdateTransform();
		});
}




/*******************************************************/


UITranslateAniamtion::UITranslateAniamtion()
{
	SetFunc([&](Vector3 s) {
		ui_->transform.localPosition = s;
		//m_ui->m_transform.UpdateTransform();
		});
}




/*******************************************************/


UITranslateOffsetAnimation::UITranslateOffsetAnimation()
{
	SetFunc([&](Vector3 offset) {
		ui_->transform.localPosition.Add(offset);
		//m_ui->m_transform.UpdateTransform();
		});
}




/*******************************************************/


UIRotationAnimation::UIRotationAnimation()
{
	SetFunc([&](float s)
		{
			/*Quaternion q;
			q.SetRotationDegY(s);
			m_ui->m_transform.m_localRotation = q;
			m_ui->m_transform.UpdateTransform();*/
			ui_->transform.localRotation.SetRotationDegZ(s);
		});
}