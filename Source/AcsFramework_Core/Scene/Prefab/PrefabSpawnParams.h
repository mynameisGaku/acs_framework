// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::game;

/**
 * 出したものへ、出した直後に施す指定。
 *
 * @details
 * 作り方 (PrefabFactoryFn) は「どういう見た目・部品か」を決める。ここが持つのは
 * 「どこへ、どんな向きで、どんな名前で置くか」という**置き方**だけ。
 *
 * 何も指定しなければ、作り方が返したものへ手を加えない。
 */
struct FPrefabSpawnParams
{
	/** 置く位置・向き・大きさ。bApplyTransform が true のときだけ使う。 */
	FTransform3D LocalTransform;

	/** 付ける名前。空なら作り方が付けた名前のまま。 */
	FString Name;

	/** LocalTransform を施すなら true。 */
	bool bApplyTransform = false;

	/** 出した直後の有効・無効。 */
	bool bEnabled = true;

	/** bEnabled を施すなら true。 */
	bool bApplyEnabled = false;
};
