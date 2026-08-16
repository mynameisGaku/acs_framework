// SPDX-License-Identifier: Apache-2.0
#include "UiFontResource.h"
#include "Common/Compat/AcsUiFont.h"

namespace
{
	/** 仮名までを収録する atlas の一辺。 */
	constexpr u32 kAtlasSize = 1024;

	/** CJK 文字まで収録する atlas の一辺。 */
	constexpr u32 kCjkAtlasSize = 4096;
}

FUiFontResource::~FUiFontResource() noexcept
{
	if ( m_bReady ) m_Font.Shutdown();
}

void FUiFontResource::Configure( f32 SizePixels, bool bIncludeCjk ) noexcept
{
	if ( SizePixels <= 0.0f ) return;

	m_SizePixels = SizePixels;
	m_bIncludeCjk = bIncludeCjk;
	m_bFailed = false;
}

bool FUiFontResource::MatchesConfiguration( f32 SizePixels, bool bIncludeCjk ) const noexcept
{
	return m_SizePixels == SizePixels && m_bIncludeCjk == bIncludeCjk;
}

FFont* FUiFontResource::Acquire( IRhiDevice& Device ) noexcept
{
	// 読み込み済みatlasをそのまま使える設定か。
	const bool bMatches = m_bReady && m_BakedSize == m_SizePixels && m_bBakedCjk == m_bIncludeCjk;
	if ( bMatches ) return &m_Font;
	if ( m_bFailed ) return nullptr;

	if ( m_bReady ) m_Font.Shutdown();
	m_bReady = false;
	m_BakedSize = m_SizePixels;
	m_bBakedCjk = m_bIncludeCjk;

	// 収録する文字範囲に必要なatlasの一辺。
	const u32 AtlasSize = m_bIncludeCjk ? kCjkAtlasSize : kAtlasSize;
	// 読み込み時間を診断へ残すための開始時刻。
	const f64 StartSeconds = CClock::SecondsSinceStartup();
	// Engine既定候補からフォントを読み込んだ結果。
	const auto Result = AcsFw::TryLoadDefaultUiFont( m_Font, Device, m_SizePixels, AtlasSize, m_bIncludeCjk );
	// 読み込みに使ったミリ秒。
	const f64 ElapsedMilliseconds = ( CClock::SecondsSinceStartup() - StartSeconds ) * 1000.0;

	m_bReady = Result.IsOk();
	if ( !m_bReady )
	{
		m_bFailed = true;
		ACS_LOG_WARN( "FUiFontResource: %.1fpx cjk=%d atlas=%u のフォントを用意できなかった", static_cast<double>( m_SizePixels ), m_bIncludeCjk ? 1 : 0, AtlasSize );
		return nullptr;
	}

	ACS_LOG_INFO( "FUiFontResource: %.1fpx cjk=%d atlas=%u を用意した (%.0f ms)", static_cast<double>( m_SizePixels ), m_bIncludeCjk ? 1 : 0, AtlasSize, ElapsedMilliseconds );
	return &m_Font;
}
