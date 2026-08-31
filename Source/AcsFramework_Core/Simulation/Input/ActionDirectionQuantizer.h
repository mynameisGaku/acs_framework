// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Simulation/ActionInput.h"
#include "AcsFramework_Core/Simulation/Input/ActionDirection2D.h"

using namespace acs;

/**
 * 連続した2軸入力を、中心のふらつきを抑えながら4方向または8方向へ変換する設定。
 *
 * @details
 * 入力や現在方向を所有しない局所値で、通常フレーム、固定ステップ、AI、再生入力へ共通利用できる。
 * 前回方向を利用側から渡すことで、入力開始より低い解除閾値を使い、中心付近での明滅を防ぐ。
 */
struct FActionDirectionQuantizer
{
	/** Noneから方向入力を始めるために超える長さ。0以上1未満。 */
	f32 ActivationThreshold = 0.5f;

	/** 入力中の方向をNoneへ戻す長さ。0以上、開始閾値以下。 */
	f32 ReleaseThreshold = 0.35f;

	/** trueなら斜めを含む8方向、falseなら成分の大きい軸を選ぶ4方向。 */
	bool bAllowDiagonal = true;

	/** 開始と解除の閾値が有限で、通常の最大入力を利用可能ならtrue。 */
	bool IsValid() const noexcept;

	/**
	 * 2軸入力を4方向または8方向へ変換する。
	 *
	 * @param Axes X正を右、Y正を上とする有限な入力。
	 * @param PreviousDirection 1回前の出力。Noneなら開始閾値、それ以外なら解除閾値を使う。
	 * @param OutDirection 成功時に今回の方向を受け取る。失敗時は変更しない。
	 * @return 設定、入力、前回方向が有効ならtrue。
	 */
	bool TryResolve( FVec2 Axes, EActionDirection2D PreviousDirection,
		EActionDirection2D& OutDirection ) const noexcept;

	/**
	 * アクション入力の異なる2軸を読み、4方向または8方向へ変換する。
	 *
	 * @param Input 変換元の汎用アクション入力。
	 * @param XAxisIndex 出力Xへ使う範囲内の軸番号。
	 * @param YAxisIndex 出力Yへ使う、Xとは異なる範囲内の軸番号。
	 * @param PreviousDirection 1回前の出力。
	 * @param OutDirection 成功時に今回の方向を受け取る。失敗時は変更しない。
	 * @return 軸番号、設定、入力、前回方向が有効ならtrue。
	 */
	bool TryResolve( const FActionInput& Input, u32 XAxisIndex,
		u32 YAxisIndex, EActionDirection2D PreviousDirection,
		EActionDirection2D& OutDirection ) const noexcept;
};
