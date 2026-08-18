// SPDX-License-Identifier: Apache-2.0
#include "ScreenSubsystem.h"

// 窓の見出しは wchar_t で渡す決まり。直す口は 1 か所へ寄せてある。
#include "AcsFramework_Core/Text/StringConvert.h"

// GameInstance スコープへ登録する。シーンを切り替えても同じ窓を指す。
ACS_REGISTER_SUBSYSTEM( CScreenSubsystem, ESubsystemScope::GameInstance )


void CScreenSubsystem::SetFullscreen( bool bFullscreen ) noexcept
{
	if ( m_Application == nullptr ) return;

	m_Application->GetWindow().SetFullscreen( bFullscreen );
}


bool CScreenSubsystem::IsFullscreen() const noexcept
{
	if ( m_Application == nullptr ) return false;

	return m_Application->GetWindow().IsFullscreen();
}


u32 CScreenSubsystem::GetWidth() const noexcept
{
	if ( m_Application == nullptr ) return 0;

	return m_Application->GetWindow().Width();
}


u32 CScreenSubsystem::GetHeight() const noexcept
{
	if ( m_Application == nullptr ) return 0;

	return m_Application->GetWindow().Height();
}


f32 CScreenSubsystem::GetAspect() const noexcept
{
	const u32 Height = GetHeight();
	if ( Height == 0 ) return 1.0f;

	return static_cast<f32>( GetWidth() ) / static_cast<f32>( Height );
}


bool CScreenSubsystem::IsMinimized() const noexcept
{
	if ( m_Application == nullptr ) return false;

	return m_Application->GetWindow().IsMinimized();
}


void CScreenSubsystem::SetTitle( const FString& Title ) noexcept
{
	if ( m_Application == nullptr ) return;

	if ( Title.IsEmpty() )
	{
		m_Application->GetWindow().SetTitle( L"" );
		return;
	}

	TArray<wchar_t> Wide;
	if ( !AcsToWide( Title, Wide ) ) return;

	m_Application->GetWindow().SetTitle( Wide.GetData() );
}
