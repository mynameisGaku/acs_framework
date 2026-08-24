// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Scene/Model3D/CollidableModel3DSpawnResult.h"

#include <acs.h>

using namespace acs;

/** 始点から終点順の支柱と、下から上順の横桟を保持する3D柵の生成結果。 */
struct FFence3DSpawnResult
{
	/** 始点から終点へ並ぶ、各支柱のノードと箱型衝突。 */
	TArray<FCollidableModel3DSpawnResult> Posts;

	/** 下から上へ並ぶ、各横桟のノードと箱型衝突。 */
	TArray<FCollidableModel3DSpawnResult> Rails;

	/** 両端を含む2本以上の支柱と1本以上の横桟を全て生成できたか返す。 */
	bool Succeeded() const noexcept
	{
		if ( Posts.Num() < 2u || Rails.IsEmpty() ) return false;
		for ( usize Index = 0u; Index < Posts.Num(); ++Index )
		{
			if ( !Posts[Index] ) return false;
		}
		for ( usize Index = 0u; Index < Rails.Num(); ++Index )
		{
			if ( !Rails[Index] ) return false;
		}
		return true;
	}

	/** 支柱も横桟も保持していない空の結果か返す。 */
	bool IsEmpty() const noexcept { return Posts.IsEmpty() && Rails.IsEmpty(); }

	/** 生成結果が保持している支柱数を返す。 */
	u32 PostCount() const noexcept { return static_cast<u32>( Posts.Num() ); }

	/** 生成結果が保持している横桟数を返す。 */
	u32 RailCount() const noexcept { return static_cast<u32>( Rails.Num() ); }

	/** 生成結果が保持している支柱と横桟の合計数を返す。 */
	u32 PartCount() const noexcept { return PostCount() + RailCount(); }

	/** 成功結果を条件式で直接調べられるようにする。 */
	explicit operator bool() const noexcept { return Succeeded(); }
};
