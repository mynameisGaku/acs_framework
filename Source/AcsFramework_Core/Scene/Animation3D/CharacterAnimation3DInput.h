// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/**
 * 1フレームのキャラクター移動からアニメーションを選ぶための入力。
 */
struct FCharacterAnimation3DInput
{
	/** 地面と平行な移動速度。向きは状態選択に不要なので長さだけを渡す。 */
	f32 HorizontalSpeed = 0.0f;

	/** 地面に立っているか。falseなら速度よりジャンプ状態を優先する。 */
	bool bGrounded = true;

	/**
	 * 状態選択へ渡せる入力か返す。
	 *
	 * @return 速度が有限で0以上ならtrue。
	 */
	bool IsValid() const noexcept;
};
