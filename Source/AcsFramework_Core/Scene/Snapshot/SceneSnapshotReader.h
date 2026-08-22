// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Scene/Snapshot/SceneSnapshotBuffer.h"

using namespace acs;
using namespace acs::game;

/**
 * バイト列からノードの木と名前を起こす係。
 *
 * @details
 * Framework 形式なら内包した ACS バイト列をエンジン (`TryLoadNodeTree`) で起こし、検証済みの
 * 名前表を DFS 先行順で戻す。目印が無い従来の ACS v2/v3/v4 生バイト列も読み込める。
 * **コンポーネントは名前から作り直される** (`CreateComponentByName`) ので、ゲーム固有の
 * コンポーネントは、その名前でエンジンに知られていなければ復元されない。
 */
class CSceneSnapshotReader
{
public:
	/**
	 * バイト列から木を起こす。
	 *
	 * @param Buffer 読み元。
	 * @return エンジンの結果 (Succeeded() で成否、Root に起こした木)。
	 */
	static FSceneLoadResult ReadFrom( const CSceneSnapshotBuffer& Buffer ) noexcept;

	/**
	 * 生のバイト列から木を起こす。
	 *
	 * @param Data 先頭。
	 * @param Size 大きさ。
	 * @return エンジンの結果。
	 */
	static FSceneLoadResult ReadFrom( const u8* Data, usize Size ) noexcept;
};
