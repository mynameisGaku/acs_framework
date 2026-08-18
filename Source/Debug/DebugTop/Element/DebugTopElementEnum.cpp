// SPDX-License-Identifier: Apache-2.0
#include "DebugTopElementEnum.h"


CDebugTopElementEnum::CDebugTopElementEnum( const FString& Label, TArray<FString> Options, i32 SelectedIndex )
	: CDebugTopElement( Label )
	, m_Options( Move( Options ) )
	, m_SelectedIndex( SelectedIndex )
{
	if ( m_Options.IsEmpty() )
	{
		m_SelectedIndex = 0;
		return;
	}
	if ( m_SelectedIndex < 0 ) m_SelectedIndex = 0;
	if ( m_SelectedIndex >= static_cast<i32>( m_Options.Num() ) )
	{
		m_SelectedIndex = static_cast<i32>( m_Options.Num() ) - 1;
	}

	m_DefaultIndex = m_SelectedIndex;

	// 選択肢を子行として持たせると、既存の展開機構がそのままコンボボックスになる。
	// 決定 (ダブルクリック) で一覧が開き、選ぶと閉じる。
	for ( usize Index = 0; Index < m_Options.Num(); ++Index )
	{
		AddChild( MakeShared<CDebugTopElementEnumOption>( m_Options[Index], *this, static_cast<i32>( Index ) ) );
	}
}


const FString& CDebugTopElementEnum::GetOptionText( usize Index ) const noexcept
{
	static const FString kEmpty;
	if ( Index >= m_Options.Num() ) return kEmpty;

	return m_Options[Index];
}


void CDebugTopElementEnumOption::OnDecide()
{
	m_Owner->SetSelectedIndex( m_Index );

	// 選んだら一覧を畳む (開きっぱなしだと下の行が押し出されたままになる)。
	m_Owner->SetExpanded( false );
}


bool CDebugTopElementEnumOption::TryGetBool( bool& bOutValue ) const noexcept
{
	bOutValue = m_Owner->GetSelectedIndex() == m_Index;
	return true;
}


bool CDebugTopElementEnumOption::TrySetBool( bool bValue )
{
	// チェックを入れる操作だけを選択として扱う (外しても選択は消せないため)。
	if ( !bValue ) return true;

	m_Owner->SetSelectedIndex( m_Index );
	return true;
}


void CDebugTopElementEnum::SetSelectedIndex( i32 SelectedIndex )
{
	if ( m_Options.IsEmpty() ) return;
	if ( SelectedIndex < 0 || SelectedIndex >= static_cast<i32>( m_Options.Num() ) ) return;
	if ( SelectedIndex == m_SelectedIndex ) return;

	m_SelectedIndex = SelectedIndex;
	NotifyChanged();
}


FString CDebugTopElementEnum::GetValueText() const
{
	if ( m_Options.IsEmpty() ) return FString();
	return m_Options[static_cast<usize>( m_SelectedIndex )];
}


void CDebugTopElementEnum::OnLeftRight( i32 Delta )
{
	SetSelectedIndex( m_SelectedIndex + Delta );
}


bool CDebugTopElementEnum::TryGetSelection( i32& OutIndex, i32& OutCount ) const noexcept
{
	if ( m_Options.IsEmpty() ) return false;

	OutIndex = m_SelectedIndex;
	OutCount = static_cast<i32>( m_Options.Num() );
	return true;
}


bool CDebugTopElementEnum::TryGetInt( i32& OutValue ) const noexcept
{
	OutValue = m_SelectedIndex;
	return true;
}


bool CDebugTopElementEnum::TrySetInt( i32 Value )
{
	SetSelectedIndex( Value );
	return true;
}


CDebugTopElementEnumOption::CDebugTopElementEnumOption( const FString& Label, CDebugTopElementEnum& Owner, i32 Index )
	: CDebugTopElement( Label )
	, m_Owner( &Owner )
	, m_Index( Index )
{
}
