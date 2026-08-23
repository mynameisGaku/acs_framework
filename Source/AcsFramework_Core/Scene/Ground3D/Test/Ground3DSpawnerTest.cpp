// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Ground3D/Ground3DSpawner.h"
#include "Common/Test/TestHarness.h"

#include <limits>

namespace
{
	/** 置いた地面ノードから表示用の平面部品を取り出す。 */
	const AMeshComponent3D* GroundMeshOf( const FCollidableModel3DSpawnResult& Ground ) noexcept
	{
		return Ground.Node != nullptr ? Ground.Node->GetComponent<AMeshComponent3D>() : nullptr;
	}
}


void RunGround3DSpawnerTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "FGround3DSpawnParams / 既定値だけで歩ける地面になる" );

	{
		const FGround3DSpawnParams Ground;
		Harness.Check( Ground.IsValid(), "既定値をそのまま使える" );
		Harness.CheckEqualF32( Ground.Size.x, 10.0f, "既定のX全幅" );
		Harness.CheckEqualF32( Ground.Size.y, 10.0f, "既定のZ全幅" );
		Harness.CheckEqualF32( Ground.Thickness, 1.0f, "上面から下へ1mの厚みを持つ" );
		Harness.Check( Ground.CollisionLayer != 0u, "既定で衝突問い合わせへ現れる" );
		Harness.Check( !Ground.bCastsShadow, "不要な裏面影を既定で作らない" );

		const FGround3DSpawnParams Sized = FGround3DSpawnParams::FromSize(
			FVec2{ 18.0f, 7.0f }, FVec3{ 2.0f, -0.5f, 3.0f } );
		Harness.Check( Sized.IsValid(), "広さと上面位置だけで設定を作れる" );
		Harness.CheckEqualF32( Sized.Size.x, 18.0f, "指定したX全幅" );
		Harness.CheckEqualF32( Sized.Size.y, 7.0f, "指定したZ全幅" );
		Harness.CheckEqualF32( Sized.Position.y, -0.5f, "指定した上面高さ" );
	}

	Harness.BeginSuite( "FGround3DSpawnParams / 表示と衝突を壊す値を配置前に弾く" );

	{
		FGround3DSpawnParams Broken;
		Broken.Size.x = 0.0f;
		Harness.Check( !Broken.IsValid(), "X全幅0を拒否する" );

		Broken = FGround3DSpawnParams{};
		Broken.Size.y = -1.0f;
		Harness.Check( !Broken.IsValid(), "負のZ全幅を拒否する" );

		Broken = FGround3DSpawnParams{};
		Broken.Position.y = std::numeric_limits<f32>::quiet_NaN();
		Harness.Check( !Broken.IsValid(), "有限でない上面位置を拒否する" );

		Broken = FGround3DSpawnParams{};
		Broken.Thickness = 0.0f;
		Harness.Check( !Broken.IsValid(), "厚み0を拒否する" );

		Broken = FGround3DSpawnParams{};
		Broken.Color.x = 1.01f;
		Harness.Check( !Broken.IsValid(), "範囲外の表面色を拒否する" );

		Broken = FGround3DSpawnParams{};
		Broken.Metallic = -0.01f;
		Harness.Check( !Broken.IsValid(), "負の金属度を拒否する" );

		Broken = FGround3DSpawnParams{};
		Broken.Roughness = std::numeric_limits<f32>::infinity();
		Harness.Check( !Broken.IsValid(), "有限でない粗さを拒否する" );

		Broken = FGround3DSpawnParams{};
		Broken.CollisionLayer = 0u;
		Harness.Check( !Broken.IsValid(), "問い合わせ不能なレイヤー0を拒否する" );
	}

	Harness.BeginSuite( "CGround3DSpawner / 表示面と直下の衝突箱を同時に置く" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		FGround3DSpawnParams Params = FGround3DSpawnParams::FromSize(
			FVec2{ 8.0f, 6.0f }, FVec3{ 2.0f, 3.0f, -4.0f } );
		Params.Thickness = 0.5f;
		Params.Color = FVec4{ 0.20f, 0.35f, 0.55f, 1.0f };
		Params.Metallic = 0.15f;
		Params.Roughness = 0.30f;
		Params.CollisionLayer = 0x4u;
		Params.Name = FStringView( "WalkableGround" );

		const FCollidableModel3DSpawnResult Ground = CGround3DSpawner::SpawnInto(
			Graph, Collision, Params );
		Harness.Check( Ground.Succeeded(), "地面と衝突を一括生成できる" );
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 1u, "表示ノードを1個だけ置く" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 1u, "衝突箱を1個だけ登録する" );

		if ( Ground.Node != nullptr )
		{
			Harness.Check( Ground.Node->Name() == FStringView( "WalkableGround" ), "指定名を付ける" );
			Harness.CheckEqualF32( Ground.Node->Local().position.x, 2.0f, "上面のX位置を使う" );
			Harness.CheckEqualF32( Ground.Node->Local().position.y, 3.0f, "上面のY位置を使う" );
			Harness.CheckEqualF32( Ground.Node->Local().scale.x, 8.0f, "表示と衝突へ同じX全幅を使う" );
			Harness.CheckEqualF32( Ground.Node->Local().scale.y, 0.5f, "ノード尺度へ厚みを入れる" );
			Harness.CheckEqualF32( Ground.Node->Local().scale.z, 6.0f, "表示と衝突へ同じZ全幅を使う" );
		}

		const AMeshComponent3D* const Mesh = GroundMeshOf( Ground );
		Harness.Check( Mesh != nullptr, "表示用の平面部品を持つ" );
		if ( Mesh != nullptr )
		{
			Harness.Check( Mesh->Primitive() == EMeshPrimitive3D::Plane, "表示形状は平面" );
			Harness.CheckEqualF32( Mesh->Color().z, 0.55f, "指定した表面色を使う" );
			Harness.CheckEqualF32( Mesh->Material().pbr.metallic, 0.15f, "指定した金属度を使う" );
			Harness.CheckEqualF32( Mesh->Material().pbr.roughness, 0.30f, "指定した粗さを使う" );
			Harness.Check( !Mesh->CastsShadow(), "既定の影指定を反映する" );
		}

		FWorldCollisionShape3D WorldShape;
		Harness.Check( Collision.TryGetWorldShape( Ground.Shape, WorldShape ), "生成したworld衝突を読める" );
		Harness.Check( WorldShape.Kind == FWorldCollisionShape3D::EKind::Box, "地面の衝突は箱" );
		Harness.CheckEqualF32( WorldShape.Box.center.x, 2.0f, "衝突中心Xは上面中心と一致する" );
		Harness.CheckEqualF32( WorldShape.Box.center.y, 2.75f, "衝突中心は上面から厚み半分だけ下" );
		Harness.CheckEqualF32( WorldShape.Box.center.z, -4.0f, "衝突中心Zは上面中心と一致する" );
		Harness.CheckEqualF32( WorldShape.Box.half_size.x, 4.0f, "衝突のX半幅" );
		Harness.CheckEqualF32( WorldShape.Box.half_size.y, 0.25f, "衝突のY半幅" );
		Harness.CheckEqualF32( WorldShape.Box.half_size.z, 3.0f, "衝突のZ半幅" );
		Harness.Check( WorldShape.Layer == 0x4u && WorldShape.bQueryable,
			"指定レイヤーで問い合わせ可能にする" );
	}

	Harness.BeginSuite( "CGround3DSpawner / 失敗時に半端な地面を残さない" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		FGround3DSpawnParams Broken;
		Broken.Thickness = -1.0f;

		const FCollidableModel3DSpawnResult Failed = CGround3DSpawner::SpawnInto(
			Graph, Collision, Broken );
		Harness.Check( !Failed.Succeeded(), "不正値を生成前に拒否する" );
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 0u, "不正値でノードを足さない" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 0u, "不正値で形状を足さない" );
	}

	{
		CSceneNodeGraph Graph;
		CSceneNodeGraph OtherGraph;
		CSceneCollision3D OtherCollision{ OtherGraph };

		const FCollidableModel3DSpawnResult Failed = CGround3DSpawner::SpawnInto(
			Graph, OtherCollision, FGround3DSpawnParams{} );
		Harness.Check( !Failed.Succeeded(), "別場面の衝突集合を拒否する" );
		Graph.ResolveStructuralChanges();
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 0u, "登録失敗時に表示ノードを巻き戻す" );
		Harness.CheckEqualU64( Graph.RegisteredCount(), 1u, "登録失敗時に識別子も解放する" );
		Harness.CheckEqualU64( OtherCollision.ShapeCount(), 0u, "別場面へ形状を残さない" );
	}
}
