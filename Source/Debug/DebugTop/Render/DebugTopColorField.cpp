#include "DebugTopColorField.h"

namespace
{
	/**
	 * 色相の帯を作る区間の数。
	 *
	 * @details
	 * 色相は 6 つの原色 (赤→黄→緑→水→青→紫→赤) の間を RGB で直線に進むので、その 6 区間を
	 * 頂点カラーで繋ぐと階調ではなく本物の連続したグラデーションになる。
	 */
	constexpr i32 kColorBarSegments = 6;

	/** 選んでいる位置を示す印の太さ (ピクセル)。 */
	constexpr f32 kColorMarkThickness = 2.0f;

	/** 十字の腕の長さ (面の高さに対する倍率)。 */
	constexpr f32 kColorMarkSpanRatio = 0.06f;

	/** 色相の帯の上下に空ける余白 (帯の高さに対する割合)。 */
	constexpr f32 kColorBarInsetRatio = 0.15f;

	/**
	 * 四隅に別々の色を置いた矩形を描く。
	 *
	 * @details
	 * CSpriteBatch::DrawTriangleVC は頂点の色をシェーダが補間するので、これを 2 枚並べれば
	 * 塗り分けではなく本物のグラデーションになる。
	 * @param Batch 描画コマンドを積む先。
	 * @param X 左上 X。
	 * @param Y 左上 Y。
	 * @param W 幅。
	 * @param H 高さ。
	 * @param TopLeft 左上の色。
	 * @param TopRight 右上の色。
	 * @param BottomRight 右下の色。
	 * @param BottomLeft 左下の色。
	 */
	void DrawGradientRect( CSpriteBatch& Batch, f32 X, f32 Y, f32 W, f32 H, const FVec4& TopLeft, const FVec4& TopRight, const FVec4& BottomRight, const FVec4& BottomLeft ) noexcept
	{
		const f32 R = X + W;
		const f32 B = Y + H;
		Batch.DrawTriangleVC( X, Y, R, Y, R, B, TopLeft, TopRight, BottomRight );
		Batch.DrawTriangleVC( X, Y, R, B, X, B, TopLeft, BottomRight, BottomLeft );
	}
}


FVec4 DebugTopMakeHsvColor( f32 Hue, f32 Saturation, f32 Value ) noexcept
{
	const f32 Scaled = ( Hue - static_cast<f32>( static_cast<i32>( Hue ) ) ) * 6.0f;
	const i32 Sector = static_cast<i32>( Scaled );
	const f32 Fraction = Scaled - static_cast<f32>( Sector );

	const f32 Low = Value * ( 1.0f - Saturation );
	const f32 Falling = Value * ( 1.0f - Saturation * Fraction );
	const f32 Rising = Value * ( 1.0f - Saturation * ( 1.0f - Fraction ) );

	switch ( Sector )
	{
	case 0:  return FVec4{ Value,   Rising,  Low,     1.0f };
	case 1:  return FVec4{ Falling, Value,   Low,     1.0f };
	case 2:  return FVec4{ Low,     Value,   Rising,  1.0f };
	case 3:  return FVec4{ Low,     Falling, Value,   1.0f };
	case 4:  return FVec4{ Rising,  Low,     Value,   1.0f };
	default: return FVec4{ Value,   Low,     Falling, 1.0f };
	}
}

