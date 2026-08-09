#pragma once

#include <acs.h>

#include "AcsFramework_Core/Input/InputRepeat.h"

#include "Debug/DebugTop/Element/DebugTopElement.h"
#include "Debug/DebugTop/Element/DebugTopElementColor.h"
#include "Debug/DebugTop/Page/DebugTopRowScroller.h"
#include "Debug/DebugTop/Page/DebugTopValueEditor.h"
#include "Debug/DebugTop/Widget/DebugTopColorPicker.h"

using namespace acs;

/** 矢印を押しっぱなしにしてから連打が始まるまでの待ち (秒)。 */
inline constexpr f32 kHoldRepeatDelaySeconds = 0.4f;

/** 矢印の連打の間隔 (秒)。キーボードより速く送れるようにしてある。 */
inline constexpr f32 kHoldRepeatIntervalSeconds = 0.06f;

/**
 * 直前の描画が置いた行 1 つ分の矩形。
 *
 * @details
 * マウスは「いま描かれているもの」に当てるしかないので、描いた側が置いた矩形をそのまま
 * 受け取って判定に使う。行の実体は持たず、入れ替わっていないかの照合用に印だけ控える。
 */
struct FDebugTopDrawnRow
{
	/** 可視行の添字。 */
	i32 RowIndex = 0;

	/**
	 * 描いた時点での行。
	 *
	 * @details 行が入れ替わっていないかの照合にだけ使う。参照はしないので解放済みでも安全。
	 */
	const CDebugTopElement* Element = nullptr;

	/** 行の左端 X。 */
	f32 X = 0.0f;

	/**
	 * 行の内容の左端 X (インデント済み)。
	 *
	 * @details
	 * 面や帯を持つ行はここを基準に描くので、押された位置もここからの距離で渡す。
	 * 行の左端 (X) を基準にすると、深い階層の行で見た目と当たり判定がずれる。
	 */
	f32 ContentX = 0.0f;

	/** 行の上端 Y。 */
	f32 Y = 0.0f;

	/** 行の幅。 */
	f32 Width = 0.0f;

	/** 行の高さ。 */
	f32 Height = 0.0f;

	/** 矢印 1 個の一辺 (0 なら左右キーに反応しない行)。 */
	f32 ArrowSize = 0.0f;

	/** 左矢印の左端 X。 */
	f32 LeftArrowX = 0.0f;

	/** 右矢印の左端 X。 */
	f32 RightArrowX = 0.0f;

	/** スライダーの溝の左端 X (0 なら範囲を持たない行)。 */
	f32 SliderX = 0.0f;

	/** スライダーの溝の幅 (0 なら範囲を持たない行)。 */
	f32 SliderWidth = 0.0f;

	/** ピン留めの星ボタンの左端 X。 */
	f32 StarX = 0.0f;

	/** ピン留めの星ボタンの一辺。 */
	f32 StarSize = 0.0f;

	/** 入力欄の左端 X (0 幅なら打ち込めない行)。 */
	f32 EditX = 0.0f;

	/** 入力欄の幅 (0 なら打ち込めない行)。 */
	f32 EditWidth = 0.0f;

	/** 色見本の左端 X (0 幅なら色の行ではない)。 */
	f32 SwatchX = 0.0f;

	/** 色見本の上端 Y。 */
	f32 SwatchY = 0.0f;

	/** 色見本の幅 (0 なら色の行ではない)。 */
	f32 SwatchWidth = 0.0f;

	/** 色見本の高さ。 */
	f32 SwatchHeight = 0.0f;
};

/**
 * マウスの押下・ドラッグ・ホイールを、行への操作へ振り分ける係。
 *
 * @details
 * 押しっぱなしの連打・スライダーの追従・ダブルクリックといった「時間を跨ぐ状態」をここに
 * 閉じ込める。行の並べ方も描き方も知らず、判定に使う矩形は描いた側から受け取る。
 *
 * 振り分け先 (カーソル・色のパネル・打ち込み) は Bind で 1 度だけ受け取る。ページが
 * 生きている間ずっと同じものを指すので、毎フレーム渡し直す必要はない。
 */
class CDebugTopRowMouse
{
public:
	/** 何も掴んでいない状態で構築する。 */
	CDebugTopRowMouse() noexcept = default;

