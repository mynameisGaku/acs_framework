// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Text/Localization/TextArgument.h"

namespace
{
	/** i64 の桁数と符号を収めるのに足りる長さ。 */
	constexpr usize kDigitBufferSize = 24u;

	/** 小数点以下に出せる桁数の上限。 */
	constexpr i32 kMaxDecimals = 9;

	/**
	 * 符号なし整数を 10 進の文字へ直して足す。
	 *
	 * @param Value 直す値。
	 * @param OutText 足す先。
	 * @return 足せたら true。
	 */
	bool AppendUnsigned( u64 Value, FString& OutText ) noexcept
	{
		char Digits[kDigitBufferSize];
		usize Count = 0u;

		// 下の桁から出るので、いったん逆順に貯めて後で返す。
		do
		{
			Digits[Count] = static_cast<char>( '0' + static_cast<u32>( Value % 10u ) );
			++Count;
			Value /= 10u;
		}
		while ( Value != 0u && Count < kDigitBufferSize );

		for ( usize Index = Count; Index > 0u; --Index )
		{
			if ( !OutText.TryAppend( Digits[Index - 1u] ) ) return false;
		}

		return true;
	}

	/**
	 * 符号付き整数を 10 進の文字へ直して足す。
	 *
	 * @details i64 の最小値は符号を反転できないので、符号なしへ移してから扱う。
	 */
	bool AppendSigned( i64 Value, FString& OutText ) noexcept
	{
		if ( Value < 0 )
		{
			if ( !OutText.TryAppend( '-' ) ) return false;

			const u64 Magnitude = ~static_cast<u64>( Value ) + 1u;

			return AppendUnsigned( Magnitude, OutText );
		}

		return AppendUnsigned( static_cast<u64>( Value ), OutText );
	}

	/** 10 の n 乗を返す。 */
	f64 PowerOfTen( i32 Exponent ) noexcept
	{
		f64 Result = 1.0;
		for ( i32 Index = 0; Index < Exponent; ++Index ) Result *= 10.0;

		return Result;
	}
}


FTextArgument FTextArgument::FromText( FStringView Value ) noexcept
{
	FTextArgument Argument;
	Argument.Kind = EKind::Text;
	Argument.TextValue = Value;

	return Argument;
}


FTextArgument FTextArgument::FromInteger( i64 Value ) noexcept
{
	FTextArgument Argument;
	Argument.Kind = EKind::Integer;
	Argument.IntegerValue = Value;

	return Argument;
}


FTextArgument FTextArgument::FromReal( f64 Value, i32 DecimalPlaces ) noexcept
{
	FTextArgument Argument;
	Argument.Kind = EKind::Real;
	Argument.RealValue = Value;
	Argument.Decimals = DecimalPlaces < 0 ? 0 : ( DecimalPlaces > kMaxDecimals ? kMaxDecimals : DecimalPlaces );

	return Argument;
}


bool FTextArgument::AppendTo( FString& OutText ) const noexcept
{
	if ( Kind == EKind::Text )
	{
		if ( TextValue.Data() == nullptr || TextValue.Size() == 0u ) return true;

		return OutText.TryAppend( TextValue );
	}

	if ( Kind == EKind::Integer ) return AppendSigned( IntegerValue, OutText );

	// 実数。負号を先に出してから、絶対値を整数部と小数部に分けて出す。
	f64 Magnitude = RealValue;
	if ( Magnitude < 0.0 )
	{
		if ( !OutText.TryAppend( '-' ) ) return false;
		Magnitude = -Magnitude;
	}

	const f64 Scale = PowerOfTen( Decimals );

	// 桁を出す前に丸める。丸め上がりで整数部が繰り上がる場合をここで吸収する。
	const u64 Scaled = static_cast<u64>( Magnitude * Scale + 0.5 );
	const u64 Whole = Scaled / static_cast<u64>( Scale );
	const u64 Fraction = Scaled - Whole * static_cast<u64>( Scale );

	if ( !AppendUnsigned( Whole, OutText ) ) return false;
	if ( Decimals <= 0 ) return true;
	if ( !OutText.TryAppend( '.' ) ) return false;

	// 小数部は上の桁から出す。0.05 が ".5" にならないよう、足りない分を 0 で埋める。
	u64 Divisor = static_cast<u64>( Scale ) / 10u;
	for ( i32 Index = 0; Index < Decimals; ++Index )
	{
		const u64 Digit = Divisor == 0u ? 0u : ( Fraction / Divisor ) % 10u;
		if ( !OutText.TryAppend( static_cast<char>( '0' + static_cast<u32>( Digit ) ) ) ) return false;

		Divisor /= 10u;
	}

	return true;
}
