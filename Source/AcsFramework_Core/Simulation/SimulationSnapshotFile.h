// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Simulation/SimulationSnapshot.h"

using namespace acs;

/**
 * ある瞬間の様子をファイルへ置き、ファイルから戻す係。
 *
 * @details
 * テープ (`CReplayFile`) と対になる。テープが «どう操作したか» で、こちらが «どうなっていたか»。
 * 2 つ揃うと「その瞬間から、その操作で」続きを再現できる。
 *
 * 置き方は `CAcsArchiveFile` (中身は Engine の `CSaveArchive`) に任せる。
 */
class CSimulationSnapshotFile
{
public:
	/**
	 * 写した様子をファイルへ置く。
	 *
	 * @param Snapshot 置くもの。
	 * @param Path 置き先のパス (UTF-8)。
	 * @return 置けたら true。
	 */
	static bool Save( const CSimulationSnapshot& Snapshot, const FString& Path ) noexcept;

	/**
	 * ファイルから様子を戻す。
	 *
	 * @details 読めなかった場合、中身は空になる。
	 * @param Path 読み元のパス (UTF-8)。
	 * @param OutSnapshot 戻す先。
	 * @return 読めたら true。
	 */
	static bool Load( const FString& Path, CSimulationSnapshot& OutSnapshot ) noexcept;
};
