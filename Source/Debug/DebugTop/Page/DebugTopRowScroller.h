// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "Debug/DebugTop/Element/DebugTopElement.h"

using namespace acs;

/**
 * 平坦化した可視行 1 つ。
 *
 * @details 木を畳んだ結果の並び。ページも配置もスクロールもこの並びを見る。
 */
struct FDebugTopVisibleRow
{
	/** 対応する行。実体は木側が所有するため、ここでは所有しない。 */
	CDebugTopElement* Element = nullptr;

	/** ルートからの深さ (インデント段数)。 */
	i32 Depth = 0;
};

/** 可視行の並び。 */
using FDebugTopVisibleRows = TArray<FDebugTopVisibleRow>;


/**
 * どの行を選んでいて、どこから何行ぶん出すかを決める係。
 *
 * @details
 * 行の高さは一定でない (色の面や折れ線は背が高い) ので、行数の割り算では出せない。
 * 実際の高さを積んで数える。行そのものは持たず、毎回外から並びを渡してもらう。
 *
 * 描画も入力も知らない。ホイールが回った・キーが押されたという判断は持ち主がして、
 * ここへは「何行動かす」だけが来る。
 */
class CDebugTopRowScroller
{
public:
	/** 先頭を選んだ状態で構築する。 */
	CDebugTopRowScroller() noexcept = default;

	/** いま選んでいる行の位置を返す。 */
	i32 GetCursorRow() const noexcept { return m_CursorRow; }

	/** 先頭に出す行の位置を返す。 */
	i32 GetScrollTop() const noexcept { return m_ScrollTop; }

	/**
	 * 選ぶ行を直接指す。
	 *
	 * @param CursorRow 選ぶ行の位置。
	 */
	void SetCursorRow( i32 CursorRow ) noexcept { m_CursorRow = CursorRow; }

	/**
	 * 選ぶ行をずらす。
	 *
	 * @details 端まで行ったら反対の端へ回り込む (一覧の行き来を早くするため)。
	 * @param Rows 並び。
	 * @param Delta ずらす行数 (負で上へ)。
	 */
	void MoveCursor( const FDebugTopVisibleRows& Rows, i32 Delta ) noexcept;

	/**
	 * 行数が減ったときに、選んでいる位置を範囲内へ詰め直す。
	 *
	 * @param Rows 並び。
	 */
	void ClampCursor( const FDebugTopVisibleRows& Rows ) noexcept;

	/**
	 * 1 行ぶんの高さを返す。
	 *
	 * @param Rows 並び。
	 * @param RowIndex 行の位置。
	 * @param BaseHeight 文字 1 行ぶんの高さ。
	 * @return その行の高さ (背の高い行は倍率が掛かる)。
	 */
	f32 GetRowHeight( const FDebugTopVisibleRows& Rows, i32 RowIndex, f32 BaseHeight ) const noexcept;

	/**
	 * 指定の位置から何行入るかを数える。
	 *
	 * @param Rows 並び。
	 * @param From 数え始める行。
	 * @param Available 使える高さ。
	 * @param BaseHeight 文字 1 行ぶんの高さ。
	 * @return 収まる行数 (1 行も入らなくても 1 を返す)。
	 */
	i32 CountRowsThatFit( const FDebugTopVisibleRows& Rows, i32 From, f32 Available, f32 BaseHeight ) const noexcept;

	/**
	 * 選んでいる行が見えるように送る。
	 *
	 * @details 描画のたびに呼ぶ。使える高さはそのとき初めて分かるため。
	 * @param Rows 並び。
	 * @param Available 一覧に使える高さ。
	 * @param BaseHeight 文字 1 行ぶんの高さ。
	 */
	void ScrollToCursor( const FDebugTopVisibleRows& Rows, f32 Available, f32 BaseHeight ) noexcept;

	/**
	 * ホイールで送る。
	 *
	 * @details 描画の外で来るので、直前の描画が控えた寸法で数え直す。
	 * @param Rows 並び。
	 * @param Wheel ホイールの回転量 (正で奥へ)。
	 */
	void ScrollByWheel( const FDebugTopVisibleRows& Rows, f32 Wheel ) noexcept;

	/**
	 * 直前の描画が使った寸法を控える。
	 *
	 * @details ホイールは描画の外で来るので、そのときに数え直せるようにしておく。
	 * @param Available 一覧に使えた高さ。
	 * @param BaseHeight 文字 1 行ぶんの高さ。
	 * @param DrawnRowCapacity 実際に描いた行数。
	 */
	void RememberViewport( f32 Available, f32 BaseHeight, i32 DrawnRowCapacity ) noexcept;

private:
	/** いま選んでいる行。 */
	i32 m_CursorRow = 0;

	/** 先頭に出している行。 */
	i32 m_ScrollTop = 0;

	/** 最後の行まで見せられる先頭位置 (直近の描画が求めたもの)。 */
	i32 m_MaxScrollTop = 0;

	/** 直近の描画が実際に置いた行数。 */
	i32 m_DrawnRowCapacity = 0;

	/** 直近の描画で一覧に使えた高さ。 */
	f32 m_LastAvailable = 0.0f;

	/** 直近の描画での文字 1 行ぶんの高さ。 */
	f32 m_LastBaseHeight = 0.0f;
};
