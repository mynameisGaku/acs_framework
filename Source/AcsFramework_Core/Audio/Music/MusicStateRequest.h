// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Audio/Music/MusicPriority.h"

using namespace acs;
using namespace acs::game;

/**
 * 「いまはこの曲であってほしい」という申告 1 件。
 *
 * @details
 * 申告する側は曲を知らない。状態 (戦闘中か、静かか) と、その中での強さを伝えるだけで、
 * どの曲が鳴るかは登録された表が決める。
 *
 * 申告はその場では効かない。フレームの決まった 1 か所で、集まったものから 1 つが選ばれる。
 */
struct FMusicStateRequest
{
	/** 望む状態。 */
	EMusicState State = EMusicState::Silent;

	/** その状態の中での強さ (0..1)。同じ状態でも曲を切り替えるために使う。 */
	f32 Intensity = 0.0f;

	/** 切り替えにかける秒数。 */
	f32 TransitionSeconds = 2.0f;

	/** 競ったときの強さ。 */
	EMusicPriority Priority = EMusicPriority::Gameplay;

	/**
	 * 相手より優先されるかを返す。
	 *
	 * @details 強さが同じなら、後から申告されたほうを採るので false を返す。
	 * @param Other 比べる相手。
	 * @return 自分が勝つなら true。
	 */
	bool IsStrongerThan( const FMusicStateRequest& Other ) const noexcept
	{
		return static_cast<i32>( Priority ) > static_cast<i32>( Other.Priority );
	}
};
