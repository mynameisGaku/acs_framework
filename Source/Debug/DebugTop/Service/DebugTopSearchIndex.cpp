// SPDX-License-Identifier: Apache-2.0
#include "DebugTopSearchIndex.h"

#include "Debug/DebugTop/Page/DebugTopEntity.h"
#include "Debug/DebugTop/DebugTopHUD.h"


const char* FDebugTopSearchHit::GetLabelText() const noexcept
{
	return Element != nullptr ? Element->GetLabel().Data() : "";
}

const char* FDebugTopSearchHit::GetPageText() const noexcept
{
	return Page != nullptr ? Page->GetName().Data() : "";
}


namespace
{
	/**
	 * ASCII の大文字を小文字へ落とす。
	 *
	 * @param Character 変換する文字。
	 * @return 小文字化した文字 (ASCII 以外はそのまま)。
	 */
	char ToLowerAscii( char Character ) noexcept
	{
		return ( Character >= 'A' && Character <= 'Z' )
			? static_cast<char>( Character + ( 'a' - 'A' ) )
			: Character;
	}

	/**
	 * 部分一致を調べる (英字は大文字小文字を区別しない)。
	 *
	 * @param Haystack 探される側。
	 * @param Needle 探す側 (空なら常に一致)。
	 * @return 含んでいれば true。
	 */
	bool ContainsIgnoreCaseImpl( const FString& Haystack, const FString& Needle ) noexcept
	{
		if ( Needle.IsEmpty() ) return true;
		if ( Needle.Size() > Haystack.Size() ) return false;

		const usize Last = Haystack.Size() - Needle.Size();
		for ( usize Start = 0; Start <= Last; ++Start )
		{
			usize Offset = 0;
			while ( Offset < Needle.Size() && ToLowerAscii( Haystack[Start + Offset] ) == ToLowerAscii( Needle[Offset] ) )
			{
				++Offset;
			}
			if ( Offset == Needle.Size() ) return true;
		}
		return false;
	}

	/**
	 * 行とその子孫から、検索語に一致するものを集める。
	 *
	 * @param Element 探す起点の行。
	 * @param Query 検索語。
	 * @param Page この行が属するページ。
	 * @param MaxHits 返す件数の上限。
	 * @param OutHits 見つかった行を積む先。
	 */
	void CollectFromElement( CDebugTopElement& Element, const FString& Query, ADebugTopEntity& Page, usize MaxHits, TArray<FDebugTopSearchHit>& OutHits )
	{
		if ( OutHits.Num() >= MaxHits ) return;

		// Entity を指す行の子は別ページの持ち物なので、そのページ側の走査で拾う。
		if ( Element.GetLinkedEntity() != nullptr ) return;

		if ( ContainsIgnoreCaseImpl( Element.GetLabel(), Query ) )
		{
			FDebugTopSearchHit Hit;
			Hit.Element = &Element;
			Hit.Page = &Page;
			OutHits.Add( Hit );
		}

		const TArray<TSharedPtr<CDebugTopElement>>& Children = Element.GetChildren();
		for ( usize Index = 0; Index < Children.Num(); ++Index )
		{
			if ( !Children[Index] ) continue;

			CollectFromElement( *Children[Index], Query, Page, MaxHits, OutHits );
		}
	}

	/**
	 * ページとその子ページから、検索語に一致する行を集める。
	 *
	 * @param Entity 探す起点のページ。
	 * @param Query 検索語。
	 * @param MaxHits 返す件数の上限。
	 * @param OutHits 見つかった行を積む先。
	 */
	void CollectFromPage( ADebugTopEntity& Entity, const FString& Query, usize MaxHits, TArray<FDebugTopSearchHit>& OutHits )
	{
		if ( OutHits.Num() >= MaxHits ) return;

		const TArray<TSharedPtr<CDebugTopElement>>& Elements = Entity.GetElements();
		for ( usize Index = 0; Index < Elements.Num(); ++Index )
		{
			if ( !Elements[Index] ) continue;

			CollectFromElement( *Elements[Index], Query, Entity, MaxHits, OutHits );
		}

		const TArray<TObjectPtr<ADebugTopEntity>>& Children = Entity.GetChildEntities();
		for ( usize Index = 0; Index < Children.Num(); ++Index )
		{
			if ( !Children[Index] ) continue;

			CollectFromPage( *Children[Index], Query, MaxHits, OutHits );
		}
	}
}


bool DebugTopContainsIgnoreCase( const FString& Haystack, const FString& Needle ) noexcept
{
	return ContainsIgnoreCaseImpl( Haystack, Needle );
}

void DebugTopCollectMatches( const ADebugTopHUD& HUD, const FString& Query, usize MaxHits, TArray<FDebugTopSearchHit>& OutHits )
{
	if ( Query.IsEmpty() ) return;

	const TArray<TObjectPtr<ADebugTopEntity>>& Entities = HUD.GetEntities();
	for ( usize Index = 0; Index < Entities.Num(); ++Index )
	{
		if ( !Entities[Index] ) continue;

		CollectFromPage( *Entities[Index], Query, MaxHits, OutHits );
	}
}
