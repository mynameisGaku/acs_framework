#include "DebugTopElementBool.h"


CDebugTopElementBool::CDebugTopElementBool( const FString& Label, bool bValue )
	: CDebugTopElement( Label )
	, m_bValue( bValue )
	, m_bDefaultValue( bValue )
{
}


void CDebugTopElementBool::SetValue( bool bValue )
{
	if ( bValue == m_bValue ) return;

	m_bValue = bValue;
	NotifyChanged();
}


FString CDebugTopElementBool::GetValueText() const
{
	return DebugTopFormatValue( m_bValue );
}


void CDebugTopElementBool::OnLeftRight( i32 Delta )
{
	(void)Delta;
	SetValue( !m_bValue );
}


void CDebugTopElementBool::OnDecide()
{
	// 子行を持つ場合は展開トグルを優先する (基底の既定動作)。
	if ( HasChildren() )
	{
		CDebugTopElement::OnDecide();
		return;
	}
	SetValue( !m_bValue );
}


bool CDebugTopElementBool::TryGetSelection( i32& OutIndex, i32& OutCount ) const noexcept
{
	OutIndex = m_bValue ? 1 : 0;
	OutCount = 2;
	return true;
}


bool CDebugTopElementBool::TryGetBool( bool& bOutValue ) const noexcept
{
	bOutValue = m_bValue;
	return true;
}


bool CDebugTopElementBool::TrySetBool( bool bValue )
{
	SetValue( bValue );
	return true;
}
