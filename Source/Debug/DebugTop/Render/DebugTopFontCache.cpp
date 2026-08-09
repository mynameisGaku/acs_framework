#include "DebugTopFontCache.h"

namespace
{
	/** 専用フォントのアトラス一辺 (ピクセル)。25px 程度なら ASCII と仮名がこれで収まる。 */
	constexpr u32 kAtlasSize = 1024;

	/** 漢字まで焼くときのアトラス一辺。常用漢字を入れるとこれだけ要る。 */
	constexpr u32 kCjkAtlasSize = 4096;

	/**
	 * 設定が変わってから焼き直すまでの待ち時間 (秒)。
	 *
	 * @details
	 * 焼き直しは数百 ms 掛かるので、変わるたびに焼くと連続で動かしている間ずっと止まる。
	 * 止まっている最中はキーの押下がフレームに畳まれて取りこぼされるため、値を動かしている
	 * つもりが飛び飛びになる。手が止まってから 1 度だけ焼く。
	 */
	constexpr f32 kRebakeDelaySeconds = 0.35f;
}


CDebugTopFontCache::~CDebugTopFontCache() noexcept
{
	if ( m_bReady ) m_Font.Shutdown();
}

void CDebugTopFontCache::SetFontSize( f32 FontSize ) noexcept
{
	if ( FontSize < 0.0f ) FontSize = 0.0f;
	if ( FontSize == m_FontSize ) return;

	m_FontSize = FontSize;

	// すぐには焼かない。手が止まるまで待つ (焼いている間は入力を取りこぼすため)。
	// その間も文字は新しい大きさで出る (古いアトラスを拡縮するので少しにじむだけ)。
	m_SettleSeconds = 0.0f;
}

void CDebugTopFontCache::SetIncludeCjk( bool bIncludeCjk ) noexcept
{
	if ( bIncludeCjk == m_bIncludeCjk ) return;

	m_bIncludeCjk = bIncludeCjk;
	m_SettleSeconds = 0.0f;
}

void CDebugTopFontCache::Update( f32 DeltaSeconds ) noexcept
{
	if ( !IsRebakePending() ) return;

	m_SettleSeconds += DeltaSeconds;
}

CDebugTopText CDebugTopFontCache::Resolve( FRenderContext& RenderContext ) noexcept
{
	if ( m_FontSize > 0.0f )
	{
		Ensure( RenderContext );

		// 専用アトラスは指定サイズそのもので焼いてあるので、拡縮せずそのまま描ける。
		if ( m_bReady ) return CDebugTopText( m_Font, m_FontSize );
	}

	if ( !RenderContext.HasFont() ) return CDebugTopText();

	// 専用アトラスを焼けなかった場合は共有フォントを拡縮する (にじむが表示は保つ)。
	return CDebugTopText( RenderContext.GetFont(), m_FontSize );
}

bool CDebugTopFontCache::IsRebakePending() const noexcept
{
	return !m_bTried || m_LoadedFontSize != m_FontSize || m_bLoadedCjk != m_bIncludeCjk;
}

void CDebugTopFontCache::Ensure( FRenderContext& RenderContext ) noexcept
{
	if ( !IsRebakePending() ) return;

	// 手が止まるまでは焼かない。初回 (まだ 1 度も焼いていない) だけは待たずに焼く。
	if ( m_bTried && m_SettleSeconds < kRebakeDelaySeconds ) return;

	IRhiDevice* const Device = RenderContext.GetRenderer().Device();
	if ( Device == nullptr ) return;   // デバイス未準備。次のフレームで焼き直す。

	m_bTried         = true;
	m_LoadedFontSize = m_FontSize;
	m_bLoadedCjk     = m_bIncludeCjk;

	// 焼き直しの前に古いアトラスを解放する (LoadFromBytes 側でも解放されるが、失敗時に残さない)。
	if ( m_bReady ) m_Font.Shutdown();
	m_bReady = false;

	const u32 AtlasSize = m_bIncludeCjk ? kCjkAtlasSize : kAtlasSize;
	const auto Result = FSample::TryLoadDefaultUIFont( m_Font, *Device, m_FontSize, AtlasSize, m_bIncludeCjk );
	m_bReady = Result.IsOk();
	if ( !m_bReady )
	{
		ACS_LOG_WARN( "CDebugTopFontCache: %.1fpx のフォントを焼けなかった (共有フォントで代用する)", static_cast<double>( m_FontSize ) );
	}
}
