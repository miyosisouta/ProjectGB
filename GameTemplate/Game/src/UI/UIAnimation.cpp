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
		m_ui->color = v;
		});
}




/*******************************************************/


UIScaleAnimation::UIScaleAnimation()
{
	SetFunc([&](Vector3 s) {
		m_ui->transform.localScale = s;
		//m_ui->m_transform.UpdateTransform();
		});
}




/*******************************************************/


UITranslateAniamtion::UITranslateAniamtion()
{
	SetFunc([&](Vector3 s) {
		m_ui->transform.localPosition = s;
		//m_ui->m_transform.UpdateTransform();
		});
}




/*******************************************************/


UITranslateOffsetAnimation::UITranslateOffsetAnimation()
{
	SetFunc([&](Vector3 offset) {
		m_ui->transform.localPosition.Add(offset);
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
			m_ui->transform.localRotation.SetRotationDegZ(s);
		});
}