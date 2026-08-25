// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Room3D/Room3DSpawner.h"
#include "Common/Test/TestHarness.h"

#include <limits>

namespace
{
	/** 部屋の一部から表示用メッシュ部品を取り出す。 */
	const AMeshComponent3D* RoomMeshOf( const FCollidableModel3DSpawnResult& Part ) noexcept
	{
		return Part.Node != nullptr ? Part.Node->GetComponent<AMeshComponent3D>() : nullptr;
	}

	/** 3成分を同じ許容差で検証する。 */
	void CheckVector( CTestHarness& Harness, FVec3 Actual,
		FVec3 Expected, const char* Message )
	{
		Harness.CheckNearF32( Actual.x, Expected.x, 0.001f, Message );
		Harness.CheckNearF32( Actual.y, Expected.y, 0.001f, Message );
		Harness.CheckNearF32( Actual.z, Expected.z, 0.001f, Message );
	}

	/** 部屋の全ノードが指定した親へ繋がっているか返す。 */
	bool AllPartsUseParent( const FRoom3DSpawnResult& Room, const ANode& Parent ) noexcept
	{
		return Room.Floor.Node != nullptr && Room.Floor.Node->Parent() == &Parent
			&& Room.PositiveZWall.Node != nullptr && Room.PositiveZWall.Node->Parent() == &Parent
			&& Room.NegativeZWall.Node != nullptr && Room.NegativeZWall.Node->Parent() == &Parent
			&& Room.PositiveXWall.Node != nullptr && Room.PositiveXWall.Node->Parent() == &Parent
			&& Room.NegativeXWall.Node != nullptr && Room.NegativeXWall.Node->Parent() == &Parent;
	}

	/** 部屋の全ノードが破棄予定ではないか返す。 */
	bool NoPartIsPendingDestroy( const FRoom3DSpawnResult& Room ) noexcept
	{
		return Room.Floor.Node != nullptr && !Room.Floor.Node->IsPendingDestroy()
			&& Room.PositiveZWall.Node != nullptr && !Room.PositiveZWall.Node->IsPendingDestroy()
			&& Room.NegativeZWall.Node != nullptr && !Room.NegativeZWall.Node->IsPendingDestroy()
			&& Room.PositiveXWall.Node != nullptr && !Room.PositiveXWall.Node->IsPendingDestroy()
			&& Room.NegativeXWall.Node != nullptr && !Room.NegativeXWall.Node->IsPendingDestroy();
	}
}


