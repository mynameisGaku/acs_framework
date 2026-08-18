// SPDX-License-Identifier: Apache-2.0
#include "DebugTopElementValue.h"

#include "Debug/DebugTop/Element/DebugTopElement.h"


bool DebugTopCaptureValue( const CDebugTopElement& Element, FDebugTopElementValue& OutValue )
{
	OutValue = FDebugTopElementValue();

	switch ( Element.GetValueKind() )
	{
	case EDebugTopValueKind::Float:
	{
		f32 Value = 0.0f;
		if ( !Element.TryGetFloat( Value ) ) return false;

		OutValue.Kind = EDebugTopValueKind::Float;
		OutValue.FloatValue = Value;
		return true;
	}
	case EDebugTopValueKind::Bool:
	{
		bool bValue = false;
		if ( !Element.TryGetBool( bValue ) ) return false;

		OutValue.Kind = EDebugTopValueKind::Bool;
		OutValue.bBoolValue = bValue;
		return true;
	}
	case EDebugTopValueKind::String:
		OutValue.Kind = EDebugTopValueKind::String;
		OutValue.StringValue = Element.GetValueText();
		return true;

	case EDebugTopValueKind::Int:
	case EDebugTopValueKind::Enum:
	{
		i32 Value = 0;
		if ( !Element.TryGetInt( Value ) ) return false;

		OutValue.Kind = Element.GetValueKind();
		OutValue.IntValue = Value;
		return true;
	}
	default:
		// 配列やベクトルは、成分の行が個別に値を持つ。親の行としては控えない。
		return false;
	}
}


bool DebugTopApplyValue( CDebugTopElement& Element, const FDebugTopElementValue& Value )
{
	if ( Element.GetValueKind() != Value.Kind ) return false;

	switch ( Value.Kind )
	{
	case EDebugTopValueKind::Float: return Element.TrySetFloat( Value.FloatValue );
	case EDebugTopValueKind::Bool:  return Element.TrySetBool( Value.bBoolValue );

	// 文字列の行は打ち込みと同じ経路で書き戻す (整形や検査を通すため)。
	case EDebugTopValueKind::String: return Element.CommitEditText( Value.StringValue );

	case EDebugTopValueKind::Int:
	case EDebugTopValueKind::Enum: return Element.TrySetInt( Value.IntValue );

	default: return false;
	}
}


bool DebugTopValuesEqual( const FDebugTopElementValue& Left, const FDebugTopElementValue& Right ) noexcept
{
	if ( Left.Kind != Right.Kind ) return false;

	switch ( Left.Kind )
	{
	case EDebugTopValueKind::Float:  return Left.FloatValue == Right.FloatValue;
	case EDebugTopValueKind::Bool:   return Left.bBoolValue == Right.bBoolValue;
	case EDebugTopValueKind::String: return Left.StringValue == Right.StringValue;

	case EDebugTopValueKind::Int:
	case EDebugTopValueKind::Enum: return Left.IntValue == Right.IntValue;

	default: return true;
	}
}
