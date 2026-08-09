#include "DebugTopFilterBox.h"

namespace
{
	/** 絞り込みを開く文字。検索の「/」と並べて覚えられるよう、隣のキーにする。 */
	constexpr char kOpenChar = '.';

	/** 欄の内側の余白 (行の高さに対する倍率)。 */
	constexpr f32 kPadRatio = 0.25f;

	/** 欄の色 (沈めて「ここへ打てる」ことを示す)。 */
	constexpr FVec4 kFieldColor{ 0.04f, 0.05f, 0.07f, 1.0f };

	/** 打ち込んでいる欄の縁の色。 */
	constexpr FVec4 kActiveBorderColor{ 0.98f, 0.78f, 0.35f, 1.0f };

	/** 打ち終えた欄の縁の色 (絞り込みが効いたままであることを示す)。 */
	constexpr FVec4 kIdleBorderColor{ 0.60f, 0.50f, 0.30f, 1.0f };

	/** 欄の文字色。 */
	constexpr FVec4 kTextColor{ 0.98f, 0.88f, 0.60f, 1.0f };

	/** 枠線の太さ (ピクセル)。 */
	constexpr f32 kBorderWidth = 1.0f;
}


void CDebugTopFilterBox::Clear() noexcept
{
	m_Edit.Cancel();
	m_Filter = FString();
}

bool CDebugTopFilterBox::Update( f32 DeltaSeconds, bool bBlockOpen ) noexcept
{
	if ( !m_Edit.IsActive() )
	{
		if ( bBlockOpen ) return false;

		// 「.」で開く。キーコードではなく打たれた文字で見るので、配列が変わっても効く。
		bool bOpen = false;
		const char* const Typed = CInput::TextInput();
		for ( const char* Cursor = Typed; Cursor != nullptr && *Cursor != '\0'; ++Cursor )
		{
			if ( *Cursor != kOpenChar ) continue;

			bOpen = true;
			break;
		}
		if ( !bOpen ) return false;

		// 前の語から続けて詰められるようにしておく (打ち直しの手間を省く)。
		m_Edit.Begin( m_Filter );
		return true;
	}

	// 取り消しは絞り込みごと解除する。元の一覧へ一息で戻れるようにするため。
	if ( CInput::IsKeyPressed( EKey::Escape ) )
	{
		Clear();
		return true;
	}

	m_Edit.Update( DeltaSeconds );

	// 一文字ごとに反映する。何が残るかを見ながら詰められる。
	m_Filter = m_Edit.GetText();

	FString Committed;
	if ( m_Edit.TryCommit( Committed ) ) m_Filter = Committed;

	return true;
}

f32 CDebugTopFilterBox::Draw( CSpriteBatch& Batch, const CDebugTopText& Text, f32 OriginX, f32 OriginY, usize MatchCount ) noexcept
{
	// 常設しない。絞り込んでいないときは一覧の場所を空けておく。
	const bool bActive = m_Edit.IsActive();
	if ( !bActive && m_Filter.IsEmpty() ) return 0.0f;

	const f32 LineHeight = Text.LineHeight();
	if ( LineHeight <= 0.0f ) return 0.0f;

	FString Label( "絞り込み: " );
	Label.Append( bActive ? m_Edit.MakeDisplayText().View() : m_Filter.View() );
	Label.AppendFormat( "   (%zu 行)", MatchCount );

	const f32 Pad = LineHeight * kPadRatio;
	const f32 Width = Text.MeasureWidth( Label.Data() ) + Pad * 2.0f;
	const f32 Height = LineHeight + Pad * 2.0f;

	Batch.DrawRect( OriginX, OriginY, Width, Height, kFieldColor );

	const FVec4 Border = bActive ? kActiveBorderColor : kIdleBorderColor;
	Batch.DrawRect( OriginX, OriginY, Width, kBorderWidth, Border );
	Batch.DrawRect( OriginX, OriginY + Height - kBorderWidth, Width, kBorderWidth, Border );
	Batch.DrawRect( OriginX, OriginY, kBorderWidth, Height, Border );
	Batch.DrawRect( OriginX + Width - kBorderWidth, OriginY, kBorderWidth, Height, Border );

	Text.Draw( Batch, Label.Data(), OriginX + Pad, OriginY + Pad, kTextColor );
	return Height;
}