void DebugTopDrawColorField( CSpriteBatch& Batch, f32 X, f32 Y, f32 Height, f32 Hue, f32 Saturation, f32 Value, f32 Opacity ) noexcept
{
	const f32 FieldWidth = Height * kDebugTopColorFieldAspectRatio;
	const f32 PlaneHeight = Height * kDebugTopColorFieldPlaneRatio;

	// 彩度・明度の面は HSV の定義そのものを 2 枚重ねて作る。
	//   HSV(h,s,v) = lerp(白, 原色(h), s) * v
	// なので「左が白・右が原色」の横グラデーションへ、「上が透明・下が黒」の縦グラデーションを
	// 重ねると、掛け算がそのままアルファ合成で出る。格子で近似する必要がない。
	FVec4 Pure = DebugTopMakeHsvColor( Hue, 1.0f, 1.0f );
	Pure.w = Opacity;
	const FVec4 White{ 1.0f, 1.0f, 1.0f, Opacity };
	DrawGradientRect( Batch, X, Y, FieldWidth, PlaneHeight, White, Pure, Pure, White );

	const FVec4 Clear{ 0.0f, 0.0f, 0.0f, 0.0f };
	const FVec4 Black{ 0.0f, 0.0f, 0.0f, Opacity };
	DrawGradientRect( Batch, X, Y, FieldWidth, PlaneHeight, Clear, Clear, Black, Black );

	// 色相の帯。原色から原色へ RGB が直線に進むので、その 6 区間を頂点カラーで繋ぐ。
	const f32 BarY = Y + PlaneHeight;
	const f32 BarHeight = Height - PlaneHeight;
	const f32 SegmentW = FieldWidth / static_cast<f32>( kColorBarSegments );
	for ( i32 Segment = 0; Segment < kColorBarSegments; ++Segment )
	{
		FVec4 Left = DebugTopMakeHsvColor( static_cast<f32>( Segment ) / static_cast<f32>( kColorBarSegments ), 1.0f, 1.0f );
		FVec4 Right = DebugTopMakeHsvColor( static_cast<f32>( Segment + 1 ) / static_cast<f32>( kColorBarSegments ), 1.0f, 1.0f );
		Left.w = Opacity;
		Right.w = Opacity;
		DrawGradientRect( Batch, X + SegmentW * static_cast<f32>( Segment ), BarY + BarHeight * kColorBarInsetRatio, SegmentW, BarHeight * ( 1.0f - kColorBarInsetRatio * 2.0f ), Left, Right, Right, Left );
	}

	// いま選んでいる色相の位置に印を置く。端でも見えるよう幅の内側へ収める。
	const FVec4 MarkColor{ 1.0f, 1.0f, 1.0f, Opacity };
	const FVec4 EdgeColor{ 0.0f, 0.0f, 0.0f, Opacity };
	f32 HueX = X + FieldWidth * Hue - kColorMarkThickness * 0.5f;
	if ( HueX < X ) HueX = X;
	if ( HueX > X + FieldWidth - kColorMarkThickness ) HueX = X + FieldWidth - kColorMarkThickness;
	Batch.DrawRect( HueX - 1.0f, BarY + BarHeight * 0.1f, kColorMarkThickness + 2.0f, BarHeight * 0.8f, EdgeColor );
	Batch.DrawRect( HueX, BarY + BarHeight * 0.1f, kColorMarkThickness, BarHeight * 0.8f, MarkColor );

	// 面のどこを選んでいるかを十字で示す。これが無いと、いまの色が面のどこに当たるか
	// 分からず、微調整のたびに当てずっぽうで押すことになる。
	const f32 MarkSpan = PlaneHeight * kColorMarkSpanRatio;

	// 端 (彩度 1、明度 0 や 1) では十字の腕が面からはみ出して枠へ乗るので、内側へ収める。
	// 色相の印と揃える。
	f32 MarkX = X + FieldWidth * Saturation;
	f32 MarkY = Y + PlaneHeight * ( 1.0f - Value );
	if ( MarkX < X + MarkSpan )                       MarkX = X + MarkSpan;
	if ( MarkX > X + FieldWidth - MarkSpan )          MarkX = X + FieldWidth - MarkSpan;
	if ( MarkY < Y + MarkSpan )                       MarkY = Y + MarkSpan;
	if ( MarkY > Y + PlaneHeight - MarkSpan )         MarkY = Y + PlaneHeight - MarkSpan;

	// 明るい色の上でも暗い色の上でも見えるよう、黒で縁取ってから白を重ねる。
	Batch.DrawRect( MarkX - MarkSpan - 1.0f, MarkY - kColorMarkThickness * 0.5f - 1.0f, MarkSpan * 2.0f + 2.0f, kColorMarkThickness + 2.0f, EdgeColor );
	Batch.DrawRect( MarkX - kColorMarkThickness * 0.5f - 1.0f, MarkY - MarkSpan - 1.0f, kColorMarkThickness + 2.0f, MarkSpan * 2.0f + 2.0f, EdgeColor );
	Batch.DrawRect( MarkX - MarkSpan, MarkY - kColorMarkThickness * 0.5f, MarkSpan * 2.0f, kColorMarkThickness, MarkColor );
	Batch.DrawRect( MarkX - kColorMarkThickness * 0.5f, MarkY - MarkSpan, kColorMarkThickness, MarkSpan * 2.0f, MarkColor );
}
