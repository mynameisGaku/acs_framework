// SPDX-License-Identifier: Apache-2.0
#include "DebugTopFavorites.h"

#include "Debug/DebugTop/DebugTopHUD.h"

namespace
{
	/** 見出しの色。 */
	constexpr FVec4 kHeaderColor{ 0.98f, 0.85f, 0.35f, 1.0f };

	/**
	 * 行とその子孫から、ピン留めされたものを集める。
	 *
	 * @param Element 探す起点の行。
	 * @param Page この行が属するページ。
	 * @param Self 集めている側のページ (自分自身は拾わない)。
	 * @param OutRows 見つかった行と、そのページの組を積む先。
	 */
	void CollectFromElement( const CDebugTopElement& Element, ADebugTopEntity& Page, const ADebugTopEntity* Self, TArray<TPair<const CDebugTopElement*, ADebugTopEntity*>>& OutRows )
	{
		// Entity を指す行の子は別ページの持ち物なので、そのページ側の走査で拾う。
		if ( Element.GetLinkedEntity() != nullptr ) return;

		if ( Element.IsFavorite() && &Page != Self )
		{
			OutRows.Add( TPair<const CDebugTopElement*, ADebugTopEntity*>{ &Element, &Page } );
		}

		const TArray<TSharedPtr<CDebugTopElement>>& Children = Element.GetChildren();
		for ( usize Index = 0; Index < Children.Num(); ++Index )
		{
			if ( !Children[Index] ) continue;

			CollectFromElement( *Children[Index], Page, Self, OutRows );
		}
	}

	/**
	 * ページとその子ページから、ピン留めされた行を集める。
	 *
	 * @param Entity 探す起点のページ。
	 * @param Self 集めている側のページ。
	 * @param OutRows 見つかった行と、そのページの組を積む先。
	 */
	void CollectFromPage( ADebugTopEntity& Entity, const ADebugTopEntity* Self, TArray<TPair<const CDebugTopElement*, ADebugTopEntity*>>& OutRows )
	{
		const TArray<TSharedPtr<CDebugTopElement>>& Elements = Entity.GetElements();
		for ( usize Index = 0; Index < Elements.Num(); ++Index )
		{
			if ( !Elements[Index] ) continue;

			CollectFromElement( *Elements[Index], Entity, Self, OutRows );
		}

		const TArray<TObjectPtr<ADebugTopEntity>>& Children = Entity.GetChildEntities();
		for ( usize Index = 0; Index < Children.Num(); ++Index )
		{
			if ( !Children[Index] ) continue;

			CollectFromPage( *Children[Index], Self, OutRows );
		}
	}
}


ADebugTopFavoritesEntity::ADebugTopFavoritesEntity( const FString& Name, ADebugTopHUD& HUD )
	: ADebugTopEntity( Name )
	, m_HUD( &HUD )
{
}

void ADebugTopFavoritesEntity::OnBuild() noexcept
{
	SetDescription( FString( "各行の上で F を押すとピン留め / 解除\n" "決定するとその行があるページへ移動します" ) );
	RebuildResults();
}

void ADebugTopFavoritesEntity::Update( f32 DeltaSeconds ) noexcept
{
	// どこかで留め外しがあったときだけ組み直す (毎フレーム作り直すとカーソルが戻ってしまう)。
	if ( m_bDirty || m_Version != CDebugTopElement::GetFavoriteVersion() ) RebuildResults();

	ADebugTopEntity::Update( DeltaSeconds );
}

void ADebugTopFavoritesEntity::RebuildResults()
{
	m_bDirty = false;
	m_Version = CDebugTopElement::GetFavoriteVersion();
	ClearElements();

	SetHeader( FString( "お気に入り" ) );
	SetHeaderColor( kHeaderColor );

	if ( m_HUD == nullptr ) return;

	TArray<TPair<const CDebugTopElement*, ADebugTopEntity*>> Rows;
	const TArray<TObjectPtr<ADebugTopEntity>>& Entities = m_HUD->GetEntities();
	for ( usize Index = 0; Index < Entities.Num(); ++Index )
	{
		if ( !Entities[Index] ) continue;

		CollectFromPage( *Entities[Index], this, Rows );
	}

	if ( Rows.IsEmpty() )
	{
		Add<CDebugTopElement>( "(まだ何も留めていません)", "各行の上で F" );
		return;
	}

	for ( usize Index = 0; Index < Rows.Num(); ++Index )
	{
		const CDebugTopElement* const Found = Rows[Index].first;
		ADebugTopEntity* const Page = Rows[Index].second;
		if ( Found == nullptr || Page == nullptr ) continue;

		// 決定するとそのページへ飛び、選んだ行にカーソルが合う。
		Add<CDebugTopElementEntityLink>( Found->GetLabel(), Page->GetName(), *this, *Page, Found );
	}
}
