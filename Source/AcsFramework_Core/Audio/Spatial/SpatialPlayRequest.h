// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::game;

/**
 * 「この場所でこれを鳴らしたい」という頼み 1 件。
 *
 * @details
 * 距離による小ささと左右位置は鳴らす側が計算する。位置は世界座標。速度は将来の用途
 * (ドップラー) のために持つが、いまは使われない。
 */
struct FSpatialPlayRequest
{
	/** 鳴らすもののパス。 */
	FString AssetPath;

	/** 鳴らす場所 (世界座標)。 */
	FVec3 Position = FVec3::Zero();

	/** 動いている速さ。 */
	FVec3 Velocity = FVec3::Zero();

	/** 距離を考えない元の音量。 */
	f32 BaseVolume = 1.0f;

	/** この距離以上では鳴らさない。 */
	f32 MaxDistance = 20.0f;

	/** 距離から音量を求める曲線。 */
	EAttenuationCurve AttenuationCurve = EAttenuationCurve::Linear;

	/** 再生速度の倍率。1で素材どおり。 */
	f32 Pitch = 1.0f;

	/** 鳴らせる形かを返す。 */
	bool IsValid() const noexcept
	{
		const bool bValidCurve = AttenuationCurve >= EAttenuationCurve::Linear && AttenuationCurve <= EAttenuationCurve::Exponential;
		return !AssetPath.IsEmpty() && std::isfinite( BaseVolume ) && BaseVolume > 0.0f && std::isfinite( MaxDistance ) && MaxDistance > 0.0f && bValidCurve && std::isfinite( Pitch ) && Pitch > 0.0f;
	}
};
