// SPDX-License-Identifier: Apache-2.0
#include "DebugTopOverlayRenderer.h"

#include "Debug/DebugTop/DebugTopHUD.h"

namespace
{
	/** HUD の行、枠、通知を 1 回で積める描画数。 */
	constexpr u32 kOverlayCapacity = 8192;

	/** HUD の背後でゲーム画面を沈める幕の色。 */
	constexpr FVec4 kBackdropColor{ 0.02f, 0.03f, 0.05f, 1.0f };
}


void FDebugTopOverlayRenderer::Draw( CRenderer& Renderer, ADebugTopHUD& HUD, FFont* SharedFont, f32 BackdropOpacity ) noexcept
{
	// 今回の重ね描きで使うコマンド列と描画先。
	IRhiCommandList* const CommandList = Renderer.CommandList();
	IRhiSwapchain* const Swapchain = Renderer.Swapchain();
	if ( CommandList == nullptr || Swapchain == nullptr ) return;

	if ( !m_bOverlayTried )
	{
		// 遅延初期化に使う描画機器。
		IRhiDevice* const Device = Renderer.Device();
		if ( Device == nullptr ) return;

		m_bOverlayTried = true;
		// SpriteBatch を利用できるか示す初期化結果。
		const auto Result = m_Overlay.Init( *Device, Renderer.ColorFormat(), kOverlayCapacity );
		m_bOverlayReady = Result.IsOk();
		if ( !m_bOverlayReady )
		{
			ACS_LOG_WARN( "FDebugTopOverlayRenderer: 重ね描き用の SpriteBatch を用意できなかった" );
		}
	}
	if ( !m_bOverlayReady ) return;

	// HUD がこのフレームだけ使う描画文脈。
	FRenderContext Context;
	auto Wiring = Context.WiringAccess();
	Wiring.BeginFrame( Renderer, *CommandList, Swapchain->Width(), Swapchain->Height() );
	Wiring.SetFont( SharedFont );

	m_Overlay.Begin( *CommandList, Swapchain->Width(), Swapchain->Height() );
	if ( BackdropOpacity > 0.0f )
	{
		// 設定された濃さを反映する背景幕の色。
		FVec4 Backdrop = kBackdropColor;
		Backdrop.w = BackdropOpacity;
		m_Overlay.DrawRect( 0.0f, 0.0f, static_cast<f32>( Swapchain->Width() ), static_cast<f32>( Swapchain->Height() ), Backdrop );
	}

	HUD.Draw( Context, m_Overlay );
	m_Overlay.End();
	Wiring.EndFrame();
}
