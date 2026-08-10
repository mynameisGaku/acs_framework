#include "LoadingScreenSubsystem.h"

#include <cmath>

namespace
{
	/** 出し入れにかける秒数。一瞬で終わる処理に被せてもちらつかない程度に短く取る。 */
	constexpr f32 kFadeSeconds = 0.18f;

	/** 背景の色 (下の画面をうっすら残す)。 */
	constexpr FVec4 kBackdropColor{ 0.03f, 0.04f, 0.06f, 0.86f };

	/** スピナーと進捗バーの色。 */
	constexpr FVec4 kAccentColor{ 0.45f, 0.70f, 0.98f, 1.0f };

	/** 文言の色。 */
	constexpr FVec4 kTextColor{ 0.92f, 0.94f, 0.97f, 1.0f };

	/** 進捗バーの溝の色。 */
	constexpr FVec4 kTrackColor{ 0.16f, 0.18f, 0.22f, 1.0f };

	/** スピナーが 1 周するのにかける秒数。 */
	constexpr f32 kSpinSeconds = 1.1f;

	/** スピナーを構成する点の数。 */
	constexpr i32 kSpinnerDots = 8;

	/** スピナーの半径 (画面の短辺に対する割合)。 */
	constexpr f32 kSpinnerRadiusRatio = 0.045f;

	/** 点 1 つの一辺 (スピナーの半径に対する割合)。 */
	constexpr f32 kSpinnerDotRatio = 0.26f;

	/** 進捗バーの幅 (画面幅に対する割合)。 */
	constexpr f32 kBarWidthRatio = 0.32f;

	/** 進捗バーの高さ (画面の短辺に対する割合)。 */
	constexpr f32 kBarHeightRatio = 0.012f;

	/** 一度に積める描画の数 (背景 + スピナー + バー + 文字で足りる量)。 */
	constexpr u32 kOverlayCapacity = 256;

	/**
	 * 濃さを掛けた色を返す。
	 *
	 * @param Color 元の色。
	 * @param Alpha 掛ける濃さ。
	 * @return 掛けた色。
	 */
	FVec4 Fade( const FVec4& Color, f32 Alpha ) noexcept
	{
		return FVec4{ Color.x, Color.y, Color.z, Color.w * Alpha };
	}
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

	m_Elapsed += DeltaSeconds;
	if ( m_Elapsed >= kSpinSeconds ) m_Elapsed -= kSpinSeconds;
}

void CLoadingScreenSubsystem::Draw( CRenderer& Renderer, const FFont* SharedFont ) noexcept
{
	if ( !IsOnScreen() ) return;

	IRhiCommandList* const CommandList = Renderer.CommandList();
	IRhiSwapchain* const Swapchain = Renderer.Swapchain();
	if ( CommandList == nullptr || Swapchain == nullptr ) return;

	// 一度も出さなければ GPU 資源を作らない。出す段になってから 1 度だけ用意する。
	if ( !m_bOverlayTried )
	{
		IRhiDevice* const Device = Renderer.Device();
		if ( Device == nullptr ) return;   // まだ用意できない。次のフレームで試す。

		m_bOverlayTried = true;
		const auto Result = m_Overlay.Init( *Device, Renderer.ColorFormat(), kOverlayCapacity );
		m_bOverlayReady = Result.IsOk();
		if ( !m_bOverlayReady )
		{
			ACS_LOG_WARN( "CLoadingScreenSubsystem: 幕の SpriteBatch を用意できなかった (無描画で続ける)" );
		}
	}
	if ( !m_bOverlayReady ) return;

	const f32 Width = static_cast<f32>( Swapchain->Width() );
	const f32 Height = static_cast<f32>( Swapchain->Height() );
	const f32 Short = Width < Height ? Width : Height;
	const f32 CenterX = Width * 0.5f;
	const f32 CenterY = Height * 0.5f;
	/** 表示世代、公開設定、共有設定の順で使うフォント。 */
	const FFont* const ActiveFont = m_DisplayScopeFont != nullptr ? m_DisplayScopeFont : ( m_Font != nullptr ? m_Font : SharedFont );

	m_Overlay.Begin( *CommandList, Swapchain->Width(), Swapchain->Height() );

	// 下の画面を覆う。完全に塗り潰さないのは、何の上で待たされているかを残すため。
	m_Overlay.DrawRect( 0.0f, 0.0f, Width, Height, Fade( kBackdropColor, m_Alpha ) );

	// スピナー。点を円周に並べ、進んだ位置ほど濃くして回っているように見せる。
	const f32 Radius = Short * kSpinnerRadiusRatio;
	const f32 DotSize = Radius * kSpinnerDotRatio;
	const f32 Turn = kSpinSeconds > 0.0f ? m_Elapsed / kSpinSeconds : 0.0f;
	for ( i32 Index = 0; Index < kSpinnerDots; ++Index )
	{
		const f32 Ratio = static_cast<f32>( Index ) / static_cast<f32>( kSpinnerDots );
		const f32 Angle = ( Ratio + Turn ) * kPi * 2.0f;

		// 先頭を濃く、後ろへ行くほど薄くする。
		const f32 Trail = 0.15f + 0.85f * Ratio;
		const f32 X = CenterX + std::cos( Angle ) * Radius - DotSize * 0.5f;
		const f32 Y = CenterY + std::sin( Angle ) * Radius - DotSize * 0.5f;
		m_Overlay.DrawRect( X, Y, DotSize, DotSize, Fade( kAccentColor, m_Alpha * Trail ) );
	}

	// 文言はスピナーの下へ。中央へ寄せるので幅を測ってから置く。
	f32 TextBottom = CenterY + Radius;
	if ( ActiveFont != nullptr && !m_Message.IsEmpty() )
	{
		const f32 TextWidth = ActiveFont->MeasureWidth( m_Message.Data() );
		const f32 TextY = CenterY + Radius + Short * 0.04f;
		m_Overlay.DrawString( *ActiveFont, m_Message.Data(), CenterX - TextWidth * 0.5f, TextY, Fade( kTextColor, m_Alpha ) );
		TextBottom = TextY + ActiveFont->LineHeight();
	}

	// 割合が分かっているときだけバーを出す。分からないのに空のバーを出すと、
	// 進んでいないように見えて不安になる。
	if ( m_Progress >= 0.0f )
	{
		const f32 BarW = Width * kBarWidthRatio;
		const f32 BarH = Short * kBarHeightRatio;
		const f32 BarX = CenterX - BarW * 0.5f;
		const f32 BarY = TextBottom + Short * 0.03f;

		m_Overlay.DrawRect( BarX, BarY, BarW, BarH, Fade( kTrackColor, m_Alpha ) );
		m_Overlay.DrawRect( BarX, BarY, BarW * m_Progress, BarH, Fade( kAccentColor, m_Alpha ) );
	}

	m_Overlay.End();
}
