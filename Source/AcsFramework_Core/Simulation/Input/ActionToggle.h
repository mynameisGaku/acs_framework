// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Simulation/ActionInput.h"

using namespace acs;

class CActionInputTracker;

/**
 * アクションを押した瞬間だけ、利用側が持つ有効・無効を反転する局所値。
 *
 * @details
 * 構え、しゃがみ、走行、照準、UI選択などの切替に使う。入力装置、場面、時間、切替後の処理は
 * 所有せず、通常フレームまたは固定ステップの入力履歴だけから同じ結果を返す。
 */
class FActionToggle
{
public:
	/** 無効な状態として構築する。 */
	FActionToggle() noexcept = default;

	/** 指定した初期状態として構築する。 */
	explicit FActionToggle( bool bEnabled ) noexcept
		: m_bEnabled( bEnabled )
	{
	}

	/**
	 * 通常フレームの入力履歴から指定アクションを処理する。
	 *
	 * @param Input 現在と前フレームを保持する入力。
	 * @param ActionIndex 反転に使うアクション番号。
	 * @param OutChanged 今回反転したならtrue。失敗時は変更しない。
	 * @return 範囲内のアクションを処理できたらtrue。範囲外では状態と出力を変えずfalse。
	 */
	bool Update( const CActionInputTracker& Input, u32 ActionIndex,
		bool& OutChanged ) noexcept;

	/**
	 * 明示した現在と前回の入力から指定アクションを処理する。
	 *
	 * @details AI、入力再生、固定ステップ、単体テストなど、装置を使わない経路で使う。
	 * @param CurrentInput 現在のアクション入力。
	 * @param PreviousInput 1回前のアクション入力。
	 * @param ActionIndex 反転に使うアクション番号。
	 * @param OutChanged 今回反転したならtrue。失敗時は変更しない。
	 * @return 範囲内のアクションを処理できたらtrue。範囲外では状態と出力を変えずfalse。
	 */
	bool Update( const FActionInput& CurrentInput,
		const FActionInput& PreviousInput, u32 ActionIndex,
		bool& OutChanged ) noexcept;

	/** 現在の有効・無効を明示値へ変更する。 */
	void SetEnabled( bool bEnabled ) noexcept { m_bEnabled = bEnabled; }

	/** 現在値を反転し、反転後が有効ならtrueを返す。 */
	bool Toggle() noexcept;

	/** 無効へ戻す。 */
	void Reset() noexcept { m_bEnabled = false; }

	/** 現在有効ならtrue。 */
	bool IsEnabled() const noexcept { return m_bEnabled; }

private:
	/** 押下によって切り替えた現在値。 */
	bool m_bEnabled = false;
};
