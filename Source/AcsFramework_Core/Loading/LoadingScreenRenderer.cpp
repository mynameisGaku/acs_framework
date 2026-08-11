#include "LoadingScreenRenderer.h"

#include <cmath>

namespace
{
	/** 背景の色 (下の画面をうっすら残す)。 */
	constexpr FVec4 kBackdropColor{ 0.03f, 0.04f, 0.06f, 0.86f };

	/** スピナーと進捗バーの色。 */
	constexpr FVec4 kAccentColor{ 0.45f, 0.70f, 0.98f, 1.0f };

	/** 文言の色。 */
	constexpr FVec4 kTextColor{ 0.92f, 0.94f, 0.97f, 1.0f };

	/** 進捗バーの溝の色。 */
	constexpr FVec4 kTrackColor{ 0.16f, 0.18f, 0.22f, 1.0f };

	/** スピナーが 1 周するのにかける秒数。 */
	constexpr f32 kSpinSeconds = 1.1f;

	/** スピナーを構成する点の数。 */
	constexpr i32 kSpinnerDots = 8;

	/** スピナーの半径 (画面の短辺に対する割合)。 */
	constexpr f32 kSpinnerRadiusRatio = 0.045f;

	/** 点 1 つの一辺 (スピナーの半径に対する割合)。 */
	constexpr f32 kSpinnerDotRatio = 0.26f;

	/** 進捗バーの幅 (画面幅に対する割合)。 */
	constexpr f32 kBarWidthRatio = 0.32f;

	/** 進捗バーの高さ (画面の短辺に対する割合)。 */
	constexpr f32 kBarHeightRatio = 0.012f;

	/** 一度に積める描画の数 (背景 + スピナー + バー + 文字で足りる量)。 */
	constexpr u32 kOverlayCapacity = 256;

	/**
	 * 濃さを掛けた色を返す。
	 *
	 * @param Color 元の色。
	 * @param Alpha 掛ける濃さ。
	 * @return 掛けた色。
	 */
	FVec4 Fade( const FVec4& Color, f32 Alpha ) noexcept
	{
		return FVec4{ Color.x, Color.y, Color.z, Color.w * Alpha };
	}
}


void FLoadingScreenRenderer::Update( f32 DeltaSeconds ) noexcept
{
	m_Elapsed += DeltaSeconds;
	if ( m_Elapsed >= kSpinSeconds ) m_Elapsed -= kSpinSeconds;
}

void FLoadingScreenRenderer::Draw( CRenderer& Renderer, const FFont* ActiveFont, const FString& Message, f32 Alpha, f32 Progress ) noexcept
{
	IRhiCommandList* const CommandList = Renderer.CommandList();
	IRhiSwapchain* const Swapchain = Renderer.Swapchain();
	if ( CommandList == nullptr || Swapchain == nullptr ) return;

	// 一度も出さなければ GPU 資源を作らない。出す段になってから 1 度だけ用意する。
	if ( !m_bOverlayTried )
	{
		IRhiDevice* const Device = Renderer.Device();
		if ( Device == nullptr ) return;   // まだ用意できない。次のフレームで試す。

		m_bOverlayTried = true;
		const auto Result = m_Overlay.Init( *Device, Renderer.ColorFormat(), kOverlayCapacity );
		m_bOverlayReady = Result.IsOk();
		if ( !m_bOverlayReady )
		{
			ACS_LOG_WARN( "CLoadingScreenSubsystem: 幕の SpriteBatch を用意できなかった (無描画で続ける)" );
		}
	}
	if ( !m_bOverlayReady ) return;

	const f32 Width = static_cast<f32>( Swapchain->Width() );
	const f32 Height = static_cast<f32>( Swapchain->Height() );
	const f32 Short = Width < Height ? Width : Height;
	const f32 CenterX = Width * 0.5f;
	const f32 CenterY = Height * 0.5f;

	m_Overlay.Begin( *CommandList, Swapchain->Width(), Swapchain->Height() );

	// 下の画面を覆う。完全に塗り潰さないのは、何の上で待たされているかを残すため。
	m_Overlay.DrawRect( 0.0f, 0.0f, Width, Height, Fade( kBackdropColor, Alpha ) );

	// スピナー。点を円周に並べ、進んだ位置ほど濃くして回っているように見せる。
	const f32 Radius = Short * kSpinnerRadiusRatio;
	const f32 DotSize = Radius * kSpinnerDotRatio;
	const f32 Turn = kSpinSeconds > 0.0f ? m_Elapsed / kSpinSeconds : 0.0f;
	for ( i32 Index = 0; Index < kSpinnerDots; ++Index )
	{
		const f32 Ratio = static_cast<f32>( Index ) / static_cast<f32>( kSpinnerDots );
		const f32 Angle = ( Ratio + Turn ) * kPi * 2.0f;

		// 先頭を濃く、後ろへ行くほど薄くする。
		const f32 Trail = 0.15f + 0.85f * Ratio;
		const f32 X = CenterX + std::cos( Angle ) * Radius - DotSize * 0.5f;
		const f32 Y = CenterY + std::sin( Angle ) * Radius - DotSize * 0.5f;
		m_Overlay.DrawRect( X, Y, DotSize, DotSize, Fade( kAccentColor, Alpha * Trail ) );
	}

	// 文言はスピナーの下へ。中央へ寄せるので幅を測ってから置く。
	f32 TextBottom = CenterY + Radius;
	if ( ActiveFont != nullptr && !Message.IsEmpty() )
	{
		const f32 TextWidth = ActiveFont->MeasureWidth( Message.Data() );
		const f32 TextY = CenterY + Radius + Short * 0.04f;
		m_Overlay.DrawString( *ActiveFont, Message.Data(), CenterX - TextWidth * 0.5f, TextY, Fade( kTextColor, Alpha ) );
		TextBottom = TextY + ActiveFont->LineHeight();
	}

	// 割合が分かっているときだけバーを出す。分からないのに空のバーを出すと、
	// 進んでいないように見えて不安になる。
	if ( Progress >= 0.0f )
	{
		const f32 BarW = Width * kBarWidthRatio;
		const f32 BarH = Short * kBarHeightRatio;
		const f32 BarX = CenterX - BarW * 0.5f;
		const f32 BarY = TextBottom + Short * 0.03f;

		m_Overlay.DrawRect( BarX, BarY, BarW, BarH, Fade( kTrackColor, Alpha ) );
		m_Overlay.DrawRect( BarX, BarY, BarW * Progress, BarH, Fade( kAccentColor, Alpha ) );
	}

	m_Overlay.End();
}
