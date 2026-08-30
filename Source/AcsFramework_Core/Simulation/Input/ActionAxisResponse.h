// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Simulation/ActionInput.h"

using namespace acs;

/**
 * アナログ軸の遊びを除き、残った操作量を0から1へ詰め直す応答設定。
 *
 * @details
 * スティック中心の微小なずれを止めるだけでなく、遊びの外側を再正規化するため、最大まで
 * 倒せる範囲を失わない。2軸へ使う場合は長さだけを変換し、移動や視点の方向を保つ。
 * 入力、時計、装置を所有しない局所値なので、通常フレーム、固定ステップ、AI、再生入力で
 * 同じ変換を使える。
 */
struct FActionAxisResponse
{
	/** 中心から無入力として扱う長さ。0以上1未満。 */
	f32 InnerDeadZone = 0.15f;

	/** 最大入力として扱う外周からの長さ。0以上1未満。 */
	f32 OuterDeadZone = 0.0f;

	/** 再正規化後の値へ掛ける指数。1は線形、1より大きいほど中心付近を穏やかにする。 */
	f32 ResponseExponent = 1.0f;

	/** 遊び幅と応答指数が有限で、利用可能な入力範囲を残していればtrue。 */
	bool IsValid() const noexcept;

	/**
	 * 1軸の入力を符号付きの-1から1へ変換する。
	 *
	 * @param RawValue 装置、AI、再生入力などが作った有限の軸値。
	 * @param OutValue 成功時に変換後の値を受け取る。失敗時は変更しない。
	 * @return 設定と入力が有効ならtrue。
	 */
	bool TryApply( f32 RawValue, f32& OutValue ) const noexcept;

	/**
	 * アクション入力の指定軸を読み、符号付きの-1から1へ変換する。
	 *
	 * @param Input 変換元の汎用アクション入力。
	 * @param AxisIndex 読み取る範囲内の軸番号。
	 * @param OutValue 成功時に変換後の値を受け取る。失敗時は変更しない。
	 * @return 軸番号、設定、入力値が有効ならtrue。
	 */
	bool TryApply( const FActionInput& Input, u32 AxisIndex, f32& OutValue ) const noexcept;

	/**
	 * 2軸の長さへ遊びと応答を適用し、元の方向を保つ。
	 *
	 * @param RawAxes 移動、視点、照準などの有限な2軸入力。
	 * @param OutAxes 成功時に長さ0から1の変換値を受け取る。失敗時は変更しない。
	 * @return 設定と両軸が有効ならtrue。
	 */
	bool TryApplyRadial( FVec2 RawAxes, FVec2& OutAxes ) const noexcept;

	/**
	 * アクション入力の異なる2軸を読み、方向を保つ応答へ変換する。
	 *
	 * @param Input 変換元の汎用アクション入力。
	 * @param XAxisIndex 出力Xへ使う範囲内の軸番号。
	 * @param YAxisIndex 出力Yへ使う、Xとは異なる範囲内の軸番号。
	 * @param OutAxes 成功時に長さ0から1の変換値を受け取る。失敗時は変更しない。
	 * @return 軸番号、設定、両入力値が有効ならtrue。
	 */
	bool TryApplyRadial( const FActionInput& Input, u32 XAxisIndex,
		u32 YAxisIndex, FVec2& OutAxes ) const noexcept;
};
