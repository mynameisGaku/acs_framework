// SPDX-License-Identifier: Apache-2.0
#include "DebugTopPathBrowser.h"

namespace
{
	/** 一覧の幅 (行の高さに対する倍率)。 */
	constexpr f32 kWidthRatio = 26.0f;

	/** 一覧に出す行数の上限 (画面に入りきらなければ更に減る)。 */
	constexpr i32 kMaxRows = 16;

	/** 内側の余白 (行の高さに対する倍率)。 */
	constexpr f32 kPadRatio = 0.5f;

	/** 段差 (行の高さに対する倍率)。名前をアイコンぶん右へ寄せる。 */
	constexpr f32 kIndentRatio = 1.2f;

	/** 下敷きの色。透かすと後ろのメニューの文字が混ざって読みにくいので、完全に塗る。 */
	constexpr FVec4 kPanelColor{ 0.05f, 0.06f, 0.09f, 1.0f };

	/** 縁の色。 */
	constexpr FVec4 kBorderColor{ 0.45f, 0.55f, 0.70f, 0.9f };

	/** 見出し (いま開いている場所) の色。 */
	constexpr FVec4 kHeaderColor{ 0.60f, 0.75f, 0.95f, 1.0f };

	/** 絞り込みの語の色。 */
	constexpr FVec4 kFilterColor{ 0.98f, 0.88f, 0.60f, 1.0f };

	/** 選んでいる行の下敷きの色。 */
	constexpr FVec4 kCursorColor{ 0.25f, 0.38f, 0.62f, 1.0f };

	/** フォルダの名前の色。 */
	constexpr FVec4 kFolderColor{ 0.98f, 0.85f, 0.55f, 1.0f };

	/** ファイルの名前の色。 */
	constexpr FVec4 kFileColor{ 0.88f, 0.90f, 0.94f, 1.0f };

	/** 操作の案内の色。 */
	constexpr FVec4 kHintColor{ 0.60f, 0.64f, 0.70f, 1.0f };

	/** 枠線の太さ (ピクセル)。 */
	constexpr f32 kBorderWidth = 1.0f;

	/**
	 * 名前に語が含まれるかを、大小の区別なく調べる。
	 *
	 * @param Name 調べる名前。
	 * @param Query 探す語 (空なら常に true)。
	 * @return 含まれていたら true。
	 */
	bool NameContains( const FString& Name, const FString& Query ) noexcept
	{
		if ( Query.IsEmpty() ) return true;
		if ( Query.Size() > Name.Size() ) return false;

		const usize Last = Name.Size() - Query.Size();
		for ( usize Start = 0; Start <= Last; ++Start )
		{
			usize Index = 0;
			while ( Index < Query.Size() && CFileSystem::AsciiLower( Name.Data()[Start + Index] ) == CFileSystem::AsciiLower( Query.Data()[Index] ) ) ++Index;

			if ( Index == Query.Size() ) return true;
		}
		return false;
	}

	/**
	 * ファイルの大きさを、桁を落とした文字列にする。
	 *
	 * @param Size バイト数。
	 * @return 「12.3 MB」のような文字列。
	 */
	FString FormatSize( u64 Size )
	{
		FString Text;
		if ( Size < 1024ull ) Text.AppendFormat( "%llu B", static_cast<unsigned long long>( Size ) );
		else if ( Size < 1024ull * 1024ull ) Text.AppendFormat( "%.1f KB", static_cast<f64>( Size ) / 1024.0 );
		else if ( Size < 1024ull * 1024ull * 1024ull ) Text.AppendFormat( "%.1f MB", static_cast<f64>( Size ) / ( 1024.0 * 1024.0 ) );
		else Text.AppendFormat( "%.1f GB", static_cast<f64>( Size ) / ( 1024.0 * 1024.0 * 1024.0 ) );
		return Text;
	}
}


CDebugTopPathBrowser* CDebugTopPathBrowser::s_Active = nullptr;


