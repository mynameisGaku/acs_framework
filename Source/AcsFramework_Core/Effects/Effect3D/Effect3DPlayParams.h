// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/** 3D エフェクトを 1 つ再生するときの明示的な指定。 */
struct FEffect3DPlayParams
{
	/** world 上の再生位置。 */
	FVec3 Position{ 0.0f, 0.0f, 0.0f };

	/** 軸ごとの向き。ゲーム側で扱いやすい度単位。 */
	FVec3 RotationDeg{ 0.0f, 0.0f, 0.0f };

	/** 軸ごとの倍率。負値は鏡写しとして許可する。 */
	FVec3 Scale{ 1.0f, 1.0f, 1.0f };

	/** 再生速度。1 が素材どおり、0 以下は無効。 */
	f32 Speed = 1.0f;

	/** 素材の先頭から飛ばすフレーム数。 */
	i32 StartFrame = 0;

	/**
	 * 位置だけを指定した、素材どおりの再生指定を作る。
	 *
	 * @param WorldPosition world 上の再生位置。
	 * @return 等倍・通常速度の指定。
	 */
	static FEffect3DPlayParams At( FVec3 WorldPosition ) noexcept;

	/**
	 * 見えない値や計算を壊す値が含まれないか確かめる。
	 *
	 * @return 位置・向き・倍率・速度が有限、各倍率が 0 でなく、速度が正で、開始位置が 0 以上なら true。
	 */
	bool IsValid() const noexcept;
};
