// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/**
 * 第三者視点キャラクターを1回進めるための明示入力。
 */
struct FThirdPersonCharacter3DInput
{
	/** xを画面右、yを画面奥とする移動操作量。 */
	FVec2 MoveAxes{};

	/** xを右回転、yを見下ろす回転とする視点操作量。 */
	FVec2 LookAxes{};

	/** 正でカメラを近づけ、負で遠ざける操作量。 */
	f32 ZoomAxis = 0.0f;

	/** 接地中のキャラクターへジャンプを要求するならtrue。 */
	bool bJumpRequested = false;

	/**
	 * 各操作量を既存のACS計算へ渡せるか返す。
	 *
	 * @return 全ての数値が有限ならtrue。
	 */
	bool IsValid() const noexcept;
};
