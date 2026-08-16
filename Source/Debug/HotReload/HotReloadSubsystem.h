// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "Debug/HotReload/HotReloadDispatcher.h"
#include "Debug/HotReload/HotReloadWatchPlan.h"
#include "Debug/HotReload/IHotReloadHandler.h"
#include "Debug/HotReload/WatcherEventSource.h"

using namespace acs;
using namespace acs::game;

/**
 * ファイルの差し替えを見張って、引き受け手へ伝えるサブシステム。
 *
 * @details
 * 見張る仕組みはエンジン (CHotReloadWatcher) が持っている。ただし**誰も持っておらず、
 * どこを見るかも決まっていない**ので、ゲームごとに次を書くことになる。ここが引き受ける。
 *
 * 1. 見張る実体を持ち、起動と終了を通す
 * 2. 見る場所 (CHotReloadWatchPlan) を流し込む
 * 3. 毎フレーム進めて、溜まった変更を引き受け手へ配る
 *
 * **何を作り直すかは持たない。** それはアセットを持っている側にしか分からないので、
 * IHotReloadHandler を実装した側が引き受ける。
 *
 * **実時間で進める。** ポーズ中でも差し替えは拾いたい (止めて眺めながら絵を差し替える、
 * という使い方が普通のため)。
 *
 * @code
 * HotReload->StartWatchingDefaults();
 * HotReload->AddHandler( MakeUnique<CHotReloadLogHandler>() );
 * @endcode
 */
class CHotReloadSubsystem : public ASubsystem
{
public:
	/** サブシステムの型 ID と診断名を実装する。 */
	ACS_SUBSYSTEM_KIND( CHotReloadSubsystem )

	/** 見張りと、そこから取り出す橋渡しを繋いで構築する。 */
	CHotReloadSubsystem() noexcept;

	/** 見張るのをやめ、エンジン側を畳む。 */
	~CHotReloadSubsystem() noexcept override;

	/** subsystem 終了時に見張りを畳む。 */
	void OnDeinitialize() noexcept override;

	/**
	 * 枠組みが既定で見る場所を見張り始める。
	 *
	 * @details アプリの起動時に 1 度だけ呼ぶ。
	 * @param DebounceSeconds 続けて変わったときにまとめる秒数。
	 * @return 見張り始められたら true。
	 */
	bool StartWatchingDefaults( f32 DebounceSeconds = 0.3f ) noexcept;

	/**
	 * 見る場所を決めて見張り始める。
	 *
	 * @param Plan 見る場所の一覧。
	 * @param DebounceSeconds 続けて変わったときにまとめる秒数。
	 * @return 見張り始められたら true。
	 */
	bool StartWatching( const CHotReloadWatchPlan& Plan, f32 DebounceSeconds = 0.3f ) noexcept;

	/**
	 * 引き受け手を足す。
	 *
	 * @details 渡したものの寿命はここが持つ。
	 * @param Handler 引き受け手。
	 * @return 足せたら true。
	 */
	bool AddHandler( TUniquePtr<IHotReloadHandler> Handler ) noexcept;

	/**
	 * 1 フレーム進める。
	 *
	 * @details
	 * 渡すのは実時間の経過秒。ゲームを止めていても差し替えは拾う。
	 * @param UnscaledDeltaSeconds 前フレームからの実経過秒。
	 */
	void Update( f32 UnscaledDeltaSeconds ) noexcept;

	/** 見張っているかを返す。 */
	bool IsWatching() const noexcept { return m_bWatching; }

	/** 見張っている場所の数を返す。 */
	u32 GetWatchedCount() const noexcept;

	/** まだ配っていない変更の数を返す。 */
	u32 GetPendingCount() const noexcept;

	/** これまでに配った件数を返す。 */
	u64 GetDispatchedCount() const noexcept { return m_Dispatcher.GetDispatchedCount(); }

	/** 引き受け手が居らず捨てた件数を返す。 */
	u64 GetUnhandledCount() const noexcept { return m_Dispatcher.GetUnhandledCount(); }

private:
	/**
	 * 見張りを起こして場所を流し込む。
	 *
	 * @param Plan 見る場所の一覧。
	 * @param DebounceSeconds まとめる秒数。
	 * @return 1 件でも見張れたら true。
	 */
	bool BeginWatch( const CHotReloadWatchPlan& Plan, f32 DebounceSeconds ) noexcept;

	/** 見張る実体。 */
	CHotReloadWatcher m_Watcher;

	/** 決めた見る場所。 */
	CHotReloadWatchPlan m_Plan;

	/** 見張りから取り出す橋渡し。 */
	CWatcherEventSource m_EventSource;

	/** 配る係。 */
	CHotReloadDispatcher m_Dispatcher;

	/** 引き受け手。寿命をここで持つ。 */
	TArray<TUniquePtr<IHotReloadHandler>> m_Handlers;

	/** 見張っているか。 */
	bool m_bWatching = false;
};
