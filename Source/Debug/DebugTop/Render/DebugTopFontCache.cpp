#include "DebugTopFontCache.h"

namespace
{
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

CDebugTopFontCache::~CDebugTopFontCache() noexcept = default;


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

		// 描画へ渡す読み込み済みの専用フォント。
		const FFont* const Font = m_FontResource.Peek();
		// 専用アトラスは指定サイズそのもので焼いてあるので、拡縮せずそのまま描ける。
		if ( Font != nullptr ) return CDebugTopText( *Font, m_FontSize );
	}

	if ( !RenderContext.HasFont() ) return CDebugTopText();

	// 専用アトラスを焼けなかった場合は共有フォントを拡縮する (にじむが表示は保つ)。
	return CDebugTopText( RenderContext.GetFont(), m_FontSize );
}

bool CDebugTopFontCache::IsRebakePending() const noexcept
{
	return !m_bTried || !m_FontResource.MatchesConfiguration( m_FontSize, m_bIncludeCjk );
}

void CDebugTopFontCache::Ensure( FRenderContext& RenderContext ) noexcept
{
	if ( !IsRebakePending() ) return;

	// 手が止まるまでは焼かない。初回 (まだ 1 度も焼いていない) だけは待たずに焼く。
	if ( m_bTried && m_SettleSeconds < kRebakeDelaySeconds ) return;

	// 専用フォントのatlas生成に使う描画device。
	IRhiDevice* const Device = RenderContext.GetRenderer().Device();
	// デバイス未準備なら次のフレームで焼き直す。
	if ( Device == nullptr ) return;

	m_FontResource.Configure( m_FontSize, m_bIncludeCjk );
	m_bTried = true;
	( void )m_FontResource.Acquire( *Device );
}
