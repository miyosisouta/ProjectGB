#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include "k2EnginePreCompile.h"
using namespace nsK2EngineLow;
using namespace nsK2Engine;

#include <src/json/json.hpp>

#include "src/Types.h"			//!< タイプ
#include "src/Math/Transform.h"	//!< トランスフォーム
#include "src/Memory/Array.h"	//!< 配列
#include "src/Util/Crc32.h"
#include "src/Util/Curve.h"     //!< カーブ
#include "src/Util/TaskSchedulerSystem.h"
#include "src/Util/Selector.h"
#include "src/collision/GhostBody.h"
#include "src/collision/PhysicalBody.h"
#include "src/Effect/EffectManager.h"
#include "src/Sound/SoundManager.h"
#include "src/Battle/BattleManager.h"
#include "src/Core/ParameterManager.h"
#include "src/Core/KeyConfig.h"