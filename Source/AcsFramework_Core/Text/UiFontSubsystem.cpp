#include "UiFontSubsystem.h"

namespace
{
	/** 仮名までを焼くときのアトラスの一辺。 */
	constexpr u32 kAtlasSize = 1024;

	/** 漢字まで焼くときのアトラスの一辺 (収録数が桁違いなので広く取る)。 */
	constexpr u32 kCjkAtlasSize = 4096;
}

// GameInstance スコープへ登録する。シーンを切り替えても焼き直さずに済む。
ACS_REGISTER_SUBSYSTEM( CUiFontSubsystem, ESubsystemScope::GameInstance )


void CUiFontSubsystem::SetSize( f32 SizePixels ) noexcept
{
	if ( SizePixels <= 0.0f ) return;

	m_SizePixels = SizePixels;

	// 一度失敗していても、設定が変われば焼き直しを試す価値がある。
	m_bFailed = false;
}


void CUiFontSubsystem::SetIncludeCjk( bool bIncludeCjk ) noexcept
{
	m_bIncludeCjk = bIncludeCjk;
	m_bFailed = false;
}


FFont* CUiFontSubsystem::Acquire( CRenderer& Renderer ) noexcept
{
	const bool bMatches = m_bReady && m_BakedSize == m_SizePixels && m_bBakedCjk == m_bIncludeCjk;
	if ( bMatches ) return &m_Font;
	if ( m_bFailed ) return m_bReady ? &m_Font : nullptr;

	IRhiDevice* const Device = Renderer.Device();
	if ( Device == nullptr ) return m_bReady ? &m_Font : nullptr;   // まだ焼けない。次のフレームで試す。

	// 焼き直しの前に古いアトラスを解放する (失敗したときに古いものを残さない)。
	if ( m_bReady ) m_Font.Shutdown();
	m_bReady = false;

	m_BakedSize = m_SizePixels;
	m_bBakedCjk = m_bIncludeCjk;

	const u32 AtlasSize = m_bIncludeCjk ? kCjkAtlasSize : kAtlasSize;

	// 漢字まで焼くと数秒かかることがある。どこで時間を使ったのかが後から分かるよう残す。
	const f64 StartSeconds = CClock::SecondsSinceStartup();
	const auto Result = FSample::TryLoadDefaultUIFont( m_Font, *Device, m_SizePixels, AtlasSize, m_bIncludeCjk );
	ACS_LOG_INFO( "CUiFontSubsystem: %.1fpx cjk=%d atlas=%u を焼いた (%.0f ms)", static_cast<double>( m_SizePixels ), m_bIncludeCjk ? 1 : 0, AtlasSize, ( CClock::SecondsSinceStartup() - StartSeconds ) * 1000.0 );

	m_bReady = Result.IsOk();
	if ( !m_bReady )
	{
		m_bFailed = true;
		ACS_LOG_WARN( "CUiFontSubsystem: %.1fpx のフォントを焼けなかった (文字を出せない)", static_cast<double>( m_SizePixels ) );
		return nullptr;
	}

	return &m_Font;
}
