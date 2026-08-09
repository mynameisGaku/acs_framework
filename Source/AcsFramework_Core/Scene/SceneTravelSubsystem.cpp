#include "SceneTravelSubsystem.h"

// 幕 (CFadeTransition / EFadeKind) は acs::game 側にある。名前を持ち込むと他と衝突するので、
// ここでは使う所だけ修飾する。

// GameInstance スコープへ登録する。シーンを跨いでも同じ実体を指すので、遷移の前後で
// 呼び出し側が持ち替えなくてよい。
ACS_REGISTER_SUBSYSTEM( CSceneTravelSubsystem, ESubsystemScope::GameInstance )


void CSceneTravelSubsystem::TravelTo( TUniquePtr<AScene> Next, ESceneTransition Transition, f32 OutSeconds, f32 InSeconds ) noexcept
{
	TravelTo( Move( Next ), TUniquePtr<CSceneTravelContext>(), Transition, OutSeconds, InSeconds );
}


void CSceneTravelSubsystem::TravelTo( TUniquePtr<AScene> Next, TUniquePtr<CSceneTravelContext> Context, ESceneTransition Transition, f32 OutSeconds, f32 InSeconds ) noexcept
{
	if ( m_Game == nullptr || !Next ) return;

	if ( Transition == ESceneTransition::Cut )
	{
		// 幕を使わない切り替え。次フレームの頭で差し替わる。
		m_Game->Scenes().ChangeScene( Move( Next ), Move( Context ) );
		return;
	}

	// 切り替えは暗転しきった時点でエンジンが行い、明転もエンジンが戻す。こちらで幕を張る
	// 必要は無く、遷移先もフェード明けの処理を書かなくてよい。
	m_Game->TransitionTo( Move( Next ), Move( Context ), OutSeconds, InSeconds );
}


void CSceneTravelSubsystem::PushScene( TUniquePtr<AScene> Next, ESceneTransition Transition, f32 OutSeconds, f32 InSeconds ) noexcept
{
	PushScene( Move( Next ), TUniquePtr<CSceneTravelContext>(), Transition, OutSeconds, InSeconds );
}


void CSceneTravelSubsystem::PushScene( TUniquePtr<AScene> Next, TUniquePtr<CSceneTravelContext> Context, ESceneTransition Transition, f32 OutSeconds, f32 InSeconds ) noexcept
{
	if ( m_Game == nullptr || !Next ) return;

	if ( Transition == ESceneTransition::Cut )
	{
		m_Game->Scenes().PushScene( Move( Next ), Move( Context ) );
		return;
	}

	// 積み下ろしには TransitionTo に当たる口が無いので、暗転を張って自分で待つ。暗転しきった
	// ところで積み、そのあと明転させる (Update が続きを行う)。
	m_PendingScene = Move( Next );
	m_PendingContext = Move( Context );
	m_Pending = EPending::Push;
	m_PendingInSeconds = InSeconds;
	m_Game->Fade().StartFade( acs::game::EFadeKind::FadeOut, OutSeconds, 0.0f, 0.0f );
}


void CSceneTravelSubsystem::PopScene( ESceneTransition Transition, f32 OutSeconds, f32 InSeconds ) noexcept
{
	PopScene( TUniquePtr<CSceneTravelContext>(), Transition, OutSeconds, InSeconds );
}


void CSceneTravelSubsystem::PopScene( TUniquePtr<CSceneTravelContext> Context, ESceneTransition Transition, f32 OutSeconds, f32 InSeconds ) noexcept
{
	if ( m_Game == nullptr ) return;

	// 最後の 1 枚を下ろすと画面が無くなる。頼まれても何もしない (持たせたものはここで捨てる)。
	if ( !CanPop() ) return;

	if ( Transition == ESceneTransition::Cut )
	{
		m_Game->Scenes().PopScene( Move( Context ) );
		return;
	}

	m_PendingContext = Move( Context );
	m_Pending = EPending::Pop;
	m_PendingInSeconds = InSeconds;
	m_Game->Fade().StartFade( acs::game::EFadeKind::FadeOut, OutSeconds, 0.0f, 0.0f );
}


u32 CSceneTravelSubsystem::GetDepth() const noexcept
{
	if ( m_Game == nullptr ) return 0;

	return m_Game->Scenes().Depth();
}


void CSceneTravelSubsystem::Update() noexcept
{
	if ( m_Pending == EPending::None || m_Game == nullptr ) return;

	// 暗転しきるまで待つ。FadeOut は下りきった後も「留まっている」状態が続き IsActive() は
	// true のままなので、そちらでは判らない。幕の濃さそのものを見る。
	if ( m_Game->Fade().OverlayAlpha() < 0.999f ) return;

	if ( m_Pending == EPending::Push ) m_Game->Scenes().PushScene( Move( m_PendingScene ), Move( m_PendingContext ) );
	else                               m_Game->Scenes().PopScene( Move( m_PendingContext ) );

	m_Pending = EPending::None;
	m_PendingScene.Reset();
	m_PendingContext.Reset();

	m_Game->Fade().StartFade( acs::game::EFadeKind::FadeIn, 0.0f, m_PendingInSeconds, 0.0f );
}
