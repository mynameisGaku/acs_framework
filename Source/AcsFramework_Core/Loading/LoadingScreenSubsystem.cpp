#include "LoadingScreenSubsystem.h"

namespace
{
	/** 出し入れにかける秒数。一瞬で終わる処理に被せてもちらつかない程度に短く取る。 */
	constexpr f32 kFadeSeconds = 0.18f;
}


// GameInstance スコープへ登録する。シーンを切り替えても出したままにできる。
ACS_REGISTER_SUBSYSTEM( CLoadingScreenSubsystem, ESubsystemScope::GameInstance )


void CLoadingScreenSubsystem::Show( const FString& Message )
{
	AdvanceDisplayRevision();
	m_Message = Message;
	m_bVisible = true;
}

void CLoadingScreenSubsystem::SetEnabled( bool bEnabled ) noexcept
{
	AdvanceDisplayRevision();
	m_bVisible = bEnabled;
}

void CLoadingScreenSubsystem::Follow( const CAssetLoaderSubsystem& Loader, const FString& Message )
{
	m_Followed = &Loader;
	m_FollowedRequest = FAssetLoadRequest();
	AdvanceFollowRevision();
	AdvanceDisplayRevision();
	m_Message = Message;

	// ここでは出さない。実際に読み込んでいることを Update で見てから出す。こうしておくと、
	// キャッシュ済みで一瞬で終わる読み込みのときに 1 フレームだけ点滅しない。
	SetProgressValue( Loader.GetProgress() );
}

void CLoadingScreenSubsystem::Unfollow() noexcept
{
	ClearFollow();
}

bool CLoadingScreenSubsystem::FollowRequest( const CAssetLoaderSubsystem& Loader, FAssetLoadRequest Request, const FString& Message )
{
	if ( !Request.IsValid() || !Loader.IsCurrent( Request ) ) return false;

	m_Followed = &Loader;
	m_FollowedRequest = Request;
	AdvanceFollowRevision();
	AdvanceDisplayRevision();
	m_Message = Message;
	SetProgressValue( Loader.GetProgress() );
	return true;
}

bool CLoadingScreenSubsystem::FollowScopedRequest( const CAssetLoaderSubsystem& Loader, FAssetLoadRequest Request, const FString& Message, u64& Revision )
{
	if ( !FollowRequest( Loader, Request, Message ) )
	{
		Revision = 0u;
		return false;
	}

	Revision = m_FollowRevision;
	return true;
}

bool CLoadingScreenSubsystem::UnfollowRequest( FAssetLoadRequest Request ) noexcept
{
	if ( !Request.IsValid() || m_FollowedRequest != Request ) return false;

	ClearFollow();
	return true;
}

bool CLoadingScreenSubsystem::IsScopedFollowCurrent( FAssetLoadRequest Request, u64 Revision ) const noexcept
{
	return Request.IsValid() && m_Followed != nullptr && m_FollowedRequest == Request && m_FollowRevision == Revision;
}

bool CLoadingScreenSubsystem::UnfollowRequest( FAssetLoadRequest Request, u64 Revision ) noexcept
{
	if ( !IsScopedFollowCurrent( Request, Revision ) ) return false;

	ClearFollow();
	return true;
}

void CLoadingScreenSubsystem::AdvanceFollowRevision() noexcept
{
	++m_FollowRevision;
	if ( m_FollowRevision == 0u ) ++m_FollowRevision;
}

void CLoadingScreenSubsystem::ClearFollow() noexcept
{
	m_Followed = nullptr;
	m_FollowedRequest = FAssetLoadRequest();
	AdvanceFollowRevision();
	AdvanceDisplayRevision();
	m_bVisible = false;
}

void CLoadingScreenSubsystem::UpdateFollow() noexcept
{
	if ( m_Followed == nullptr ) return;
	if ( m_FollowedRequest.IsValid() && !m_Followed->IsCurrent( m_FollowedRequest ) )
	{
		ClearFollow();
		return;
	}

	if ( m_Followed->IsLoading() )
	{
		m_bVisible = true;
		SetProgressValue( m_Followed->GetProgress() );
		return;
	}

	// 読み終わった。幕を下ろして、見に行くのもここで終わる。
	ClearFollow();
}

