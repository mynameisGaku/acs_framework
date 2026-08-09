#include "DebugTopDraw.h"

#include <cmath>


void DebugTopDrawLine( CSpriteBatch& Batch, f32 X1, f32 Y1, f32 X2, f32 Y2, f32 Thickness, const FVec4& Color ) noexcept
{
	const f32 Dx = X2 - X1;
	const f32 Dy = Y2 - Y1;
	const f32 LengthSq = Dx * Dx + Dy * Dy;
	if ( LengthSq <= 0.0f )
	{
		// 長さが無い線は点として置く (標本が 1 つだけのとき)。
		Batch.DrawRect( X1 - Thickness * 0.5f, Y1 - Thickness * 0.5f, Thickness, Thickness, Color );
		return;
	}

	// 線に垂直な向きへ太さの半分ずつ広げて、帯を三角形 2 枚で埋める。
	const f32 Length = std::sqrt( LengthSq );
	const f32 HalfX = -Dy / Length * Thickness * 0.5f;
	const f32 HalfY = Dx / Length * Thickness * 0.5f;

	Batch.DrawTriangle( X1 + HalfX, Y1 + HalfY, X1 - HalfX, Y1 - HalfY, X2 + HalfX, Y2 + HalfY, Color );
	Batch.DrawTriangle( X1 - HalfX, Y1 - HalfY, X2 - HalfX, Y2 - HalfY, X2 + HalfX, Y2 + HalfY, Color );
}


CDebugTopText::CDebugTopText( const FFont& Font, f32 FontSize ) noexcept
	: m_Font( &Font )
{
	// 焼いたサイズが取れない (未ロード) 場合は等倍のままにしておく。
	const f32 PixelSize = Font.PixelSize();
	if ( PixelSize > 0.0f && FontSize > 0.0f )
	{
		m_Scale = FontSize / PixelSize;
	}
}

f32 CDebugTopText::LineHeight() const noexcept
{
	if ( m_Font == nullptr ) return 0.0f;
	return m_Font->LineHeight() * m_Scale;
}

f32 CDebugTopText::MeasureWidth( const char* Text ) const noexcept
{
	if ( m_Font == nullptr || Text == nullptr ) return 0.0f;

	// FFont::MeasureWidth は改行を無視して全文字を足すため、行ごとに測り直す。
	f32 MaxWidth = 0.0f;
	f32 LineWidth = 0.0f;
	const char* Cursor = Text;
	while ( true )
	{
		const u32 CodePoint = DecodeUtf8( &Cursor );
		if ( CodePoint == 0 ) break;

		if ( CodePoint == '\n' )
		{
			if ( LineWidth > MaxWidth ) MaxWidth = LineWidth;
			LineWidth = 0.0f;
			continue;
		}

		FGlyphInfo Glyph{};
		if ( !m_Font->GetGlyph( CodePoint, Glyph ) ) continue;

		LineWidth += Glyph.x_advance;
	}
	if ( LineWidth > MaxWidth ) MaxWidth = LineWidth;

	return MaxWidth * m_Scale;
}

f32 CDebugTopText::MeasureHeight( const char* Text ) const noexcept
{
	if ( m_Font == nullptr || Text == nullptr ) return 0.0f;

	i32 LineCount = 1;
	for ( const char* Cursor = Text; *Cursor != '\0'; ++Cursor )
	{
		if ( *Cursor == '\n' ) ++LineCount;
	}
	return static_cast<f32>( LineCount ) * LineHeight();
}

void CDebugTopText::Draw( CSpriteBatch& Batch, const char* Text, f32 X, f32 Y, const FVec4& Color ) const noexcept
{
	if ( m_Font == nullptr || Text == nullptr ) return;

	IRhiTexture* const Atlas = m_Font->AtlasTexture();
	if ( Atlas == nullptr ) return;

	// (X, Y) は行の左上。グリフのオフセットはベースライン基準なので基準線を持ち回る。
	f32 PenX = X;
	f32 Baseline = Y + m_Font->Ascent() * m_Scale;

	const char* Cursor = Text;
	while ( true )
	{
		const u32 CodePoint = DecodeUtf8( &Cursor );
		if ( CodePoint == 0 ) break;

		if ( CodePoint == '\n' )
		{
			PenX = X;
			Baseline += LineHeight();
			continue;
		}

		FGlyphInfo Glyph{};
		if ( !m_Font->GetGlyph( CodePoint, Glyph ) ) continue;

		Batch.DrawSub( *Atlas, PenX + Glyph.x_offset * m_Scale, Baseline + Glyph.y_offset * m_Scale, Glyph.width * m_Scale, Glyph.height * m_Scale, Glyph.u0, Glyph.v0, Glyph.u1, Glyph.v1, Color );
		PenX += Glyph.x_advance * m_Scale;
	}
}


