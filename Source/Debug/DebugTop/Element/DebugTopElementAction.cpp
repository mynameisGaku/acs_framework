#include "DebugTopElementAction.h"


CDebugTopElementAction::CDebugTopElementAction( const FString& Label, const FString& SubTitle, FSimpleDelegate Action )
	: CDebugTopElement( Label, SubTitle )
	, m_Action( Action )
{
}


void CDebugTopElementAction::OnDecide()
{
	m_Action.ExecuteIfBound();
}
