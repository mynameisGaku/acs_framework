// SPDX-License-Identifier: Apache-2.0
#include "DebugTopElement.h"


u32 CDebugTopElement::s_FavoriteVersion = 0;

u32 CDebugTopElement::s_ValueVersion = 0;

TDelegate<void( CDebugTopElement& )> CDebugTopElement::s_ChangeListener;


void CDebugTopElement::SetFavorite( bool bFavorite ) noexcept
{
	if ( m_bFavorite == bFavorite ) return;

	m_bFavorite = bFavorite;
	++s_FavoriteVersion;
}


FString DebugTopFormatValue( i32 Value )
{
	FString Text;
	Text.AppendFormat( "%d", Value );
	return Text;
}

FString DebugTopFormatValue( f32 Value )
{
	FString Text;
	Text.AppendFormat( "%.3f", static_cast<double>( Value ) );
	return Text;
}

FString DebugTopFormatValue( bool bValue )
{
	return FString( bValue ? "ON" : "OFF" );
}


CDebugTopElement::CDebugTopElement( const FString& Label, const FString& SubTitle )
	: m_Label( Label )
	, m_SubTitle( SubTitle )
{
}


FString CDebugTopElement::GetValueText() const
{
	return m_SubTitle;
}


FString CDebugTopElement::GetDisplayLabel() const
{
	FString Text;
	if ( m_LabelProvider.TryExecute( Text ) ) return Text;

	return m_Label;
}


void CDebugTopElement::OnDecide()
{
	if ( m_bExpandable && HasChildren() )
	{
		m_bExpanded = !m_bExpanded;
	}
}


void CDebugTopElement::OnLeftRight( i32 Delta )
{
	(void)Delta;
}


TSharedPtr<CDebugTopElement> CDebugTopElement::AddChild( TSharedPtr<CDebugTopElement> Child )
{
	if ( Child )
	{
		m_Children.Add( Child );
	}
	return Child;
}


bool CDebugTopElement::RemoveChild( const CDebugTopElement* Child )
{
	if ( Child == nullptr ) return false;

	for ( usize Index = 0; Index < m_Children.Num(); ++Index )
	{
		if ( m_Children[Index].Get() != Child ) continue;

		m_Children.RemoveAt( Index );
		return true;
	}
	return false;
}


bool CDebugTopElement::ShouldShowMarker() const noexcept
{
	switch ( m_MarkerVisibility )
	{
	case EDebugTopMarkerVisibility::Always: return true;
	case EDebugTopMarkerVisibility::Never:  return false;

	// 開閉できる行に印を出す。子行を持つ行だけとは限らない (折れ線のように、
	// 子を持たないが中身を出し入れする行もある)。
	default:                                return CanCollapse();
	}
}


void CDebugTopElement::SetTextColor( const FVec4& Color ) noexcept
{
	m_TextColor.Color = Color;
	m_TextColor.bSet = true;
}


void CDebugTopElement::SetValueColor( const FVec4& Color ) noexcept
{
	m_ValueColor.Color = Color;
	m_ValueColor.bSet = true;
}
