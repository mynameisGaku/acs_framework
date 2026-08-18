// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Input/InputRepeat.h"

using namespace acs;

/**
 * ゲームパッドの方向入力を、押しっぱなしのリピート付きで拾う。
 *
 * @details
 * 方向キーと違い、スティックは倒しっぱなしになるため、そのまま毎フレーム流すと 1 回倒した
 * だけでカーソルが飛んでいく。ここで「倒した瞬間に 1 回 → 少し待ってから一定間隔で連射」へ
 * 均す (連射そのものは FInputRepeat が持つ)。キーボード側 (CDebugTopKeyNav) と同じ作法を
 * 共有しているので、どちらで触っても同じ感触になる。
 * 十字キーと左スティックは同じ入力として扱い、どちらでも同じように動く。
 * 接続を気にせず毎フレーム Update してよい (未接続なら何も起きない)。
 */
class CDebugTopGamepadNav
{
public:
	/**
	 * 1 フレーム進める。
	 *
	 * @param DeltaSeconds 前フレームからの経過秒。
	 */
	void Update( f32 DeltaSeconds ) noexcept;

	/** このフレームの上下入力を返す (上で -1、下で +1、無ければ 0)。 */
	i32 GetVertical() const noexcept { return m_Vertical; }

	/** このフレームの左右入力を返す (左で -1、右で +1、無ければ 0)。 */
	i32 GetHorizontal() const noexcept { return m_Horizontal; }

	/** このフレームに決定 (A ボタン) が押されたかを返す。 */
	bool IsDecidePressed() const noexcept { return m_bDecide; }

	/** このフレームに取り消し (B ボタン) が押されたかを返す。 */
	bool IsCancelPressed() const noexcept { return m_bCancel; }

	/** いずれかのポートにゲームパッドが繋がっているかを返す。 */
	bool IsConnected() const noexcept { return m_bConnected; }

private:
	/** このフレームの上下入力。 */
	i32 m_Vertical = 0;

	/** このフレームの左右入力。 */
	i32 m_Horizontal = 0;

	/** 上下の連射。 */
	FInputRepeat m_VerticalRepeat;

	/** 左右の連射。 */
	FInputRepeat m_HorizontalRepeat;

	/** このフレームに決定が押されたか。 */
	bool m_bDecide = false;

	/** このフレームに取り消しが押されたか。 */
	bool m_bCancel = false;

	/** いずれかのポートにゲームパッドが繋がっているか。 */
	bool m_bConnected = false;
};
