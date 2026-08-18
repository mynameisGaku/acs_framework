// SPDX-License-Identifier: Apache-2.0
#include "AppSubsystem.h"

// GameInstance スコープへ登録する。シーンを切り替えても同じアプリを指す。
ACS_REGISTER_SUBSYSTEM( CAppSubsystem, ESubsystemScope::GameInstance )


void CAppSubsystem::Quit() noexcept
{
	if ( m_Application == nullptr ) return;

	m_Application->Quit();
}


f32 CAppSubsystem::GetFps() const noexcept
{
	if ( m_Application == nullptr ) return 0.0f;

	return m_Application->FPS();
}


f32 CAppSubsystem::GetUnscaledDeltaSeconds() const noexcept
{
	if ( m_Application == nullptr ) return 0.0f;

	return m_Application->DeltaTime();
}


u64 CAppSubsystem::GetFrameCount() const noexcept
{
	if ( m_Application == nullptr ) return 0;

	return m_Application->FrameCount();
}
