// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Scene/Light3D/Lamp3DSpawnResult.h"
#include "AcsFramework_Core/Scene/Model3D/CollidableModel3DSpawnResult.h"

/** 衝突付きポストと、その上の発光球・点光源を保持する街灯の生成結果。 */
struct FStreetLamp3DSpawnResult
{
	/** ポストの表示ノードと箱型衝突。 */
	FCollidableModel3DSpawnResult Post;

	/** ポスト上端へ置いた発光球と点光源。 */
	FLamp3DSpawnResult Lamp;

	/** ポスト、衝突、発光球、点光源を全て生成できたか返す。 */
	bool Succeeded() const noexcept { return Post && Lamp; }

	/** ノードも形状番号も所有情報も保持しない空の結果か返す。 */
	bool IsEmpty() const noexcept
	{
		return !Post.Node && !Post.Shape.IsValid() && Lamp.IsEmpty();
	}

	/** ポスト、発光球、点光源の生成済みノード数を返す。 */
	u32 PartCount() const noexcept
	{
		return ( Post ? 1u : 0u ) + ( Lamp ? 2u : 0u );
	}

	/** 成功結果を条件式で直接調べられるようにする。 */
	explicit operator bool() const noexcept { return Succeeded(); }
};
