// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Audio/Spatial/SpatialPlayRequest.h"

using namespace acs;
using namespace acs::game;

class CAudioSubsystem;

/**
 * 距離で小さくし、左右位置を付けた音を、鳴らす側へ流す係。
 *
 * @details
 * 純粋な出力値の決定は`ComputeSpatialSfxMix`へ任せ、ここはvoice開始とEngineへの反映だけを行う。
 */
class CSpatialSfxRouter
{
public:
	/**
	 * 1 件を鳴らす。
	 *
	 * @details 距離で小さくなりきって聞こえない場合は鳴らさない。
	 * @param Spatial 距離と向きを計算する側。
	 * @param Audio 鳴らす側。
	 * @param SourceId 位置を登録した番号。
	 * @param Request 何をどこで鳴らすか。
	 * @return 鳴らしたら true。
	 */
	bool Route( CSpatialAudio& Spatial, CAudioSubsystem& Audio, u32 SourceId, const FSpatialPlayRequest& Request ) noexcept;

	/** 直近に求まった «どちらから聞こえるか» (-1 左 .. +1 右) を返す。 */
	f32 GetLastPan() const noexcept { return m_LastPan; }

	/** 直近に実際へ渡した音量を返す。 */
	f32 GetLastVolume() const noexcept { return m_LastVolume; }

	/** 小さくなりきって鳴らさなかった数を返す。 */
	u64 GetSkippedCount() const noexcept { return m_SkippedCount; }

	/** 出力先や素材の問題で鳴らせなかった数を返す。 */
	u64 GetFailedCount() const noexcept { return m_FailedCount; }

private:
	/** 直近の左右の値。 */
	f32 m_LastPan = 0.0f;

	/** 直近の音量。 */
	f32 m_LastVolume = 0.0f;

	/** 鳴らさなかった数。 */
	u64 m_SkippedCount = 0u;

	/** 出力先や素材の問題で失敗した数。 */
	u64 m_FailedCount = 0u;
};
