// SPDX-License-Identifier: Apache-2.0
#include "DebugTopElementText.h"

#include "Debug/DebugTop/Widget/DebugTopPathBrowser.h"


CDebugTopElementString::CDebugTopElementString( const FString& Label, const FString& Value )
	: CDebugTopElement( Label )
	, m_Value( Value )
	, m_DefaultValue( Value )
{
}


void CDebugTopElementString::SetValue( const FString& Value )
{
	if ( m_Value == Value ) return;

	m_Value = Value;

	// 1 行に収める行なので、改行はそのまま持たせない。
	for ( usize Index = 0; Index < m_Value.Size(); ++Index )
	{
		char& Character = m_Value[Index];
		if ( Character == '\n' || Character == '\r' ) Character = ' ';
	}

	NotifyChanged();
}


bool CDebugTopElementString::CommitEditText( const FString& Text )
{
	SetValue( Text );
	return true;
}


void CDebugTopElementString::ResetToDefault()
{
	SetValue( m_DefaultValue );
}


bool CDebugTopElementString::IsModified() const noexcept
{
	return !( m_Value == m_DefaultValue );
}


CDebugTopElementPath::CDebugTopElementPath( const FString& Label, const FString& Value, EDebugTopPickKind Kind )
	: CDebugTopElementString( Label, Value )
	, m_Kind( Kind )
{
}


void CDebugTopElementPath::OnDecide()
{
	// まずメニューの中の一覧で選ばせる。ゲームが止まらず、全画面でも裏へ回らない。
	if ( DebugTopBrowsePath( m_Kind, GetValue(), FDebugTopPathChosen::CreateRaw<&CDebugTopElementPath::SetValue>( this ) ) ) return;

	// メニューを出していない場面 (一覧の受け皿が無い) では OS のダイアログに頼る。
	FString Title = m_Kind == EDebugTopPickKind::Folder
		? FString( "フォルダを選ぶ: " )
		: FString( "ファイルを選ぶ: " );
	Title.Append( GetLabel().View() );

	// 取り消されたら今の値をそのまま残す。
	FString Picked;
	if ( !DebugTopPickPath( m_Kind, Title, GetValue(), Picked ) ) return;

	SetValue( Picked );
}
