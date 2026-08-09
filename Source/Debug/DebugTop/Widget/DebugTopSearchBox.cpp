#include "DebugTopSearchBox.h"

#include "Debug/DebugTop/Input/DebugTopCursor.h"

namespace
{
	/** 一度に出す検索候補の上限 (多すぎると探すのが目的なのに探しにくくなる)。 */
	constexpr usize kMaxSearchHits = 8;

	/** 検索欄の幅 (行の高さに対する倍率)。 */
	constexpr f32 kSearchWidthRatio = 16.0f;

	/** 検索欄の内側の余白 (行の高さに対する倍率)。 */
	constexpr f32 kSearchPadRatio = 0.35f;

	/** 欄であることを示す小さな四角の一辺 (行の高さに対する倍率)。 */
	constexpr f32 kSearchIconRatio = 0.32f;

	/** 入力欄の内側の色 (沈めて「ここへ打てる」ことを示す)。 */
	constexpr FVec4 kInputFieldColor{ 0.04f, 0.05f, 0.07f, 1.0f };

	/** 打ち込んでいない検索欄の縁の色。 */
	constexpr FVec4 kSearchIdleBorder{ 0.30f, 0.34f, 0.42f, 1.0f };

	/** 打ち込んでいる検索欄の縁の色。 */
	constexpr FVec4 kSearchActiveBorder{ 0.45f, 0.70f, 0.98f, 1.0f };

	/** 検索欄へ出す案内の文字色。 */
	constexpr FVec4 kSearchHintColor{ 0.45f, 0.49f, 0.56f, 1.0f };

	/** 候補の下敷きの色。 */
	constexpr FVec4 kSearchPanelColor{ 0.07f, 0.08f, 0.11f, 1.0f };

	/** 選んでいる候補の下敷きの色。 */
	constexpr FVec4 kSearchSelectedColor{ 0.20f, 0.35f, 0.60f, 1.0f };

	/** 候補の行の文字色。 */
	constexpr FVec4 kSearchLabelColor{ 1.0f, 1.0f, 1.0f, 1.0f };

	/** 候補のページ名の文字色。 */
	constexpr FVec4 kSearchPageColor{ 0.60f, 0.75f, 0.95f, 1.0f };

	/** 検索欄へ出す案内。 */
	constexpr const char* kSearchHintText = "/ で検索";

	/** 枠線の太さ (ピクセル)。 */
	constexpr f32 kBorderThickness = 1.0f;
}


