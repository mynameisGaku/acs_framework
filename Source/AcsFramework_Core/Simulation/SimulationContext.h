// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Simulation/ActionInput.h"
#include "AcsFramework_Core/Simulation/DeterministicRandom.h"
#include "AcsFramework_Core/Simulation/SimulationEventQueue.h"

using namespace acs;

/**
 * 1 ステップを進めるために要るもの一式。
 *
 * @details
 * ロジックが結果を変えるために読んでよいのは、**ここに入っているものだけ**という取り決めにする。
 * 現在時刻・World・シングルトン・グローバル変数を直接覗くと、同じ入力から同じ結果を作れなくなる。
 * 覗きたくなったものが出てきたら、それは実質的に入力なので、ここへ足すことを検討する。
 *
 * 「今ステップ押された」といった差分は、前ステップと突き合わせて初めて決まる。両方を
 * 知っているのはここなので、ここが答える。
 */
struct FSimulationContext
{
	/** このステップの入力。 */
	FActionInput Input;

	/** 1 つ前のステップの入力。 */
	FActionInput PreviousInput;

	/** このステップのティック番号。 */
	u32 Tick = 0u;

	/** 1 ステップの秒数。実時間ではなく、必ずこれを使う。 */
	f32 StepSeconds = 0.0f;

	/** 種を握れる乱数。所有はしない。 */
	CDeterministicRandom* Random = nullptr;

	/** 起きたことを置く先。所有はしない。 */
	CSimulationEventQueue* Events = nullptr;

	/**
	 * 押されているかを返す。
	 *
	 * @param ActionIndex アクション番号。
	 * @return 押されていれば true。
	 */
	bool IsDown( u32 ActionIndex ) const noexcept { return Input.IsDown( ActionIndex ); }

	/**
	 * このステップで押されたかを返す。
	 *
	 * @param ActionIndex アクション番号。
	 * @return 前は押されておらず、いま押されていれば true。
	 */
	bool WasPressed( u32 ActionIndex ) const noexcept
	{
		return Input.IsDown( ActionIndex ) && !PreviousInput.IsDown( ActionIndex );
	}

	/**
	 * このステップで離されたかを返す。
	 *
	 * @param ActionIndex アクション番号。
	 * @return 前は押されており、いま押されていなければ true。
	 */
	bool WasReleased( u32 ActionIndex ) const noexcept
	{
		return !Input.IsDown( ActionIndex ) && PreviousInput.IsDown( ActionIndex );
	}

	/**
	 * 軸の値を返す。
	 *
	 * @param AxisIndex 軸番号。
	 * @return 軸の値。
	 */
	f32 GetAxis( u32 AxisIndex ) const noexcept { return Input.GetAxis( AxisIndex ); }

	/**
	 * 起きたことを置く。
	 *
	 * @details ティック番号はここで入れるので、呼ぶ側は入れなくてよい。
	 * @param Id 何が起きたか。
	 * @param Target 誰に起きたか。
	 * @param ValueA 付随する数値。
	 * @param ValueB 付随する数値。
	 * @param ValueC 付随する数値。
	 * @return 置けたら true。
	 */
	bool Raise( u32 Id, u32 Target = 0u, f32 ValueA = 0.0f, f32 ValueB = 0.0f, f32 ValueC = 0.0f ) const noexcept
	{
		if ( Events == nullptr ) return false;

		FSimulationEvent Event;
		Event.Id = Id;
		Event.Tick = Tick;
		Event.Target = Target;
		Event.ValueA = ValueA;
		Event.ValueB = ValueB;
		Event.ValueC = ValueC;

		return Events->Push( Event );
	}
};
