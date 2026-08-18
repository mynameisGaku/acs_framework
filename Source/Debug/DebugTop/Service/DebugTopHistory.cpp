// SPDX-License-Identifier: Apache-2.0
#include "DebugTopHistory.h"

#include "Debug/DebugTop/DebugTopHUD.h"
#include "Debug/DebugTop/Element/DebugTopElement.h"
#include "Debug/DebugTop/Page/DebugTopEntity.h"

namespace
{
	/** 変更が途切れたとみなすまでの間 (秒)。これより間が空いたら、次の変更は別の手にする。 */
	constexpr f32 kMergeIdleSeconds = 0.5f;

	/** 控えておく手の数の上限。古い方から捨てる。 */
	constexpr usize kMaxEntries = 64;

	/**
	 * 行 1 つ (と、その子行) を集める。
	 *
	 * @details
	 * 保存の走査 (DebugTopVisitSettings) と似ているが別物。あちらは保存キーを組み立て、
	 * 保存対象の行だけを渡す。取り消しは保存しない行も戻せないと困るので、こちらは全ての
	 * 行をそのまま集める。
	 * @param Element 対象の行。
	 * @param OutElements 集めた行の書き込み先。
	 */
	void CollectElement( CDebugTopElement& Element, TArray<CDebugTopElement*>& OutElements )
	{
		// Entity を指す行 (遷移行・インライン展開行) は、その Entity 側の走査で拾うので降りない。
		if ( Element.GetLinkedEntity() != nullptr ) return;

		OutElements.Add( &Element );

		const TArray<TSharedPtr<CDebugTopElement>>& Children = Element.GetChildren();
		for ( usize Index = 0; Index < Children.Num(); ++Index )
		{
			if ( !Children[Index] ) continue;

			CollectElement( *Children[Index], OutElements );
		}
	}

	/**
	 * ページ 1 つ (と、その子ページ) の行を集める。
	 *
	 * @param Entity 対象のページ。
	 * @param OutElements 集めた行の書き込み先。
	 */
	void CollectEntity( const ADebugTopEntity& Entity, TArray<CDebugTopElement*>& OutElements )
	{
		const TArray<TSharedPtr<CDebugTopElement>>& Elements = Entity.GetElements();
		for ( usize Index = 0; Index < Elements.Num(); ++Index )
		{
			if ( !Elements[Index] ) continue;

			CollectElement( *Elements[Index], OutElements );
		}

		const TArray<TObjectPtr<ADebugTopEntity>>& Children = Entity.GetChildEntities();
		for ( usize Index = 0; Index < Children.Num(); ++Index )
		{
			if ( !Children[Index] ) continue;

			CollectEntity( *Children[Index], OutElements );
		}
	}

	/**
	 * メニュー全体の行を集める。
	 *
	 * @param HUD 対象のメニュー。
	 * @param OutElements 集めた行の書き込み先。
	 */
	void CollectAll( const ADebugTopHUD& HUD, TArray<CDebugTopElement*>& OutElements )
	{
		const TArray<TObjectPtr<ADebugTopEntity>>& Entities = HUD.GetEntities();
		for ( usize Index = 0; Index < Entities.Num(); ++Index )
		{
			if ( !Entities[Index] ) continue;

			CollectEntity( *Entities[Index], OutElements );
		}
	}
}


CDebugTopHistory::~CDebugTopHistory() noexcept
{
	// 差しっぱなしにすると、解体後の自分へ通知が来る。
	CDebugTopElement::SetChangeListener( TDelegate<void( CDebugTopElement& )>() );
}


void CDebugTopHistory::Begin( const ADebugTopHUD& HUD )
{
	m_HUD = &HUD;
	Clear();

	// 変更の通知は「変わった後」に来るので、最初の 1 手の変更前の値をここで控えておく。
	TArray<CDebugTopElement*> Elements;
	CollectAll( HUD, Elements );
	for ( usize Index = 0; Index < Elements.Num(); ++Index )
	{
		FDebugTopElementValue Value;
		if ( !DebugTopCaptureValue( *Elements[Index], Value ) ) continue;

		m_Current.Add( Elements[Index], Move( Value ) );
	}

	CDebugTopElement::SetChangeListener( TDelegate<void( CDebugTopElement& )>::CreateRaw<&CDebugTopHistory::OnElementChanged>( this ) );
}