void DebugTopDrawCheckBox( CSpriteBatch& Batch, f32 X, f32 Y, f32 Size, bool bChecked, const FVec4& Color ) noexcept
{
	if ( Size <= 0.0f ) return;

	/** 枠線の太さ。細くしすぎると小さいサイズで消えるので下限を設ける。 */
	f32 Border = Size * 0.1f;
	if ( Border < 1.0f ) Border = 1.0f;

	// 枠を 4 辺の細い矩形で描く (塗りつぶしの矩形しか無いため)。
	Batch.DrawRect( X, Y, Size, Border, Color );
	Batch.DrawRect( X, Y + Size - Border, Size, Border, Color );
	Batch.DrawRect( X, Y, Border, Size, Color );
	Batch.DrawRect( X + Size - Border, Y, Border, Size, Color );

	if ( !bChecked ) return;

	// レ点は太さを持たせた 2 本の線。三角形 2 枚ずつで幅のある線を作る。
	const f32 Thickness = Size * 0.16f;
	const f32 ShortFromX = X + Size * 0.22f;
	const f32 ShortFromY = Y + Size * 0.52f;
	const f32 CornerX = X + Size * 0.42f;
	const f32 CornerY = Y + Size * 0.72f;
	const f32 LongToX = X + Size * 0.78f;
	const f32 LongToY = Y + Size * 0.28f;

	Batch.DrawTriangle( ShortFromX, ShortFromY, CornerX, CornerY, CornerX, CornerY - Thickness, Color );
	Batch.DrawTriangle( ShortFromX, ShortFromY, ShortFromX, ShortFromY - Thickness, CornerX, CornerY - Thickness, Color );
	Batch.DrawTriangle( CornerX, CornerY, LongToX, LongToY, LongToX, LongToY - Thickness, Color );
	Batch.DrawTriangle( CornerX, CornerY, CornerX, CornerY - Thickness, LongToX, LongToY - Thickness, Color );
}


void DebugTopDrawSlider( CSpriteBatch& Batch, f32 X, f32 Y, f32 Width, f32 Height, f32 Ratio, const FVec4& Color ) noexcept
{
	if ( Width <= 0.0f || Height <= 0.0f ) return;

	if ( Ratio < 0.0f ) Ratio = 0.0f;
	if ( Ratio > 1.0f ) Ratio = 1.0f;

	// 溝は同じ色を薄くして描く (別の色を持たせると行ごとの色指定と喧嘩するため)。
	const FVec4 GrooveColor{ Color.x, Color.y, Color.z, Color.w * 0.25f };
	Batch.DrawRect( X, Y, Width, Height, GrooveColor );

	const f32 FilledWidth = Width * Ratio;
	if ( FilledWidth > 0.0f ) Batch.DrawRect( X, Y, FilledWidth, Height, Color );

	// つまみ。溝だけだと端の位置が読み取りにくい。
	const f32 KnobWidth = Height;
	f32 KnobX = X + FilledWidth - KnobWidth * 0.5f;
	if ( KnobX < X ) KnobX = X;
	if ( KnobX > X + Width - KnobWidth ) KnobX = X + Width - KnobWidth;
	Batch.DrawRect( KnobX, Y - Height * 0.5f, KnobWidth, Height * 2.0f, Color );
}


void DebugTopDrawTriangle( CSpriteBatch& Batch, EDebugTopTriangle Direction, f32 X, f32 Y, f32 Width, f32 Height, const FVec4& Color ) noexcept
{
	if ( Width <= 0.0f || Height <= 0.0f ) return;

	const f32 Right = X + Width;
	const f32 Bottom = Y + Height;
	const f32 CenterX = X + Width * 0.5f;
	const f32 CenterY = Y + Height * 0.5f;

	switch ( Direction )
	{
	case EDebugTopTriangle::Left:
		Batch.DrawTriangle( Right, Y, Right, Bottom, X, CenterY, Color );
		break;

	case EDebugTopTriangle::Right:
		Batch.DrawTriangle( X, Y, X, Bottom, Right, CenterY, Color );
		break;

	case EDebugTopTriangle::Up:
		Batch.DrawTriangle( X, Bottom, Right, Bottom, CenterX, Y, Color );
		break;

	default:
		Batch.DrawTriangle( X, Y, Right, Y, CenterX, Bottom, Color );
		break;
	}
}
