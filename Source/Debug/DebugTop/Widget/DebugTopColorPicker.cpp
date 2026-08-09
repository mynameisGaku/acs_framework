#include "DebugTopColorPicker.h"

#include "Debug/DebugTop/Render/DebugTopColorField.h"

namespace
{
	/** 色を選ぶ面の高さ (文字 1 行の高さに対する倍率)。 */
	constexpr f32 kPickerFieldHeightRatio = 6.0f;

	/** パネルの内側の余白 (文字 1 行の高さに対する倍率)。 */
	constexpr f32 kPickerPadRatio = 0.3f;

	/** 起点とパネルの間隔 (文字 1 行の高さに対する倍率)。 */
	constexpr f32 kPickerGapRatio = 0.2f;

	/** パネルの背景。行の上へ浮かせるので透かさない。 */
	constexpr FVec4 kPickerPanelColor{ 0.07f, 0.08f, 0.11f, 1.0f };

	/** パネルの縁。 */
	constexpr FVec4 kPickerBorderColor{ 0.45f, 0.52f, 0.65f, 1.0f };

	/** パネルの縁の太さ (ピクセル)。 */
	constexpr f32 kPickerBorderWidth = 1.0f;
}


void CDebugTopColorPicker::Open( CDebugTopElementColor& Target, f32 AnchorX, f32 AnchorY ) noexcept
{
	Close();

	m_Target = &Target;
	m_Target->SetPickerOpen( true );
	m_AnchorX = AnchorX;
	m_AnchorY = AnchorY;
}

void CDebugTopColorPicker::Close() noexcept
{
	if ( m_Target != nullptr ) m_Target->SetPickerOpen( false );

	m_Target = nullptr;
	m_PanelWidth = 0.0f;
	m_PanelHeight = 0.0f;
	m_bDragging = false;
}

bool CDebugTopColorPicker::Update() noexcept
{
	// まだ一度も描いていない間は矩形が無いので、当たり判定のしようがない。
	if ( m_Target == nullptr || m_PanelWidth <= 0.0f ) return false;

	const FVec2 Mouse = CInput::MousePos();
	const bool bInside = Mouse.x >= m_PanelX && Mouse.x <= m_PanelX + m_PanelWidth && Mouse.y >= m_PanelY && Mouse.y <= m_PanelY + m_PanelHeight;

	if ( !CInput::IsMouseButtonDown( EMouseButton::Left ) ) m_bDragging = false;

	if ( CInput::IsMouseButtonPressed( EMouseButton::Left ) )
	{
		// パネルの外を押したら閉じる。閉じ方を探させないため、どこを押しても閉じる。
		if ( !bInside )
		{
			Close();
			return true;
		}
		m_bDragging = true;
	}

	if ( m_bDragging )
	{
		// 押した位置は面 (枠の内側) の座標系で渡す。掴んだまま外へ出ても追従する。
		m_Target->PickAt( Mouse.x - m_FieldX, Mouse.y - m_FieldY, m_FieldWidth, m_FieldHeight );
		return true;
	}

	// 中に居るだけでも、下の行が反応しないよう食っておく。
	return bInside;
}

void CDebugTopColorPicker::Draw( CSpriteBatch& Batch, FRenderContext& RenderContext, f32 BaseHeight ) noexcept
{
	if ( m_Target == nullptr ) return;

	const f32 Pad = BaseHeight * kPickerPadRatio;
	const f32 FieldH = BaseHeight * kPickerFieldHeightRatio;
	const f32 Width = FieldH * kDebugTopColorFieldAspectRatio + Pad * 2.0f;
	const f32 Height = FieldH + Pad * 2.0f;

	// 起点の右隣へ出す。真下だと成分の数値を覆ってしまい、動かしながら数字を追えない。
	const f32 Gap = BaseHeight * kPickerGapRatio;
	f32 X = m_AnchorX + Gap;
	f32 Y = m_AnchorY - Height * 0.5f;

	// 画面からはみ出すなら内側へ寄せる (端の行でも全部見えるように)。
	const f32 ScreenW = static_cast<f32>( RenderContext.Width() );
	const f32 ScreenH = static_cast<f32>( RenderContext.Height() );
	if ( X + Width > ScreenW ) X = ScreenW - Width - Gap;
	if ( X < 0.0f ) X = 0.0f;
	if ( Y + Height > ScreenH ) Y = ScreenH - Height - Gap;
	if ( Y < 0.0f ) Y = Gap;

	m_PanelX = X;
	m_PanelY = Y;
	m_PanelWidth = Width;
	m_PanelHeight = Height;

	// 背景と枠。行の上へ浮かせるので、下が透けないよう不透明で敷く。
	Batch.DrawRect( X - kPickerBorderWidth, Y - kPickerBorderWidth, Width + kPickerBorderWidth * 2.0f, Height + kPickerBorderWidth * 2.0f, kPickerBorderColor );
	Batch.DrawRect( X, Y, Width, Height, kPickerPanelColor );

	f32 Hue = 0.0f;
	f32 Saturation = 0.0f;
	f32 Value = 0.0f;
	m_Target->GetPickerState( Hue, Saturation, Value );

	// 押されたときに同じ座標系で渡せるよう、面そのものの矩形も控える。
	m_FieldX = X + Pad;
	m_FieldY = Y + Pad;
	m_FieldWidth = FieldH * kDebugTopColorFieldAspectRatio;
	m_FieldHeight = FieldH;

	DebugTopDrawColorField( Batch, m_FieldX, m_FieldY, FieldH, Hue, Saturation, Value, 1.0f );
}
