// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Simulation/ActionInput.h"

using namespace acs;

class CActionInputTracker;

/**
 * ゲーム状態ごとに通してよいアクションと軸だけを残す局所設定。
 *
 * @details
 * メニュー、演出、会話、操作不能中などで、ゲーム入力の一部だけを止めるために使う。
 * 現在と前回の入力へ同じ設定を適用できるため、禁止中から再許可した時点で押しっぱなしの操作を
 * 新しい押下として誤認しない。入力、場面、装置は所有しない。
 */
class FActionInputMask
{
public:
	/** 全アクションと全軸を通す設定を構築する。 */
	FActionInputMask() noexcept = default;

	/** 全アクションと全軸を通す設定を返す。 */
	static FActionInputMask All() noexcept;

	/** 全アクションと全軸を止める設定を返す。 */
	static FActionInputMask None() noexcept;

	/** 全アクションと全軸を通す設定へ戻す。 */
	void EnableAll() noexcept;

	/** 全アクションと全軸を止める。 */
	void DisableAll() noexcept;

	/**
	 * 指定アクションを通すか変更する。
	 *
	 * @param ActionIndex 変更するアクション番号。
	 * @param bEnabled 通すならtrue、止めるならfalse。
	 * @return 範囲内ならtrue。範囲外では設定を変えずfalse。
	 */
	bool SetActionEnabled( u32 ActionIndex, bool bEnabled ) noexcept;

	/**
	 * 指定軸を通すか変更する。
	 *
	 * @param AxisIndex 変更する軸番号。
	 * @param bEnabled 通すならtrue、止めるならfalse。
	 * @return 範囲内ならtrue。範囲外では設定を変えずfalse。
	 */
	bool SetAxisEnabled( u32 AxisIndex, bool bEnabled ) noexcept;

	/** 指定アクションを通すならtrue。範囲外ならfalse。 */
	bool IsActionEnabled( u32 ActionIndex ) const noexcept;

	/** 指定軸を通すならtrue。範囲外ならfalse。 */
	bool IsAxisEnabled( u32 AxisIndex ) const noexcept;

	/** 通すアクションを表す32bit値を返す。 */
	u32 GetActionMask() const noexcept { return m_ActionMask; }

	/** 通す軸を表す下位`kActionAxisCount`bit値を返す。 */
	u32 GetAxisMask() const noexcept { return m_AxisMask; }

	/**
	 * 保存値などからアクションと軸の許可bitをまとめて設定する。
	 *
	 * @param ActionMask 通すアクションを表す32bit値。
	 * @param AxisMask 通す軸を表し、上位の未使用bitが0の値。
	 * @return 軸bitが有効なら両方を反映してtrue。不正なら従来設定を保ちfalse。
	 */
	bool TrySetMasks( u32 ActionMask, u32 AxisMask ) noexcept;

	/**
	 * 指定入力から許可していないアクションと軸を0にした値を返す。
	 *
	 * @param Input 装置、AI、再生などから得た入力。
	 * @return 許可した操作だけを残した入力。
	 */
	FActionInput Apply( const FActionInput& Input ) const noexcept;

	/**
	 * 現在と前回の入力へ同じ設定を適用する。
	 *
	 * @details 出力を入力と同じ変数へ指定しても、両方を変換してから書き戻す。
	 * @param CurrentInput 現在のアクション入力。
	 * @param PreviousInput 1回前のアクション入力。
	 * @param OutCurrentInput 許可した現在入力の出力先。
	 * @param OutPreviousInput 許可した前回入力の出力先。
	 */
	void ApplyHistory( const FActionInput& CurrentInput,
		const FActionInput& PreviousInput,
		FActionInput& OutCurrentInput,
		FActionInput& OutPreviousInput ) const noexcept;

	/**
	 * 通常フレーム用トラッカーの現在と前回へ同じ設定を適用する。
	 *
	 * @param Input 現在と前フレームを保持する入力。
	 * @param OutCurrentInput 許可した現在入力の出力先。
	 * @param OutPreviousInput 許可した前回入力の出力先。
	 */
	void ApplyHistory( const CActionInputTracker& Input,
		FActionInput& OutCurrentInput,
		FActionInput& OutPreviousInput ) const noexcept;

	/**
	 * この設定と相手の両方が許可する操作だけを通す設定を返す。
	 *
	 * @details ゲーム、画面、演出など複数の制限を安全側へ積み重ねるために使う。
	 * @param Other 追加で重ねる設定。
	 * @return 両方の許可範囲が重なる設定。
	 */
	FActionInputMask Intersect( const FActionInputMask& Other ) const noexcept;

private:
	/** 全アクションを通すbit値。 */
	static constexpr u32 kAllActionMask = ~static_cast<u32>( 0u );

	/** 全軸を通し、未使用の上位bitを持たない値。 */
	static constexpr u32 kAllAxisMask = ( 1u << kActionAxisCount ) - 1u;

	/** 通すアクションを表すbit値。 */
	u32 m_ActionMask = kAllActionMask;

	/** 通す軸を表すbit値。 */
	u32 m_AxisMask = kAllAxisMask;
};
