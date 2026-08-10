// SPDX-License-Identifier: Apache-2.0

#include "AcsFramework_Core/Loading/LoadingScreenDisplayScope.h"

#include "AcsFramework_Core/Loading/LoadingScreenSubsystem.h"

CLoadingScreenDisplayScope::CLoadingScreenDisplayScope( CLoadingScreenSubsystem& Loading ) noexcept
	: m_Loading( &Loading )
{
}

CLoadingScreenDisplayScope::~CLoadingScreenDisplayScope() noexcept
{
	Reset();
}

bool CLoadingScreenDisplayScope::Show( const FString& Message )
{
	if ( m_Loading == nullptr ) return false;

	/** 表示取得後に保持する世代。 */
	u64 Revision = 0u;
	if ( !m_Loading->AcquireDisplayScope( Message, Revision ) )
	{
		m_Revision = 0u;
		return false;
	}

	m_Revision = Revision;
	return true;
}

bool CLoadingScreenDisplayScope::SetMessage( const FString& Message )
{
	return m_Loading != nullptr && m_Loading->SetDisplayScopeMessage( m_Revision, Message );
}

bool CLoadingScreenDisplayScope::SetProgress( f32 Ratio ) noexcept
{
	return m_Loading != nullptr && m_Loading->SetDisplayScopeProgress( m_Revision, Ratio );
}

bool CLoadingScreenDisplayScope::SetFont( const FFont* Font ) noexcept
{
	return m_Loading != nullptr && m_Loading->SetDisplayScopeFont( m_Revision, Font );
}

bool CLoadingScreenDisplayScope::Reset() noexcept
{
	if ( m_Loading == nullptr || m_Revision == 0u )
	{
		m_Revision = 0u;
		return false;
	}

	/** 解除呼出しの成否。 */
	const bool bReleased = m_Loading->ReleaseDisplayScope( m_Revision );
	m_Revision = 0u;
	return bReleased;
}

bool CLoadingScreenDisplayScope::IsActive() const noexcept
{
	return m_Loading != nullptr && m_Loading->IsDisplayScopeCurrent( m_Revision );
}
