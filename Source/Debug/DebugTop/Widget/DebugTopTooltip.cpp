#include "DebugTopTooltip.h"

namespace
{
	/** 同じ行を指し続けてから出るまでの秒数。動かしている間ずっと出ると鬱陶しい。 */
	constexpr f32 kHoverDelaySeconds = 0.35f;

	/** ポインタと吹き出しの間隔 (行の高さに対する倍率)。 */
	constexpr f32 kCursorGapRatio = 0.7f;

	/** 吹き出しの内側の余白 (行の高さに対する倍率)。 */
	constexpr f32 kPaddingRatio = 0.4f;

	/** 画面端との間隔 (行の高さに対する倍率)。 */
	constexpr f32 kScreenMarginRatio = 0.3f;

	/** 吹き出しの下敷きの色。行の上へ浮かせるので、ほぼ不透明で敷く。 */
	constexpr FVec4 kPanelColor{ 0.06f, 0.07f, 0.10f, 0.96f };

	/** 吹き出しの縁の色。 */
	constexpr FVec4 kBorderColor{ 0.45f, 0.55f, 0.70f, 0.90f };

	/** 吹き出しの縁の太さ (ピクセル)。 */
	constexpr f32 kBorderWidth = 1.0f;

	/** 吹き出しの文字色。 */
	constexpr FVec4 kTextColor{ 0.90f, 0.92f, 0.96f, 1.0f };
}


void CDebugTopTooltip::Update( const CDebugTopElement* Element, f32 DeltaSeconds ) noexcept
{
	// 説明を持たない行は出しようがないので、指していないのと同じに扱う。
	if ( Element != nullptr && Element->GetDescription().IsEmpty() ) Element = nullptr;

	if ( Element != m_Element )
	{
		m_Element = Element;
		m_HoverSeconds = 0.0f;
		return;
	}
	if ( m_Element == nullptr ) return;

	m_HoverSeconds += DeltaSeconds;
}

void CDebugTopTooltip::Draw( CSpriteBatch& Batch, const CDebugTopText& Text, f32 ScreenWidth, f32 ScreenHeight ) noexcept
{
	if ( m_Element == nullptr || m_HoverSeconds < kHoverDelaySeconds ) return;

	const f32 LineHeight = Text.LineHeight();
	if ( LineHeight <= 0.0f ) return;

	const FString& Description = m_Element->GetDescription();
	const f32 TextWidth = Text.MeasureWidth( Description.Data() );
	const f32 TextHeight = Text.MeasureHeight( Description.Data() );
	if ( TextWidth <= 0.0f || TextHeight <= 0.0f ) return;

	const f32 Padding = LineHeight * kPaddingRatio;
	const f32 PanelWidth = TextWidth + Padding * 2.0f;
	const f32 PanelHeight = TextHeight + Padding * 2.0f;

	// ポインタの右下へ置く。指しているものを吹き出しで隠さないため。
	const FVec2 Mouse = CInput::MousePos();
	const f32 Gap = LineHeight * kCursorGapRatio;
	f32 PanelX = Mouse.x + Gap;
	f32 PanelY = Mouse.y + Gap;

	// 画面からはみ出すなら、ポインタの反対側へ回して内側に収める。
	const f32 Margin = LineHeight * kScreenMarginRatio;
	if ( PanelX + PanelWidth > ScreenWidth - Margin ) PanelX = Mouse.x - Gap - PanelWidth;
	if ( PanelY + PanelHeight > ScreenHeight - Margin ) PanelY = Mouse.y - Gap - PanelHeight;
	if ( PanelX < Margin ) PanelX = Margin;
	if ( PanelY < Margin ) PanelY = Margin;

	Batch.DrawRect( PanelX, PanelY, PanelWidth, PanelHeight, kPanelColor );

	// 縁は 4 辺を細い矩形で描く (CSpriteBatch に枠線の描画が無いため)。
	Batch.DrawRect( PanelX, PanelY, PanelWidth, kBorderWidth, kBorderColor );
	Batch.DrawRect( PanelX, PanelY + PanelHeight - kBorderWidth, PanelWidth, kBorderWidth, kBorderColor );
	Batch.DrawRect( PanelX, PanelY, kBorderWidth, PanelHeight, kBorderColor );
	Batch.DrawRect( PanelX + PanelWidth - kBorderWidth, PanelY, kBorderWidth, PanelHeight, kBorderColor );

	Text.Draw( Batch, Description.Data(), PanelX + Padding, PanelY + Padding, kTextColor );
}