bool DebugTopBrowsePath( EDebugTopPickKind Kind, const FString& Initial, FDebugTopPathChosen OnChosen )
{
	CDebugTopPathBrowser* const Active = CDebugTopPathBrowser::GetActive();
	if ( Active == nullptr ) return false;

	Active->Open( Kind, Initial, OnChosen );
	return true;
}


void CDebugTopPathBrowser::Open( EDebugTopPickKind Kind, const FString& Initial, FDebugTopPathChosen OnChosen )
{
	m_Kind = Kind;
	m_OnChosen = OnChosen;
	m_bOpen = true;
	m_Filter = FString();
	m_Cursor = 0;
	m_Scroll = 0;

	// ファイルのパスを渡された場合は、その親を開いて当のファイルへカーソルを合わせる。
	FString Focus;
	m_Directory = Initial;
	if ( !m_Directory.IsEmpty() && !DebugTopIsDirectory( m_Directory ) )
	{
		const FString Parent = DebugTopParentPath( m_Directory );

		// 親が取れないほど短いものは、そもそもパスとして扱えない。
		if ( Parent.IsEmpty() ) m_Directory = FString();
		else
		{
			Focus = FString( FStringView( m_Directory.Data() + Parent.Size() + 1, m_Directory.Size() - Parent.Size() - 1 ) );
			m_Directory = Parent;
		}
	}

	Rebuild( Focus );
}


void CDebugTopPathBrowser::Close() noexcept
{
	m_bOpen = false;
	m_Entries.Reset();
	m_Visible.Reset();
	m_Filter = FString();
}


void CDebugTopPathBrowser::Rebuild( const FString& Focus )
{
	m_Entries.Reset();

	if ( m_Directory.IsEmpty() )
	{
		// 根より上。ドライブを並べて、そこから選ばせる。
		TArray<FString> Drives;
		DebugTopReadDrives( Drives );
		for ( usize Index = 0; Index < Drives.Num(); ++Index )
		{
			FDebugTopDirEntry Entry;
			Entry.Name = Drives[Index];
			Entry.bDirectory = true;
			m_Entries.Add( Move( Entry ) );
		}
	}
	else
	{
		DebugTopReadDirectory( m_Directory, m_Entries );
	}

	// フォルダを選ばせているときにファイルを並べても選べないので、出さない。
	m_Visible.Reset();
	for ( usize Index = 0; Index < m_Entries.Num(); ++Index )
	{
		if ( m_Kind == EDebugTopPickKind::Folder && !m_Entries[Index].bDirectory ) continue;
		if ( !NameContains( m_Entries[Index].Name, m_Filter ) ) continue;

		m_Visible.Add( m_Entries[Index] );
	}

	m_Cursor = 0;
	m_Scroll = 0;
	if ( Focus.IsEmpty() ) return;

	for ( usize Index = 0; Index < m_Visible.Num(); ++Index )
	{
		if ( !( m_Visible[Index].Name == Focus ) ) continue;

		m_Cursor = static_cast<i32>( Index );
		break;
	}
}


void CDebugTopPathBrowser::GoUp()
{
	// いま居た場所を控えておき、上がった先でそこへカーソルを合わせる (辿り直しやすい)。
	const FString Previous = m_Directory;
	const FString Parent = DebugTopParentPath( m_Directory );

	FString Focus;
	if ( !Parent.IsEmpty() && Previous.Size() > Parent.Size() + 1 )
	{
		Focus = FString( FStringView( Previous.Data() + Parent.Size() + 1, Previous.Size() - Parent.Size() - 1 ) );
	}
	else
	{
		Focus = Previous;
	}

	m_Directory = Parent;
	m_Filter = FString();
	Rebuild( Focus );
}


