// SPDX-License-Identifier: Apache-2.0
#include "Common/Test/TestHarness.h"
#include "Common/Text/InternedNamePool.h"


void RunInternedNamePoolTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "CInternedNamePool / 同じ名前は 1 つ" );

	{
		CInternedNamePool Pool;

		const char* const First = Pool.Intern( FString( "Scene/Update" ) );
		const char* const Second = Pool.Intern( FString( "Scene/Update" ) );

		Harness.Check( First != nullptr, "写せる" );
		Harness.Check( First == Second, "同じ名前なら同じポインタ" );
		Harness.CheckEqualU64( Pool.Num(), 1u, "二重に持たない" );

		const char* const Other = Pool.Intern( FString( "Scene/Render" ) );
		Harness.Check( Other != First, "違う名前は違うポインタ" );
		Harness.CheckEqualU64( Pool.Num(), 2u, "件数" );

		Harness.Check( Pool.Find( FString( "Scene/Update" ) ) == First, "探すと同じものが返る" );
		Harness.Check( Pool.Find( FString( "Nope" ) ) == nullptr, "無い名前は nullptr" );
	}

	Harness.BeginSuite( "CInternedNamePool / 増やしてもポインタが動かない" );

	{
		// ここが崩れると、先に渡した名前を Engine 側が握ったまま別物を指す。
		// 配列が伸びる回数を確実に跨ぐよう、多めに入れる。
		CInternedNamePool Pool;

		constexpr usize kCount = 64u;
		TArray<const char*> Pointers;

		for ( usize Index = 0u; Index < kCount; ++Index )
		{
			FString Name;
			Name.AppendFormat( "Category/%zu", Index );

			const char* const Stable = Pool.Intern( Name );
			Pointers.TryAdd( Stable );
		}

		Harness.CheckEqualU64( Pool.Num(), kCount, "全て入る" );

		bool bAllStable = true;
		for ( usize Index = 0u; Index < kCount; ++Index )
		{
			FString Name;
			Name.AppendFormat( "Category/%zu", Index );

			if ( Pool.Find( Name ) != Pointers[Index] ) bAllStable = false;
		}

		Harness.Check( bAllStable, "最初に返したポインタが後からも有効" );

		// 中身も読めること (解放済み領域を指していないことの確認)。
		bool bContentOk = true;
		for ( usize Index = 0u; Index < kCount; ++Index )
		{
			FString Expected;
			Expected.AppendFormat( "Category/%zu", Index );

			if ( !( FStringView( Pointers[Index] ) == Expected.View() ) ) bContentOk = false;
		}

		Harness.Check( bContentOk, "中身も保たれている" );
	}

	Harness.BeginSuite( "CInternedNamePool / 空文字列" );

	{
		CInternedNamePool Pool;

		const char* const Empty = Pool.Intern( FString() );
		Harness.Check( Empty != nullptr, "空文字列も写せる" );
		Harness.Check( Empty[0] == '\0', "空文字列として読める" );
	}
}
