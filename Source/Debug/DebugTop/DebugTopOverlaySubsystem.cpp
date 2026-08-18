// SPDX-License-Identifier: Apache-2.0
#include "DebugTopOverlaySubsystem.h"

// GameInstance スコープへ登録する。シーンを切り替えても同じメニューを持ち続ける。
ACS_REGISTER_SUBSYSTEM( CDebugTopOverlaySubsystem, ESubsystemScope::GameInstance )


ADebugTopHUD& CDebugTopOverlaySubsystem::GetHUD()
{
	if ( !m_HUD )
	{
		m_HUD = NewObject<ADebugTopHUD>();
		m_HUD->Build();
	}
	return *m_HUD;
}

bool CDebugTopOverlaySubsystem::Update( f32 DeltaSeconds )
{
	// メニューが組み立てられるまでは、切替入力とゲーム時間を掴まない。
	if ( !m_HUD ) return false;

	if ( m_ToggleKey != EKey::Unknown && CInput::IsKeyPressed( m_ToggleKey ) )
	{
		Toggle();

		// 出した / 消したフレームの押下をメニューやゲームへ流さない (F1 で行が実行されない
		// ようにするため)。
		return m_bVisible && m_bPauseWhileVisible;
	}

	if ( !m_bVisible ) return false;

	m_HUD->Update( DeltaSeconds );

	return m_bPauseWhileVisible;
}

void CDebugTopOverlaySubsystem::Draw( CRenderer& Renderer, FFont* SharedFont )
{
	if ( !m_bVisible || !m_HUD ) return;
	m_Renderer.Draw( Renderer, *m_HUD, SharedFont, m_BackdropOpacity );
}