void CDebugTopPathBrowser::Decide()
{
	if ( m_Visible.IsEmpty() ) return;
	if ( m_Cursor < 0 || static_cast<usize>( m_Cursor ) >= m_Visible.Num() ) return;

	const FDebugTopDirEntry& Entry = m_Visible[static_cast<usize>( m_Cursor )];
	if ( Entry.bDirectory )
	{
		// ドライブの一覧から選んだときは、名前そのものが根のパスになっている。
		m_Directory = m_Directory.IsEmpty() ? Entry.Name : DebugTopJoinPath( m_Directory, Entry.Name );
		m_Filter = FString();
		Rebuild( FString() );
		return;
	}

	Finish( DebugTopJoinPath( m_Directory, Entry.Name ) );
}


void CDebugTopPathBrowser::ChooseCurrentFolder()
{
	if ( m_Kind != EDebugTopPickKind::Folder ) return;
	if ( m_Directory.IsEmpty() ) return;

	Finish( m_Directory );
}


void CDebugTopPathBrowser::Finish( const FString& Path )
{
	// 先に閉じてから渡す。受け取った側がまた開くこともあるため。
	const FDebugTopPathChosen OnChosen = m_OnChosen;
	Close();
	OnChosen.ExecuteIfBound( Path );
}


bool CDebugTopPathBrowser::Update( f32 DeltaSeconds )
{
	if ( !m_bOpen ) return false;

	if ( CInput::IsKeyPressed( EKey::Escape ) )
	{
		Close();
		return true;
	}

	// 打った文字で名前を絞り込む。専用の欄を出さず、打てばそのまま効くようにする。
	bool bFilterChanged = false;
	const char* const Typed = CInput::TextInput();
	for ( const char* Cursor = Typed; Cursor != nullptr && *Cursor != '\0'; ++Cursor )
	{
		// 制御文字 (Enter や Tab) は絞り込みの語に混ぜない。
		if ( static_cast<unsigned char>( *Cursor ) < 0x20 ) continue;

		m_Filter.Append( *Cursor );
		bFilterChanged = true;
	}

	// 語が空のときの Backspace は「1 つ上のフォルダへ」。打った語を消し切ってから上がれる。
	if ( CInput::IsKeyPressed( EKey::Backspace ) )
	{
		if ( m_Filter.IsEmpty() )
		{
			GoUp();
			return true;
		}

		m_Filter = FString( FStringView( m_Filter.Data(), m_Filter.Size() - 1 ) );
		bFilterChanged = true;
	}

	if ( bFilterChanged ) Rebuild( FString() );

	m_KeyNav.Update( DeltaSeconds );
	m_Cursor += m_KeyNav.GetVertical();

	const i32 Count = static_cast<i32>( m_Visible.Num() );
	if ( Count > 0 )
	{
		// 端で止めず回り込ませる (メニューの一覧と揃える)。
		m_Cursor %= Count;
		if ( m_Cursor < 0 ) m_Cursor += Count;
	}
	else
	{
		m_Cursor = 0;
	}

	// カーソルが出ている範囲から外れたら、範囲の方を寄せる。
	if ( m_Cursor < m_Scroll ) m_Scroll = m_Cursor;
	if ( m_Cursor >= m_Scroll + m_VisibleRowCount ) m_Scroll = m_Cursor - m_VisibleRowCount + 1;
	if ( m_Scroll > Count - m_VisibleRowCount ) m_Scroll = Count - m_VisibleRowCount;
	if ( m_Scroll < 0 ) m_Scroll = 0;

	if ( CInput::IsKeyPressed( EKey::Enter ) ) Decide();

	// フォルダを選ばせているときは、決定が「降りる」なので、選び終える操作を別に置く。
	if ( CInput::IsKeyPressed( EKey::Tab ) ) ChooseCurrentFolder();

	return true;
}


