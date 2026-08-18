// SPDX-License-Identifier: Apache-2.0
#include "DebugTopSnapshot.h"

#include "Debug/DebugTop/DebugTopHUD.h"
#include "Debug/DebugTop/Page/DebugTopEntity.h"

#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace
{
	/** 1 段のインデント。 */
	constexpr const char* kIndent = "  ";

	/**
	 * インデントを積む。
	 *
	 * @param OutText 積む先。
	 * @param Depth 段数。
	 */
	void AppendIndent( FString& OutText, i32 Depth )
	{
		for ( i32 Index = 0; Index < Depth; ++Index ) OutText.Append( kIndent );
	}

	/**
	 * 行 1 つ (と、その子行) を書き出す。
	 *
	 * @details
	 * 畳んでいる行の下も書く。報告に添えるものなので、画面に出ているかどうかとは関係なく
	 * 全部残したい。
	 * @param Element 対象の行。
	 * @param Depth 段数。
	 * @param OutText 書き出す先。
	 */
	void AppendElement( const CDebugTopElement& Element, i32 Depth, FString& OutText )
	{
		// Entity を指す行は、その Entity 側の走査で書くので降りない (二重に出さない)。
		if ( Element.GetLinkedEntity() != nullptr ) return;

		AppendIndent( OutText, Depth );
		OutText.Append( Element.GetDisplayLabel().View() );

		const FString Value = Element.GetValueText();
		if ( !Value.IsEmpty() )
		{
			OutText.Append( " = " );
			OutText.Append( Value.View() );
			if ( !Element.GetUnit().IsEmpty() )
			{
				OutText.Append( " " );
				OutText.Append( Element.GetUnit().View() );
			}
		}

		// 範囲の外に出ている値は、報告を読む側がすぐ気付けるよう印を添える。
		if ( Element.IsValueWarned() ) OutText.Append( "   <- 範囲外" );

		OutText.Append( "\n" );

		const TArray<TSharedPtr<CDebugTopElement>>& Children = Element.GetChildren();
		for ( usize Index = 0; Index < Children.Num(); ++Index )
		{
			if ( !Children[Index] ) continue;

			AppendElement( *Children[Index], Depth + 1, OutText );
		}
	}

	/**
	 * ページ 1 つ (と、その子ページ) を書き出す。
	 *
	 * @param Entity 対象のページ。
	 * @param Depth 段数。
	 * @param OutText 書き出す先。
	 */
	void AppendPage( const ADebugTopEntity& Entity, i32 Depth, FString& OutText )
	{
		AppendIndent( OutText, Depth );
		OutText.Append( "[" );
		OutText.Append( Entity.GetName().View() );
		OutText.Append( "]\n" );

		const TArray<TSharedPtr<CDebugTopElement>>& Elements = Entity.GetElements();
		for ( usize Index = 0; Index < Elements.Num(); ++Index )
		{
			if ( !Elements[Index] ) continue;

			AppendElement( *Elements[Index], Depth + 1, OutText );
		}

		const TArray<TObjectPtr<ADebugTopEntity>>& Children = Entity.GetChildEntities();
		for ( usize Index = 0; Index < Children.Num(); ++Index )
		{
			if ( !Children[Index] ) continue;

			AppendPage( *Children[Index], Depth + 1, OutText );
		}
	}
}


FString DebugTopMakeSnapshotText( const ADebugTopHUD& HUD )
{
	FString Text;

	const TArray<TObjectPtr<ADebugTopEntity>>& Entities = HUD.GetEntities();
	for ( usize Index = 0; Index < Entities.Num(); ++Index )
	{
		if ( !Entities[Index] ) continue;

		AppendPage( *Entities[Index], 0, Text );
	}
	return Text;
}

bool DebugTopCopyToClipboard( const FString& Text )
{
	if ( Text.IsEmpty() ) return false;

	// クリップボードはワイド文字で載せる (日本語のラベルをそのまま貼れるように)。
	const int WideLength = ::MultiByteToWideChar( CP_UTF8, 0, Text.Data(), -1, nullptr, 0 );
	if ( WideLength <= 0 ) return false;

	if ( !::OpenClipboard( nullptr ) ) return false;

	bool bCopied = false;
	if ( ::EmptyClipboard() )
	{
		// 載せた後の所有権はクリップボードへ移るので、成功したら解放しない。
		const HGLOBAL Handle = ::GlobalAlloc( GMEM_MOVEABLE, static_cast<SIZE_T>( WideLength ) * sizeof( wchar_t ) );
		if ( Handle != nullptr )
		{
			wchar_t* const Buffer = static_cast<wchar_t*>( ::GlobalLock( Handle ) );
			if ( Buffer != nullptr )
			{
				::MultiByteToWideChar( CP_UTF8, 0, Text.Data(), -1, Buffer, WideLength );
				::GlobalUnlock( Handle );
				bCopied = ::SetClipboardData( CF_UNICODETEXT, Handle ) != nullptr;
			}
			if ( !bCopied ) ::GlobalFree( Handle );
		}
	}
	::CloseClipboard();
	return bCopied;
}
