// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Scene/Fence3D/Fence3DSpawnResult.h"
#include "AcsFramework_Core/Scene/Model3D/CollidableModel3DSpawnResult.h"

/** 床板と幅軸の負側・正側にある柵2組の生成結果。 */
struct FBridge3DSpawnResult
{
	/** 入口から出口まで続く床板のノードと箱型衝突。 */
	FCollidableModel3DSpawnResult Deck;

	/** 幅軸の負側へ置いた支柱と横桟。 */
	FFence3DSpawnResult NegativeSideRailing;

	/** 幅軸の正側へ置いた支柱と横桟。 */
	FFence3DSpawnResult PositiveSideRailing;

	/** 床板と両側柵を全て生成して衝突登録できたか返す。 */
	bool Succeeded() const noexcept
	{
		return Deck && NegativeSideRailing && PositiveSideRailing;
	}

	/** ノードも形状番号も保持していない空の結果か返す。 */
	bool IsEmpty() const noexcept
	{
		return !Deck.Node && !Deck.Shape.IsValid()
			&& NegativeSideRailing.IsEmpty() && PositiveSideRailing.IsEmpty();
	}

	/** 床板、支柱、横桟の合計数を返す。 */
	u32 PartCount() const noexcept
	{
		return ( Deck ? 1u : 0u ) + NegativeSideRailing.PartCount()
			+ PositiveSideRailing.PartCount();
	}

	/** 成功結果を条件式で直接調べられるようにする。 */
	explicit operator bool() const noexcept { return Succeeded(); }
};
