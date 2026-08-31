// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Simulation/ActionInput.h"

using namespace acs;

class CActionInputTracker;

/**
 * 複数アクションの同時押しと、押されていてはいけないアクションを判定する局所設定。
 *
 * @details
 * 修飾操作を伴うショートカット、構えながらの特殊操作、複数ボタン入力などに使う。
 * 必要な全操作が押され、禁止した操作が1つも押されていないときだけ有効になる。
 * 入力、装置、場面、時間は所有しない。
 */
class FActionChord
{
public:
	/** 必要操作が未設定の無効な組み合わせを構築する。 */
	FActionChord() noexcept = default;

	/**
	 * 1つの必要操作から組み合わせを構築する。
	 *
	 * @param RequiredActionIndex 必ず押されている必要があるアクション番号。
	 * @details 範囲外なら必要操作が空の無効な設定になる。
	 */
	explicit FActionChord( u32 RequiredActionIndex ) noexcept;

	/** 必要操作が1つ以上あり、必要と禁止のbitが重ならなければtrue。 */
	bool IsValid() const noexcept;

	/**
	 * 必ず押されている必要があるアクションを追加する。
	 *
	 * @param ActionIndex 追加するアクション番号。
	 * @return 範囲内かつ禁止操作でなければtrue。失敗時は設定を変えない。
	 */
	bool RequireAction( u32 ActionIndex ) noexcept;

	/**
	 * 押されていてはいけないアクションを追加する。
	 *
	 * @param ActionIndex 追加するアクション番号。
	 * @return 範囲内かつ必要操作でなければtrue。失敗時は設定を変えない。
	 */
	bool ForbidAction( u32 ActionIndex ) noexcept;

	/**
	 * 指定アクションを必要・禁止のどちらからも外す。
	 *
	 * @param ActionIndex 外すアクション番号。
	 * @return 範囲内ならtrue。範囲外では設定を変えずfalse。
	 */
	bool IgnoreAction( u32 ActionIndex ) noexcept;

	/** 必要操作と禁止操作をすべて空にする。 */
	void Reset() noexcept;

	/**
	 * 保存値などから必要操作と禁止操作をまとめて設定する。
	 *
	 * @param RequiredActionMask 1bit以上立った必要操作のbit値。
	 * @param ForbiddenActionMask 必要操作と重ならない禁止操作のbit値。
	 * @return 矛盾がなければ両方を反映してtrue。不正なら従来設定を保ちfalse。
	 */
	bool TrySetMasks(
		u32 RequiredActionMask, u32 ForbiddenActionMask ) noexcept;

	/** 指定アクションが必要操作ならtrue。範囲外ならfalse。 */
	bool IsActionRequired( u32 ActionIndex ) const noexcept;

	/** 指定アクションが禁止操作ならtrue。範囲外ならfalse。 */
	bool IsActionForbidden( u32 ActionIndex ) const noexcept;

	/** 必要操作を表す32bit値を返す。 */
	u32 GetRequiredActionMask() const noexcept { return m_RequiredActionMask; }

	/** 禁止操作を表す32bit値を返す。 */
	u32 GetForbiddenActionMask() const noexcept { return m_ForbiddenActionMask; }

	/**
	 * 指定入力で組み合わせ条件が成立していればtrue。
	 *
	 * @param Input 判定するアクション入力。
	 */
	bool IsActive( const FActionInput& Input ) const noexcept;

	/** 通常フレーム用トラッカーの現在入力で条件が成立していればtrue。 */
	bool IsActive( const CActionInputTracker& Input ) const noexcept;

	/**
	 * 今回の入力で組み合わせが新しく有効になったならtrue。
	 *
	 * @details 必要操作のどれかが今回押された場合だけ有効化する。禁止操作を離しただけではtrueにしない。
	 * @param CurrentInput 現在のアクション入力。
	 * @param PreviousInput 1回前のアクション入力。
	 */
	bool WasActivated( const FActionInput& CurrentInput,
		const FActionInput& PreviousInput ) const noexcept;

	/** 通常フレーム用トラッカーで組み合わせが新しく有効になったならtrue。 */
	bool WasActivated( const CActionInputTracker& Input ) const noexcept;

	/**
	 * 前回成立していた組み合わせが今回無効になったならtrue。
	 *
	 * @details 必要操作を離した場合と、禁止操作を押した場合のどちらも無効化として扱う。
	 * @param CurrentInput 現在のアクション入力。
	 * @param PreviousInput 1回前のアクション入力。
	 */
	bool WasDeactivated( const FActionInput& CurrentInput,
		const FActionInput& PreviousInput ) const noexcept;

	/** 通常フレーム用トラッカーで組み合わせが無効になったならtrue。 */
	bool WasDeactivated( const CActionInputTracker& Input ) const noexcept;

private:
	/** 必ず押されている必要があるアクションのbit値。 */
	u32 m_RequiredActionMask = 0u;

	/** 押されていてはいけないアクションのbit値。 */
	u32 m_ForbiddenActionMask = 0u;
};