void CDebugTopHistory::Update( f32 DeltaSeconds ) noexcept
{
	if ( m_MergeElement == nullptr ) return;

	m_IdleSeconds += DeltaSeconds;
	if ( m_IdleSeconds < kMergeIdleSeconds ) return;

	// 手が止まった。次の変更は別の手として積む。
	m_MergeElement = nullptr;
}


void CDebugTopHistory::OnElementChanged( CDebugTopElement& Element )
{
	// 自分が書き戻したぶんを控えると、取り消しが取り消しを生んで戻れなくなる。
	if ( m_bApplying ) return;

	FDebugTopElementValue After;
	if ( !DebugTopCaptureValue( Element, After ) ) return;

	FDebugTopElementValue* const Known = m_Current.Find( &Element );
	if ( Known == nullptr )
	{
		// 控え始めた後に足された行。変更前の値が分からないので、いまの値から控え直す。
		m_Current.Add( &Element, Move( After ) );
		return;
	}

	if ( DebugTopValuesEqual( *Known, After ) ) return;

	// 長押しで送っている最中は 1 つの手として畳む。変更前の値は最初のものを残し、
	// 変更後の値だけを更新する。
	const bool bMerge = m_MergeElement == &Element && m_Undo.Num() > 0;
	if ( bMerge )
	{
		m_Undo[m_Undo.Num() - 1].After = After;
	}
	else
	{
		FEntry Entry;
		Entry.Element = &Element;
		Entry.Before = *Known;
		Entry.After = After;
		Entry.Label = Element.GetLabel();
		m_Undo.Add( Move( Entry ) );

		// 古い手から捨てる (先頭を 1 つ落とす)。
		while ( m_Undo.Num() > kMaxEntries ) m_Undo.RemoveAt( 0 );

		// 新しく手を打ったら、やり直せる先は無くなる。
		m_Redo.Reset();
	}

	*Known = Move( After );
	m_MergeElement = &Element;
	m_IdleSeconds = 0.0f;
}


bool CDebugTopHistory::Undo( FString& OutLabel )
{
	return Step( m_Undo, m_Redo, true, OutLabel );
}


bool CDebugTopHistory::Redo( FString& OutLabel )
{
	return Step( m_Redo, m_Undo, false, OutLabel );
}


bool CDebugTopHistory::Step( TArray<FEntry>& From, TArray<FEntry>& To, bool bUseBefore, FString& OutLabel )
{
	while ( From.Num() > 0 )
	{
		FEntry Entry = Move( From[From.Num() - 1] );
		From.Pop();

		// 行が消えていたら、その手は捨てて次の手へ移る。
		if ( Entry.Element == nullptr || !IsAlive( *Entry.Element ) ) continue;

		const FDebugTopElementValue& Target = bUseBefore ? Entry.Before : Entry.After;

		m_bApplying = true;
		const bool bApplied = DebugTopApplyValue( *Entry.Element, Target );
		m_bApplying = false;
		if ( !bApplied ) continue;

		// 書き戻したぶんは通知を止めてあるので、控えを手で合わせる。
		if ( FDebugTopElementValue* const Known = m_Current.Find( Entry.Element ) ) *Known = Target;

		OutLabel = Entry.Label;
		To.Add( Move( Entry ) );

		// 戻した直後の変更は、戻した手と畳まない。
		m_MergeElement = nullptr;
		return true;
	}

	return false;
}


bool CDebugTopHistory::IsAlive( const CDebugTopElement& Element ) const noexcept
{
	if ( m_HUD == nullptr ) return false;

	TArray<CDebugTopElement*> Elements;
	CollectAll( *m_HUD, Elements );
	for ( usize Index = 0; Index < Elements.Num(); ++Index )
	{
		if ( Elements[Index] == &Element ) return true;
	}
	return false;
}


void CDebugTopHistory::Clear() noexcept
{
	m_Undo.Reset();
	m_Redo.Reset();
	m_MergeElement = nullptr;
	m_IdleSeconds = 0.0f;
}
