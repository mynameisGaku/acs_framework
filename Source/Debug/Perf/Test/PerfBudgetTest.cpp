// SPDX-License-Identifier: Apache-2.0
#include "Common/Test/TestHarness.h"
#include "Debug/Perf/PerfBudgetSnapshot.h"
#include "Debug/Perf/PerfCategoryPlan.h"


void RunPerfBudgetTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "CPerfCategoryPlan / 同じ名前は差し替え" );

	{
		CPerfCategoryPlan Plan;

		Harness.Check( Plan.Add( FString( "A" ), 1.0f, 0u ), "足せる" );
		Harness.Check( Plan.Add( FString( "B" ), 2.0f, 0u ), "別の名前も足せる" );
		Harness.CheckEqualU64( Plan.Num(), 2u, "件数" );

		Harness.Check( Plan.Add( FString( "A" ), 5.0f, 128u ), "同じ名前をもう一度" );
		Harness.CheckEqualU64( Plan.Num(), 2u, "増えない" );

		bool bUpdated = false;
		for ( usize Index = 0u; Index < Plan.Num(); ++Index )
		{
			const FPerfCategoryDefinition& Definition = Plan.Get( Index );
			if ( FStringView( Definition.Name ) == FStringView( "A" ) )
			{
				bUpdated = ( Definition.BudgetMilliseconds == 5.0f ) && ( Definition.BudgetBytes == 128u );
			}
		}

		Harness.Check( bUpdated, "予算だけが差し替わる" );
	}

	Harness.BeginSuite( "CPerfCategoryPlan / 名前が動かない" );

	{
		// CPerfBudget は名前を複製せずポインタで持つ。ここが動くと表示が壊れる。
		CPerfCategoryPlan Plan;
		Plan.Add( FString( "First" ), 1.0f, 0u );

		const char* const FirstName = Plan.Get( 0u ).Name;

		for ( u32 Index = 0u; Index < 32u; ++Index )
		{
			FString Name;
			Name.AppendFormat( "Filler/%u", Index );
			Plan.Add( Name, 1.0f, 0u );
		}

		Harness.Check( Plan.Get( 0u ).Name == FirstName, "後から足してもポインタが動かない" );
		Harness.Check( Plan.FindStableName( FString( "First" ) ) == FirstName, "同じものが引ける" );
		Harness.Check( Plan.FindStableName( FString( "Nope" ) ) == nullptr, "無い名前は nullptr" );
	}

	Harness.BeginSuite( "CPerfBudgetSnapshot / 写して並べ替える" );

	{
		CPerfBudget Budget;
		Budget.SetFrameBudget( 16.6f );

		CPerfCategoryPlan Plan;
		Plan.Add( FString( "Light" ), 10.0f, 0u );
		Plan.Add( FString( "Heavy" ), 1.0f, 0u );
		Plan.ApplyTo( Budget );

		Budget.BeginFrame();
		Budget.RecordTimeMs( Plan.FindStableName( FString( "Light" ) ), 1.0f );   // 10% 使用
		Budget.RecordTimeMs( Plan.FindStableName( FString( "Heavy" ) ), 2.0f );   // 200% 使用
		Budget.EndFrame();

		CPerfBudgetSnapshot Snapshot;
		Snapshot.CaptureFrom( Budget );

		Harness.CheckEqualU64( Snapshot.Num(), 2u, "2 件が写る" );
		Harness.CheckEqualU64( Snapshot.CountOverBudget(), 1u, "超過は 1 件" );

		Snapshot.SortByTimePressure();

		const usize HeavyIndex = Snapshot.FindIndexByCategory( "Heavy" );
		Harness.CheckEqualU64( HeavyIndex, 0u, "使用率の高いものが先頭" );
		Harness.Check( Snapshot.Get( 0u ).IsOverTimeBudget(), "先頭は超過している" );
		Harness.Check( !Snapshot.Get( 1u ).IsOverTimeBudget(), "2 番目は超過していない" );

		Harness.Check( Snapshot.FindIndexByCategory( "Nope" ) >= Snapshot.Num(), "無い名前は範囲外" );
		Harness.Check( Snapshot.FindIndexByCategory( nullptr ) >= Snapshot.Num(), "nullptr も範囲外" );

		Harness.Check( !Snapshot.MakeRowText( 0u ).IsEmpty(), "行の文字列が作れる" );
		Harness.Check( Snapshot.MakeRowText( 99u ).IsEmpty(), "範囲外は空" );
	}

	Harness.BeginSuite( "FPerfBudgetRow / 上限なしの扱い" );

	{
		FPerfBudgetRow Row;
		Row.SpentMilliseconds = 5.0f;
		Row.BudgetMilliseconds = 0.0f;

		Harness.CheckEqualF32( Row.GetTimePressure(), 0.0f, "上限なしなら使用率 0" );
		Harness.Check( !Row.IsOverTimeBudget(), "上限なしは超過しない" );

		Row.BudgetBytes = 0u;
		Row.SpentBytes = 999u;
		Harness.Check( !Row.IsOverMemoryBudget(), "メモリも上限なしなら超過しない" );

		Row.BudgetBytes = 100u;
		Harness.Check( Row.IsOverMemoryBudget(), "上限を超えれば超過" );
		Harness.Check( Row.IsOverBudget(), "どちらか超えていれば超過" );
	}
}
