// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::game;

/**
 * 3D が映ることを実際に見て確かめるための場面。
 *
 * @details
 * この枠組みの目当ては「3D を、少ない手数で、綺麗に」なので、**何も設定していない状態で
 * どう見えるか**が肝になる。それを目で確かめられる最小の場面。
 *
 * 描くのはエンジン (`ALegacyScene3DAdapter`) が全部やる。ここがするのは、
 * 木に物と光を置くことだけ。
 *
 * ここに置く物は「既定のままどう見えるか」を見るためのものなので、
 * **素材ファイルを要らない形 (立方体・球・板) だけで組む**。素材が無い環境でも必ず映る。
 */
class ADemo3DScene : public ALegacyScene3DAdapter
{
public:
	/** 物と光を置き、カメラを引く。 */
	void OnEnter() noexcept override;

	/**
	 * 見え方の変化が分かるように、置いた物をゆっくり回す。
	 *
	 * @param DeltaSeconds 経過秒。
	 */
	void OnUpdate( f32 DeltaSeconds ) noexcept override;

private:
	/**
	 * 1 秒ぶんためて、平均のフレーム時間を 1 行だけ出す。
	 *
	 * @details
	 * **雲は «どこを向いているか» で値段が変わる。** 上を向けば画面全部が雲になり、
	 * 地平線を見れば 1 本のレイが数十 km を貫く。重さの話をするには数字が要る。
	 *
	 * @param DeltaSeconds 経過秒。
	 */
	void ReportFrameTime( f32 DeltaSeconds ) noexcept;

	/** 回す対象。所有はしない (木が持っている)。 */
	ANode* m_Spinner = nullptr;

	/** 回した量 (度)。 */
	f32 m_SpinDegrees = 0.0f;

	/** 直近 1 秒ぶんの経過秒の合計。 */
	f32 m_FrameTimeAccum = 0.0f;

	/** 直近 1 秒ぶんのフレーム数。 */
	u32 m_FrameCount = 0u;
};
