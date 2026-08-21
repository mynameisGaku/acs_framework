// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::game;

/** 聞こえないとみなし、voiceを消費しない音量の境界。 */
inline constexpr f32 kSpatialSfxInaudibleVolume = 0.001f;

/** 位置と基準音量から決まる、1回の3D効果音の出力値。 */
struct FSpatialSfxMix
{
	/** 距離減衰と基準音量を合成した音量。 */
	f32 Volume = 0.0f;

	/** 聴く向きを基準にした左右位置。-1が左、+1が右。 */
	f32 Pan = 0.0f;

	/** voiceを使って鳴らすだけの音量が残っているか。 */
	bool bAudible = false;
};

/**
 * 登録済み3D音源から、出力へ渡す音量と左右位置を決める。
 *
 * @param Spatial 聴く位置と音源を保持する状態。
 * @param SourceId 登録済み音源の番号。
 * @param BaseVolume 距離減衰前の基準音量。
 * @param InaudibleVolume これ以下なら鳴らさない音量。
 * @return 同じ入力から常に同じ値になる出力値。
 */
FSpatialSfxMix ComputeSpatialSfxMix( const CSpatialAudio& Spatial, u32 SourceId, f32 BaseVolume, f32 InaudibleVolume = kSpatialSfxInaudibleVolume ) noexcept;
