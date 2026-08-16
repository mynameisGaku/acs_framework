// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Text/Localization/LocalizationTableParser.h"

#include "AcsFramework_Core/Text/Localization/LocaleName.h"

namespace
{
	/** 空白として落とす文字かどうか。 */
	bool IsSpace( char Value ) noexcept
	{
		return Value == ' ' || Value == '\t' || Value == '\r';
	}

	/** 前後の空白を落とした範囲を返す。 */
	FStringView Trim( FStringView Text ) noexcept
	{
		usize Begin = 0u;
		usize End = Text.Size();

		while ( Begin < End && IsSpace( Text.Data()[Begin] ) ) ++Begin;
		while ( End > Begin && IsSpace( Text.Data()[End - 1u] ) ) --End;

		return FStringView( Text.Data() + Begin, End - Begin );
	}

	/**
	 * 次の改行までを 1 行として切り出す。
	 *
	 * @param Text 読み元。
	 * @param Cursor 読み位置。読んだぶん進む。
	 * @return 切り出した行 (改行は含まない)。
	 */
	FStringView ReadLine( FStringView Text, usize& Cursor ) noexcept
	{
		const usize Begin = Cursor;
		while ( Cursor < Text.Size() && Text.Data()[Cursor] != '\n' ) ++Cursor;

		const usize End = Cursor;
		if ( Cursor < Text.Size() ) ++Cursor;

		return FStringView( Text.Data() + Begin, End - Begin );
	}

	/** その行が `[...]` の見出しかどうか。 */
	bool IsLocaleHeader( FStringView Line ) noexcept
	{
		return Line.Size() >= 2u && Line.Data()[0] == '[' && Line.Data()[Line.Size() - 1u] == ']';
	}
}


FLocalizationParseResult CLocalizationTableParser::ParseInto( CLocaleCatalog& OutCatalog, FStringView Text ) noexcept
{
	FLocalizationParseResult Result;
	if ( Text.Data() == nullptr || Text.Size() == 0u ) return Result;

	ELocale CurrentLocale = ELocale::Default;
	bool bHasLocale = false;
	bool bSkippingSection = false;

	usize Cursor = 0u;
	while ( Cursor < Text.Size() )
	{
		const FStringView Line = Trim( ReadLine( Text, Cursor ) );

		if ( Line.Size() == 0u ) continue;
		if ( Line.Data()[0] == '#' ) continue;

		if ( IsLocaleHeader( Line ) )
		{
			const FStringView Name = Trim( FStringView( Line.Data() + 1u, Line.Size() - 2u ) );

			// 知らない言語は、その節ごと飛ばす。1 行ずつ数えると数が膨らんで読めなくなるので、
			// 節の見出し 1 つぶんだけ «落とした» に数える。
			bSkippingSection = !CLocaleName::TryParse( Name, CurrentLocale );
			if ( bSkippingSection ) ++Result.Skipped;
			else bHasLocale = true;

			continue;
		}

		if ( bSkippingSection ) continue;

		if ( !bHasLocale )
		{
			// 言語の見出しより前に書かれた行。どの言語か決まらないので足せない。
			Result.bMissingLocaleHeader = true;
			++Result.Skipped;
			continue;
		}

		const usize Separator = Line.Find( '=' );
		if ( Separator == static_cast<usize>( -1 ) )
		{
			++Result.Skipped;
			continue;
		}

		const FStringView Key = Trim( FStringView( Line.Data(), Separator ) );
		const FStringView Value = Trim( FStringView( Line.Data() + Separator + 1u, Line.Size() - Separator - 1u ) );

		if ( Key.Size() == 0u )
		{
			++Result.Skipped;
			continue;
		}

		FString KeyText;
		FString ValueText;
		if ( !KeyText.TryAppend( Key ) || ( Value.Size() != 0u && !ValueText.TryAppend( Value ) ) )
		{
			++Result.Skipped;
			continue;
		}

		if ( OutCatalog.Register( CurrentLocale, KeyText, ValueText ) ) ++Result.Registered;
		else ++Result.Skipped;
	}

	return Result;
}
