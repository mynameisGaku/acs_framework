// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Text/Localization/TextFormatter.h"

namespace
{
	/** 番号として受け付ける桁数の上限 (これを越えるものは差し込み口とみなさない)。 */
	constexpr usize kMaxIndexDigits = 3u;

	/**
	 * `{` の続きを読み、差し込み口かどうかを見る。
	 *
	 * @param Text 読み元。
	 * @param OpenBrace `{` の位置。
	 * @param OutIndex 差し込み口だったときの番号。
	 * @param OutNextPosition 差し込み口だったときの `}` の次の位置。
	 * @return 差し込み口なら true。
	 */
	bool TryReadPlaceholder( FStringView Text, usize OpenBrace, usize& OutIndex, usize& OutNextPosition ) noexcept
	{
		usize Cursor = OpenBrace + 1u;
		usize Value = 0u;
		usize DigitCount = 0u;

		while ( Cursor < Text.Size() )
		{
			const char Current = Text.Data()[Cursor];

			if ( Current >= '0' && Current <= '9' )
			{
				if ( DigitCount >= kMaxIndexDigits ) return false;

				Value = Value * 10u + static_cast<usize>( Current - '0' );
				++DigitCount;
				++Cursor;
				continue;
			}

			if ( Current == '}' && DigitCount != 0u )
			{
				OutIndex = Value;
				OutNextPosition = Cursor + 1u;

				return true;
			}

			return false;
		}

		return false;
	}
}


FString CTextFormatter::Format( FStringView Text, const FTextArgument* Arguments, usize ArgumentCount ) noexcept
{
	FString Result;
	if ( Text.Data() == nullptr || Text.Size() == 0u ) return Result;

	// 差し込みで縮むことはまず無いので、元の長さぶんは先に確保しておく。
	Result.Reserve( Text.Size() );

	usize Cursor = 0u;
	while ( Cursor < Text.Size() )
	{
		const char Current = Text.Data()[Cursor];

		if ( Current == '{' )
		{
			// "{{" は `{` そのもの。
			if ( Cursor + 1u < Text.Size() && Text.Data()[Cursor + 1u] == '{' )
			{
				if ( !Result.TryAppend( '{' ) ) return Result;
				Cursor += 2u;
				continue;
			}

			usize Index = 0u;
			usize NextPosition = 0u;
			if ( TryReadPlaceholder( Text, Cursor, Index, NextPosition ) )
			{
				// 範囲の外はそのまま残す。消してしまうと «なぜ出ないのか» が追えなくなる。
				if ( Index < ArgumentCount && Arguments != nullptr )
				{
					if ( !Arguments[Index].AppendTo( Result ) ) return Result;
				}
				else
				{
					for ( usize Copy = Cursor; Copy < NextPosition; ++Copy )
					{
						if ( !Result.TryAppend( Text.Data()[Copy] ) ) return Result;
					}
				}

				Cursor = NextPosition;
				continue;
			}
		}
		else if ( Current == '}' && Cursor + 1u < Text.Size() && Text.Data()[Cursor + 1u] == '}' )
		{
			if ( !Result.TryAppend( '}' ) ) return Result;
			Cursor += 2u;
			continue;
		}

		if ( !Result.TryAppend( Current ) ) return Result;
		++Cursor;
	}

	return Result;
}


FString CTextFormatter::FormatOne( FStringView Text, const FTextArgument& Argument ) noexcept
{
	return Format( Text, &Argument, 1u );
}
