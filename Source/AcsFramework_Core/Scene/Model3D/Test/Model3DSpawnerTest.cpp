// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Model3D/Model3DSpawner.h"
#include "Common/Test/TestHarness.h"

namespace
{
	/** 置いたノードから見た目の部品を取り出す。 */
	const AMeshComponent3D* MeshOf( ANode* Node ) noexcept
	{
		if ( Node == nullptr ) return nullptr;

		return Node->GetComponent<AMeshComponent3D>();
	}
}


void RunModel3DSpawnerTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "FModel3DSpawnParams / 何も書かなくても置ける" );

	{
		const FModel3DSpawnParams Params;

		Harness.Check( Params.IsValid(), "既定のままで置ける" );
		Harness.Check( Params.Primitive == EMeshPrimitive3D::Cube, "既定の形は立方体" );
		Harness.CheckEqualF32( Params.Scale.x, 1.0f, "既定は等倍" );
		Harness.CheckEqualF32( Params.Color.w, 1.0f, "既定は不透明" );
		Harness.Check( Params.bCastsShadow, "既定で影を落とす" );
	}

	Harness.BeginSuite( "FModel3DSpawnParams / 見えなくなる指定を弾く" );

	{
		// どちらも «置けたのに何も見えない» という、一番追いにくい形になる。
		FModel3DSpawnParams ZeroScale;
		ZeroScale.Scale = FVec3{ 1.0f, 0.0f, 1.0f };
		Harness.Check( !ZeroScale.IsValid(), "0 倍は弾く" );

		FModel3DSpawnParams Mirrored;
		Mirrored.Scale = FVec3{ -1.0f, 1.0f, 1.0f };
		Harness.Check( Mirrored.IsValid(), "負の倍率は鏡写しとして通す" );

		FModel3DSpawnParams EmptyMesh;
		EmptyMesh.Primitive = EMeshPrimitive3D::Mesh;
		Harness.Check( !EmptyMesh.IsValid(), "モデルを指しているのに場所が空なら弾く" );

		const FModel3DSpawnParams FromMesh =
			FModel3DSpawnParams::FromMesh( FStringView( "hero.mdl" ), FVec3{ 0.0f, 0.0f, 0.0f } );
		Harness.Check( FromMesh.IsValid(), "場所があれば通る" );
		Harness.Check( FromMesh.Primitive == EMeshPrimitive3D::Mesh, "モデル指定になる" );
	}

	Harness.BeginSuite( "CModel3DSpawner / 置く" );

	{
		TObjectPtr<ANode> Parent = NewObject<ANode>();
		Harness.Check( Parent.Get() != nullptr, "親を作れる" );

		FModel3DSpawnParams Params =
			FModel3DSpawnParams::FromPrimitive( EMeshPrimitive3D::Sphere, FVec3{ 1.0f, 2.0f, 3.0f } );
		Params.Color = FVec4{ 1.0f, 0.2f, 0.2f, 1.0f };
		Params.bCastsShadow = false;
		Params.Name = FStringView( "Ball" );

		ANode* const Placed = CModel3DSpawner::SpawnInto( *Parent, Params );

		Harness.Check( Placed != nullptr, "置ける" );
		Harness.CheckEqualU64( Parent->ChildCount(), 1u, "親の下に付く" );

		if ( Placed != nullptr )
		{
			Harness.CheckEqualF32( Placed->Local().position.y, 2.0f, "場所が入る" );
			Harness.CheckEqualF32( Placed->Local().scale.x, 1.0f, "大きさが入る" );
			Harness.Check( Placed->Name() == FStringView( "Ball" ), "名前が付く" );
		}

		const AMeshComponent3D* const Mesh = MeshOf( Placed );
		Harness.Check( Mesh != nullptr, "見た目の部品が付く" );

		if ( Mesh != nullptr )
		{
			Harness.Check( Mesh->Primitive() == EMeshPrimitive3D::Sphere, "形が入る" );
			Harness.CheckEqualF32( Mesh->Color().y, 0.2f, "色が入る" );
			Harness.Check( !Mesh->CastsShadow(), "影の指定が入る" );
		}
	}

	Harness.BeginSuite( "CModel3DSpawner / シーンへ識別子付きで置く" );

	{
		CSceneNodeGraph Graph;
		ANode* const Placed = CModel3DSpawner::SpawnInto( Graph,
			FModel3DSpawnParams::FromPrimitive( EMeshPrimitive3D::Plane, FVec3{ 2.0f, 0.0f, -1.0f } ) );

		Harness.Check( Placed != nullptr, "置ける" );
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 1u, "ルートの下に付く" );
		Harness.CheckEqualU64( Graph.RegisteredCount(), 2u, "ルートと配置物が登録される" );

		if ( Placed != nullptr )
		{
			Harness.Check( Placed->Id().IsValid(), "有効な識別子が付く" );
			Harness.Check( Graph.Get( Placed->Id() ) == Placed, "識別子から同じノードを取れる" );
		}
	}

	Harness.BeginSuite( "CModel3DSpawner / モデルを置く" );

	{
		TObjectPtr<ANode> Parent = NewObject<ANode>();

		ANode* const Placed = CModel3DSpawner::SpawnInto( *Parent,
			FModel3DSpawnParams::FromMesh( FStringView( "hero.mdl" ), FVec3{ 0.0f, 0.0f, 5.0f } ) );

		const AMeshComponent3D* const Mesh = MeshOf( Placed );
		Harness.Check( Mesh != nullptr, "置ける" );

		if ( Mesh != nullptr )
		{
			Harness.Check( Mesh->MeshPath() == FStringView( "hero.mdl" ), "モデルの場所が入る" );
			Harness.Check( Mesh->Primitive() == EMeshPrimitive3D::Mesh, "形はモデルになる" );
		}
	}

	Harness.BeginSuite( "CModel3DSpawner / 向きを度で受ける" );

	{
		TObjectPtr<ANode> Parent = NewObject<ANode>();

		FModel3DSpawnParams Params;
		Params.RotationDeg = FVec3{ 0.0f, 90.0f, 0.0f };

		ANode* const Placed = CModel3DSpawner::SpawnInto( *Parent, Params );
		Harness.Check( Placed != nullptr, "置ける" );

		if ( Placed != nullptr )
		{
			// 度をラジアンへ直し忘れると、ここが 90 のまま扱われて明後日の方向を向く。
			// y 軸まわり 90 度なら、四元数の y 成分は sin(45°) = 約 0.707。
			const FQuat Rotation = Placed->Local().rotation;
			Harness.Check( Rotation.y > 0.69f && Rotation.y < 0.72f, "度として扱われている" );

			const FVec3 Euler = Placed->Local().EulerDeg();
			Harness.Check( Euler.y > 89.0f && Euler.y < 91.0f, "度で戻ってくる" );
		}
	}

	Harness.BeginSuite( "CModel3DSpawner / 置けないときは何も足さない" );

	{
		TObjectPtr<ANode> Parent = NewObject<ANode>();

		FModel3DSpawnParams Broken;
		Broken.Scale = FVec3{ 0.0f, 0.0f, 0.0f };

		Harness.Check( CModel3DSpawner::SpawnInto( *Parent, Broken ) == nullptr, "置けない" );
		Harness.CheckEqualU64( Parent->ChildCount(), 0u, "半端なノードが残らない" );
	}

	Harness.BeginSuite( "CModel3DSpawner / シーンへ置けないときも何も足さない" );

	{
		CSceneNodeGraph Graph;
		FModel3DSpawnParams Broken;
		Broken.Scale = FVec3{ 0.0f, 1.0f, 1.0f };

		Harness.Check( CModel3DSpawner::SpawnInto( Graph, Broken ) == nullptr, "置けない" );
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 0u, "半端な子が残らない" );
		Harness.CheckEqualU64( Graph.RegisteredCount(), 1u, "識別子の空きも消費しない" );
	}

	Harness.BeginSuite( "CModel3DSpawner / 積み上げる" );

	{
		TObjectPtr<ANode> Parent = NewObject<ANode>();

		for ( usize Index = 0u; Index < 4u; ++Index )
		{
			const f32 Height = static_cast<f32>( Index );
			CModel3DSpawner::SpawnInto( *Parent,
				FModel3DSpawnParams::FromPrimitive( EMeshPrimitive3D::Cube, FVec3{ 0.0f, Height, 0.0f } ) );
		}

		Harness.CheckEqualU64( Parent->ChildCount(), 4u, "並べて置ける" );

		// 置いた後に動かせること。これができないとゲームにならない。
		ANode* const Second = Parent->Child( 1u );
		Harness.Check( Second != nullptr, "後から取り出せる" );

		if ( Second != nullptr )
		{
			Second->Local().position.x = 5.0f;
			Harness.CheckEqualF32( Second->Local().position.x, 5.0f, "後から動かせる" );
		}
	}
}
