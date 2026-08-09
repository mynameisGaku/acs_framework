#pragma once

#include <acs.h>

using namespace acs;

/**
 * 三角マーカーの向き。
 */
enum class EDebugTopTriangle : u8
{
	/** 左向き (値を減らす矢印)。 */
	Left,

	/** 右向き (値を増やす矢印 / 折り畳み中の行のマーカー)。 */
	Right,

	/** 下向き (展開中の行のマーカー / 下に続きがある印)。 */
	Down,

	/** 上向き (上に続きがある印)。 */
	Up,
};


/**
 * 文字サイズを指定して描くための、フォントと倍率の組。
 *
 * @details
 * CSpriteBatch::DrawString はアトラスへ焼いたピクセルサイズでしか描けないため、グリフを
 * 自前で並べて任意サイズへ拡縮する。計測 (MeasureWidth) と描画で同じ倍率を通すので、
 * 測った幅と実際に描かれる幅が食い違わない。
 * 焼いたサイズより大きく描くとにじむため、拡大して使うなら ADebugTopHUD::SetFontSize で
 * そのサイズ専用のアトラスを焼かせる (この型はそのフォントをそのまま等倍で描く)。
 * アトラスに無いグリフ (既定のアトラスは ASCII・仮名・全角記号のみで、漢字や ← ▼ 等の
 * 記号を持たない) は描かれず幅も 0 になる。
 */
class CDebugTopText
{
public:
	/** 無効な状態で構築する (描画も計測も何もしない)。 */
	CDebugTopText() noexcept = default;

	/**
	 * 描画に使うフォントと文字サイズを束ねて構築する。
	 *
	 * @param Font 描画に使うフォント (寿命は呼び出し側が保証する)。
	 * @param FontSize 描きたい文字のピクセルサイズ (0 以下でフォント本来のサイズ)。
	 */
	CDebugTopText( const FFont& Font, f32 FontSize ) noexcept;

	/** 描画できる状態かを返す。 */
	bool IsValid() const noexcept { return m_Font != nullptr; }

	/** 描画に使うフォントを返す。 */
	const FFont& GetFont() const noexcept { return *m_Font; }

	/** フォント本来のサイズに対する拡縮率を返す (1 なら等倍)。 */
	f32 GetScale() const noexcept { return m_Scale; }

	/** 1 行の高さ (ピクセル) を返す。 */
	f32 LineHeight() const noexcept;

	/**
	 * 描画幅を測る。
	 *
	 * @param Text 計測する UTF-8 文字列。
	 * @return 幅 (ピクセル)。改行を含む場合は最も長い行の幅。
	 */
	f32 MeasureWidth( const char* Text ) const noexcept;

	/**
	 * 描画高さを測る。
	 *
	 * @param Text 計測する UTF-8 文字列。
	 * @return 高さ (ピクセル)。行数 × 行の高さ。
	 */
	f32 MeasureHeight( const char* Text ) const noexcept;

	/**
	 * UTF-8 テキストを描く。
	 *
	 * @details \\n で改行する。アトラスに無いグリフは飛ばす。
	 * @param Batch 描画コマンドを積む先。
	 * @param Text 描く UTF-8 文字列。
	 * @param X 行の左上 X (ピクセル)。
	 * @param Y 行の左上 Y (ピクセル)。
	 * @param Color 文字色。
	 */
	void Draw( CSpriteBatch& Batch, const char* Text, f32 X, f32 Y, const FVec4& Color ) const noexcept;

private:
	/** 描画に使うフォント。所有はしない。 */
	const FFont* m_Font = nullptr;

	/** フォント本来のサイズに対する拡縮率。 */
	f32 m_Scale = 1.0f;
};


/**
 * 三角マーカーを描く。
 *
 * @details
 * 矢印 (←→) や展開マーカー (▼▽) は既定のグリフアトラスに無く、文字として描いても
 * 何も出ない。文字に頼らず図形で描くことで、どのフォントでも必ず出るようにする。
 * @param Batch 描画コマンドを積む先。
 * @param Direction 三角形の向き。
 * @param X 収める矩形の左端 X (ピクセル)。
 * @param Y 収める矩形の上端 Y (ピクセル)。
 * @param Width 収める矩形の幅 (ピクセル)。
 * @param Height 収める矩形の高さ (ピクセル)。
 * @param Color 塗りつぶし色。
 */
void DebugTopDrawTriangle( CSpriteBatch& Batch, EDebugTopTriangle Direction, f32 X, f32 Y, f32 Width, f32 Height, const FVec4& Color ) noexcept;

/**
 * チェックボックスを描く。
 *
 * @details
 * 枠と、チェック時のレ点を図形で描く。ON / OFF の文字より、並んだときに状態を追いやすい。
 * @param Batch 描画コマンドを積む先。
 * @param X 左端 X (ピクセル)。
 * @param Y 上端 Y (ピクセル)。
 * @param Size 一辺の長さ (ピクセル)。
 * @param bChecked チェックを入れるか。
 * @param Color 枠とレ点の色。
 */
void DebugTopDrawCheckBox( CSpriteBatch& Batch, f32 X, f32 Y, f32 Size, bool bChecked, const FVec4& Color ) noexcept;

/**
 * 太さを持った線分を描く。
 *
 * @details CSpriteBatch に線の描画が無いので、三角形 2 枚の帯として描く。
 * @param Batch 描画コマンドを積む先。
 * @param X1 始点 X (ピクセル)。
 * @param Y1 始点 Y (ピクセル)。
 * @param X2 終点 X (ピクセル)。
 * @param Y2 終点 Y (ピクセル)。
 * @param Thickness 線の太さ (ピクセル)。
 * @param Color 線の色。
 */
void DebugTopDrawLine( CSpriteBatch& Batch, f32 X1, f32 Y1, f32 X2, f32 Y2, f32 Thickness, const FVec4& Color ) noexcept;

/**
 * 値の位置を示すバーを描く。
 *
 * @details 下限から上限までを溝で表し、現在位置までを塗る。数字だけより増減が掴みやすい。
 * @param Batch 描画コマンドを積む先。
 * @param X 左端 X (ピクセル)。
 * @param Y 上端 Y (ピクセル)。
 * @param Width 溝の幅 (ピクセル)。
 * @param Height 溝の高さ (ピクセル)。
 * @param Ratio 下限を 0、上限を 1 としたときの現在位置。
 * @param Color 塗りの色 (溝は薄く描く)。
 */
void DebugTopDrawSlider( CSpriteBatch& Batch, f32 X, f32 Y, f32 Width, f32 Height, f32 Ratio, const FVec4& Color ) noexcept;