bool CDebugTopSearchBox::Update( f32 DeltaSeconds, bool bBlockOpen ) noexcept
{
	const FVec2 Mouse = CInput::MousePos();
	const bool bOverField = Mouse.x >= m_FieldX && Mouse.x <= m_FieldX + m_FieldWidth && Mouse.y >= m_FieldY && Mouse.y <= m_FieldY + m_FieldHeight;

	if ( !m_Edit.IsActive() )
	{
		// 値を打ち込んでいる最中は割り込ませない (「/」もその値の一部なので)。
		if ( bBlockOpen ) return false;

		// 欄の上ではカーソルを縦棒にして、押せば打てることを示す。
		if ( bOverField ) DebugTopSetCursor( EDebugTopCursor::Text );

		// 欄を押しても開く (「/」を知らなくても使えるように)。
		bool bOpen = bOverField && CInput::IsMouseButtonPressed( EMouseButton::Left );

		// 「/」で開く。キーコードではなく打たれた文字で見るので、配列が変わっても効く。
		if ( !bOpen )
		{
			const char* const Typed = CInput::TextInput();
			for ( const char* Cursor = Typed; Cursor != nullptr && *Cursor != '\0'; ++Cursor )
			{
				if ( *Cursor != '/' ) continue;

				bOpen = true;
				break;
			}
		}
		if ( !bOpen ) return false;

		m_Edit.Begin( FString() );
		m_Hits.Reset();
		m_Cursor = 0;
		m_bDirty = true;
		return true;
	}

	// 開いている間の押下。候補を押したらそれを選び、欄と候補の外を押したら閉じる。
	if ( CInput::IsMouseButtonPressed( EMouseButton::Left ) )
	{
		const i32 HitRow = ( m_ListHeight > 0.0f && m_RowHeight > 0.0f && Mouse.x >= m_FieldX && Mouse.x <= m_FieldX + m_FieldWidth && Mouse.y >= m_ListY && Mouse.y <= m_ListY + m_ListHeight )
			? static_cast<i32>( ( Mouse.y - m_ListY ) / m_RowHeight )
			: -1;

		if ( HitRow >= 0 && static_cast<usize>( HitRow ) < m_Hits.Num() )
		{
			m_Cursor = HitRow;
			m_Chosen = m_Hits[static_cast<usize>( HitRow )];
			m_bHasChosen = true;
			Close();
			DebugTopSetCursor( EDebugTopCursor::Arrow );
			return true;
		}
		if ( !bOverField )
		{
			Close();
			DebugTopSetCursor( EDebugTopCursor::Arrow );
			return true;
		}
	}

	// 取り消しは決定より先に見る。
	if ( CInput::IsKeyPressed( EKey::Escape ) )
	{
		Close();
		return true;
	}

	const FString Before = m_Edit.GetText();
	m_Edit.Update( DeltaSeconds );
	if ( !( m_Edit.GetText() == Before ) ) m_bDirty = true;

	// 候補の中を上下で選ぶ。一覧と同じく押しっぱなしで送れる。
	m_KeyNav.Update( DeltaSeconds );
	m_Cursor += m_KeyNav.GetVertical();

	if ( m_bDirty ) RebuildHits();

	const i32 HitCount = static_cast<i32>( m_Hits.Num() );
	if ( HitCount > 0 )
	{
		// 端で止めず回り込ませる (候補は数件なので、行き過ぎても戻すより早い)。
		m_Cursor %= HitCount;
		if ( m_Cursor < 0 ) m_Cursor += HitCount;
	}
	else
	{
		m_Cursor = 0;
	}

	FString Committed;
	if ( m_Edit.TryCommit( Committed ) )
	{
		if ( m_Cursor >= 0 && static_cast<usize>( m_Cursor ) < m_Hits.Num() )
		{
			m_Chosen = m_Hits[static_cast<usize>( m_Cursor )];
			m_bHasChosen = true;
		}
		m_Hits.Reset();
	}

	return true;
}

const FDebugTopSearchHit* CDebugTopSearchBox::ConsumeChosen() noexcept
{
	if ( !m_bHasChosen ) return nullptr;

	m_bHasChosen = false;
	return &m_Chosen;
}

void CDebugTopSearchBox::RebuildHits()
{
	m_bDirty = false;
	m_Hits.Reset();
	m_Collector.ExecuteIfBound( m_Edit.GetText(), m_Hits );

	// 一度に出せる数は欄の側の都合なので、集める側に上限を押し付けずここで落とす。
	if ( m_Hits.Num() > kMaxSearchHits ) m_Hits.SetNum( kMaxSearchHits );

	if ( m_Cursor >= static_cast<i32>( m_Hits.Num() ) ) m_Cursor = 0;
}

void CDebugTopSearchBox::Close() noexcept
{
	m_Edit.Cancel();
	m_Hits.Reset();
	m_ListHeight = 0.0f;
}

f32 CDebugTopSearchBox::MeasureWidth( const CDebugTopText& Text ) const noexcept
{
	return Text.LineHeight() * kSearchWidthRatio;
}

