// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Character3D/CharacterMover3D.h"
#include "Common/Test/TestHarness.h"

#include <limits>

namespace
{
	/** 名前付きノードを指定親へ置く。 */
	ANode* SpawnNode( CSceneNodeGraph& Graph, const char* Name, ANode* Parent = nullptr ) noexcept
	{
		const FScene3DSpawnResult Spawned = Graph.TrySpawn( FStringView( Name ), Parent );
		return Spawned ? Spawned.Node : nullptr;
	}

	/** 上面Y=0の広い床を衝突集合へ登録する。 */
	FCollisionShapeId3D AddFloor( CSceneNodeGraph& Graph, CSceneCollision3D& Collision ) noexcept
	{
		ANode* const Floor = SpawnNode( Graph, "CharacterFloor" );
		if ( Floor == nullptr ) return {};
		Floor->SetPosition( FVec3{ 0.0f, -0.5f, 0.0f } );
		return Collision.TryAddBox( *Floor, FVec3{}, FVec3{ 10.0f, 0.5f, 10.0f }, 0x1u );
	}

	/** 小さな浮動小数誤差を許して比較する。 */
	void CheckNear( CTestHarness& Harness, f32 Actual, f32 Expected, const char* Label ) noexcept
	{
		const f32 Difference = Actual > Expected ? Actual - Expected : Expected - Actual;
		Harness.Check( Difference <= 1.0e-4f, Label );
	}
}


