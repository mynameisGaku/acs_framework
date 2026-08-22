// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Scene/Character3D/ThirdPersonCharacter3DInput.h"
#include "AcsFramework_Core/Simulation/ActionInput.h"

/**
 * 汎用アクション入力を第三者視点操作へ割り当てる値。
 *
 * @details 入力装置や時刻を取得せず、現在と前回の入力だけから1回分の操作を作る。
 */
struct FThirdPersonCharacter3DActionSet
{
	/** 左右移動へ使う軸番号。 */
	u32 MoveRightAxis = 0u;

	/** 前後移動へ使う軸番号。 */
	u32 MoveForwardAxis = 1u;

	/** 左右の視点回転へ使う軸番号。 */
	u32 LookYawAxis = 2u;

	/** 上下の視点回転へ使う軸番号。 */
	u32 LookPitchAxis = 3u;

	/** ジャンプへ使うアクション番号。 */
	u32 JumpAction = 0u;

	/** カメラを近づけるアクション番号。 */
	u32 ZoomInAction = 1u;

	/** カメラを遠ざけるアクション番号。 */
	u32 ZoomOutAction = 2u;

	/** 基本速度から走行速度へ切り替えるアクション番号。 */
	u32 RunAction = kActionButtonCount;

	/**
	 * 既存3操作へ走行アクションを明示追加した値を作る。
	 *
	 * @param ActionIndex 走行へ使う範囲内のアクション番号。
	 * @return 指定番号を走行へ設定した値。範囲外ならIsValidが拒否する値。
	 */
	static FThirdPersonCharacter3DActionSet WithRunAction( u32 ActionIndex = 3u ) noexcept
	{
		FThirdPersonCharacter3DActionSet Actions;
		Actions.RunAction = ActionIndex < kActionButtonCount ? ActionIndex : kActionButtonCount + 1u;
		return Actions;
	}

	/** 走行アクションが明示的に有効ならtrueを返す。 */
	bool HasRunAction() const noexcept { return RunAction < kActionButtonCount; }

	/**
	 * 軸とアクションの割り当てが利用可能か返す。
	 *
	 * @return 各番号が範囲内で、有効な同じ種類の番号に重複がなければtrue。
	 */
	bool IsValid() const noexcept;

	/**
	 * 現在と前回の汎用入力から第三者視点の明示入力を作る。
	 *
	 * @details 軸は個別に-1から1へ制限する。走行は押している間だけ有効にする。ジャンプは押した瞬間だけ要求し、
	 * ズームの近接と遠隔が同時なら中立にする。失敗時はOutInputを変更しない。
	 * @param CurrentInput 今回の汎用アクション入力。
	 * @param PreviousInput 前回の汎用アクション入力。
	 * @param OutInput 成功時に確定した第三者視点入力を受け取る値。
	 * @return 割り当てと参照する軸値が有効ならtrue。
	 */
	bool TryEvaluate( const FActionInput& CurrentInput, const FActionInput& PreviousInput, FThirdPersonCharacter3DInput& OutInput ) const noexcept;
};
