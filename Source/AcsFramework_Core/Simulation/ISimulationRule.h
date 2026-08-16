// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Simulation/SimulationContext.h"

using namespace acs;

/**
 * 1 ステップぶん、盤面を進める規則。
 *
 * @details
 * ゲームのルールそのもの。**ここがゲーム側の実装点**で、枠組みはこれを決まった間隔で
 * 呼ぶことしかしない。
 *
 * 中で守ること。
 *
 * - 結果へ影響する値は `FSimulationContext` から取る (現在時刻・World・シングルトンを見ない)
 * - 経過時間は `Context.StepSeconds` を使う (実時間の dt を持ち込まない)
 * - 乱数は `Context.Random` から引く (自前の乱数を持たない)
 * - 音・絵は鳴らさず出さず、`Context.Raise()` で «起きたこと» を置く
 *
 * これを守ると、同じ入力列と同じ種から必ず同じ盤面になる。ゲームを起動せずに
 * 1 万ステップ回すことも、バグの起きた入力列を保存して再生することもできる。
 *
 * @code
 * void CMyRule::AdvanceStep( const FSimulationContext& Context ) noexcept override
 * {
 *     m_Player.X += Context.GetAxis( 0u ) * m_Speed * Context.StepSeconds;
 *
 *     if ( Context.WasPressed( kActionFire ) ) Context.Raise( kEventFired, m_Player.Id );
 * }
 * @endcode
 */
class ISimulationRule
{
public:
	/** 派生を正しく破棄するための仮想デストラクタ。 */
	virtual ~ISimulationRule() noexcept = default;

	/**
	 * 1 ステップ進める。
	 *
	 * @param Context このステップの入力・時間・乱数・置き場。
	 */
	virtual void AdvanceStep( const FSimulationContext& Context ) noexcept = 0;

	/**
	 * 盤面を初期状態へ戻す。
	 *
	 * @details 再生を始めるとき、記録を取り直すときに呼ばれる。既定では何もしない。
	 */
	virtual void ResetState() noexcept {}
};
