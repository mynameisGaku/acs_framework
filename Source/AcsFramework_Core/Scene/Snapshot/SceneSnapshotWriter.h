// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Scene/Snapshot/SceneSnapshotBuffer.h"

using namespace acs;
using namespace acs::game;

/**
 * ノードの木と名前をバイト列へ落とす係。
 *
 * @details
 * 親子関係、変換、描画状態、コンポーネントはエンジン (`TrySaveNodeTree`) に任せ、その
 * バイト列を変更せず Framework 形式へ内包する。ACS 形式 v4 に無いノード名は、同じ DFS
 * 先行順の名前表として後ろへ添える。
 */
class CSceneSnapshotWriter
{
public:
	/**
	 * 木をバイト列へ落とす。
	 *
	 * @details ACS が測った大きさと名前表の大きさを合わせ、使い回す入れ物を必要分だけ広げる。
	 * @param Root 起点のノード。
	 * @param Buffer 落とす先。
	 * @return 保存結果 (Succeeded() で成否、BytesWritten で Framework 形式全体の大きさ)。
	 */
	static FSceneSaveResult WriteTo( const ANode& Root, CSceneSnapshotBuffer& Buffer ) noexcept;
};
