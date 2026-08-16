// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Scene/Snapshot/SceneSnapshotBuffer.h"

using namespace acs;
using namespace acs::game;

/**
 * バイト列からノードの木を起こす係。
 *
 * @details
 * 起こすのはエンジン (`TryLoadNodeTree`) が行う。ここは入れ物から先頭と大きさを取り出して
 * 渡すだけ。**コンポーネントは名前から作り直される** (`CreateComponentByName`) ので、
 * ゲーム固有のコンポーネントは、その名前でエンジンに知られていなければ復元されない。
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