void RunCharacterMover3DTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "CSceneCollision3D / ACSのキャラクター次状態を同期済み形状から求める" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		Harness.Check( AddFloor( Graph, Collision ).IsValid(), "床を登録できる" );

		FKinematicCharacterState3D State;
		State.Position = FVec3{ 0.0f, 0.501f, 0.0f };
		FKinematicCharacterMovementParams3D Params;
		Params.GravityAcceleration = 0.0f;
		FKinematicCharacterMovementResult3D Result;

		Harness.Check( Collision.TryMoveCharacter( {}, State, 0.0f, Params, Result ), "同期した床から次状態を計算できる" );
		Harness.Check( Result.NextState.Grounded && Result.HitGround, "床への接地を返す" );
		CheckNear( Harness, Result.NextState.Position.y, 0.501f, "接触間隔を保つ" );
	}

	Harness.BeginSuite( "CCharacterMover3D / 足元原点のノードを床上で動かしてジャンプする" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		Harness.Check( AddFloor( Graph, Collision ).IsValid(), "移動用の床を登録できる" );
		ANode* const Player = SpawnNode( Graph, "Character" );
		Harness.Check( Player != nullptr, "移動対象を置ける" );
		if ( Player == nullptr ) return;
		Player->SetPosition( FVec3{ 0.0f, 0.001f, 0.0f } );

		FKinematicCharacterMovementParams3D Params;
		Params.GravityAcceleration = 0.0f;
		CCharacterMover3D Mover;
		Harness.Check( Mover.Bind( Collision, *Player, FVec3{ 0.0f, 0.5f, 0.0f }, Params ), "足元から球中心を指定して接続できる" );
		Mover.SetCollisionFilter( {}, 0x1u );
		Harness.Check( Mover.Move( FVec2{ 2.0f, 0.0f }, false, 0.5f ), "床上を進める" );
		CheckNear( Harness, Player->World().position.x, 1.0f, "水平移動をノードへ反映する" );
		CheckNear( Harness, Player->World().position.y, 0.001f, "足元の高さを保つ" );
		Harness.Check( Mover.IsGrounded(), "接地状態を保持する" );

		Params.GravityAcceleration = 10.0f;
		Params.JumpSpeed = 5.0f;
		Harness.Check( Mover.SetMovementParams( Params ), "ジャンプ設定へ変えられる" );
		Harness.Check( Mover.Move( FVec2{}, true, 0.1f ), "接地中にジャンプできる" );
		Harness.Check( Mover.LastResult().Jumped && !Mover.IsGrounded(), "ジャンプ事象と空中状態を返す" );
		CheckNear( Harness, Player->World().position.y, 0.401f, "上向き移動を足元へ反映する" );
		CheckNear( Harness, Mover.Velocity().y, 4.0f, "重力後の上向き速度を保持する" );
	}

	Harness.BeginSuite( "CCharacterMover3D / 世界移動を親ノードの座標へ戻す" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		ANode* const Parent = SpawnNode( Graph, "CharacterParent" );
		ANode* const Player = Parent != nullptr ? SpawnNode( Graph, "NestedCharacter", Parent ) : nullptr;
		Harness.Check( Parent != nullptr && Player != nullptr, "親子ノードを置ける" );
		if ( Parent == nullptr || Player == nullptr ) return;
		Parent->SetPosition( FVec3{ 10.0f, 2.0f, -3.0f } );
		Parent->RotateDeg( FVec3{ 0.0f, 90.0f, 0.0f } );

		FKinematicCharacterMovementParams3D Params;
		Params.GravityAcceleration = 0.0f;
		CCharacterMover3D Mover;
		Harness.Check( Mover.Bind( Collision, *Player, FVec3{}, Params ), "親を持つノードへ接続できる" );
		const FVec3 Before = Player->World().position;
		Harness.Check( Mover.Move( FVec2{ 1.0f, 0.0f }, false, 1.0f ), "世界X方向へ進める" );
		const FVec3 After = Player->World().position;
		CheckNear( Harness, After.x, Before.x + 1.0f, "親の回転に依存せず世界Xへ進む" );
		CheckNear( Harness, After.y, Before.y, "世界Yを保つ" );
		CheckNear( Harness, After.z, Before.z, "世界Zを保つ" );
	}

	Harness.BeginSuite( "CCharacterMover3D / 操作量をカメラ基準の水平移動へ変える" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		ANode* const Player = SpawnNode( Graph, "CameraRelativeCharacter" );
		Harness.Check( Player != nullptr, "カメラ基準移動の対象を置ける" );
		if ( Player == nullptr ) return;

		FKinematicCharacterMovementParams3D Params;
		Params.GravityAcceleration = 0.0f;
		CCharacterMover3D Mover;
		Harness.Check( Mover.Bind( Collision, *Player, FVec3{}, Params ), "カメラ基準移動へ接続できる" );

		CCamera Camera;
		Camera.SetLookAt( FVec3{ 0.0f, 2.0f, -5.0f }, FVec3{ 0.0f, 2.0f, 0.0f } );
		Harness.Check( Mover.MoveFromCamera( Camera, FVec2{ 0.0f, 1.0f }, 4.0f, false, 0.5f ), "カメラ前方へ進める" );
		CheckNear( Harness, Player->World().position.x, 0.0f, "正面入力で世界Xを保つ" );
		CheckNear( Harness, Player->World().position.z, 2.0f, "正面入力を世界Zへ反映する" );

		Camera.SetLookAt( FVec3{ -5.0f, 2.0f, 2.0f }, FVec3{ 0.0f, 2.0f, 2.0f } );
		Harness.Check( Mover.MoveFromCamera( Camera, FVec2{ 0.0f, 1.0f }, 2.0f, false, 0.5f ), "横を向いたカメラの前方へ進める" );
		CheckNear( Harness, Player->World().position.x, 1.0f, "カメラ前方の世界Xへ進む" );
		CheckNear( Harness, Player->World().position.z, 2.0f, "横向き時の世界Zを保つ" );

		const FVec3 BeforeDiagonal = Player->World().position;
		Harness.Check( Mover.MoveFromCamera( Camera, FVec2{ 1.0f, 1.0f }, 2.0f, false, 1.0f ), "斜め入力で進める" );
		const FVec3 DiagonalTranslation = Player->World().position - BeforeDiagonal;
		CheckNear( Harness, Length( DiagonalTranslation ), 2.0f, "斜め入力を最大速度へ制限する" );
	}

	Harness.BeginSuite( "CCharacterMover3D / 実際の移動方向へ滑らかに向く" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		ANode* const Player = SpawnNode( Graph, "TurningCharacter" );
		Harness.Check( Player != nullptr, "向き制御の対象を置ける" );
		if ( Player == nullptr ) return;

		FKinematicCharacterMovementParams3D Params;
		Params.GravityAcceleration = 0.0f;
		CCharacterMover3D Mover;
		Harness.Check( Mover.Bind( Collision, *Player, FVec3{}, Params ), "向き制御へ接続できる" );
		Harness.Check( Mover.Move( FVec2{ 1.0f, 0.0f }, false, 1.0f ), "世界Xへ移動できる" );
		Harness.Check( Mover.TurnTowardMovement( 45.0f, 1.0f ), "1秒分だけ世界Xへ向ける" );
		FVec3 Forward = Rotate( Player->World().rotation, FVec3::Forward() );
		CheckNear( Harness, Forward.x, 0.7071068f, "45度回転後の前方向X" );
		CheckNear( Harness, Forward.z, 0.7071068f, "45度回転後の前方向Z" );

		Harness.Check( Mover.TurnTowardMovement( 45.0f, 1.0f ), "残り45度を回せる" );
		Forward = Rotate( Player->World().rotation, FVec3::Forward() );
		CheckNear( Harness, Forward.x, 1.0f, "移動方向へ到達する" );
		CheckNear( Harness, Forward.z, 0.0f, "回り過ぎない" );

		const FQuat RotationBeforeStop = Player->Local().rotation;
		Harness.Check( Mover.Move( FVec2{}, false, 1.0f ) && Mover.TurnTowardMovement( 360.0f, 1.0f ), "停止中も正常に処理できる" );
		Harness.Check( Player->Local().rotation.x == RotationBeforeStop.x && Player->Local().rotation.y == RotationBeforeStop.y && Player->Local().rotation.z == RotationBeforeStop.z && Player->Local().rotation.w == RotationBeforeStop.w, "停止中は回転を変更しない" );
	}

	Harness.BeginSuite( "CCharacterMover3D / 親が回っていても世界の移動方向へ向く" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		ANode* const Parent = SpawnNode( Graph, "TurningParent" );
		ANode* const Player = Parent != nullptr ? SpawnNode( Graph, "NestedTurningCharacter", Parent ) : nullptr;
		Harness.Check( Parent != nullptr && Player != nullptr, "向き制御用の親子ノードを置ける" );
		if ( Parent == nullptr || Player == nullptr ) return;
		Parent->RotateDeg( FVec3{ 0.0f, 90.0f, 0.0f } );

		FKinematicCharacterMovementParams3D Params;
		Params.GravityAcceleration = 0.0f;
		CCharacterMover3D Mover;
		Harness.Check( Mover.Bind( Collision, *Player, FVec3{}, Params ), "親付きの向き制御へ接続できる" );
		Harness.Check( Mover.Move( FVec2{ 0.0f, 1.0f }, false, 1.0f ), "世界Zへ移動できる" );
		Harness.Check( Mover.TurnTowardMovement( 360.0f, 1.0f ), "親座標へ戻して世界Zを向ける" );
		const FVec3 Forward = Rotate( Player->World().rotation, FVec3::Forward() );
		CheckNear( Harness, Forward.x, 0.0f, "親回転後の世界前方向X" );
		CheckNear( Harness, Forward.z, 1.0f, "親回転後も世界Zを向く" );
	}

	Harness.BeginSuite( "CCharacterMover3D / 不正値と解除でノードと状態を保つ" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		ANode* const Player = SpawnNode( Graph, "SafeCharacter" );
		Harness.Check( Player != nullptr, "安全性確認用ノードを置ける" );
		if ( Player == nullptr ) return;

		CCharacterMover3D Mover;
		Harness.Check( Mover.Bind( Collision, *Player ), "既定値で接続できる" );
		const FVec3 PositionBefore = Player->Position();
		const FKinematicCharacterState3D StateBefore = Mover.State();
		FKinematicCharacterMovementParams3D BrokenParams = Mover.MovementParams();
		BrokenParams.ContactOffset = BrokenParams.Radius;
		Harness.Check( !Mover.SetMovementParams( BrokenParams ), "半径以上の接触間隔を拒否する" );
		CheckNear( Harness, Mover.MovementParams().ContactOffset, 0.001f, "不正設定で既存値を保つ" );

		const f32 Infinity = std::numeric_limits<f32>::infinity();
		Harness.Check( !Mover.Move( FVec2{}, false, Infinity ), "有限でない経過秒を拒否する" );
		CheckNear( Harness, Player->Position().x, PositionBefore.x, "失敗時にノードXを保つ" );
		CheckNear( Harness, Player->Position().y, PositionBefore.y, "失敗時にノードYを保つ" );
		CheckNear( Harness, Mover.State().Position.x, StateBefore.Position.x, "失敗時に状態を保つ" );

		CCamera VerticalCamera;
		VerticalCamera.SetLookAt( FVec3{}, FVec3{ 0.0f, 1.0f, 0.0f }, FVec3::Forward() );
		Harness.Check( !Mover.MoveFromCamera( VerticalCamera, FVec2{ 0.0f, 1.0f }, 4.0f, false, 1.0f ), "水平前方を作れないカメラを拒否する" );
		Harness.Check( !Mover.MoveFromCamera( VerticalCamera, FVec2{}, -1.0f, false, 1.0f ), "負の最大速度を拒否する" );
		CheckNear( Harness, Player->Position().x, PositionBefore.x, "カメラ変換失敗でもノードXを保つ" );
		CheckNear( Harness, Mover.State().Position.x, StateBefore.Position.x, "カメラ変換失敗でも状態を保つ" );
		const FQuat RotationBeforeFailure = Player->Local().rotation;
		Harness.Check( !Mover.TurnTowardMovement( Infinity, 1.0f ) && !Mover.TurnTowardMovement( 90.0f, -1.0f ), "不正な回転速度と時刻を拒否する" );
		Harness.Check( Player->Local().rotation.x == RotationBeforeFailure.x && Player->Local().rotation.y == RotationBeforeFailure.y && Player->Local().rotation.z == RotationBeforeFailure.z && Player->Local().rotation.w == RotationBeforeFailure.w, "向き制御失敗でも回転を保つ" );

		Harness.Check( !Mover.Bind( Collision, *Player, FVec3{ Infinity, 0.0f, 0.0f } ), "有限でない球中心を拒否する" );
		Harness.Check( Mover.IsBound(), "再接続失敗で既存接続を保つ" );
		Mover.Unbind();
		Harness.Check( !Mover.IsBound() && !Mover.Move( FVec2{}, false, 0.0f ), "解除後は移動しない" );
	}
}
