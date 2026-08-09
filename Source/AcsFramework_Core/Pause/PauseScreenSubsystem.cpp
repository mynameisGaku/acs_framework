#include "PauseScreenSubsystem.h"

namespace
{
	/** 出し入れにかける秒数。一瞬だけ止めてもちらつかない程度に短く取る。 */
	constexpr f32 kFadeSeconds = 0.14f;

	/** 背景の色 (下の画面をうっすら残す。何を止めたのかが分かるように)。 */
	constexpr FVec4 kBackdropColor{ 0.02f, 0.03f, 0.05f, 0.62f };

	/** 印と文言の色。 */
	constexpr FVec4 kAccentColor{ 0.95f, 0.96f, 0.98f, 1.0f };

	/** 印 (縦棒 2 本) の高さ (画面の短辺に対する割合)。 */
	constexpr f32 kMarkHeightRatio = 0.085f;

	/** 縦棒 1 本の幅 (印の高さに対する割合)。 */
	constexpr f32 kMarkBarRatio = 0.30f;

	/** 縦棒どうしの間隔 (印の高さに対する割合)。 */
	constexpr f32 kMarkGapRatio = 0.24f;

	/** 印と文言の間隔 (画面の短辺に対する割合)。 */
	constexpr f32 kTextMarginRatio = 0.045f;

	/** 一度に積める描画の数 (背景 + 印 + 文字で足りる量)。 */
	constexpr u32 kOverlayCapacity = 128;

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


// GameInstance スコープへ登録する。シーンを切り替えても出したままにできる。
ACS_REGISTER_SUBSYSTEM( CPauseScreenSubsystem, ESubsystemScope::GameInstance )


void CPauseScreenSubsystem::Show( const FString& Message )
{
	m_Message = Message;
	m_bVisible = true;
}


void CPauseScreenSubsystem::Follow( const CTimeSubsystem& Time, const FString& Reason, const FString& Message )
{
	m_Followed = &Time;
	m_Reason = Reason;
	m_Message = Message;

	// ここでは出さない。実際にその理由で止まっていることを Update で見てから出す。
}


void CPauseScreenSubsystem::Unfollow() noexcept
{
	m_Followed = nullptr;
	m_bVisible = false;
}


void CPauseScreenSubsystem::UpdateFollow() noexcept
{
	if ( m_Followed == nullptr ) return;

	// 他の理由 (デバッグメニュー等) で止まっていても出さない。開発用の道具でゲームの
	// ポーズ画面が出ては、何が起きているのか分からなくなる。
	m_bVisible = m_Followed->IsPausedBy( m_Reason );
}


void CPauseScreenSubsystem::SetMessage( const FString& Message )
{
	m_Message = Message;
}


void CPauseScreenSubsystem::Update( f32 DeltaSeconds ) noexcept
{
	UpdateFollow();

	// 出し入れは滑らかに繋ぐ。
	const f32 Step = kFadeSeconds > 0.0f ? DeltaSeconds / kFadeSeconds : 1.0f;
	m_Alpha += m_bVisible ? Step : -Step;
	if ( m_Alpha < 0.0f ) m_Alpha = 0.0f;
	if ( m_Alpha > 1.0f ) m_Alpha = 1.0f;
}


void CPauseScreenSubsystem::Draw( CRenderer& Renderer, const FFont* SharedFont ) noexcept
{
	if ( !IsOnScreen() ) return;

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

	// 下の画面を覆う。ロード画面より薄くする (待たせているのではなく、止めているだけなので)。
	m_Overlay.DrawRect( 0.0f, 0.0f, Width, Height, Fade( kBackdropColor, m_Alpha ) );

	// 印は縦棒 2 本。文字を出さない場合でも、止まっていることがひと目で分かるようにする。
	const f32 MarkHeight = Short * kMarkHeightRatio;
	const f32 BarWidth = MarkHeight * kMarkBarRatio;
	const f32 Gap = MarkHeight * kMarkGapRatio;
	const f32 MarkY = CenterY - MarkHeight * 0.5f;
	m_Overlay.DrawRect( CenterX - Gap * 0.5f - BarWidth, MarkY, BarWidth, MarkHeight, Fade( kAccentColor, m_Alpha ) );
	m_Overlay.DrawRect( CenterX + Gap * 0.5f, MarkY, BarWidth, MarkHeight, Fade( kAccentColor, m_Alpha ) );

	// 文言は印の下へ。中央へ寄せるので幅を測ってから置く。
	if ( m_Message.IsEmpty() ) { m_Overlay.End(); return; }
	if ( SharedFont == nullptr && m_Font == nullptr )
	{
		// 黙って文字だけ消えると原因を探しにくいので、1 度だけ知らせる。
		if ( !m_bFontWarned )
		{
			m_bFontWarned = true;
			ACS_LOG_WARN( "CPauseScreenSubsystem: フォントが無いので文言を出せない (SetFont で渡すこと)" );
		}
		m_Overlay.End();
		return;
	}

	const FFont& Font = m_Font != nullptr ? *m_Font : *SharedFont;
	const f32 TextWidth = Font.MeasureWidth( m_Message.Data() );
	m_Overlay.DrawString( Font, m_Message.Data(), CenterX - TextWidth * 0.5f, MarkY + MarkHeight + Short * kTextMarginRatio, Fade( kAccentColor, m_Alpha ) );

	m_Overlay.End();
}
