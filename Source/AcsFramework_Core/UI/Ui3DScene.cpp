// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/UI/Ui3DScene.h"

void AUi3DScene::OnEnter() noexcept
{
	ALegacyScene3DAdapter::OnEnter();
	m_WorldLabels.Bind( Graph() );
	if ( !m_InteractionFocus.Bind( Graph(), m_WorldLabels ) ) ACS_LOG_WARN( "AUi3DScene: 3D視線フォーカスを場面へ接続できなかった" );
	m_Ui.Init();
}


void AUi3DScene::OnExit() noexcept
{
	m_Ui.Shutdown();
	m_InteractionFocus.Unbind();
	m_WorldLabels.Unbind();
	ALegacyScene3DAdapter::OnExit();
}


void AUi3DScene::OnUpdate( f32 DeltaSeconds ) noexcept
{
	ALegacyScene3DAdapter::OnUpdate( DeltaSeconds );
	m_Ui.Tick( DeltaSeconds );
}


void AUi3DScene::OnEvent( const FEvent& Event ) noexcept
{
	m_Ui.HandleInput( Event );
	ALegacyScene3DAdapter::OnEvent( Event );
}


void AUi3DScene::OnDrawHud( FRenderContext& Context, CSpriteBatch& Sprites ) noexcept
{
	ALegacyScene3DAdapter::OnDrawHud( Context, Sprites );
	m_WorldLabels.Draw( Camera(), Context, Sprites );
	m_Ui.Draw( Context );
}


FInteractionFocus3DUpdateResult AUi3DScene::UpdateInteractionFocus( bool bActivateRequested ) noexcept
{
	return m_InteractionFocus.Update( Camera(), bActivateRequested );
}
