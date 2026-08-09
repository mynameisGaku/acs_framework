// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Event/EventSubscription.h"

using namespace acs;


/**
 * 型で配る知らせを、どこからでもやり取りできるようにするサブシステム。
 *
 * @details
 * 仕組みそのものはエンジン (CMessageBroker) が持っている。ただしアプリへの参照は普通の
 * ゲームコードからは辿れないので、この層で受け取って GetSubsystem<CEventSubsystem>() から
 * 使えるようにする。
 *
 * 素通しにせず 2 つ足してある。
 *
 * 1. **外し忘れが起きない購読** (FEventSubscription)。持ち主が死ねば購読も外れる。
 * 2. **メンバ関数をそのまま渡せる形。** エンジンの口は void(const E&, void*) を要求するが、
 *    受ける側にとって void* は余分なので、こちらで畳んで渡す。
 *
 * 知らせの型は「ただのデータ」であればよい (継承も登録も要らない)。型そのものが宛先になる。
 *
 * @code
 * // 知らせの型
 * struct FScoreChanged { i32 Score = 0; };
 *
 * // 受け取る側 (控えをメンバに持つ)
 * void AMyHud::OnEnter()
 * {
 *     m_ScoreSub = GetSubsystem<CEventSubsystem>()->Subscribe<FScoreChanged, &AMyHud::OnScoreChanged>( this );
 * }
 * void AMyHud::OnScoreChanged( const FScoreChanged& Event ) { m_Score = Event.Score; }
 *
 * // 出す側 (受け取る相手を知らない)
 * Events->Publish( FScoreChanged{ 1200 } );
 * @endcode
 */
class CEventSubsystem : public ASubsystem
{
public:
	/** サブシステムの型 ID と診断名を実装する。 */
	ACS_SUBSYSTEM_KIND( CEventSubsystem )

	/**
	 * 知らせを配る仕組みを持っているものを受け取る。
	 *
	 * @details アプリの起動時に 1 度だけ呼ぶ。渡さない間は何を頼まれても何も起きない。
	 * @param Application 仕組みを持っているもの。
	 */
	void Bind( CApplication& Application ) noexcept { m_Application = &Application; }

	/**
	 * メンバ関数で購読する。
	 *
	 * @details
	 * 受ける関数は void( const E& ) の形。返ってきた控えを持っている間だけ届く。
	 * 捨てると自動で外れるので、持ち主のメンバとして持つこと。
	 * @tparam E 受け取る知らせの型。
	 * @tparam Method 受ける関数 (&AMyClass::OnSomething の形で渡す)。
	 * @tparam TOwner 受ける関数を持つ型。
	 * @param Instance 受ける実体。
	 * @return 購読の控え (配線できなければ空の控え)。
	 */
	template<typename E, auto Method, typename TOwner>
	FEventSubscription Subscribe( TOwner* Instance ) noexcept
	{
		if ( m_Application == nullptr || Instance == nullptr ) return FEventSubscription();

		return FEventSubscription( *this, m_Application->GetEvents().Subscribe<E>( &MethodThunk<E, Method, TOwner>, Instance ) );
	}

	/**
	 * 知らせを配る。
	 *
	 * @details 受け取る相手が 1 つも無くても構わない (その場合は何も起きない)。
	 * @tparam E 配る知らせの型。
	 * @param Payload 配る中身。
	 */
	template<typename E>
	void Publish( const E& Payload ) noexcept
	{
		if ( m_Application == nullptr ) return;

		m_Application->GetEvents().Publish( Payload );
	}

	/**
	 * その型を受け取る相手の数を返す。
	 *
	 * @tparam E 数える知らせの型。
	 * @return 受け取る相手の数。
	 */
	template<typename E>
	u32 GetSubscriberCount() const noexcept
	{
		if ( m_Application == nullptr ) return 0;

		return m_Application->GetEvents().SubscriberCount( GetEventTypeId<E>() );
	}

	/**
	 * 控えを使わずに外す。
	 *
	 * @details FEventSubscription が自分を畳むときに呼ぶ。普通は直接呼ばない。
	 * @param Handle 外す控え。
	 * @return 外せたら true。
	 */
	bool Unsubscribe( FSubscriptionHandle Handle ) noexcept;

private:
	/**
	 * void* を畳んでメンバ関数へ渡す中継。
	 *
	 * @param Payload 配られた中身。
	 * @param User 受ける実体。
	 */
	template<typename E, auto Method, typename TOwner>
	static void MethodThunk( const void* Payload, void* User ) noexcept
	{
		( static_cast<TOwner*>( User )->*Method )( *static_cast<const E*>( Payload ) );
	}

	/** 知らせを配る仕組みを持っているもの。所有はしない。 */
	CApplication* m_Application = nullptr;
};
