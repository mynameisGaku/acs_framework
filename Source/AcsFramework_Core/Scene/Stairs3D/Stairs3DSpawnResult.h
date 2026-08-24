// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Scene/Model3D/CollidableModel3DSpawnResult.h"

#include <acs.h>

using namespace acs;

/** 衝突付き直方体を低い側から順に保持する3D階段の生成結果。 */
struct FStairs3DSpawnResult
{
	/** 低い側から高い側へ並ぶ、各段のノードと箱型衝突。 */
	TArray<FCollidableModel3DSpawnResult> Steps;

	/**
	 * 1段以上を全て生成して衝突登録できたか返す。
	 *
	 * @return 1段以上あり、全要素のノードと形状番号が有効ならtrue。
	 */
	bool Succeeded() const noexcept
	{
		if ( Steps.IsEmpty() ) return false;
		for ( usize Index = 0u; Index < Steps.Num(); ++Index )
		{
			if ( !Steps[Index] ) return false;
		}
		return true;
	}

	/** ノードも形状番号も保持していない空の結果か返す。 */
	bool IsEmpty() const noexcept { return Steps.IsEmpty(); }

	/** 生成結果が保持している段数を返す。 */
	u32 StepCount() const noexcept { return static_cast<u32>( Steps.Num() ); }

	/** 成功結果を条件式で直接調べられるようにする。 */
	explicit operator bool() const noexcept { return Succeeded(); }
};