void CLoadingScreenSubsystem::SetProgress( f32 Ratio ) noexcept
{
	AdvanceDisplayRevision();
	SetProgressValue( Ratio );
}

void CLoadingScreenSubsystem::SetProgressValue( f32 Ratio ) noexcept
{
	if ( Ratio < 0.0f )
	{
		m_Progress = -1.0f;
		return;
	}
	if ( Ratio > 1.0f ) Ratio = 1.0f;

	m_Progress = Ratio;
}

void CLoadingScreenSubsystem::SetMessage( const FString& Message )
{
	AdvanceDisplayRevision();
	m_Message = Message;
}

void CLoadingScreenSubsystem::SetFont( const FFont* Font ) noexcept
{
	AdvanceDisplayRevision();
	m_Font = Font;
}

bool CLoadingScreenSubsystem::AcquireDisplayScope( const FString& Message, u64& Revision )
{
	if ( m_Followed != nullptr )
	{
		Revision = 0u;
		return false;
	}

	AdvanceDisplayRevision();
	m_Message = Message;
	SetProgressValue( -1.0f );
	m_bVisible = true;
	Revision = m_DisplayRevision;
	return true;
}

bool CLoadingScreenSubsystem::IsDisplayScopeCurrent( u64 Revision ) const noexcept
{
	return Revision != 0u && m_Followed == nullptr && m_DisplayRevision == Revision;
}

bool CLoadingScreenSubsystem::SetDisplayScopeMessage( u64 Revision, const FString& Message )
{
	if ( !IsDisplayScopeCurrent( Revision ) ) return false;

	m_Message = Message;
	return true;
}

bool CLoadingScreenSubsystem::SetDisplayScopeProgress( u64 Revision, f32 Ratio ) noexcept
{
	if ( !IsDisplayScopeCurrent( Revision ) ) return false;

	SetProgressValue( Ratio );
	return true;
}

bool CLoadingScreenSubsystem::SetDisplayScopeFont( u64 Revision, const FFont* Font ) noexcept
{
	if ( !IsDisplayScopeCurrent( Revision ) ) return false;

	m_DisplayScopeFont = Font;
	return true;
}

bool CLoadingScreenSubsystem::ReleaseDisplayScope( u64 Revision ) noexcept
{
	if ( !IsDisplayScopeCurrent( Revision ) ) return false;

	m_bVisible = false;
	AdvanceDisplayRevision();
	return true;
}

void CLoadingScreenSubsystem::AdvanceDisplayRevision() noexcept
{
	ClearDisplayScopeFont();
	++m_DisplayRevision;
	if ( m_DisplayRevision == 0u ) ++m_DisplayRevision;
}

void CLoadingScreenSubsystem::ClearDisplayScopeFont() noexcept
{
	m_DisplayScopeFont = nullptr;
}

void CLoadingScreenSubsystem::Update( f32 DeltaSeconds ) noexcept
{
	// 見に行っている相手の進み具合を先に取り込む。ここで出し入れと値が決まる。
	UpdateFollow();

	// 出し入れは滑らかに繋ぐ。出しっぱなしでも回り続けるようスピナーは常に進める。
	const f32 Step = kFadeSeconds > 0.0f ? DeltaSeconds / kFadeSeconds : 1.0f;
	m_Alpha += m_bVisible ? Step : -Step;
	if ( m_Alpha < 0.0f ) m_Alpha = 0.0f;
	if ( m_Alpha > 1.0f ) m_Alpha = 1.0f;

	if ( !IsOnScreen() ) return;

	m_Renderer.Update( DeltaSeconds );
}

void CLoadingScreenSubsystem::Draw( CRenderer& Renderer, const FFont* SharedFont ) noexcept
{
	if ( !IsOnScreen() ) return;

	/** 表示世代、公開設定、共有設定の順で使うフォント。 */
	const FFont* const ActiveFont = m_DisplayScopeFont != nullptr ? m_DisplayScopeFont : ( m_Font != nullptr ? m_Font : SharedFont );
	m_Renderer.Draw( Renderer, ActiveFont, m_Message, m_Alpha, m_Progress );
}
