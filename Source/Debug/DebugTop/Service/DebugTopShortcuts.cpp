#include "DebugTopShortcuts.h"

#include "Debug/DebugTop/DebugTopHUD.h"
#include "Debug/DebugTop/Page/DebugTopEntity.h"

namespace
{
	/**
	 * 行とその子行から、押されたキーに割り当たっているものを実行する。
	 *
	 * @details 畳んでいる行の下も見る。ショートカットは「辿らずに叩く」ためのものなので、
	 * 開いているかどうかで効き方が変わってはいけない。
	 * @param Element 探す起点の行。
	 * @param bOutRan 1 つでも実行したら true を書き込む。
	 */
	void RunInElement( CDebugTopElement& Element, bool& bOutRan )
	{
		const EKey Shortcut = Element.GetShortcut();
		if ( Shortcut != EKey::Unknown && CInput::IsKeyPressed( Shortcut ) )
		{
			Element.OnDecide();
			bOutRan = true;
		}

		const TArray<TSharedPtr<CDebugTopElement>>& Children = Element.GetChildren();
		for ( usize Index = 0; Index < Children.Num(); ++Index )
		{
			if ( !Children[Index] ) continue;

			RunInElement( *Children[Index], bOutRan );
		}
	}

	/**
	 * ページとその子ページを辿って実行する。
	 *
	 * @param Entity 探す起点のページ。
	 * @param bOutRan 1 つでも実行したら true を書き込む。
	 */
	void RunInPage( ADebugTopEntity& Entity, bool& bOutRan )
	{
		const TArray<TSharedPtr<CDebugTopElement>>& Elements = Entity.GetElements();
		for ( usize Index = 0; Index < Elements.Num(); ++Index )
		{
			if ( !Elements[Index] ) continue;

			RunInElement( *Elements[Index], bOutRan );
		}

		const TArray<TObjectPtr<ADebugTopEntity>>& Children = Entity.GetChildEntities();
		for ( usize Index = 0; Index < Children.Num(); ++Index )
		{
			if ( !Children[Index] ) continue;

			RunInPage( *Children[Index], bOutRan );
		}
	}
}


bool DebugTopRunShortcuts( ADebugTopHUD& HUD )
{
	bool bRan = false;

	const TArray<TObjectPtr<ADebugTopEntity>>& Entities = HUD.GetEntities();
	for ( usize Index = 0; Index < Entities.Num(); ++Index )
	{
		if ( !Entities[Index] ) continue;

		RunInPage( *Entities[Index], bRan );
	}
	return bRan;
}
