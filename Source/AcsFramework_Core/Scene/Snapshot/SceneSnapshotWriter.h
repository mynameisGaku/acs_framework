// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Scene/Snapshot/SceneSnapshotBuffer.h"

using namespace acs;
using namespace acs::game;

/**
 * ノードの木をバイト列へ落とす係。
 *
 * @details
 * 落とすのはエンジン (`TrySaveNodeTree`) が行う。ここが引き受けるのは、**必要な大きさが
 * 事前に分からない**という一点。エンジンは足りないときに «あと何バイト要るか» を返すので、
 * 受けて広げてもう一度呼ぶ。この繰り返しを呼ぶ側に書かせない。
 */
class CSceneSnapshotWriter
{
public:
	/**
	 * 木をバイト列へ落とす。
	 *
	 * @details 入れ物が足りなければ、必要な大きさまで広げて一度だけやり直す。
	 * @param Root 起点のノード。
	 * @param Buffer 落とす先。
	 * @return エンジンの結果 (Succeeded() で成否、BytesWritten で書けた大きさ)。
	 */
	static FSceneSaveResult WriteTo( const ANode& Root, CSceneSnapshotBuffer& Buffer ) noexcept;

private:
	/**
	 * 一度だけ落としてみる。
	 *
	 * @param Root 起点のノード。
	 * @param Buffer 落とす先。
	 * @return エンジンの結果。
	 */
	static FSceneSaveResult TryWriteOnce( const ANode& Root, CSceneSnapshotBuffer& Buffer ) noexcept;
};
