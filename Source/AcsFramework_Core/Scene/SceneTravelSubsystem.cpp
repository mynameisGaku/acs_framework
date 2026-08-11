// SPDX-License-Identifier: Apache-2.0
#include "SceneTravelSubsystem.h"

// GameInstance の寿命で所有し、シーン遷移の前後も同じ実体を維持する。
ACS_REGISTER_SUBSYSTEM( CSceneTravelSubsystem, ESubsystemScope::GameInstance )


void CSceneTravelSubsystem::TravelTo( TUniquePtr<AScene> Next, ESceneTransition Transition, f32 OutSeconds, f32 InSeconds ) noexcept
{
	TravelTo( Move( Next ), TUniquePtr<CSceneTravelContext>(), Transition, OutSeconds, InSeconds );
}


void CSceneTravelSubsystem::TravelTo( TUniquePtr<AScene> Next, TUniquePtr<CSceneTravelContext> Context, ESceneTransition Transition, f32 OutSeconds, f32 InSeconds ) noexcept
{
	if ( m_Game == nullptr ) return;
	m_Controller.TravelTo( *m_Game, Move( Next ), Move( Context ), Transition, OutSeconds, InSeconds );
}


void CSceneTravelSubsystem::PushScene( TUniquePtr<AScene> Next, ESceneTransition Transition, f32 OutSeconds, f32 InSeconds ) noexcept
{
	PushScene( Move( Next ), TUniquePtr<CSceneTravelContext>(), Transition, OutSeconds, InSeconds );
}


void CSceneTravelSubsystem::PushScene( TUniquePtr<AScene> Next, TUniquePtr<CSceneTravelContext> Context, ESceneTransition Transition, f32 OutSeconds, f32 InSeconds ) noexcept
{
	if ( m_Game == nullptr ) return;
	m_Controller.PushScene( *m_Game, Move( Next ), Move( Context ), Transition, OutSeconds, InSeconds );
}


void CSceneTravelSubsystem::PopScene( ESceneTransition Transition, f32 OutSeconds, f32 InSeconds ) noexcept
{
	PopScene( TUniquePtr<CSceneTravelContext>(), Transition, OutSeconds, InSeconds );
}


void CSceneTravelSubsystem::PopScene( TUniquePtr<CSceneTravelContext> Context, ESceneTransition Transition, f32 OutSeconds, f32 InSeconds ) noexcept
{
	if ( m_Game == nullptr ) return;
	m_Controller.PopScene( *m_Game, Move( Context ), Transition, OutSeconds, InSeconds );
}


u32 CSceneTravelSubsystem::GetDepth() const noexcept
{
	if ( m_Game == nullptr ) return 0;
	return m_Game->Scenes().Depth();
}


void CSceneTravelSubsystem::Update() noexcept
{
	if ( m_Game == nullptr ) return;
	m_Controller.Update( *m_Game );
}