void CDebugTopPathBrowser::Draw( CSpriteBatch& Batch, const CDebugTopText& Text, f32 ScreenWidth, f32 ScreenHeight ) noexcept
{
	if ( !m_bOpen || !Text.IsValid() ) return;

	const f32 LineHeight = Text.LineHeight();
	if ( LineHeight <= 0.0f ) return;

	const f32 Pad = LineHeight * kPadRatio;
	const f32 Width = LineHeight * kWidthRatio;

	// 見出し 1 行 + 一覧 + 案内 1 行。画面に入る範囲で行数を決める。
	i32 Rows = kMaxRows;
	const f32 MaxHeight = ScreenHeight * 0.8f;
	while ( Rows > 1 && Pad * 2.0f + LineHeight * static_cast<f32>( Rows + 2 ) > MaxHeight ) --Rows;
	m_VisibleRowCount = Rows;

	const f32 Height = Pad * 2.0f + LineHeight * static_cast<f32>( Rows + 2 );
	const f32 X = ( ScreenWidth - Width ) * 0.5f;
	const f32 Y = ( ScreenHeight - Height ) * 0.5f;

	Batch.DrawRect( X, Y, Width, Height, kPanelColor );
	Batch.DrawRect( X, Y, Width, kBorderWidth, kBorderColor );
	Batch.DrawRect( X, Y + Height - kBorderWidth, Width, kBorderWidth, kBorderColor );
	Batch.DrawRect( X, Y, kBorderWidth, Height, kBorderColor );
	Batch.DrawRect( X + Width - kBorderWidth, Y, kBorderWidth, Height, kBorderColor );

	// 見出しにはいま開いている場所を出す。絞り込み中はその語も添える。
	FString Header = m_Directory.IsEmpty() ? FString( "PC" ) : m_Directory;
	if ( !m_Filter.IsEmpty() )
	{
		Header.Append( "  [" );
		Header.Append( m_Filter.View() );
		Header.Append( ']' );
	}
	Text.Draw( Batch, Header.Data(), X + Pad, Y + Pad, m_Filter.IsEmpty() ? kHeaderColor : kFilterColor );

	const f32 ListY = Y + Pad + LineHeight;
	for ( i32 Row = 0; Row < Rows; ++Row )
	{
		const i32 Index = m_Scroll + Row;
		if ( Index < 0 || static_cast<usize>( Index ) >= m_Visible.Num() ) break;

		const FDebugTopDirEntry& Entry = m_Visible[static_cast<usize>( Index )];
		const f32 RowY = ListY + LineHeight * static_cast<f32>( Row );

		if ( Index == m_Cursor ) Batch.DrawRect( X + kBorderWidth, RowY, Width - kBorderWidth * 2.0f, LineHeight, kCursorColor );

		// フォルダには右向きの三角を置いて、降りられることを見た目で示す。
		if ( Entry.bDirectory ) DebugTopDrawTriangle( Batch, EDebugTopTriangle::Right, X + Pad, RowY + LineHeight * 0.25f, LineHeight * 0.4f, LineHeight * 0.5f, kFolderColor );

		Text.Draw( Batch, Entry.Name.Data(), X + Pad + LineHeight * kIndentRatio, RowY, Entry.bDirectory ? kFolderColor : kFileColor );

		if ( Entry.bDirectory ) continue;

		// 大きさは右端へ寄せる (名前の長さが揃わないため)。
		const FString SizeText = FormatSize( Entry.Size );
		Text.Draw( Batch, SizeText.Data(), X + Width - Pad - Text.MeasureWidth( SizeText.Data() ), RowY, kHintColor );
	}

	if ( m_Visible.IsEmpty() ) Text.Draw( Batch, "(なし)", X + Pad + LineHeight * kIndentRatio, ListY, kHintColor );

	const char* const Hint = m_Kind == EDebugTopPickKind::Folder
		? "Enter: 開く / Tab: ここに決める / Backspace: 上へ / Esc: やめる"
		: "Enter: 開く・選ぶ / Backspace: 上へ / Esc: やめる";
	Text.Draw( Batch, Hint, X + Pad, Y + Height - Pad - LineHeight, kHintColor );
}
