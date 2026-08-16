// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Audio/Spatial/SpatialSourceRegistry.h"
#include "Common/Test/TestHarness.h"


void RunSpatialSourceRegistryTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "CSpatialSourceRegistry / 0 を配らない・重複しない" );

	{
		CSpatialSourceRegistry Registry;

		const u32 First = Registry.Acquire();
		const u32 Second = Registry.Acquire();
		const u32 Third = Registry.Acquire();

		Harness.Check( First != 0u && Second != 0u && Third != 0u, "0 は配らない (無効値として使うため)" );
		Harness.Check( First != Second && Second != Third && First != Third, "同じ番号を二重に配らない" );
		Harness.CheckEqualU64( Registry.GetActiveCount(), 3u, "貸している数" );
		Harness.CheckEqualU64( Registry.GetAcquiredCount(), 3u, "延べ数" );
	}

	Harness.BeginSuite( "CSpatialSourceRegistry / 返すと使い回す" );

	{
		// 使い回さないと、長く遊ぶうちに番号が尽きる。
		CSpatialSourceRegistry Registry;

		const u32 First = Registry.Acquire();
		const u32 Second = Registry.Acquire();

		Registry.Release( First );
		Harness.CheckEqualU64( Registry.GetActiveCount(), 1u, "返すと減る" );

		const u32 Reused = Registry.Acquire();
		Harness.Check( Reused == First, "返した番号が再び配られる" );
		Harness.Check( Reused != Second, "貸したままのものは配られない" );
		Harness.CheckEqualU64( Registry.GetActiveCount(), 2u, "また増える" );
		Harness.CheckEqualU64( Registry.GetAcquiredCount(), 3u, "延べ数は増え続ける" );
	}

	Harness.BeginSuite( "CSpatialSourceRegistry / おかしな返し方に耐える" );

	{
		CSpatialSourceRegistry Registry;

		Registry.Release( 0u );
		Harness.CheckEqualU64( Registry.GetActiveCount(), 0u, "0 を返しても何も起きない" );

		const u32 Id = Registry.Acquire();
		Registry.Release( Id );
		Registry.Release( Id );   // 二重に返す

		// 二重返却は防げない (番号だけを預かる係なので) が、数が負へ回り込まないこと。
		Harness.Check( Registry.GetActiveCount() == 0u, "貸している数が回り込まない" );
	}
}
