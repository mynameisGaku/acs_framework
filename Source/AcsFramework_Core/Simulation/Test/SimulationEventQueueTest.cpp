// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/SimulationEventQueue.h"
#include "Common/Test/TestHarness.h"


void RunSimulationEventQueueTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "CSimulationEventQueue / 溜めて読んで捨てる" );

	{
		CSimulationEventQueue Queue;

		FSimulationEvent Event;
		Event.Id = 7u;
		Event.Tick = 3u;
		Event.ValueA = 1.5f;

		Harness.Check( Queue.Push( Event ), "溜められる" );
		Harness.CheckEqualU64( Queue.Num(), 1u, "件数" );
		Harness.Check( Queue.Get( 0u ).Equals( Event ), "同じ内容が返る" );

		Queue.Clear();
		Harness.CheckEqualU64( Queue.Num(), 0u, "捨てられる" );
		Harness.CheckEqualU64( Queue.GetDroppedCount(), 0u, "捨てても «溢れ» にはならない" );
	}

	Harness.BeginSuite( "CSimulationEventQueue / 上限で止まる" );

	{
		// 歯止めが無いと、ロジックの暴走でメモリを食い潰す。
		CSimulationEventQueue Queue;
		Queue.SetCapacity( 4u );

		FSimulationEvent Event;
		for ( u32 Index = 0u; Index < 10u; ++Index )
		{
			Event.Id = Index;
			Queue.Push( Event );
		}

		Harness.CheckEqualU64( Queue.Num(), 4u, "上限で止まる" );
		Harness.CheckEqualU64( Queue.GetDroppedCount(), 6u, "溢れた数を数えている" );
		Harness.CheckEqualU64( Queue.Get( 0u ).Id, 0u, "先に来たものが残る" );
	}

	Harness.BeginSuite( "FSimulationEvent / 同じ内容の判定" );

	{
		FSimulationEvent A;
		A.Id = 1u; A.Tick = 2u; A.Target = 3u; A.ValueA = 4.0f; A.ValueB = 5.0f; A.ValueC = 6.0f;

		FSimulationEvent B = A;
		Harness.Check( A.Equals( B ), "同じなら true" );

		B.ValueC = 6.5f;
		Harness.Check( !A.Equals( B ), "1 つ違えば false" );

		B = A;
		B.Tick = 99u;
		Harness.Check( !A.Equals( B ), "ティックの違いも見る" );
	}
}
