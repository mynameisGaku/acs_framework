// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "Debug/HotReload/HotReloadWatchEntry.h"

using namespace acs;
using namespace acs::game;

/**
 * 「どこを見るか」の一覧。
 *
 * @details
 * 監視の仕掛けはエンジン (CHotReloadWatcher) が持っている。ここは**決めた場所を持つだけ**。
 * 溜める側 (Add) と、エンジンへ流し込む側 (ApplyTo) を分けてあるので、起動時に何度足しても
 * 実際に監視が始まるのは ApplyTo を呼んだ 1 か所になる。
 *
 * エンジン側には上限がある (パス 256、フォルダ 64)。溢れた場合は流し込みの戻り値で分かる。
 */
class CHotReloadWatchPlan
{
public:
	/**
	 * フォルダを 1 件足す。
	 *
	 * @param Path フォルダのパス。
	 * @param bRecursive 下の階層まで見るなら true。
	 * @return 足せたら true。
	 */
	bool AddDirectory( const FString& Path, bool bRecursive = true );

	/**
	 * ファイルを 1 件足す。
	 *
	 * @param Path ファイルのパス。
	 * @return 足せたら true。
	 */
	bool AddFile( const FString& Path );

	/**
	 * 枠組みが既定で見る場所を足す。
	 *
	 * @details `Assets` の下。ゲーム固有の置き場所は Add で足す。
	 */
	void AddFrameworkDefaults();

	/**
	 * 決めた場所をエンジンへ流し込む。
	 *
	 * @param Watcher 流し込む先。
	 * @return 実際に登録できた件数。
	 */
	usize ApplyTo( CHotReloadWatcher& Watcher ) const noexcept;

	/** 足した数を返す。 */
	usize Num() const noexcept { return m_Entries.Num(); }

	/**
	 * 足した場所を返す。
	 *
	 * @param Index 0 以上 Num() 未満。
	 * @return 監視する場所。
	 */
	const FHotReloadWatchEntry& Get( usize Index ) const noexcept { return m_Entries[Index]; }

private:
	/** 決めた場所。 */
	TArray<FHotReloadWatchEntry> m_Entries;
};
