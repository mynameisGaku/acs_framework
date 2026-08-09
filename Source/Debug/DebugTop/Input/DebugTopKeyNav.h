#pragma once

#include <acs.h>

#include "AcsFramework_Core/Input/InputRepeat.h"

using namespace acs;

/**
 * 矢印キー (とテンキー) の方向入力をまとめて見る係。
 *
 * @details
 * 押しっぱなしにすると、少し待ってから一定間隔で動き続ける。1 回だけ押したときは
 * その 1 回だけ効く。行が数十ある一覧を、キーを叩き続けずに辿れるようにするためのもの。
 *
 * ゲームパッド (CDebugTopGamepadNav) と同じ連射の作法を共有しているので、どちらで触っても
 * 同じ感触になる。
 *
 * 押した結果どうするかは知らない。向きを返すだけ。
 */
class CDebugTopKeyNav
{
public:
	/** 入力なしの状態で構築する。 */
	CDebugTopKeyNav() noexcept = default;

	/**
	 * 1 フレーム進める。
	 *
	 * @param DeltaSeconds 前フレームからの経過秒。
	 */
	void Update( f32 DeltaSeconds ) noexcept;

	/** 上下の入力を返す (上で -1、下で +1、無ければ 0)。 */
	i32 GetVertical() const noexcept { return m_Vertical; }

	/** 左右の入力を返す (左で -1、右で +1、無ければ 0)。 */
	i32 GetHorizontal() const noexcept { return m_Horizontal; }

private:
	/** 上下の連射。 */
	FInputRepeat m_VerticalRepeat;

	/** 左右の連射。 */
	FInputRepeat m_HorizontalRepeat;

	/** この回に効かせる上下の量。 */
	i32 m_Vertical = 0;

	/** この回に効かせる左右の量。 */
	i32 m_Horizontal = 0;
};
