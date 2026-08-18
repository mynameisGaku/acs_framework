// SPDX-License-Identifier: Apache-2.0
#include "DebugTopElementColor.h"


namespace
{
	/**
	 * 色相・彩度・明度から RGB を作る。
	 *
	 * @param Hue 色相 (0..1)。
	 * @param Saturation 彩度 (0..1)。
	 * @param Value 明度 (0..1)。
	 * @param OutR 赤の書き込み先。
	 * @param OutG 緑の書き込み先。
	 * @param OutB 青の書き込み先。
	 */
	void HsvToRgb( f32 Hue, f32 Saturation, f32 Value, f32& OutR, f32& OutG, f32& OutB ) noexcept
	{
		/** 色相を 6 分割したときの区画。 */
		const f32 Scaled = ( Hue - static_cast<f32>( static_cast<i32>( Hue ) ) ) * 6.0f;
		const i32 Sector = static_cast<i32>( Scaled );
		const f32 Fraction = Scaled - static_cast<f32>( Sector );

		const f32 Low = Value * ( 1.0f - Saturation );
		const f32 Falling = Value * ( 1.0f - Saturation * Fraction );
		const f32 Rising = Value * ( 1.0f - Saturation * ( 1.0f - Fraction ) );

		switch ( Sector )
		{
		case 0:  OutR = Value;   OutG = Rising;  OutB = Low;     break;
		case 1:  OutR = Falling; OutG = Value;   OutB = Low;     break;
		case 2:  OutR = Low;     OutG = Value;   OutB = Rising;  break;
		case 3:  OutR = Low;     OutG = Falling; OutB = Value;   break;
		case 4:  OutR = Rising;  OutG = Low;     OutB = Value;   break;
		default: OutR = Value;   OutG = Low;     OutB = Falling; break;
		}
	}

	/**
	 * RGB から彩度と明度を取り出す。
	 *
	 * @details
	 * 色相は取らない。明度や彩度が 0 になると RGB から色相を復元できず、そのたびに赤へ
	 * 飛んでしまうため、色相は行の側で覚えたものを使う。
	 * @param Color 対象の色。
	 * @param OutSaturation 彩度 (0..1) の書き込み先。
	 * @param OutValue 明度 (0..1) の書き込み先。
	 */
	void RgbToSaturationValue( const FVec4& Color, f32& OutSaturation, f32& OutValue ) noexcept
	{
		f32 Max = Color.x;
		if ( Color.y > Max ) Max = Color.y;
		if ( Color.z > Max ) Max = Color.z;

		f32 Min = Color.x;
		if ( Color.y < Min ) Min = Color.y;
		if ( Color.z < Min ) Min = Color.z;

		OutValue = Max;
		OutSaturation = Max > 0.0f ? ( Max - Min ) / Max : 0.0f;
	}
}


void CDebugTopElementColor::PickAt( f32 LocalX, f32 LocalY, f32 Width, f32 Height )
{
	if ( Width <= 0.0f || Height <= 0.0f ) return;

	// 面と帯の割り当ては描画側と同じ定数を使う (上側が彩度・明度、下側が色相)。
	const f32 FieldHeight = Height * kDebugTopColorFieldPlaneRatio;

	f32 X = LocalX / Width;
	if ( X < 0.0f ) X = 0.0f;
	if ( X > 1.0f ) X = 1.0f;

	const FVec4 Current = GetValue();

	if ( LocalY < FieldHeight )
	{
		// 彩度・明度の面。左が白、右が原色、上が明るく下が暗い。
		f32 Y = LocalY / FieldHeight;
		if ( Y < 0.0f ) Y = 0.0f;
		if ( Y > 1.0f ) Y = 1.0f;

		f32 R = 0.0f, G = 0.0f, B = 0.0f;
		HsvToRgb( m_Hue, X, 1.0f - Y, R, G, B );
		SetValue( FVec4{ R, G, B, Current.w } );
		return;
	}

	// 色相の帯。押した位置がそのまま色相になる。
	m_Hue = X;

	// 彩度と明度はいまの色のまま保つ。ここで 1 に戻すと、淡い色を作った後で色相を
	// 動かしただけで原色へ飛んでしまい、狙った色を追い込めない。
	f32 Saturation = 0.0f;
	f32 Value = 0.0f;
	RgbToSaturationValue( Current, Saturation, Value );

	// 黒や白から始めた場合は動かす余地が無いので、そのときだけ原色を入れる。
	if ( Saturation <= 0.0f ) Saturation = 1.0f;
	if ( Value <= 0.0f )      Value = 1.0f;

	f32 R = 0.0f, G = 0.0f, B = 0.0f;
	HsvToRgb( m_Hue, Saturation, Value, R, G, B );
	SetValue( FVec4{ R, G, B, Current.w } );
}


void CDebugTopElementColor::GetPickerState( f32& OutHue, f32& OutSaturation, f32& OutValue ) const noexcept
{
	OutHue = m_Hue;
	RgbToSaturationValue( GetValue(), OutSaturation, OutValue );
}


CDebugTopElementColor::CDebugTopElementColor( const FString& Label, const FVec4& Value )
	: CDebugTopElement( Label )
{
	// 成分ごとにスライダー付きの行を持たせる。値はこの子行が持つので、保存もそのまま乗る。
	static const char* const kNames[4] = { "R", "G", "B", "A" };
	const f32 Initial[4] = { Value.x, Value.y, Value.z, Value.w };

	// 面は行として持たない。色見本を押したときだけ浮かせて出す (常に置くと縦に嵩み、
	// 段差の中に大きな四角が挟まって読みにくい)。畳んでいる間も数値で確かめられる。
	for ( usize Index = 0; Index < 4; ++Index )
	{
		m_Components[Index] = Add<CDebugTopElementFloat>( FString( kNames[Index] ), Initial[Index], 0.0f, 1.0f, 0.05f );
	}
}


FVec4 CDebugTopElementColor::GetValue() const noexcept
{
	FVec4 Color{ 0.0f, 0.0f, 0.0f, 1.0f };
	if ( m_Components[0] != nullptr ) Color.x = m_Components[0]->GetValue();
	if ( m_Components[1] != nullptr ) Color.y = m_Components[1]->GetValue();
	if ( m_Components[2] != nullptr ) Color.z = m_Components[2]->GetValue();
	if ( m_Components[3] != nullptr ) Color.w = m_Components[3]->GetValue();
	return Color;
}


void CDebugTopElementColor::SetValue( const FVec4& Value )
{
	const f32 Components[4] = { Value.x, Value.y, Value.z, Value.w };
	for ( usize Index = 0; Index < 4; ++Index )
	{
		if ( m_Components[Index] == nullptr ) continue;

		m_Components[Index]->SetValue( Components[Index] );
	}
}


bool CDebugTopElementColor::TryGetColorSwatch( FVec4& OutColor ) const noexcept
{
	OutColor = GetValue();
	return true;
}
