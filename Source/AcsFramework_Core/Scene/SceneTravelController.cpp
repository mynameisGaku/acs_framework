// SPDX-License-Identifier: Apache-2.0
#include "SceneTravelController.h"


void FSceneTravelController::TravelTo( CGame& Game, TUniquePtr<AScene> Next, TUniquePtr<CSceneTravelContext> Context, ESceneTransition Transition, f32 OutSeconds, f32 InSeconds ) noexcept
{
	if ( !Next ) return;

	if ( Transition == ESceneTransition::Cut )
	{
		Game.Scenes().ChangeScene( Move( Next ), Move( Context ) );
		return;
	}

	Game.TransitionTo( Move( Next ), Move( Context ), OutSeconds, InSeconds );
}


void FSceneTravelController::PushScene( CGame& Game, TUniquePtr<AScene> Next, TUniquePtr<CSceneTravelContext> Context, ESceneTransition Transition, f32 OutSeconds, f32 InSeconds ) noexcept
{
	if ( !Next ) return;

	if ( Transition == ESceneTransition::Cut )
	{
		Game.Scenes().PushScene( Move( Next ), Move( Context ) );
		return;
	}

	m_PendingScene = Move( Next );
	m_PendingContext = Move( Context );
	m_Pending = EPending::Push;
	m_PendingInSeconds = InSeconds;
	Game.Fade().StartFade( acs::game::EFadeKind::FadeOut, OutSeconds, 0.0f, 0.0f );
}


void FSceneTravelController::PopScene( CGame& Game, TUniquePtr<CSceneTravelContext> Context, ESceneTransition Transition, f32 OutSeconds, f32 InSeconds ) noexcept
{
	if ( Game.Scenes().Depth() <= 1 ) return;

	if ( Transition == ESceneTransition::Cut )
	{
		Game.Scenes().PopScene( Move( Context ) );
		return;
	}

	m_PendingContext = Move( Context );
	m_Pending = EPending::Pop;
	m_PendingInSeconds = InSeconds;
	Game.Fade().StartFade( acs::game::EFadeKind::FadeOut, OutSeconds, 0.0f, 0.0f );
}


void FSceneTravelController::Update( CGame& Game ) noexcept
{
	if ( m_Pending == EPending::None ) return;
	if ( Game.Fade().OverlayAlpha() < 0.999f ) return;

	if ( m_Pending == EPending::Push ) Game.Scenes().PushScene( Move( m_PendingScene ), Move( m_PendingContext ) );
	else                                Game.Scenes().PopScene( Move( m_PendingContext ) );

	m_Pending = EPending::None;
	m_PendingScene.Reset();
	m_PendingContext.Reset();
	Game.Fade().StartFade( acs::game::EFadeKind::FadeIn, 0.0f, m_PendingInSeconds, 0.0f );
}
