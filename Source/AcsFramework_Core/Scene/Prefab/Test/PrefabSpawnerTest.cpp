// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Prefab/PrefabRegistrar.h"
#include "AcsFramework_Core/Scene/Prefab/PrefabSpawner.h"
#include "Common/Test/TestHarness.h"

namespace
{
	/** 作られた回数。作り方が実際に呼ばれたかを見る。 */
	u32 g_MakeCount = 0u;

	/** 試験用の作り方。名前だけ付けたノードを返す。 */
	TObjectPtr<ANode> MakeTestNode( void* UserData ) noexcept
	{
		++g_MakeCount;

		TObjectPtr<ANode> Node = NewObject<ANode>();
		if ( Node.Get() != nullptr ) Node->SetName( FStringView( "FromFactory" ) );

		if ( UserData != nullptr ) *static_cast<u32*>( UserData ) += 1u;

		return Node;
	}

	/** 何も返さない作り方。作れなかった場合の道を通す。 */
	TObjectPtr<ANode> MakeNothing( void* ) noexcept
	{
		return TObjectPtr<ANode>();
	}
}


void RunPrefabTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "CPrefabRegistrar / 名前を写してから登録する" );

	{
		CPrefabSystem Prefabs;
		CInternedNamePool Names;
		CPrefabRegistrar Registrar( Prefabs, Names );

		// 呼び出し側の FString が消えても対応表が壊れないこと。
		{
			FString Temporary;
			Temporary.TryAppend( FStringView( "Enemy/Slime" ) );
			Harness.Check( Registrar.Add( Temporary, &MakeTestNode, nullptr ), "登録できる" );
		}

		Harness.CheckEqualU64( Registrar.GetAddedCount(), 1u, "登録した数" );
		Harness.Check( Prefabs.FindByName( "Enemy/Slime" ).IsValid(), "消えた FString の後でも名前で引ける" );

		Harness.Check( !Registrar.Add( FString( "Bad" ), nullptr, nullptr ), "作り方が無いものは登録しない" );
	}

	Harness.BeginSuite( "CPrefabSpawner / 所有ごと受け取る" );

	{
		g_MakeCount = 0u;

		CPrefabSystem Prefabs;
		CInternedNamePool Names;
		CPrefabRegistrar Registrar( Prefabs, Names );
		Registrar.Add( FString( "Thing" ), &MakeTestNode, nullptr );

		const FPrefabId Id = Prefabs.FindByName( "Thing" );
		Harness.Check( Id.IsValid(), "識別子が引ける" );

		FPrefabSpawnParams Params;
		TObjectPtr<ANode> Detached = CPrefabSpawner::SpawnDetached( Prefabs, Id, Params );

		Harness.Check( Detached.Get() != nullptr, "出てくる" );
		Harness.CheckEqualU64( g_MakeCount, 1u, "作り方が 1 回だけ呼ばれる" );
		Harness.Check( Detached->Name() == FStringView( "FromFactory" ), "作り方が付けた名前のまま" );
	}

	Harness.BeginSuite( "CPrefabSpawner / 置き方を施す" );

	{
		CPrefabSystem Prefabs;
		CInternedNamePool Names;
		CPrefabRegistrar Registrar( Prefabs, Names );
		Registrar.Add( FString( "Thing" ), &MakeTestNode, nullptr );

		const FPrefabId Id = Prefabs.FindByName( "Thing" );

		FPrefabSpawnParams Params;
		Params.Name = FString( "Renamed" );
		Params.bApplyTransform = true;
		Params.LocalTransform.position = FVec3{ 3.0f, 4.0f, 5.0f };
		Params.bApplyEnabled = true;
		Params.bEnabled = false;

		TObjectPtr<ANode> Node = CPrefabSpawner::SpawnDetached( Prefabs, Id, Params );
		Harness.Check( Node.Get() != nullptr, "出てくる" );

		if ( Node.Get() != nullptr )
		{
			Harness.Check( Node->Name() == FStringView( "Renamed" ), "名前が差し替わる" );
			Harness.CheckEqualF32( Node->Local().position.x, 3.0f, "位置 x" );
			Harness.CheckEqualF32( Node->Local().position.z, 5.0f, "位置 z" );
			Harness.Check( !Node->IsEnabled(), "有効・無効が施される" );
		}
	}

	Harness.BeginSuite( "CPrefabSpawner / 指定しなければ手を加えない" );

	{
		// 既定の FPrefabSpawnParams で «勝手に原点へ動く» ようだと、作り方の意図が消える。
		CPrefabSystem Prefabs;
		CInternedNamePool Names;
		CPrefabRegistrar Registrar( Prefabs, Names );
		Registrar.Add( FString( "Thing" ), &MakeTestNode, nullptr );

		const FPrefabId Id = Prefabs.FindByName( "Thing" );

		TObjectPtr<ANode> Node = CPrefabSpawner::SpawnDetached( Prefabs, Id, FPrefabSpawnParams() );
		Harness.Check( Node.Get() != nullptr, "出てくる" );

		if ( Node.Get() != nullptr )
		{
			Harness.Check( Node->Name() == FStringView( "FromFactory" ), "名前はそのまま" );
			Harness.Check( Node->IsEnabled(), "有効のまま" );
		}
	}

	Harness.BeginSuite( "CPrefabSpawner / 親へ付けると所有が移る" );

	{
		CPrefabSystem Prefabs;
		CInternedNamePool Names;
		CPrefabRegistrar Registrar( Prefabs, Names );
		Registrar.Add( FString( "Child" ), &MakeTestNode, nullptr );

		const FPrefabId Id = Prefabs.FindByName( "Child" );

		TObjectPtr<ANode> Parent = NewObject<ANode>();
		Harness.Check( Parent.Get() != nullptr, "親を作れる" );

		FPrefabSpawnParams Params;
		Params.Name = FString( "Attached" );

		ANode* const Attached = CPrefabSpawner::SpawnAttached( Prefabs, Id, *Parent, Params );

		Harness.Check( Attached != nullptr, "出てくる" );
		Harness.CheckEqualU64( Parent->ChildCount(), 1u, "親の子になっている" );

		if ( Attached != nullptr )
		{
			Harness.Check( Parent->Child( 0u ) == Attached, "返ったものが、その子" );
			Harness.Check( Attached->Name() == FStringView( "Attached" ), "置き方も施されている" );
		}
	}

	Harness.BeginSuite( "CPrefabSpawner / 作れなかったとき" );

	{
		CPrefabSystem Prefabs;
		CInternedNamePool Names;
		CPrefabRegistrar Registrar( Prefabs, Names );
		Registrar.Add( FString( "Empty" ), &MakeNothing, nullptr );

		const FPrefabId Id = Prefabs.FindByName( "Empty" );

		TObjectPtr<ANode> Parent = NewObject<ANode>();

		Harness.Check( CPrefabSpawner::SpawnDetached( Prefabs, Id, FPrefabSpawnParams() ).Get() == nullptr, "空が返る" );
		Harness.Check( CPrefabSpawner::SpawnAttached( Prefabs, Id, *Parent, FPrefabSpawnParams() ) == nullptr, "nullptr が返る" );
		Harness.CheckEqualU64( Parent->ChildCount(), 0u, "親に子は増えない" );
	}

	Harness.BeginSuite( "CPrefabSpawner / 作り方へ渡したものが届く" );

	{
		CPrefabSystem Prefabs;
		CInternedNamePool Names;
		CPrefabRegistrar Registrar( Prefabs, Names );

		u32 UserCounter = 0u;
		Registrar.Add( FString( "WithUser" ), &MakeTestNode, &UserCounter );

		const FPrefabId Id = Prefabs.FindByName( "WithUser" );
		CPrefabSpawner::SpawnDetached( Prefabs, Id, FPrefabSpawnParams() );
		CPrefabSpawner::SpawnDetached( Prefabs, Id, FPrefabSpawnParams() );

		Harness.CheckEqualU64( UserCounter, 2u, "登録時に渡したものが毎回届く" );
	}
}