void RunRoom3DSpawnerTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "FRoom3DSpawnParams / 既定値だけで天井なし部屋になる" );

	{
		const FRoom3DSpawnParams Room;
		Harness.Check( Room.IsValid(), "既定値をそのまま使える" );
		Harness.CheckEqualF32( Room.InnerSize.x, 8.0f, "既定の内寸X" );
		Harness.CheckEqualF32( Room.InnerSize.y, 8.0f, "既定の内寸Z" );
		Harness.CheckEqualF32( Room.WallHeight, 3.0f, "既定の壁高" );
		Harness.CheckEqualF32( Room.WallThickness, 0.25f, "既定の壁厚" );
		Harness.CheckEqualF32( Room.FloorThickness, 0.5f, "既定の床厚" );
		Harness.Check( Room.CollisionLayer != 0u, "既定で衝突問い合わせへ現れる" );

		const FRoom3DSpawnParams Sized = FRoom3DSpawnParams::FromInnerSize(
			FVec2{ 12.0f, 7.0f }, 4.0f, FVec3{ 2.0f, -0.5f, 3.0f } );
		Harness.Check( Sized.IsValid(), "内寸、壁高、床上面位置だけで設定を作れる" );
		Harness.CheckEqualF32( Sized.InnerSize.x, 12.0f, "指定した内寸X" );
		Harness.CheckEqualF32( Sized.InnerSize.y, 7.0f, "指定した内寸Z" );
		Harness.CheckEqualF32( Sized.WallHeight, 4.0f, "指定した壁高" );
		Harness.CheckEqualF32( Sized.FloorTopPosition.y, -0.5f, "指定した床上面高さ" );
		Harness.Check( FRoom3DSpawnResult{}.IsEmpty(), "既定の生成結果は空" );
	}

	Harness.BeginSuite( "FRoom3DSpawnParams / 半端な部屋を作る値を配置前に弾く" );

	{
		FRoom3DSpawnParams Broken;
		Broken.InnerSize.x = 0.0f;
		Harness.Check( !Broken.IsValid(), "内寸Xの0を拒否する" );

		Broken = FRoom3DSpawnParams{};
		Broken.InnerSize.y = -1.0f;
		Harness.Check( !Broken.IsValid(), "負の内寸Zを拒否する" );

		Broken = FRoom3DSpawnParams{};
		Broken.FloorTopPosition.y = std::numeric_limits<f32>::quiet_NaN();
		Harness.Check( !Broken.IsValid(), "有限でない床上面位置を拒否する" );

		Broken = FRoom3DSpawnParams{};
		Broken.WallHeight = 0.0f;
		Harness.Check( !Broken.IsValid(), "壁高0を拒否する" );

		Broken = FRoom3DSpawnParams{};
		Broken.WallThickness = -0.1f;
		Harness.Check( !Broken.IsValid(), "負の壁厚を拒否する" );

		Broken = FRoom3DSpawnParams{};
		Broken.FloorThickness = std::numeric_limits<f32>::infinity();
		Harness.Check( !Broken.IsValid(), "有限でない床厚を拒否する" );

		Broken = FRoom3DSpawnParams{};
		Broken.FloorColor.x = 1.01f;
		Harness.Check( !Broken.IsValid(), "範囲外の床色を拒否する" );

		Broken = FRoom3DSpawnParams{};
		Broken.WallColor.w = -0.01f;
		Harness.Check( !Broken.IsValid(), "範囲外の壁色を拒否する" );

		Broken = FRoom3DSpawnParams{};
		Broken.FloorMetallic = -0.01f;
		Harness.Check( !Broken.IsValid(), "負の床金属度を拒否する" );

		Broken = FRoom3DSpawnParams{};
		Broken.WallRoughness = std::numeric_limits<f32>::quiet_NaN();
		Harness.Check( !Broken.IsValid(), "有限でない壁粗さを拒否する" );

		Broken = FRoom3DSpawnParams{};
		Broken.CollisionLayer = 0u;
		Harness.Check( !Broken.IsValid(), "問い合わせ不能なレイヤー0を拒否する" );

		Broken = FRoom3DSpawnParams{};
		Broken.WallThickness = std::numeric_limits<f32>::max();
		Harness.Check( !Broken.IsValid(), "計算後に有限でない外寸を拒否する" );

		Broken = FRoom3DSpawnParams{};
		Broken.FloorTopPosition.y = std::numeric_limits<f32>::max();
		Broken.WallHeight = std::numeric_limits<f32>::max();
		Harness.Check( !Broken.IsValid(), "計算後に有限でない壁中心を拒否する" );
	}

	Harness.BeginSuite( "CRoom3DSpawner / 内寸を保って床と四方の壁を置く" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		FRoom3DSpawnParams Params = FRoom3DSpawnParams::FromInnerSize(
			FVec2{ 6.0f, 4.0f }, 3.0f, FVec3{ 1.0f, 2.0f, 3.0f } );
		Params.WallThickness = 0.2f;
		Params.FloorThickness = 0.5f;
		Params.FloorColor = FVec4{ 0.18f, 0.24f, 0.32f, 1.0f };
		Params.WallColor = FVec4{ 0.62f, 0.58f, 0.50f, 1.0f };
		Params.FloorMetallic = 0.1f;
		Params.FloorRoughness = 0.7f;
		Params.WallMetallic = 0.2f;
		Params.WallRoughness = 0.4f;
		Params.bFloorCastsShadow = true;
		Params.bWallsCastShadow = false;
		Params.CollisionLayer = 0x4u;

		FRoom3DSpawnResult Room = CRoom3DSpawner::SpawnInto( Graph, Collision, Params );
		Harness.Check( Room.Succeeded(), "床と四方の壁を一括生成できる" );
		Harness.Check( !Room.IsEmpty(), "成功結果は空ではない" );
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 5u, "表示ノードを5個だけ置く" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 5u, "箱型衝突を5個だけ登録する" );

		if ( Room.Floor.Node != nullptr )
		{
			Harness.Check( Room.Floor.Node->Name() == FStringView( "RoomFloor" ), "床へ固定名を付ける" );
			Harness.CheckEqualF32( Room.Floor.Node->Local().position.x, 1.0f, "床上面中心Xを使う" );
			Harness.CheckEqualF32( Room.Floor.Node->Local().position.y, 2.0f, "床上面中心Yを使う" );
			Harness.CheckNearF32( Room.Floor.Node->Local().scale.x, 6.4f, 0.001f, "床を両側の壁厚まで広げる" );
			Harness.CheckEqualF32( Room.Floor.Node->Local().scale.y, 0.5f, "床尺度へ厚みを入れる" );
			Harness.CheckNearF32( Room.Floor.Node->Local().scale.z, 4.4f, 0.001f, "床奥行きを両側の壁厚まで広げる" );
		}

		if ( Room.PositiveZWall.Node != nullptr )
		{
			Harness.Check( Room.PositiveZWall.Node->Name() == FStringView( "RoomPositiveZWall" ), "Z正壁へ固定名を付ける" );
			Harness.CheckEqualF32( Room.PositiveZWall.Node->Local().position.y, 3.5f, "壁中心を床上面から壁高半分上へ置く" );
			Harness.CheckNearF32( Room.PositiveZWall.Node->Local().position.z, 5.1f, 0.001f, "Z正壁を内寸の外側へ置く" );
			Harness.CheckNearF32( Room.PositiveZWall.Node->Local().scale.x, 6.4f, 0.001f, "Z壁を外周X幅まで伸ばす" );
			Harness.CheckEqualF32( Room.PositiveZWall.Node->Local().scale.y, 3.0f, "Z壁へ壁高を使う" );
			Harness.CheckEqualF32( Room.PositiveZWall.Node->Local().scale.z, 0.2f, "Z壁へ壁厚を使う" );
		}

		if ( Room.NegativeZWall.Node != nullptr )
		{
			Harness.CheckNearF32( Room.NegativeZWall.Node->Local().position.z, 0.9f, 0.001f,
				"Z負壁を内寸の外側へ置く" );
		}

		if ( Room.PositiveXWall.Node != nullptr )
		{
			Harness.CheckNearF32( Room.PositiveXWall.Node->Local().position.x, 4.1f, 0.001f,
				"X正壁を内寸の外側へ置く" );
			Harness.CheckEqualF32( Room.PositiveXWall.Node->Local().scale.x, 0.2f, "X壁へ壁厚を使う" );
			Harness.CheckEqualF32( Room.PositiveXWall.Node->Local().scale.z, 4.0f, "X壁へ内寸Zを使う" );
		}

		if ( Room.NegativeXWall.Node != nullptr )
		{
			Harness.CheckNearF32( Room.NegativeXWall.Node->Local().position.x, -2.1f, 0.001f,
				"X負壁を内寸の外側へ置く" );
		}

		const AMeshComponent3D* const FloorMesh = RoomMeshOf( Room.Floor );
		const AMeshComponent3D* const WallMesh = RoomMeshOf( Room.PositiveZWall );
		Harness.Check( FloorMesh != nullptr && FloorMesh->Primitive() == EMeshPrimitive3D::Plane,
			"床は平面表示を使う" );
		Harness.Check( WallMesh != nullptr && WallMesh->Primitive() == EMeshPrimitive3D::Cube,
			"壁は立方体表示を使う" );
		if ( FloorMesh != nullptr )
		{
			Harness.CheckEqualF32( FloorMesh->Color().x, 0.18f, "指定した床色を使う" );
			Harness.CheckEqualF32( FloorMesh->Material().pbr.metallic, 0.1f, "指定した床金属度を使う" );
			Harness.CheckEqualF32( FloorMesh->Material().pbr.roughness, 0.7f, "指定した床粗さを使う" );
			Harness.Check( FloorMesh->CastsShadow(), "指定した床の影設定を使う" );
		}
		if ( WallMesh != nullptr )
		{
			Harness.CheckEqualF32( WallMesh->Color().x, 0.62f, "指定した壁色を使う" );
			Harness.CheckEqualF32( WallMesh->Material().pbr.metallic, 0.2f, "指定した壁金属度を使う" );
			Harness.CheckEqualF32( WallMesh->Material().pbr.roughness, 0.4f, "指定した壁粗さを使う" );
			Harness.Check( !WallMesh->CastsShadow(), "指定した壁の影設定を使う" );
		}

		FWorldCollisionShape3D FloorShape;
		FWorldCollisionShape3D WallShape;
		Harness.Check( Collision.TryGetWorldShape( Room.Floor.Shape, FloorShape ), "床のworld衝突を読める" );
		Harness.Check( Collision.TryGetWorldShape( Room.PositiveXWall.Shape, WallShape ), "壁のworld衝突を読める" );
		Harness.Check( FloorShape.Kind == FWorldCollisionShape3D::EKind::Box
			&& WallShape.Kind == FWorldCollisionShape3D::EKind::Box, "床と壁の衝突は箱" );
		Harness.CheckNearF32( FloorShape.Box.center.y, 1.75f, 0.001f, "床衝突を上面から下へ持たせる" );
		Harness.CheckNearF32( FloorShape.Box.half_size.x, 3.2f, 0.001f, "床衝突を外周X幅へ揃える" );
		Harness.CheckNearF32( WallShape.Box.center.x, 4.1f, 0.001f, "壁衝突中心を表示中心へ揃える" );
		Harness.CheckNearF32( WallShape.Box.half_size.z, 2.0f, 0.001f, "X壁衝突のZ半寸法" );
		Harness.Check( FloorShape.Layer == 0x4u && WallShape.Layer == 0x4u,
			"床と壁へ同じ衝突レイヤーを使う" );

		Harness.Check( CRoom3DSpawner::Destroy( Graph, Collision, Room ), "確認後の部屋を片付けられる" );
	}

	Harness.BeginSuite( "CRoom3DSpawner / 指定親の座標を5個へ共通適用する" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		const FScene3DSpawnResult ParentResult = Graph.TrySpawn( FStringView( "RoomRoot" ) );
		Harness.Check( ParentResult.Succeeded(), "部屋を繋ぐ親を作れる" );
		if ( ParentResult.Node != nullptr ) ParentResult.Node->SetPosition( FVec3{ 10.0f, 1.0f, -5.0f } );

		FRoom3DSpawnParams Params = FRoom3DSpawnParams::FromInnerSize(
			FVec2{ 6.0f, 4.0f }, 3.0f, FVec3{ 1.0f, 2.0f, 3.0f } );
		Params.FloorThickness = 0.5f;
		FRoom3DSpawnResult Room = CRoom3DSpawner::SpawnInto(
			Graph, Collision, Params, ParentResult.Node );
		Harness.Check( Room.Succeeded(), "指定親の下へ部屋を置ける" );
		if ( ParentResult.Node != nullptr )
		{
			Harness.CheckEqualU64( ParentResult.Node->ChildCount(), 5u, "親の直下へ5個を置く" );
			Harness.Check( AllPartsUseParent( Room, *ParentResult.Node ), "全5個が同じ親を使う" );
		}

		FWorldCollisionShape3D FloorShape;
		Harness.Check( Collision.TryGetWorldShape( Room.Floor.Shape, FloorShape ), "親移動後の床衝突を読める" );
		Harness.CheckNearF32( FloorShape.Box.center.x, 11.0f, 0.001f, "親のX移動を床へ反映する" );
		Harness.CheckNearF32( FloorShape.Box.center.y, 2.75f, 0.001f, "親のY移動を床へ反映する" );
		Harness.CheckNearF32( FloorShape.Box.center.z, -2.0f, 0.001f, "親のZ移動を床へ反映する" );
		Harness.Check( CRoom3DSpawner::Destroy( Graph, Collision, Room ), "親付き部屋を片付けられる" );
	}

	Harness.BeginSuite( "CRoom3DSpawner / 床と四方の壁を同期更新する" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		const FScene3DSpawnResult Parent = Graph.TrySpawn(
			FStringView( "RoomRoot" ) );
		FRoom3DSpawnResult Room = CRoom3DSpawner::SpawnInto(
			Graph, Collision, FRoom3DSpawnParams{}, Parent.Node );
		ANode* const Floor = Room.Floor.Node;
		ANode* const PositiveZWall = Room.PositiveZWall.Node;
		ANode* const NegativeZWall = Room.NegativeZWall.Node;
		ANode* const PositiveXWall = Room.PositiveXWall.Node;
		ANode* const NegativeXWall = Room.NegativeXWall.Node;
		const FNodeId FloorId = Graph.IdOf( Floor );
		const FNodeId PositiveZWallId = Graph.IdOf( PositiveZWall );
		const FNodeId NegativeZWallId = Graph.IdOf( NegativeZWall );
		const FNodeId PositiveXWallId = Graph.IdOf( PositiveXWall );
		const FNodeId NegativeXWallId = Graph.IdOf( NegativeXWall );
		const FCollisionShapeId3D FloorShapeId = Room.Floor.Shape;
		const FCollisionShapeId3D PositiveZShapeId = Room.PositiveZWall.Shape;
		const FCollisionShapeId3D NegativeZShapeId = Room.NegativeZWall.Shape;
		const FCollisionShapeId3D PositiveXShapeId = Room.PositiveXWall.Shape;
		const FCollisionShapeId3D NegativeXShapeId = Room.NegativeXWall.Shape;

		FRoom3DSpawnParams Updated = FRoom3DSpawnParams::FromInnerSize(
			FVec2{ 10.0f, 6.0f }, 4.0f, FVec3{ 2.0f, 1.0f, -3.0f } );
		Updated.WallThickness = 0.4f;
		Updated.FloorThickness = 0.8f;
		Updated.FloorColor = FVec4{ 0.12f, 0.24f, 0.36f, 1.0f };
		Updated.WallColor = FVec4{ 0.72f, 0.60f, 0.48f, 1.0f };
		Updated.FloorMetallic = 0.15f;
		Updated.FloorRoughness = 0.75f;
		Updated.WallMetallic = 0.35f;
		Updated.WallRoughness = 0.25f;
		Updated.bFloorCastsShadow = true;
		Updated.bWallsCastShadow = false;
		Updated.CollisionLayer = 0x40u;

		Harness.Check( CRoom3DSpawner::TryApplyTo(
			Graph, Collision, Room, Updated ),
			"有効な新指定を床と四方の壁へ一括反映できる" );
		Harness.Check( Room.Floor.Shape == FloorShapeId
			&& Room.PositiveZWall.Shape == PositiveZShapeId
			&& Room.NegativeZWall.Shape == NegativeZShapeId
			&& Room.PositiveXWall.Shape == PositiveXShapeId
			&& Room.NegativeXWall.Shape == NegativeXShapeId,
			"更新しても5形状の世代付き番号を保つ" );
		Harness.Check( Graph.IdOf( Floor ) == FloorId
			&& Graph.IdOf( PositiveZWall ) == PositiveZWallId
			&& Graph.IdOf( NegativeZWall ) == NegativeZWallId
			&& Graph.IdOf( PositiveXWall ) == PositiveXWallId
			&& Graph.IdOf( NegativeXWall ) == NegativeXWallId,
			"更新しても5ノードの世代付き番号を保つ" );
		Harness.CheckEqualU64( Graph.RegisteredCount(), 7u,
			"更新で5ノードを作り直さない" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 5u,
			"更新で5形状を作り直さない" );
		Harness.Check( Parent.Node != nullptr
			&& AllPartsUseParent( Room, *Parent.Node ),
			"更新後も5部分の共通親を保つ" );

		CheckVector( Harness,
			Floor != nullptr ? Floor->Local().position : FVec3{},
			FVec3{ 2.0f, 1.0f, -3.0f },
			"床を新しい上面中心へ移す" );
		CheckVector( Harness,
			Floor != nullptr ? Floor->Local().scale : FVec3{},
			FVec3{ 10.8f, 0.8f, 6.8f },
			"床を壁厚込みの新しい外寸へ更新する" );
		CheckVector( Harness,
			PositiveZWall != nullptr
				? PositiveZWall->Local().position : FVec3{},
			FVec3{ 2.0f, 3.0f, 0.2f },
			"Z正壁を新しい内寸の外側へ移す" );
		CheckVector( Harness,
			NegativeZWall != nullptr
				? NegativeZWall->Local().position : FVec3{},
			FVec3{ 2.0f, 3.0f, -6.2f },
			"Z負壁を新しい内寸の外側へ移す" );
		CheckVector( Harness,
			PositiveZWall != nullptr
				? PositiveZWall->Local().scale : FVec3{},
			FVec3{ 10.8f, 4.0f, 0.4f },
			"Z壁を新しい外周幅と壁寸法へ更新する" );
		CheckVector( Harness,
			PositiveXWall != nullptr
				? PositiveXWall->Local().position : FVec3{},
			FVec3{ 7.2f, 3.0f, -3.0f },
			"X正壁を新しい内寸の外側へ移す" );
		CheckVector( Harness,
			NegativeXWall != nullptr
				? NegativeXWall->Local().position : FVec3{},
			FVec3{ -3.2f, 3.0f, -3.0f },
			"X負壁を新しい内寸の外側へ移す" );
		CheckVector( Harness,
			PositiveXWall != nullptr
				? PositiveXWall->Local().scale : FVec3{},
			FVec3{ 0.4f, 4.0f, 6.0f },
			"X壁を新しい壁厚、高さ、内寸Zへ更新する" );
		Harness.Check( Floor != nullptr
			&& Floor->Name() == FStringView( "RoomFloor" )
			&& PositiveZWall != nullptr
			&& PositiveZWall->Name() == FStringView( "RoomPositiveZWall" )
			&& NegativeZWall != nullptr
			&& NegativeZWall->Name() == FStringView( "RoomNegativeZWall" )
			&& PositiveXWall != nullptr
			&& PositiveXWall->Name() == FStringView( "RoomPositiveXWall" )
			&& NegativeXWall != nullptr
			&& NegativeXWall->Name() == FStringView( "RoomNegativeXWall" ),
			"更新後も5部分の役割名を揃える" );

		const AMeshComponent3D* const FloorMesh = RoomMeshOf( Room.Floor );
		const AMeshComponent3D* const WallMesh = RoomMeshOf( Room.PositiveZWall );
		Harness.Check( FloorMesh != nullptr
			&& FloorMesh->Primitive() == EMeshPrimitive3D::Plane
			&& FloorMesh->CastsShadow(),
			"床の平面表示と影設定を同期更新する" );
		Harness.Check( WallMesh != nullptr
			&& WallMesh->Primitive() == EMeshPrimitive3D::Cube
			&& !WallMesh->CastsShadow(),
			"壁の立方体表示と影設定を同期更新する" );
		if ( FloorMesh != nullptr )
		{
			Harness.CheckEqualF32( FloorMesh->Color().z, 0.36f,
				"床へ新しい表面色を反映する" );
			Harness.CheckEqualF32( FloorMesh->Material().pbr.metallic, 0.15f,
				"床へ新しい金属度を反映する" );
			Harness.CheckEqualF32( FloorMesh->Material().pbr.roughness, 0.75f,
				"床へ新しい粗さを反映する" );
		}
		if ( WallMesh != nullptr )
		{
			Harness.CheckEqualF32( WallMesh->Color().x, 0.72f,
				"四方の壁へ新しい表面色を反映する" );
			Harness.CheckEqualF32( WallMesh->Material().pbr.metallic, 0.35f,
				"四方の壁へ新しい金属度を反映する" );
			Harness.CheckEqualF32( WallMesh->Material().pbr.roughness, 0.25f,
				"四方の壁へ新しい粗さを反映する" );
		}

		FWorldCollisionShape3D FloorWorldShape;
		FWorldCollisionShape3D PositiveXWorldShape;
		Harness.Check( Collision.TryGetWorldShape(
			Room.Floor.Shape, FloorWorldShape )
			&& Collision.TryGetWorldShape(
				Room.PositiveXWall.Shape, PositiveXWorldShape ),
			"更新後の床と壁のworld衝突を読める" );
		CheckVector( Harness, FloorWorldShape.Box.center,
			FVec3{ 2.0f, 0.6f, -3.0f },
			"床衝突を新しい上面直下へ揃える" );
		CheckVector( Harness, FloorWorldShape.Box.half_size,
			FVec3{ 5.4f, 0.4f, 3.4f },
			"床衝突を新しい外寸と厚みへ揃える" );
		CheckVector( Harness, PositiveXWorldShape.Box.center,
			FVec3{ 7.2f, 3.0f, -3.0f },
			"X正壁の衝突中心を表示へ揃える" );
		CheckVector( Harness, PositiveXWorldShape.Box.half_size,
			FVec3{ 0.2f, 2.0f, 3.0f },
			"X正壁の衝突半寸法を表示へ揃える" );
		Harness.Check( FloorWorldShape.Layer == 0x40u
			&& PositiveXWorldShape.Layer == 0x40u,
			"床と四方の壁を新しい衝突レイヤーへ更新する" );
	}

	Harness.BeginSuite( "CRoom3DSpawner / 更新失敗時は5部分を変えない" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		const FScene3DSpawnResult Parent = Graph.TrySpawn(
			FStringView( "RoomRoot" ) );
		const FScene3DSpawnResult OtherParent = Graph.TrySpawn(
			FStringView( "OtherRoot" ) );
		FRoom3DSpawnParams Initial;
		Initial.FloorTopPosition = FVec3{ 1.0f, 0.5f, 2.0f };
		Initial.CollisionLayer = 0x2u;
		FRoom3DSpawnResult Room = CRoom3DSpawner::SpawnInto(
			Graph, Collision, Initial, Parent.Node );
		ANode* const Floor = Room.Floor.Node;
		ANode* const PositiveZWall = Room.PositiveZWall.Node;
		ANode* const PositiveXWall = Room.PositiveXWall.Node;
		ANode* const NegativeXWall = Room.NegativeXWall.Node;
		const FVec3 FloorPosition = Floor != nullptr
			? Floor->Local().position : FVec3{};
		const FVec3 PositiveZPosition = PositiveZWall != nullptr
			? PositiveZWall->Local().position : FVec3{};
		const FVec3 PositiveXPosition = PositiveXWall != nullptr
			? PositiveXWall->Local().position : FVec3{};

		FRoom3DSpawnParams Updated = FRoom3DSpawnParams::FromInnerSize(
			FVec2{ 12.0f, 7.0f }, 4.0f, FVec3{ 8.0f, 1.0f, -4.0f } );
		Updated.CollisionLayer = 0x80u;
		FRoom3DSpawnParams Invalid = Updated;
		Invalid.InnerSize.x = 0.0f;
		Harness.Check( !CRoom3DSpawner::TryApplyTo(
			Graph, Collision, Room, Invalid ),
			"不正な新指定を拒否する" );
		CheckVector( Harness,
			Floor != nullptr ? Floor->Local().position : FVec3{},
			FloorPosition, "不正入力では床を変えない" );
		CheckVector( Harness,
			PositiveZWall != nullptr
				? PositiveZWall->Local().position : FVec3{},
			PositiveZPosition, "不正入力ではZ壁を変えない" );

		CSceneNodeGraph WrongGraph;
		CSceneCollision3D WrongCollision{ WrongGraph };
		Harness.Check( !CRoom3DSpawner::TryApplyTo(
			WrongGraph, WrongCollision, Room, Updated ),
			"別場面からの更新を拒否する" );
		CSceneCollision3D EmptyCollision{ Graph };
		Harness.Check( !CRoom3DSpawner::TryApplyTo(
			Graph, EmptyCollision, Room, Updated ),
			"5形状を持たない衝突集合を拒否する" );

		if ( NegativeXWall != nullptr && OtherParent.Node != nullptr )
			NegativeXWall->Reparent( *OtherParent.Node );
		Graph.ResolveStructuralChanges();
		Harness.Check( NegativeXWall != nullptr
			&& NegativeXWall->Parent() == OtherParent.Node,
			"共通親を崩す更新検証を準備できる" );
		Harness.Check( !CRoom3DSpawner::TryApplyTo(
			Graph, Collision, Room, Updated ),
			"5部分が共通親を失った更新を拒否する" );
		CheckVector( Harness,
			PositiveXWall != nullptr
				? PositiveXWall->Local().position : FVec3{},
			PositiveXPosition, "共通親不一致ではX壁を変えない" );
		if ( NegativeXWall != nullptr && Parent.Node != nullptr )
			NegativeXWall->Reparent( *Parent.Node );
		Graph.ResolveStructuralChanges();

		FRoom3DSpawnResult Forged = Room;
		Forged.PositiveXWall.Shape = Room.NegativeXWall.Shape;
		Harness.Check( !CRoom3DSpawner::TryApplyTo(
			Graph, Collision, Forged, Updated ),
			"重複した形状番号を更新前に拒否する" );
		Forged = Room;
		Forged.NegativeZWall.Node = Room.PositiveZWall.Node;
		Harness.Check( !CRoom3DSpawner::TryApplyTo(
			Graph, Collision, Forged, Updated ),
			"別部品をZ負壁に見せた結果を更新前に拒否する" );

		FWorldCollisionShape3D FloorWorldShape;
		FWorldCollisionShape3D PositiveZWorldShape;
		Harness.Check( Collision.TryGetWorldShape(
			Room.Floor.Shape, FloorWorldShape )
			&& Collision.TryGetWorldShape(
				Room.PositiveZWall.Shape, PositiveZWorldShape ),
			"全失敗後も元の形状を読める" );
		Harness.Check( FloorWorldShape.Layer == 0x2u
			&& PositiveZWorldShape.Layer == 0x2u,
			"全失敗後も元の衝突レイヤーを保つ" );
		CheckVector( Harness, FloorWorldShape.Box.center,
			FVec3{ 1.0f, 0.25f, 2.0f },
			"全失敗後も元の床衝突中心を保つ" );
		const AMeshComponent3D* const FloorMesh = RoomMeshOf( Room.Floor );
		Harness.Check( FloorMesh != nullptr
			&& FloorMesh->Color().x == Initial.FloorColor.x,
			"全失敗後も元の床表面色を保つ" );
		Harness.Check( Floor != nullptr
			&& Floor->Name() == FStringView( "RoomFloor" )
			&& PositiveZWall != nullptr
			&& PositiveZWall->Name() == FStringView( "RoomPositiveZWall" ),
			"全失敗後も5部分の名前を保つ" );

		const FNodeId ParentId = Graph.IdOf( Parent.Node );
		Harness.Check( Graph.Destroy( ParentId ),
			"破棄予定祖先の更新検証を準備できる" );
		Harness.Check( !CRoom3DSpawner::TryApplyTo(
			Graph, Collision, Room, Updated ),
			"共通親が破棄予定なら5部分更新を拒否する" );
		CheckVector( Harness,
			Floor != nullptr ? Floor->Local().position : FVec3{},
			FloorPosition, "破棄予定祖先による失敗でも床を変えない" );
		Harness.Check( !CRoom3DSpawner::TryApplyTo(
			Graph, Collision, FRoom3DSpawnResult{}, Updated ),
			"空の部屋結果を拒否する" );
	}

	Harness.BeginSuite( "CRoom3DSpawner / 不正入力と別場面で半端物を残さない" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		FRoom3DSpawnParams Broken;
		Broken.WallHeight = 0.0f;

		const FRoom3DSpawnResult Failed = CRoom3DSpawner::SpawnInto(
			Graph, Collision, Broken );
		Harness.Check( !Failed.Succeeded() && Failed.IsEmpty(), "不正値を生成前に拒否する" );
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 0u, "不正値でノードを足さない" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 0u, "不正値で形状を足さない" );
	}

	{
		CSceneNodeGraph Graph;
		CSceneNodeGraph OtherGraph;
		CSceneCollision3D OtherCollision{ OtherGraph };

		const FRoom3DSpawnResult Failed = CRoom3DSpawner::SpawnInto(
			Graph, OtherCollision, FRoom3DSpawnParams{} );
		Harness.Check( !Failed.Succeeded() && Failed.IsEmpty(), "別場面の衝突集合を拒否する" );
		Graph.ResolveStructuralChanges();
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 0u, "登録失敗時に床ノードを巻き戻す" );
		Harness.CheckEqualU64( Graph.RegisteredCount(), 1u, "登録失敗時に識別子も解放する" );
		Harness.CheckEqualU64( OtherCollision.ShapeCount(), 0u, "別場面へ形状を残さない" );
	}

	Harness.BeginSuite( "CRoom3DSpawner / 5組を検証してから一括破棄する" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		FRoom3DSpawnResult Room = CRoom3DSpawner::SpawnInto(
			Graph, Collision, FRoom3DSpawnParams{} );
		Harness.Check( Room.Succeeded(), "破棄確認用の部屋を置ける" );
		Harness.Check( CRoom3DSpawner::Destroy( Graph, Collision, Room ), "部屋全体を一括破棄できる" );
		Harness.Check( Room.IsEmpty() && !Room.Succeeded(), "成功時だけ呼出側結果を空にする" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 0u, "5形状を直ちに外す" );
		Graph.ResolveStructuralChanges();
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 0u, "5ノードを残さない" );
		Harness.CheckEqualU64( Graph.RegisteredCount(), 1u, "5個の識別子を解放する" );
		Harness.Check( !CRoom3DSpawner::Destroy( Graph, Collision, Room ), "空結果の二重破棄を拒否する" );
	}

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		FRoom3DSpawnResult Room = CRoom3DSpawner::SpawnInto(
			Graph, Collision, FRoom3DSpawnParams{} );
		const FCollisionShapeId3D OriginalShape = Room.NegativeZWall.Shape;
		Room.NegativeZWall.Shape = Room.PositiveZWall.Shape;

		Harness.Check( !CRoom3DSpawner::Destroy( Graph, Collision, Room ),
			"重複した形状番号を破棄前に拒否する" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 5u, "重複結果でも形状を外さない" );
		Harness.Check( NoPartIsPendingDestroy( Room ), "重複結果でもノードを破棄予定にしない" );

		Room.NegativeZWall.Shape = OriginalShape;
		Harness.Check( CRoom3DSpawner::Destroy( Graph, Collision, Room ), "結果を戻せば片付けられる" );
	}

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		FRoom3DSpawnResult Room = CRoom3DSpawner::SpawnInto(
			Graph, Collision, FRoom3DSpawnParams{} );
		const FCollisionShapeId3D PositiveShape = Room.PositiveXWall.Shape;
		Room.PositiveXWall.Shape = Room.NegativeXWall.Shape;
		Room.NegativeXWall.Shape = PositiveShape;

		Harness.Check( !CRoom3DSpawner::Destroy( Graph, Collision, Room ),
			"別ノードの形状対を破棄前に拒否する" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 5u, "不正な対応でも形状を外さない" );
		Harness.Check( NoPartIsPendingDestroy( Room ), "不正な対応でもノードを破棄予定にしない" );

		Room.NegativeXWall.Shape = Room.PositiveXWall.Shape;
		Room.PositiveXWall.Shape = PositiveShape;
		Harness.Check( CRoom3DSpawner::Destroy( Graph, Collision, Room ), "対応を戻せば片付けられる" );
	}

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		CSceneNodeGraph OtherGraph;
		CSceneCollision3D OtherCollision{ OtherGraph };
		FRoom3DSpawnResult Room = CRoom3DSpawner::SpawnInto(
			Graph, Collision, FRoom3DSpawnParams{} );

		Harness.Check( !CRoom3DSpawner::Destroy( OtherGraph, OtherCollision, Room ),
			"別場面からの破棄を拒否する" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 5u, "元場面の形状を保つ" );
		Harness.Check( NoPartIsPendingDestroy( Room ), "元場面のノードを保つ" );
		Harness.Check( CRoom3DSpawner::Destroy( Graph, Collision, Room ), "元場面なら片付けられる" );
	}

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		FRoom3DSpawnResult Room = CRoom3DSpawner::SpawnInto(
			Graph, Collision, FRoom3DSpawnParams{} );
		const FNodeId FloorNode = Graph.IdOf( Room.Floor.Node );
		Harness.Check( Graph.Destroy( FloorNode ), "床を先に破棄予定へ移せる" );
		Harness.Check( CRoom3DSpawner::Destroy( Graph, Collision, Room ),
			"一部が破棄予定でも残りと形状を片付けられる" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 0u, "破棄予定を含む5形状を外す" );
	}
}
