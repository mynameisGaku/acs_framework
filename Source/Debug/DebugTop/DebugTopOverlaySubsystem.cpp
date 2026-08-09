#include "DebugTopOverlaySubsystem.h"

namespace
{
	/** 一度に積める描画の数。行・枠・吹き出し・通知で足りる量。 */
	constexpr u32 kOverlayCapacity = 8192;

	/** 下のゲームを沈める幕の色。完全に隠さないのは、何の上で触っているかを残すため。 */
	constexpr FVec4 kBackdropColor{ 0.02f, 0.03f, 0.05f, 1.0f };
}


// GameInstance スコープへ登録する。シーンを切り替えても同じメニューを持ち続ける。
ACS_REGISTER_SUBSYSTEM( CDebugTopOverlaySubsystem, ESubsystemScope::GameInstance )


ADebugTopHUD& CDebugTopOverlaySubsystem::GetHUD()
{
	if ( !m_HUD )
	{
		m_HUD = NewObject<ADebugTopHUD>();
		m_HUD->Build();
	}
	return *m_HUD;
}

bool CDebugTopOverlaySubsystem::Update( f32 DeltaSeconds )
{
	if ( m_ToggleKey != EKey::Unknown && CInput::IsKeyPressed( m_ToggleKey ) )
	{
		Toggle();

		// 出した / 消したフレームの押下をメニューやゲームへ流さない (F1 で行が実行されない
		// ようにするため)。
		return m_bVisible && m_bPauseWhileVisible;
	}

	if ( !m_bVisible ) return false;

	if ( m_HUD ) m_HUD->Update( DeltaSeconds );

	return m_bPauseWhileVisible;
}

void CDebugTopOverlaySubsystem::Draw( CRenderer& Renderer, FFont* SharedFont )
{
	if ( !m_bVisible || !m_HUD ) return;

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
			ACS_LOG_WARN( "CDebugTopOverlaySubsystem: 重ね描き用の SpriteBatch を用意できなかった" );
		}
	}
	if ( !m_bOverlayReady ) return;

	// 描画の文脈は自前で組み立てる。CGame が持っているものはシーンを描き終えた時点で
	// 畳まれているので、そのまま使うと落ちる (ロード画面が CRenderer を直に見ているのと同じ理由)。
	FRenderContext Context;
	Context._BeginFrame( Renderer, *CommandList, Swapchain->Width(), Swapchain->Height() );
	Context._SetFont( SharedFont );

	// メニューはシーンの上へ丸ごと重ねる。下のゲームは動いたまま見える。
	m_Overlay.Begin( *CommandList, Swapchain->Width(), Swapchain->Height() );

	// 先に幕を敷いて下のゲームを沈める。敷かないと下の文字とメニューの文字が重なって
	// どちらも読めない。透かしてあるので、動いていることは分かる。
	if ( m_BackdropOpacity > 0.0f )
	{
		FVec4 Backdrop = kBackdropColor;
		Backdrop.w = m_BackdropOpacity;
		m_Overlay.DrawRect( 0.0f, 0.0f, static_cast<f32>( Swapchain->Width() ), static_cast<f32>( Swapchain->Height() ), Backdrop );
	}

	m_HUD->Draw( Context, m_Overlay );
	m_Overlay.End();

	Context._EndFrame();
}
