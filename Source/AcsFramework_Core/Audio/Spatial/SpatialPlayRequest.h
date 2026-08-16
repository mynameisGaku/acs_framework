// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/**
 * 「この場所でこれを鳴らしたい」という頼み 1 件。
 *
 * @details
 * 距離による小ささは鳴らす側が計算するので、頼む側は**元の音量**だけを決めればよい。
 * 位置は世界座標。速度は将来の用途 (ドップラー) のために持つが、いまは使われない。
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

	/** 鳴らせる形かを返す。 */
	bool IsValid() const noexcept { return !AssetPath.IsEmpty() && BaseVolume > 0.0f; }
};
