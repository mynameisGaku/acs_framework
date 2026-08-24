// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Block3D/Block3DSpawner.h"
#include "Common/Test/TestHarness.h"

#include <limits>

namespace
{
	/** 置いた直方体ノードから表示用の立方体部品を取り出す。 */
	const AMeshComponent3D* BlockMeshOf( const FCollidableModel3DSpawnResult& Block ) noexcept
	{
		return Block.Node != nullptr ? Block.Node->GetComponent<AMeshComponent3D>() : nullptr;
	}
}


void RunBlock3DSpawnerTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "FBlock3DSpawnParams / 既定値だけで衝突付き直方体になる" );

	{
		const FBlock3DSpawnParams Block;
		Harness.Check( Block.IsValid(), "既定値をそのまま使える" );
		Harness.CheckEqualF32( Block.Size.x, 1.0f, "既定のX全寸法" );
		Harness.CheckEqualF32( Block.Size.y, 1.0f, "既定のY全寸法" );
		Harness.CheckEqualF32( Block.Size.z, 1.0f, "既定のZ全寸法" );
		Harness.Check( Block.CollisionLayer != 0u, "既定で衝突問い合わせへ現れる" );
		Harness.Check( Block.bCastsShadow, "既定で立体の影を落とす" );

		const FBlock3DSpawnParams Sized = FBlock3DSpawnParams::FromSize(
			FVec3{ 4.0f, 2.0f, 0.5f }, FVec3{ 2.0f, 1.0f, -3.0f } );
		Harness.Check( Sized.IsValid(), "全寸法と中心位置だけで設定を作れる" );
		Harness.CheckEqualF32( Sized.Size.x, 4.0f, "指定したX全寸法" );
		Harness.CheckEqualF32( Sized.Size.y, 2.0f, "指定したY全寸法" );
		Harness.CheckEqualF32( Sized.Size.z, 0.5f, "指定したZ全寸法" );
		Harness.CheckEqualF32( Sized.Position.y, 1.0f, "指定した中心高さ" );
	}

	Harness.BeginSuite( "FBlock3DSpawnParams / 表示と衝突を壊す値を配置前に弾く" );

	{
		FBlock3DSpawnParams Broken;
		Broken.Size.x = 0.0f;
		Harness.Check( !Broken.IsValid(), "X全寸法0を拒否する" );

		Broken = FBlock3DSpawnParams{};
		Broken.Size.y = -1.0f;
		Harness.Check( !Broken.IsValid(), "負のY全寸法を拒否する" );

		Broken = FBlock3DSpawnParams{};
		Broken.Size.z = std::numeric_limits<f32>::infinity();
		Harness.Check( !Broken.IsValid(), "有限でないZ全寸法を拒否する" );

		Broken = FBlock3DSpawnParams{};
		Broken.Position.y = std::numeric_limits<f32>::quiet_NaN();
		Harness.Check( !Broken.IsValid(), "有限でない中心位置を拒否する" );

		Broken = FBlock3DSpawnParams{};
		Broken.RotationDeg.x = std::numeric_limits<f32>::infinity();
		Harness.Check( !Broken.IsValid(), "有限でない回転を拒否する" );

		Broken = FBlock3DSpawnParams{};
		Broken.Color.w = 1.01f;
		Harness.Check( !Broken.IsValid(), "範囲外の表面色を拒否する" );

		Broken = FBlock3DSpawnParams{};
		Broken.Metallic = -0.01f;
		Harness.Check( !Broken.IsValid(), "負の金属度を拒否する" );

		Broken = FBlock3DSpawnParams{};
		Broken.Roughness = std::numeric_limits<f32>::quiet_NaN();
		Harness.Check( !Broken.IsValid(), "有限でない粗さを拒否する" );

		Broken = FBlock3DSpawnParams{};
		Broken.CollisionLayer = 0u;
		Harness.Check( !Broken.IsValid(), "問い合わせ不能なレイヤー0を拒否する" );
	}

	Harness.BeginSuite( "CBlock3DSpawner / 表示と箱型衝突を同じローカル寸法で置く" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		FBlock3DSpawnParams Params = FBlock3DSpawnParams::FromSize(
			FVec3{ 2.0f, 4.0f, 6.0f }, FVec3{ 1.0f, 2.0f, 3.0f } );
		Params.RotationDeg = FVec3{ 0.0f, 90.0f, 0.0f };
		Params.Color = FVec4{ 0.20f, 0.35f, 0.55f, 1.0f };
		Params.Metallic = 0.15f;
		Params.Roughness = 0.30f;
		Params.bCastsShadow = false;
		Params.CollisionLayer = 0x4u;
		Params.Name = FStringView( "SolidWall" );

		const FCollidableModel3DSpawnResult Block = CBlock3DSpawner::SpawnInto(
			Graph, Collision, Params );
		Harness.Check( Block.Succeeded(), "直方体と衝突を一括生成できる" );
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 1u, "表示ノードを1個だけ置く" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 1u, "衝突箱を1個だけ登録する" );

		if ( Block.Node != nullptr )
		{
			Harness.Check( Block.Node->Name() == FStringView( "SolidWall" ), "指定名を付ける" );
			Harness.CheckEqualF32( Block.Node->Local().position.x, 1.0f, "指定した中心Xを使う" );
			Harness.CheckEqualF32( Block.Node->Local().position.y, 2.0f, "指定した中心Yを使う" );
			Harness.CheckEqualF32( Block.Node->Local().position.z, 3.0f, "指定した中心Zを使う" );
			Harness.CheckEqualF32( Block.Node->Local().scale.x, 2.0f, "表示と衝突へ同じX全寸法を使う" );
			Harness.CheckEqualF32( Block.Node->Local().scale.y, 4.0f, "表示と衝突へ同じY全寸法を使う" );
			Harness.CheckEqualF32( Block.Node->Local().scale.z, 6.0f, "表示と衝突へ同じZ全寸法を使う" );
			Harness.CheckNearF32( Block.Node->Local().EulerDeg().y, 90.0f, 0.1f,
				"指定した度数で向ける" );
		}

		const AMeshComponent3D* const Mesh = BlockMeshOf( Block );
		Harness.Check( Mesh != nullptr, "表示用の立方体部品を持つ" );
		if ( Mesh != nullptr )
		{
			Harness.Check( Mesh->Primitive() == EMeshPrimitive3D::Cube, "表示形状は立方体" );
			Harness.CheckEqualF32( Mesh->Color().z, 0.55f, "指定した表面色を使う" );
			Harness.CheckEqualF32( Mesh->Material().pbr.metallic, 0.15f, "指定した金属度を使う" );
			Harness.CheckEqualF32( Mesh->Material().pbr.roughness, 0.30f, "指定した粗さを使う" );
			Harness.Check( !Mesh->CastsShadow(), "指定した影設定を反映する" );
		}

		FWorldCollisionShape3D WorldShape;
		Harness.Check( Collision.TryGetWorldShape( Block.Shape, WorldShape ), "生成したworld衝突を読める" );
		Harness.Check( WorldShape.Kind == FWorldCollisionShape3D::EKind::Box, "直方体の衝突は箱" );
		Harness.CheckNearF32( WorldShape.Box.center.x, 1.0f, 0.001f, "衝突中心Xは表示中心と一致する" );
		Harness.CheckNearF32( WorldShape.Box.center.y, 2.0f, 0.001f, "衝突中心Yは表示中心と一致する" );
		Harness.CheckNearF32( WorldShape.Box.center.z, 3.0f, 0.001f, "衝突中心Zは表示中心と一致する" );
		Harness.CheckNearF32( WorldShape.Box.half_size.x, 3.0f, 0.001f,
			"Y軸90度回転後のZ半寸法をworld X軸へ反映する" );
		Harness.CheckNearF32( WorldShape.Box.half_size.y, 2.0f, 0.001f,
			"Y半寸法をworld Y軸へ反映する" );
		Harness.CheckNearF32( WorldShape.Box.half_size.z, 1.0f, 0.001f,
			"Y軸90度回転後のX半寸法をworld Z軸へ反映する" );
		Harness.Check( WorldShape.Layer == 0x4u && WorldShape.bQueryable,
			"指定レイヤーで問い合わせ可能にする" );
	}

	Harness.BeginSuite( "CBlock3DSpawner / 斜め表示をworld軸平行箱で安全側に包む" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		FBlock3DSpawnParams Params = FBlock3DSpawnParams::FromSize(
			FVec3{ 2.0f, 2.0f, 6.0f }, FVec3{} );
		Params.RotationDeg = FVec3{ 0.0f, 45.0f, 0.0f };

		const FCollidableModel3DSpawnResult Block = CBlock3DSpawner::SpawnInto(
			Graph, Collision, Params );
		Harness.Check( Block.Succeeded(), "45度回転した直方体を置ける" );

		FWorldCollisionShape3D WorldShape;
		Harness.Check( Collision.TryGetWorldShape( Block.Shape, WorldShape ),
			"斜め直方体のworld衝突を読める" );
		Harness.CheckNearF32( WorldShape.Box.half_size.x, 2.828427f, 0.001f,
			"回転後の8頂点を包むworld X半寸法を使う" );
		Harness.CheckNearF32( WorldShape.Box.half_size.y, 1.0f, 0.001f,
			"回転しないworld Y半寸法を保つ" );
		Harness.CheckNearF32( WorldShape.Box.half_size.z, 2.828427f, 0.001f,
			"回転後の8頂点を包むworld Z半寸法を使う" );
	}

	Harness.BeginSuite( "CBlock3DSpawner / 失敗時に半端な直方体を残さない" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		FBlock3DSpawnParams Broken;
		Broken.Size.z = 0.0f;

		const FCollidableModel3DSpawnResult Failed = CBlock3DSpawner::SpawnInto(
			Graph, Collision, Broken );
		Harness.Check( !Failed.Succeeded(), "不正値を生成前に拒否する" );
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 0u, "不正値でノードを足さない" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 0u, "不正値で形状を足さない" );
	}

	{
		CSceneNodeGraph Graph;
		CSceneNodeGraph OtherGraph;
		CSceneCollision3D OtherCollision{ OtherGraph };

		const FCollidableModel3DSpawnResult Failed = CBlock3DSpawner::SpawnInto(
			Graph, OtherCollision, FBlock3DSpawnParams{} );
		Harness.Check( !Failed.Succeeded(), "別場面の衝突集合を拒否する" );
		Graph.ResolveStructuralChanges();
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 0u, "登録失敗時に表示ノードを巻き戻す" );
		Harness.CheckEqualU64( Graph.RegisteredCount(), 1u, "登録失敗時に識別子も解放する" );
		Harness.CheckEqualU64( OtherCollision.ShapeCount(), 0u, "別場面へ形状を残さない" );
	}
}
