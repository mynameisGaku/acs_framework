// SPDX-License-Identifier: Apache-2.0
#include "DebugTopGamepad.h"

namespace
{
	/** 見に行くポートの数 (誰が持っていても動くように全ポートを見る)。 */
	constexpr u32 kPlayerCount = 4;

	/** スティックを倒したとみなす閾値。 */
	constexpr f32 kStickThreshold = 0.5f;

	/**
	 * 全ポートのいずれかでボタンが押されたかを返す。
	 *
	 * @param Button 対象のボタン。
	 * @return 押されていたら true。
	 */
	bool IsAnyPadButtonPressed( EGamepadButton Button ) noexcept
	{
		for ( u32 Player = 0; Player < kPlayerCount; ++Player )
		{
			if ( CInput::IsGamepadButtonPressed( Player, Button ) ) return true;
		}
		return false;
	}

	/**
	 * 全ポートのいずれかでボタンが押されているかを返す。
	 *
	 * @param Button 対象のボタン。
	 * @return 押下中なら true。
	 */
	bool IsAnyPadButtonDown( EGamepadButton Button ) noexcept
	{
		for ( u32 Player = 0; Player < kPlayerCount; ++Player )
		{
			if ( CInput::IsGamepadButtonDown( Player, Button ) ) return true;
		}
		return false;
	}

	/**
	 * 全ポートの中で最も大きく倒れている軸の値を返す。
	 *
	 * @param Axis 対象の軸。
	 * @return 絶対値が最大の値。
	 */
	f32 AnyPadAxisValue( EGamepadAxis Axis ) noexcept
	{
		f32 Strongest = 0.0f;
		for ( u32 Player = 0; Player < kPlayerCount; ++Player )
		{
			const f32 Value = CInput::GamepadAxisValue( Player, Axis );
			const f32 Magnitude = Value < 0.0f ? -Value : Value;
			const f32 StrongestMagnitude = Strongest < 0.0f ? -Strongest : Strongest;
			if ( Magnitude > StrongestMagnitude ) Strongest = Value;
		}
		return Strongest;
	}
}


void CDebugTopGamepadNav::Update( f32 DeltaSeconds ) noexcept
{
	m_bConnected = false;
	for ( u32 Player = 0; Player < kPlayerCount; ++Player )
	{
		if ( !CInput::IsGamepadConnected( Player ) ) continue;

		m_bConnected = true;
		break;
	}

	if ( !m_bConnected )
	{
		// 抜かれた瞬間に入力が残らないよう、状態ごと畳んでおく。
		m_Vertical = 0;
		m_Horizontal = 0;
		m_VerticalRepeat.Reset();
		m_HorizontalRepeat.Reset();
		m_bDecide = false;
		m_bCancel = false;
		return;
	}

	// 十字キーとスティックを同じ入力として合流させる (スティックは上が -Y)。
	i32 RawVertical = 0;
	if ( IsAnyPadButtonDown( EGamepadButton::Up ) )   RawVertical = -1;
	if ( IsAnyPadButtonDown( EGamepadButton::Down ) ) RawVertical = 1;
	if ( RawVertical == 0 )
	{
		const f32 StickY = AnyPadAxisValue( EGamepadAxis::LeftY );
		if ( StickY >= kStickThreshold )       RawVertical = -1;
		else if ( StickY <= -kStickThreshold ) RawVertical = 1;
	}

	i32 RawHorizontal = 0;
	if ( IsAnyPadButtonDown( EGamepadButton::Left ) )  RawHorizontal = -1;
	if ( IsAnyPadButtonDown( EGamepadButton::Right ) ) RawHorizontal = 1;
	if ( RawHorizontal == 0 )
	{
		const f32 StickX = AnyPadAxisValue( EGamepadAxis::LeftX );
		if ( StickX >= kStickThreshold )       RawHorizontal = 1;
		else if ( StickX <= -kStickThreshold ) RawHorizontal = -1;
	}

	m_Vertical = m_VerticalRepeat.Step( RawVertical, DeltaSeconds );
	m_Horizontal = m_HorizontalRepeat.Step( RawHorizontal, DeltaSeconds );

	m_bDecide = IsAnyPadButtonPressed( EGamepadButton::A );
	m_bCancel = IsAnyPadButtonPressed( EGamepadButton::B );
}
