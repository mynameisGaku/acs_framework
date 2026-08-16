// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/**
 * 曲の状態を申告するときの強さ。
 *
 * @details
 * 同じフレームに複数から申告が来ることがある (地形は Calm、戦闘は Combat、演出は Cinematic)。
 * どれを採るかを毎回書かずに済むよう、強さで決める。数が大きいほうが勝つ。
 */
enum class EMusicPriority : i32
{
	/** 場所や時間帯など、常に鳴っている土台。 */
	Ambient = 0,

	/** 戦闘や危険など、遊びの状況から決まるもの。 */
	Gameplay = 10,

	/** 演出。始まっている間は他を抑える。 */
	Cinematic = 20,
};
