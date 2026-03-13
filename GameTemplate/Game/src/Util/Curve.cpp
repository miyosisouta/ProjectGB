/**
 * UIBase.cpp
 * UIの基本的な処理をするクラス群
 */
#include "stdafx.h"
#include "UIBase.h"


 // ============================================
 // 画像を使うUI関連
 // ============================================


UIImage::UIImage()
{
}


UIImage::~UIImage()
{
}


void UIImage::Update()
{
}


void UIImage::Render(RenderContext& rc)
{
}




/************************************/


UIGauge::UIGauge()
{
}


UIGauge::~UIGauge()
{
}


void UIGauge::Update()
{
	m_transform.UpdateTransform();
	m_spriteRender.SetPosition(m_transform.m_position);
	m_spriteRender.SetScale(m_transform.m_scale);
	m_spriteRender.SetRotation(m_transform.m_rotation);
	m_spriteRender.Update();
}


void UIGauge::Render(RenderContext& rc)
{
	m_spriteRender.Draw(rc);
}


void UIGauge::Initialize(const char* assetName, const float width, const float height, const Vector3& position, const Vector3& scale, const Quaternion& rotation)
{
	m_transform.m_localPosition = position;
	m_transform.m_localScale = scale;
	m_transform.m_localRotation = rotation;

	m_spriteRender.Init(assetName, width, height);
	m_spriteRender.SetPosition(position);
	m_spriteRender.SetScale(scale);
	m_spriteRender.SetRotation(rotation);
	m_spriteRender.Update();
}




/************************************/


UIIcon::UIIcon()
{
}


UIIcon::~UIIcon()
{
}

void UIIcon::Update()
{
	//// @too for test
	//for (auto& animation : m_uiAnimationList)
	//{
	//	//UpdateAnimation();
	//	animation->Update();
	//}

	UpdateAnimation();

	m_spriteRender.SetMulColor(m_color);
	m_transform.UpdateTransform();
	m_spriteRender.SetPosition(m_transform.m_position);
	m_spriteRender.SetScale(m_transform.m_scale);
	m_spriteRender.SetRotation(m_transform.m_rotation);
	m_spriteRender.Update();

	//for (auto& animation : m_uiAnimationList) {
	//	animation->Update();
	//}
}


void UIIcon::Render(RenderContext& rc)
{
	if (isDraw) {
		m_spriteRender.Draw(rc);
	}
}


void UIIcon::Initialize(const char* assetName, const float width, const float height)
{
	m_spriteRender.Init(assetName, width, height);
}




/********************************/


UIDigit::UIDigit()
{
}


UIDigit::~UIDigit()
{
}


void UIDigit::Update()
{
	if (m_number != m_requestNumber) {
		m_number = m_requestNumber;
		for (int i = 0; i < m_digit; ++i) {
			UpdateNumber(i+1, m_number);
		}
	}

	UpdateAnimation();

	m_transform.UpdateTransform();
	for (int i = 0; i < m_renderList.size(); ++i)
	{
		auto* spriteRender = m_renderList[i];
		UpdatePosition(i);
		spriteRender->SetScale(m_transform.m_scale);
		spriteRender->SetRotation(m_transform.m_rotation);
		spriteRender->SetMulColor(m_color);
		spriteRender->Update();
	}

	/*for (auto& animation : m_uiAnimationList) {
		animation->Update();
	}*/
}


void UIDigit::Render(RenderContext& rc)
{
	if (!isDraw)
	{
		return;
	}
	for (SpriteRender* spriteRender : m_renderList)
	{
		spriteRender->Draw(rc);
	}
}

void UIDigit::Initialize(const char* assetName, const int digit, const int number, const float widht, const float height, const Vector3& position, const Vector3& scale, const Quaternion& rotation)
{
	m_assetPath = assetName;
	m_digit = digit;
	m_number = number;
	w = widht;
	h = height;

	m_transform.m_localPosition = position;
	m_transform.m_localScale = scale;
	m_transform.m_localRotation = rotation;

	for (int i = 0; i < digit; i++)
	{
		SpriteRender* spriteRender = new SpriteRender;
		spriteRender->Init(assetName, widht, height);
		spriteRender->SetPosition(position);
		spriteRender->SetScale(scale);
		spriteRender->SetRotation(rotation);
		m_renderList.push_back(spriteRender);
		UpdateNumber(i+1, m_number);	// 桁なので＋１する
	}
}


void UIDigit::UpdateNumber(const int targetDigit, const int number)
{
	// NOTE: targetDigitは1以上の値になっている
	K2_ASSERT(targetDigit >= 1, "桁指定が間違えています。\n");

	// いらない
	const int targetRenderIndex = targetDigit - 1;
	SpriteRender* nextRender = nullptr;
	// 次のやつをつくる
	if (targetRenderIndex < m_renderList.size()) {
		nextRender = m_renderList[targetRenderIndex];
	} else {
		nextRender = new SpriteRender();
		m_renderList.push_back(nextRender);
	}

	// 対象の桁の数字
	const int targetDigitNumber = GetDigit(targetDigit);
	std::string assetNname = m_assetPath + "/0.dds";
	// 数字の部分を桁の数字で変える
	assetNname[assetNname.size() - 5] = '0' + targetDigitNumber;
	nextRender->Init(assetNname.c_str(), w, h);
}


void UIDigit::UpdatePosition(const int index)
{
	SpriteRender* render = m_renderList[index];
	Vector3 position = m_transform.m_position;
	position.x -= w * index;
	render->SetPosition(position);
}


int UIDigit::GetDigit(int digit)
{
	// NOTE: targetDigitは1以上の値になっている
	K2_ASSERT(digit >= 1, "桁指定が間違えています。\n");
	digit -= 1;
	int divisor = static_cast<int>(pow(10, digit));
	return (m_number / divisor) % 10;
}




/************************************/


UICanvas::UICanvas()
{
	m_uiList.clear();
}


UICanvas::~UICanvas()
{
	//for(auto* ui : m_uiList) {
	//	// トランスフォームの親子関係を解除
	//	m_transform.RemoveChild(&ui->m_transform);
	//	// キャンバス上にあるUIを削除
	//	delete ui;
	//	ui = nullptr;
	//}
	m_uiList.clear();
}


void UICanvas::Update()
{
	m_transform.UpdateTransform();

	for (auto& ui : m_uiList) {
		ui.second->Update();
	}
}

void UICanvas::Render(RenderContext& rc)
{
	for (auto& ui : m_uiList) {
		ui.second->Render(rc);
	}
}


