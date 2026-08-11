// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/**
 * シーンを跨いで残しておく状態の置き場。
 *
 * @details
 * 置き場そのものはエンジン (CGame の AppState) が持っている。ただし CGame への参照は普通の
 * ゲームコードからは辿れないので、この層で受け取って GetSubsystem<CAppStateSubsystem>()
 * から使えるようにする。
 *
 * **置けるのは 1 つだけ。** 別の型で作り直すと、前に置いてあったものは捨てられる。持ち回りたい
 * ものが複数あるなら、それらをまとめた 1 つの型を作って置くこと。
 *
 * 使い分け:
 *
 * | 置き場 | 生きている間 | 向くもの |
 * |---|---|---|
 * | ここ (AppState) | アプリが終わるまで | 進行状況、所持品、通しの成績 |
 * | CSceneTravelContext | 1 回の遷移だけ | 遷移先へ渡す引数、モーダルの答え |
 * | シーンのメンバ | そのシーンだけ | 画面ごとの一時的なもの |
 */
class CAppStateSubsystem : public ASubsystem
{
public:
	/** サブシステムの型 ID と診断名を実装する。 */
	ACS_SUBSYSTEM_KIND( CAppStateSubsystem )

	/**
	 * 置き場を持っているものを受け取る。
	 *
	 * @details アプリの起動時に 1 度だけ呼ぶ。渡さない間は何を頼まれても何も起きない。
	 * @param Game 置き場を持っているもの。
	 */
	void Bind( CGame& Game ) noexcept { m_Game = &Game; }

	/**
	 * 新しく作って置く (前に置いてあったものは捨てられる)。
	 *
	 * @tparam T 置く型。
	 * @tparam TArgs T のコンストラクタへ渡す引数の型。
	 * @param Arguments T のコンストラクタへ渡す引数。
	 * @return 置いたもの (配線前は nullptr)。
	 */
	template<typename T, typename... TArgs>
	T* Create( TArgs&&... Arguments ) noexcept
	{
		if ( m_Game == nullptr ) return nullptr;

		return &m_Game->EmplaceAppState<T>( Forward<TArgs>( Arguments )... );
	}

	/**
	 * 置いてあるものを返す。
	 *
	 * @details 置いてあるものが別の型なら nullptr。
	 * @tparam T 取り出す型。
	 * @return 置いてあるもの (無ければ nullptr)。
	 */
	template<typename T>
	T* Get() noexcept
	{
		if ( m_Game == nullptr ) return nullptr;

		return m_Game->AppState<T>();
	}

	/**
	 * 置いてあればそれを、無ければ既定で作って返す。
	 *
	 * @details
	 * 「最初に触った所が作る」を書かずに済ませるための口。置いてあるものが別の型なら、
	 * そちらは捨てられて作り直しになる。
	 * @tparam T 取り出す型。
	 * @return 置いてあるもの (配線前は nullptr)。
	 */
	template<typename T>
	T* GetOrCreate() noexcept
	{
		if ( T* const Existing = Get<T>() ) return Existing;

		return Create<T>();
	}

	/**
	 * その型が置いてあるかを返す。
	 *
	 * @tparam T 調べる型。
	 * @return 置いてあれば true。
	 */
	template<typename T>
	bool Has() noexcept { return Get<T>() != nullptr; }

private:
	/** 置き場を持っているもの。所有はしない (アプリが持っている)。 */
	CGame* m_Game = nullptr;
};
