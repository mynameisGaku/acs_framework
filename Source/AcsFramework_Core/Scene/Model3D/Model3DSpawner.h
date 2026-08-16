// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Scene/Model3D/Model3DSpawnParams.h"

using namespace acs;
using namespace acs::game;

/**
 * 3D の見えるものをシーンへ 1 つ置く係。
 *
 * @details
 * 置いたものは **ノード 1 つ + `AMeshComponent3D` 1 つ** になる。描くのはエンジンの側で、
 * ここはその形を組み立てて親へ繋ぐだけ。
 *
 * @code
 * ANode* const Hero = CModel3DSpawner::SpawnInto( Scene.Root(),
 *     FModel3DSpawnParams::FromMesh( FStringView( "hero.mdl" ), FVec3{ 0.0f, 0.0f, 5.0f } ) );
 * @endcode
 *
 * 置いた後に動かすときは、返ってきたノードの `Local()` を書き換える。
 * 消すときはノードを親から外す。
 */
class CModel3DSpawner
{
public:
	/**
	 * 指定どおりに置く。
	 *
	 * @param Parent 繋ぐ先。置いたものはこの下にぶら下がる。
	 * @param Params 何をどこへ置くか。
	 * @return 置いたノード。置けなかったら nullptr (**親には何も足さない**)。
	 */
	static ANode* SpawnInto( ANode& Parent, const FModel3DSpawnParams& Params ) noexcept;

private:
	/**
	 * 場所・向き・大きさを入れる。
	 *
	 * @param Node 入れる先。
	 * @param Params 入れる中身。
	 */
	static void ApplyTransform( ANode& Node, const FModel3DSpawnParams& Params ) noexcept;

	/**
	 * 見た目の部品を付ける。
	 *
	 * @param Node 付ける先。
	 * @param Params 付ける中身。
	 */
	static void ApplyMesh( ANode& Node, const FModel3DSpawnParams& Params ) noexcept;
};
