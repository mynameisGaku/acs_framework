// SPDX-License-Identifier: Apache-2.0

#include "LoadingScreenFollowScope.h"

#include "AcsFramework_Core/Assets/AssetLoaderSubsystem.h"
#include "AcsFramework_Core/Loading/LoadingScreenSubsystem.h"

CLoadingScreenFollowScope::CLoadingScreenFollowScope( CLoadingScreenSubsystem& Loading ) noexcept
	: m_Loading( &Loading )
{
}

CLoadingScreenFollowScope::~CLoadingScreenFollowScope() noexcept
{
	Reset();
}

bool CLoadingScreenFollowScope::Follow( const CAssetLoaderSubsystem& Loader, FAssetLoadRequest Request, const FString& Message )
{
	if ( m_Loading == nullptr || !Request.IsValid() ) return false;

	// subsystemから受け取る追従世代を保持する。
	u64 Revision = 0u;
	if ( !m_Loading->FollowScopedRequest( Loader, Request, Message, Revision ) ) return false;

	m_Request = Request;
	m_Revision = Revision;
	return true;
}

bool CLoadingScreenFollowScope::Reset() noexcept
{
	if ( m_Loading == nullptr || !m_Request.IsValid() ) return false;

	// 解除前に所有していた要求を退避する。
	const FAssetLoadRequest Request = m_Request;
	// 退避した要求に対応する追従世代を退避する。
	const u64 Revision = m_Revision;
	m_Request = FAssetLoadRequest();
	m_Revision = 0u;
	return m_Loading->UnfollowRequest( Request, Revision );
}

bool CLoadingScreenFollowScope::Owns( FAssetLoadRequest Request ) const noexcept
{
	return m_Loading != nullptr && Request.IsValid() && m_Request == Request && m_Loading->IsScopedFollowCurrent( Request, m_Revision );
}

FAssetLoadRequest CLoadingScreenFollowScope::GetRequest() const noexcept
{
	return Owns( m_Request ) ? m_Request : FAssetLoadRequest();
}
