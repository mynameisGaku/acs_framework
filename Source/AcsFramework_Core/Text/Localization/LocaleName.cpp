// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Text/Localization/LocaleName.h"

#include "Common/Compat/AcsEnumReflection.h"

namespace
{
	/** 大文字を小文字へ寄せる。 */
	char ToLower( char Value ) noexcept
	{
		return ( Value >= 'A' && Value <= 'Z' ) ? static_cast<char>( Value + ( 'a' - 'A' ) ) : Value;
	}

	/** 区切りとして無視する文字かどうか。 */
	bool IsSeparator( char Value ) noexcept
	{
		return Value == '-' || Value == '_';
	}

	/**
	 * 区切りと大文字小文字を無視して比べる。
	 *
	 * @details "zh-cn" と "ZhCn" を同じものとして扱うため。
	 */
	bool MatchesLoosely( FStringView Left, FStringView Right ) noexcept
	{
		usize LeftIndex = 0u;
		usize RightIndex = 0u;

		for ( ;; )
		{
			while ( LeftIndex < Left.Size() && IsSeparator( Left.Data()[LeftIndex] ) ) ++LeftIndex;
			while ( RightIndex < Right.Size() && IsSeparator( Right.Data()[RightIndex] ) ) ++RightIndex;

			const bool bLeftEnded = LeftIndex >= Left.Size();
			const bool bRightEnded = RightIndex >= Right.Size();
			if ( bLeftEnded || bRightEnded ) return bLeftEnded && bRightEnded;

			if ( ToLower( Left.Data()[LeftIndex] ) != ToLower( Right.Data()[RightIndex] ) ) return false;

			++LeftIndex;
			++RightIndex;
		}
	}
}


bool CLocaleName::TryParse( FStringView Text, ELocale& OutLocale ) noexcept
{
	if ( Text.Data() == nullptr || Text.Size() == 0u ) return false;

	const AcsFw::FEnumNameView* const Names = AcsFw::EnumNames<ELocale>();
	const usize Count = AcsFw::EnumCount<ELocale>();

	for ( usize Index = 0u; Index < Count; ++Index )
	{
		if ( Names[Index].IsEmpty() ) continue;

		if ( MatchesLoosely( Text, FStringView( Names[Index].Data, Names[Index].Size ) ) )
		{
			OutLocale = AcsFw::EnumFromIndex<ELocale>( Index );

			return true;
		}
	}

	return false;
}


FStringView CLocaleName::ToText( ELocale Locale ) noexcept
{
	const AcsFw::FEnumNameView Name = AcsFw::EnumToString( Locale );
	if ( Name.IsEmpty() ) return FStringView();

	return FStringView( Name.Data, Name.Size );
}
