#include "DebugTopSettingsVisitor.h"

#include "Debug/DebugTop/Page/DebugTopEntity.h"
#include "Debug/DebugTop/DebugTopHUD.h"

namespace
{
	/** キーの階層を繋ぐ区切り。 */
	constexpr char kKeySeparator = '/';

	/**
	 * キーを 1 段繋ぐ。
	 *
	 * @param Parent 親までのキー (空なら Segment だけになる)。
	 * @param Segment 足す段。
	 * @return 繋いだキー。
	 */
	FString JoinKey( const FString& Parent, const FString& Segment )
	{
		if ( Parent.IsEmpty() ) return Segment;

		FString Key = Parent;
		Key.Append( kKeySeparator );
		Key.Append( Segment.View() );
		return Key;
	}

	/**
	 * 同じキーが既に使われていれば #2、#3 … を足して重複を避ける。
	 *
	 * @param Key 使いたいキー。
	 * @param UsedKeys これまでに使ったキー (呼び出しごとに追記される)。
	 * @return 実際に使うキー。
	 */
	FString MakeUniqueKey( const FString& Key, TArray<FString>& UsedKeys )
	{
		/** 既に使われているキーかを返す。 */
		const auto IsUsed = [&UsedKeys]( const FString& Candidate ) noexcept
		{
			for ( usize Index = 0; Index < UsedKeys.Num(); ++Index )
			{
				if ( UsedKeys[Index] == Candidate ) return true;
			}
			return false;
		};

		FString Unique = Key;
		for ( i32 Suffix = 2; IsUsed( Unique ); ++Suffix )
		{
			Unique = Key;
			Unique.AppendFormat( "#%d", Suffix );
		}

		UsedKeys.Add( Unique );
		return Unique;
	}

	/**
	 * 行 1 つ (と、その子行) を訪問する。
	 *
	 * @param Element 対象の行。
	 * @param ParentKey 親までのキー。
	 * @param UsedKeys 重複を避けるために使ったキーの控え。
	 * @param Visitor 値を持つ行を受け取る訪問者。
	 */
	void VisitElement( CDebugTopElement& Element, const FString& ParentKey, TArray<FString>& UsedKeys, IDebugTopSettingsVisitor& Visitor )
	{
		// Entity を指す行 (遷移行・インライン展開行) は、その Entity 側の走査で拾うので降りない。
		if ( Element.GetLinkedEntity() != nullptr ) return;

		// 絶対キーが指定されていればメニュー内の位置に依存させない。
		const bool bAbsolute = !Element.GetSaveKey().IsEmpty();
		const FString BaseKey = bAbsolute
			? Element.GetSaveKey()
			: JoinKey( ParentKey, Element.GetLabel() );
		const FString Key = MakeUniqueKey( BaseKey, UsedKeys );

		// 値を持たない行にもピン留めは付くので、種別に関わらず 1 度は渡す。
		if ( Element.IsSaveable() ) Visitor.OnRow( Key, Element );

		switch ( Element.IsSaveable() ? Element.GetValueKind() : EDebugTopValueKind::None )
		{
		case EDebugTopValueKind::Int:
		case EDebugTopValueKind::Float:
		case EDebugTopValueKind::Bool:
		case EDebugTopValueKind::Enum:
		case EDebugTopValueKind::String:
			Visitor.OnValue( Key, Element );
			break;

		default:
			// 値を持たない行 (カテゴリ行・Action 行) と、要素ごとに持つ配列行そのものは対象外。
			break;
		}

		const TArray<TSharedPtr<CDebugTopElement>>& Children = Element.GetChildren();
		for ( usize Index = 0; Index < Children.Num(); ++Index )
		{
			if ( !Children[Index] ) continue;

			VisitElement( *Children[Index], Key, UsedKeys, Visitor );
		}
	}

	/**
	 * ページ 1 つ (と、その子ページ) を訪問する。
	 *
	 * @param Entity 対象のページ。
	 * @param ParentKey 親までのキー。
	 * @param UsedKeys 重複を避けるために使ったキーの控え。
	 * @param Visitor 値を持つ行を受け取る訪問者。
	 */
	void VisitEntity( const ADebugTopEntity& Entity, const FString& ParentKey, TArray<FString>& UsedKeys, IDebugTopSettingsVisitor& Visitor )
	{
		const FString EntityKey = JoinKey( ParentKey, Entity.GetName() );

		const TArray<TSharedPtr<CDebugTopElement>>& Elements = Entity.GetElements();
		for ( usize Index = 0; Index < Elements.Num(); ++Index )
		{
			if ( !Elements[Index] ) continue;

			VisitElement( *Elements[Index], EntityKey, UsedKeys, Visitor );
		}

		const TArray<TObjectPtr<ADebugTopEntity>>& Children = Entity.GetChildEntities();
		for ( usize Index = 0; Index < Children.Num(); ++Index )
		{
			if ( !Children[Index] ) continue;

			VisitEntity( *Children[Index], EntityKey, UsedKeys, Visitor );
		}
	}
}


void DebugTopVisitSettings( const ADebugTopHUD& HUD, IDebugTopSettingsVisitor& Visitor )
{
	TArray<FString> UsedKeys;

	const TArray<TObjectPtr<ADebugTopEntity>>& Entities = HUD.GetEntities();
	for ( usize Index = 0; Index < Entities.Num(); ++Index )
	{
		if ( !Entities[Index] ) continue;

		VisitEntity( *Entities[Index], FString(), UsedKeys, Visitor );
	}
}
