// SPDX-License-Identifier: Apache-2.0
#include "UiFontSubsystem.h"

// GameInstance スコープへ登録する。シーンを切り替えても焼き直さずに済む。
ACS_REGISTER_SUBSYSTEM( CUiFontSubsystem, ESubsystemScope::GameInstance )


void CUiFontSubsystem::SetSize( f32 SizePixels ) noexcept
{
	if ( SizePixels <= 0.0f ) return;

	m_Resource.Configure( SizePixels, m_Resource.IsIncludeCjk() );
}


void CUiFontSubsystem::SetIncludeCjk( bool bIncludeCjk ) noexcept
{
	m_Resource.Configure( m_Resource.GetSize(), bIncludeCjk );
}


FFont* CUiFontSubsystem::Acquire( CRenderer& Renderer ) noexcept
{
	// 共有フォントのatlas生成に使う描画device。
	IRhiDevice* const Device = Renderer.Device();
	if ( Device == nullptr ) return m_Resource.Peek();

	return m_Resource.Acquire( *Device );
}
