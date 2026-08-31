// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Simulation/ActionInput.h"
#include "AcsFramework_Core/Simulation/Input/ActionInputMask.h"
#include "AcsFramework_Core/Simulation/Input/ActionInputMaskStackState.h"

using namespace acs;

class CActionInputTracker;

/**
 * 入れ子になったゲーム状態の入力制限を、後から重ねた順に安全に戻す局所stack。
 *
 * @details
 * ポーズ中に確認画面を開く場合など、開始と終了が入れ子になる制限へ使う。各層には既存の
 * `FActionInputMask`をそのまま積み、全層が許可した操作だけを通す。画面、入力装置や所有者は
 * 持たず、開始側が`Push()`、対応する終了側が`Pop()`を呼ぶ。
 */
class FActionInputMaskStack
{
public:
	/** 層を持たず、全入力を通すstackを構築する。 */
	FActionInputMaskStack() noexcept = default;

	/**
	 * 新しい入力制限を最上層へ重ねる。
	 *
	 * @param Mask 追加する入力許可設定。
	 * @return 容量内で追加できたらtrue。満杯なら状態を変えずfalse。
	 */
	bool Push( const FActionInputMask& Mask ) noexcept;

	/** 最上層を1つ外す。空なら状態を変えずfalse。 */
	bool Pop() noexcept;

	/**
	 * 最上層だけを別の入力制限へ置き換える。
	 *
	 * @param Mask 新しい最上層の入力許可設定。
	 * @return 層があり置き換えられたらtrue。空なら状態を変えずfalse。
	 */
	bool TryReplaceTop( const FActionInputMask& Mask ) noexcept;

	/** 全層を外して、全入力を通す状態へ戻す。 */
	void Clear() noexcept;

	/** 現在積まれている層数を返す。 */
	u32 GetLayerCount() const noexcept { return m_LayerCount; }

	/** 層が1つも無ければtrue。 */
	bool IsEmpty() const noexcept { return m_LayerCount == 0u; }

	/**
	 * 現在の最上層を取得する。
	 *
	 * @param OutMask 最上層を書き込む先。
	 * @return 層があり取得できたらtrue。空なら出力を変えずfalse。
	 */
	bool TryGetTop( FActionInputMask& OutMask ) const noexcept;

	/** 全層が許可した操作だけを通す合成済みmaskを返す。 */
	FActionInputMask GetCombinedMask() const noexcept
	{
		return m_CombinedMask;
	}

	/** 指定入力へ全層の制限を適用した値を返す。 */
	FActionInput Apply( const FActionInput& Input ) const noexcept;

	/** 現在と前回の入力へ全層の制限を同時に適用する。 */
	void ApplyHistory( const FActionInput& CurrentInput,
		const FActionInput& PreviousInput,
		FActionInput& OutCurrentInput,
		FActionInput& OutPreviousInput ) const noexcept;

	/** 通常フレーム用トラッカーの現在と前回へ全層の制限を適用する。 */
	void ApplyHistory( const CActionInputTracker& Input,
		FActionInput& OutCurrentInput,
		FActionInput& OutPreviousInput ) const noexcept;

	/** 層順と各許可bitを保存可能な値として返す。 */
	FActionInputMaskStackState CaptureState() const noexcept;

	/**
	 * 保存した入力mask stackを復元する。
	 *
	 * @param State `CaptureState`で取得した矛盾のない状態。
	 * @return 復元できたらtrue。不正状態では現在値を一切変えずfalse。
	 */
	bool RestoreState( const FActionInputMaskStackState& State ) noexcept;

private:
	/** 現在の全層から許可範囲の積を作り直す。 */
	void RebuildCombinedMask_Internal() noexcept;

	/** 下層から順に保持する入力許可設定。 */
	FActionInputMask m_Layers[kActionInputMaskStackCapacity] = {};

	/** 現在積まれている層数。 */
	u32 m_LayerCount = 0u;

	/** 全層が許可した操作だけを残す合成済み設定。 */
	FActionInputMask m_CombinedMask;
};