f32 CDebugTopSearchBox::Draw( CSpriteBatch& Batch, const CDebugTopText& Text, f32 OriginX, f32 OriginY ) noexcept
{
	const f32 LineHeight = Text.LineHeight();
	const f32 Pad = LineHeight * kSearchPadRatio;
	const f32 FieldW = LineHeight * kSearchWidthRatio;
	const f32 FieldH = LineHeight + Pad * 2.0f;
	const bool bActive = m_Edit.IsActive();

	// 押されたかの判定は次のフレームでこの矩形を見る。
	m_FieldX = OriginX;
	m_FieldY = OriginY;
	m_FieldWidth = FieldW;
	m_FieldHeight = FieldH;
	m_RowHeight = LineHeight;
	m_ListHeight = 0.0f;

	// 打っていないときも欄そのものは出しておく。ここへ打てることが見た目で分からないと、
	// どこからでも検索できても気付いてもらえない。
	Batch.DrawRect( OriginX, OriginY, FieldW, FieldH, kInputFieldColor );

	const FVec4 Border = bActive ? kSearchActiveBorder : kSearchIdleBorder;
	Batch.DrawRect( OriginX, OriginY, FieldW, kBorderThickness, Border );
	Batch.DrawRect( OriginX, OriginY + FieldH - kBorderThickness, FieldW, kBorderThickness, Border );
	Batch.DrawRect( OriginX, OriginY, kBorderThickness, FieldH, Border );
	Batch.DrawRect( OriginX + FieldW - kBorderThickness, OriginY, kBorderThickness, FieldH, Border );

	// 虫眼鏡の代わりに小さな四角を置いて、欄であることを分かりやすくする。
	const f32 IconSize = LineHeight * kSearchIconRatio;
	const f32 IconX = OriginX + Pad;
	Batch.DrawRect( IconX, OriginY + ( FieldH - IconSize ) * 0.5f, IconSize, IconSize, bActive ? kSearchActiveBorder : kSearchHintColor );

	const f32 TextX = IconX + IconSize + Pad;
	if ( bActive )
	{
		Text.Draw( Batch, m_Edit.MakeDisplayText().Data(), TextX, OriginY + Pad, kSearchActiveBorder );
	}
	else
	{
		Text.Draw( Batch, kSearchHintText, TextX, OriginY + Pad, kSearchHintColor );
	}

	if ( !bActive || m_Hits.IsEmpty() ) return FieldH;

	// 候補は欄のすぐ下へ重ねて出す。ページを開かずその場で選べるようにするため。
	const f32 ListY = OriginY + FieldH;
	const f32 ListH = LineHeight * static_cast<f32>( m_Hits.Num() );
	m_ListY = ListY;
	m_ListHeight = ListH;
	Batch.DrawRect( OriginX, ListY, FieldW, ListH, kSearchPanelColor );
	Batch.DrawRect( OriginX, ListY, kBorderThickness, ListH, Border );
	Batch.DrawRect( OriginX + FieldW - kBorderThickness, ListY, kBorderThickness, ListH, Border );
	Batch.DrawRect( OriginX, ListY + ListH - kBorderThickness, FieldW, kBorderThickness, Border );

	for ( usize Index = 0; Index < m_Hits.Num(); ++Index )
	{
		const FDebugTopSearchHit& Hit = m_Hits[Index];
		const f32 RowY = ListY + LineHeight * static_cast<f32>( Index );

		if ( static_cast<i32>( Index ) == m_Cursor )
		{
			Batch.DrawRect( OriginX + kBorderThickness, RowY, FieldW - kBorderThickness * 2.0f, LineHeight, kSearchSelectedColor );
		}

		Text.Draw( Batch, Hit.GetLabelText(), OriginX + Pad * 2.0f, RowY, kSearchLabelColor );

		// どのページの行かが分からないと、飛んだ後に迷う。
		const char* const PageName = Hit.GetPageText();
		const f32 PageX = OriginX + FieldW - Pad * 2.0f - Text.MeasureWidth( PageName );
		Text.Draw( Batch, PageName, PageX, RowY, kSearchPageColor );
	}
	return FieldH;
}
