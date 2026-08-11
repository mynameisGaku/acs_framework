// SPDX-License-Identifier: MIT
#include "PauseScreenRenderer.h"

namespace
{
	/** 背後の画面へ重ねる色。 */
	constexpr FVec4 kBackdropColor{ 0.02f, 0.03f, 0.05f, 0.62f };

	/** ポーズ記号と文言の色。 */
	constexpr FVec4 kAccentColor{ 0.95f, 0.96f, 0.98f, 1.0f };

	/** ポーズ記号の高さが画面の短辺に占める割合。 */
	constexpr f32 kMarkHeightRatio = 0.085f;

	/** ポーズ記号を構成する棒の幅が記号の高さに占める割合。 */
	constexpr f32 kMarkBarRatio = 0.30f;

	/** ポーズ記号を構成する棒の間隔が記号の高さに占める割合。 */
	constexpr f32 kMarkGapRatio = 0.24f;

	/** ポーズ記号と文言の間隔が画面の短辺に占める割合。 */
	constexpr f32 kTextMarginRatio = 0.045f;

	/** 背景、記号、文言を一度に描くための最大要素数。 */
	constexpr u32 kOverlayCapacity = 128;

	/** 指定した濃さを色の不透明度へ掛けて返す。 */
	FVec4 Fade( const FVec4& Color, f32 Alpha ) noexcept
	{
		return FVec4{ Color.x, Color.y, Color.z, Color.w * Alpha };
	}
}

void FPauseScreenRenderer::Draw( CRenderer& Renderer, const FString& Message, const FFont* PreferredFont, const FFont* SharedFont, f32 Alpha ) noexcept
{
	IRhiCommandList* const CommandList = Renderer.CommandList();
	IRhiSwapchain* const Swapchain = Renderer.Swapchain();
	if ( CommandList == nullptr || Swapchain == nullptr ) return;

	if ( !m_bOverlayTried )
	{
		IRhiDevice* const Device = Renderer.Device();
		if ( Device == nullptr ) return;

		m_bOverlayTried = true;
		const auto Result = m_Overlay.Init( *Device, Renderer.ColorFormat(), kOverlayCapacity );
		m_bOverlayReady = Result.IsOk();
		if ( !m_bOverlayReady )
		{
			ACS_LOG_WARN( "CPauseScreenSubsystem: 幕の SpriteBatch を用意できなかった (無描画で続ける)" );
		}
	}
	if ( !m_bOverlayReady ) return;

	const f32 Width = static_cast<f32>( Swapchain->Width() );
	const f32 Height = static_cast<f32>( Swapchain->Height() );
	const f32 Short = Width < Height ? Width : Height;
	const f32 CenterX = Width * 0.5f;
	const f32 CenterY = Height * 0.5f;

	m_Overlay.Begin( *CommandList, Swapchain->Width(), Swapchain->Height() );
	m_Overlay.DrawRect( 0.0f, 0.0f, Width, Height, Fade( kBackdropColor, Alpha ) );

	const f32 MarkHeight = Short * kMarkHeightRatio;
	const f32 BarWidth = MarkHeight * kMarkBarRatio;
	const f32 Gap = MarkHeight * kMarkGapRatio;
	const f32 MarkY = CenterY - MarkHeight * 0.5f;
	m_Overlay.DrawRect( CenterX - Gap * 0.5f - BarWidth, MarkY, BarWidth, MarkHeight, Fade( kAccentColor, Alpha ) );
	m_Overlay.DrawRect( CenterX + Gap * 0.5f, MarkY, BarWidth, MarkHeight, Fade( kAccentColor, Alpha ) );

	if ( Message.IsEmpty() ) { m_Overlay.End(); return; }
	if ( SharedFont == nullptr && PreferredFont == nullptr )
	{
		if ( !m_bFontWarned )
		{
			m_bFontWarned = true;
			ACS_LOG_WARN( "CPauseScreenSubsystem: フォントが無いので文言を出せない (SetFont で渡すこと)" );
		}
		m_Overlay.End();
		return;
	}

	const FFont& Font = PreferredFont != nullptr ? *PreferredFont : *SharedFont;
	const f32 TextWidth = Font.MeasureWidth( Message.Data() );
	m_Overlay.DrawString( Font, Message.Data(), CenterX - TextWidth * 0.5f, MarkY + MarkHeight + Short * kTextMarginRatio, Fade( kAccentColor, Alpha ) );

	m_Overlay.End();
}
