// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Animation3D/AnimatedModel3DSpawner.h"
#include "Common/Test/TestHarness.h"

#include <limits>

namespace
{
	/** 配置テスト用の最小骨付きモデルを作る。 */
	TSharedPtr<ASkinnedMeshAsset> MakeAnimatedMesh() noexcept
	{
		TSharedPtr<ASkinnedMeshAsset> Mesh = MakeShared<ASkinnedMeshAsset>();
		if ( !Mesh ) return Mesh;

		FSkinnedVertex Vertex{};
		Vertex.weights[0] = 1.0f;
		Mesh->Vertices().Add( Vertex );
		Mesh->Indices().Add( 0u );
		Mesh->Bones().Add( FBone{} );

		FAnimation Idle;
		Idle.name = FString( "Idle" );
		Idle.duration = 2.0f;
		Mesh->Animations().Add( Move( Idle ) );

		FAnimation Wave;
		Wave.name = FString( "Wave" );
		Wave.duration = 0.5f;
		Mesh->Animations().Add( Move( Wave ) );
		return Mesh;
	}

	/** ノードから骨付きモデル部品を取り出す。 */
	ASkinnedMeshComponent3D* SkinOf( ANode* Node ) noexcept
	{
		return Node != nullptr ? Node->GetComponent<ASkinnedMeshComponent3D>() : nullptr;
	}
}


void RunAnimatedModel3DSpawnerTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "FAnimatedModel3DSpawnParams / 見えない入力を弾く" );

	{
		FAnimatedModel3DSpawnParams Empty;
		Harness.Check( !Empty.IsValid(), "パスもアセットも無ければ弾く" );

		const FAnimatedModel3DSpawnParams FromModel = FAnimatedModel3DSpawnParams::FromModel(
			FStringView( "Models/Hero.fbx" ), FVec3{ 1.0f, 2.0f, 3.0f } );
		Harness.Check( FromModel.IsValid(), "パスがあれば読み込みへ渡せる" );

		FAnimatedModel3DSpawnParams ZeroScale = FromModel;
		ZeroScale.Scale.y = 0.0f;
		Harness.Check( !ZeroScale.IsValid(), "0倍を弾く" );

		FAnimatedModel3DSpawnParams NonFinite = FromModel;
		NonFinite.Position.x = std::numeric_limits<f32>::infinity();
		Harness.Check( !NonFinite.IsValid(), "有限でない座標を弾く" );

		FAnimatedModel3DSpawnParams NegativeColor = FromModel;
		NegativeColor.Color.z = -0.1f;
		Harness.Check( !NegativeColor.IsValid(), "負の色を弾く" );
	}

	Harness.BeginSuite( "CAnimatedModel3DSpawner / 識別子付きで置いて再生する" );

	{
		CSceneNodeGraph Graph;
		FAnimatedModel3DSpawnParams Params;
		Params.MeshAsset = MakeAnimatedMesh();
		Harness.Check( static_cast<bool>( Params.MeshAsset ), "テスト用モデルを作れる" );
		if ( !Params.MeshAsset ) return;
		Params.Position = FVec3{ 1.0f, 2.0f, 3.0f };
		Params.Scale = FVec3{ 2.0f, 2.0f, 2.0f };
		Params.Color = FVec3{ 0.25f, 0.50f, 0.75f };
		Params.Name = FStringView( "AnimatedHero" );

		ANode* const Placed = CAnimatedModel3DSpawner::SpawnInto( Graph, Params );
		ASkinnedMeshComponent3D* const Skin = SkinOf( Placed );
		Harness.Check( Placed != nullptr, "置ける" );
		Harness.Check( Skin != nullptr, "骨付き部品が付く" );
		Harness.CheckEqualU64( Graph.RegisteredCount(), 2u, "識別子が登録される" );

		if ( Placed != nullptr )
		{
			Harness.Check( Placed->Id().IsValid(), "有効な識別子が付く" );
			Harness.Check( Placed->Name() == FStringView( "AnimatedHero" ), "名前が付く" );
			Harness.CheckEqualF32( Placed->Local().position.y, 2.0f, "位置が入る" );
			Harness.CheckEqualF32( Placed->Local().scale.x, 2.0f, "大きさが入る" );
		}

		if ( Skin != nullptr )
		{
			Harness.Check( Skin->IsRenderable(), "描画可能なモデルが入る" );
			Harness.Check( Skin->Player().IsPlaying(), "最初のクリップを自動再生する" );
			Harness.CheckEqualF32( Skin->Color().z, 0.75f, "色が入る" );
		}
	}

	Harness.BeginSuite( "CAnimatedModel3DSpawner / 名前で初期クリップを選ぶ" );

	{
		TObjectPtr<ANode> Parent = NewObject<ANode>();
		FAnimatedModel3DSpawnParams Params;
		Params.MeshAsset = MakeAnimatedMesh();
		Params.InitialAnimation = FStringView( "Wave" );
		Params.bLoop = false;

		ANode* const Placed = CAnimatedModel3DSpawner::SpawnInto( *Parent, Params );
		ASkinnedMeshComponent3D* const Skin = SkinOf( Placed );
		Harness.Check( Skin != nullptr, "名前で選んで置ける" );

		if ( Skin != nullptr )
		{
			Skin->OnUpdate( 1.0f );
			Harness.Check( !Skin->Player().IsPlaying(), "選んだ非ループクリップが終端で止まる" );
			Harness.CheckEqualF32( Skin->Player().Time(), 0.5f, "選んだクリップの長さで止まる" );
		}
	}

	Harness.BeginSuite( "CAnimatedModel3DSpawner / 失敗時は木を変えない" );

	{
		TObjectPtr<ANode> Parent = NewObject<ANode>();
		FAnimatedModel3DSpawnParams Params;
		Params.MeshAsset = MakeAnimatedMesh();
		Params.InitialAnimation = FStringView( "Missing" );

		Harness.Check( CAnimatedModel3DSpawner::SpawnInto( *Parent, Params ) == nullptr,
			"存在しないクリップを弾く" );
		Harness.CheckEqualU64( Parent->ChildCount(), 0u, "半端なノードを残さない" );

		CSceneNodeGraph Graph;
		FAnimatedModel3DSpawnParams EmptyAsset;
		EmptyAsset.MeshPath = FStringView( "Models/Hero.fbx" );
		Harness.Check( CAnimatedModel3DSpawner::SpawnInto( Graph, EmptyAsset ) == nullptr,
			"読み込み先なしでパスだけを渡したら弾く" );
		Harness.CheckEqualU64( Graph.RegisteredCount(), 1u, "識別子の空きも消費しない" );
	}
}