	/**
	 * 振り分け先を配線する。
	 *
	 * @details ページの構築時に 1 度だけ呼ぶ。渡したものはページと同じだけ生きること。
	 * @param Rows 可視行の並び (押された行を引くのに使う)。
	 * @param Scroller カーソルとスクロール。
	 * @param ColorPicker 色を選ぶパネル。
	 * @param ValueEditor 値の打ち込み。
	 */
	void Bind( const FDebugTopVisibleRows& Rows, CDebugTopRowScroller& Scroller, CDebugTopColorPicker& ColorPicker, CDebugTopValueEditor& ValueEditor ) noexcept;

	/** 描画が始まったので、前フレームの矩形を捨てる。 */
	void BeginFrame( usize ExpectedRowCount ) noexcept;

	/**
	 * いま描いた行の矩形を控える。
	 *
	 * @param Row 描いた行の矩形。
	 */
	void AddDrawnRow( const FDebugTopDrawnRow& Row ) { m_DrawnRows.Add( Row ); }

	/**
	 * マウスを 1 フレーム進める。
	 *
	 * @param DeltaSeconds 前フレームからの経過秒 (連打とダブルクリックの判定に使う)。
	 */
	void Update( f32 DeltaSeconds ) noexcept;

	/** マウスを重ねている可視行の添字を返す (無ければ -1)。 */
	i32 GetHoverRow() const noexcept { return m_HoverRow; }

private:
	/**
	 * 指定の位置に最も近い、描いた行を探す。
	 *
	 * @param Position 画面座標。
	 * @return 描いた行の添字 (一覧から離れていれば -1)。
	 */
	i32 FindNearestDrawnRow( FVec2 Position ) const noexcept;

	/** 掴んでいるものを離した後の後始末 (連打とダブルクリックの間隔を切る)。 */
	void ResetClickTracking() noexcept;

	/** 可視行の並び。所有はしない (ページが持っている)。 */
	const FDebugTopVisibleRows* m_Rows = nullptr;

	/** カーソルとスクロール。所有はしない。 */
	CDebugTopRowScroller* m_Scroller = nullptr;

	/** 色を選ぶパネル。所有はしない。 */
	CDebugTopColorPicker* m_ColorPicker = nullptr;

	/** 値の打ち込み。所有はしない。 */
	CDebugTopValueEditor* m_ValueEditor = nullptr;

	/** 直近の描画が実際に描いた行の矩形。当たり判定はこれを見る。 */
	TArray<FDebugTopDrawnRow> m_DrawnRows;

	/** マウスを重ねている可視行の添字 (押したらここが選ばれる。無ければ -1)。 */
	i32 m_HoverRow = -1;

	/** 直前に押した可視行の添字 (ダブルクリックの判定に使う。-1 で無効)。 */
	i32 m_LastClickRow = -1;

	/** 直前に押してからの経過秒 (ダブルクリックの判定に使う)。 */
	f32 m_ClickElapsed = 0.0f;

	/** 押しっぱなしにしている矢印の連射。 */
	FInputRepeat m_HoldRepeat { kHoldRepeatDelaySeconds, kHoldRepeatIntervalSeconds };

	/** 押しっぱなしにしている矢印の向き (0 なら押していない)。 */
	i32 m_HoldDirection = 0;

	/** 押しっぱなしにしている矢印を持つ行。所有はしない。 */
	const CDebugTopElement* m_HoldElement = nullptr;

	/** ドラッグ中の行。所有はしない (nullptr なら掴んでいない)。 */
	const CDebugTopElement* m_DragElement = nullptr;

	/** ドラッグ中のスライダーの左端 X。 */
	f32 m_DragSliderX = 0.0f;

	/** ドラッグ中のスライダーの幅 (0 なら行そのものへ位置を渡す)。 */
	f32 m_DragSliderWidth = 0.0f;

	/** ドラッグ中の行の内容の左端 X。 */
	f32 m_DragRowX = 0.0f;

	/** ドラッグ中の行の上端 Y。 */
	f32 m_DragRowY = 0.0f;

	/** ドラッグ中の行の幅。 */
	f32 m_DragRowWidth = 0.0f;

	/** ドラッグ中の行の高さ。 */
	f32 m_DragRowHeight = 0.0f;
};
