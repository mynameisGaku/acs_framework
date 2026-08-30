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

	/**
	 * いまの盤面をバイト列として差し出す。
	 *
	 * @details
	 * 実装すると「ここから始める」ができるようになる。バグの出た瞬間を保存して後から
	 * そこへ戻る、長い記録の途中から再生する、1 フレーム戻して見る、が可能になる。
	 *
	 * **結果に影響する値を漏らさず入れること。** 1 つでも欠けると、戻した後の盤面が
	 * 元と違う道を進む。時計と乱数は枠組みが別に写すので、ここへ入れなくてよい。
	 *
	 * 既定では false を返す (この規則は途中保存に対応しない、の意)。
	 * @param OutBytes 書き出し先。呼ばれた時点で空とは限らないので、必要なら空にしてから積む。
	 * @return 差し出せたら true。
	 */
	virtual bool TrySaveState( TArray<u8>& OutBytes ) const noexcept
	{
		(void)OutBytes;
		return false;
	}

	/**
	 * バイト列から盤面を戻す。
	 *
	 * @details
	 * TrySaveState が書いたものがそのまま渡る。形が違う (版が古いなど) 場合は false を返すこと。
	 * 全fieldを一時候補へ読み、形と値をすべて検証できた場合だけ実際の盤面へ反映すること。
	 * falseを返す場合は盤面を一切変更してはならない。この契約により、枠組みは時計、乱数、
	 * 入力履歴も含めて中途半端な復元を防ぐ。
	 * @param Bytes 読み元の先頭。
	 * @param Size 読み元の大きさ。
	 * @return 戻せたら true。
	 */
	virtual bool TryRestoreState( const u8* Bytes, usize Size ) noexcept
	{
		(void)Bytes;
		(void)Size;
		return false;
	}
};
