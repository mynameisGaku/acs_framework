// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/AcsFramework.h"
#include "Common/Test/TestHarness.h"

#include <limits>

namespace
{
	/** 統合操作の公開状態だけを検証する最小3D場面。 */
	class CTestThirdPersonScene3D final : public ALegacyScene3DAdapter
	{
	};

	/** 名前付きノードを場面へ置く。 */
	ANode* SpawnNode( CTestThirdPersonScene3D& Scene, const char* Name ) noexcept
	{
		const FScene3DSpawnResult Spawned = Scene.Graph().TrySpawn( FStringView( Name ) );
		return Spawned ? Spawned.Node : nullptr;
	}

	/** 上面Y=0の広い床を衝突集合へ登録する。 */
	FCollisionShapeId3D AddFloor( CTestThirdPersonScene3D& Scene, CSceneCollision3D& Collision ) noexcept
	{
		ANode* const Floor = SpawnNode( Scene, "ThirdPersonFloor" );
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

	/** 指定名のテスト用クリップをモデルへ追加する。 */
	void AddAnimation( ASkinnedMeshAsset& Mesh, const char* Name ) noexcept
	{
		FAnimation Animation;
		Animation.name = FString( Name );
		Animation.duration = 2.0f;
		Mesh.Animations().Add( Move( Animation ) );
	}

	/** 待機・歩き・走り・ジャンプを持つ最小の骨付きモデルを作る。 */
	TSharedPtr<ASkinnedMeshAsset> MakeCharacterMesh() noexcept
	{
		TSharedPtr<ASkinnedMeshAsset> Mesh = MakeShared<ASkinnedMeshAsset>();
		if ( !Mesh ) return Mesh;

		FSkinnedVertex Vertex{};
		Vertex.weights[0] = 1.0f;
		Mesh->Vertices().Add( Vertex );
		Mesh->Indices().Add( 0u );
		Mesh->Bones().Add( FBone{} );
		AddAnimation( *Mesh, "Idle" );
		AddAnimation( *Mesh, "Walk" );
		AddAnimation( *Mesh, "Run" );
		AddAnimation( *Mesh, "Jump" );
		return Mesh;
	}
}


void RunThirdPersonCharacter3DTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "CThirdPersonCharacter3D / 1入力で視点、移動、向き、追従を更新する" );

	{
		const FThirdPersonCharacter3DInput DefaultInput;
		Harness.Check( DefaultInput.IsValid() && DefaultInput.MoveAxes.x == 0.0f && DefaultInput.MoveAxes.y == 0.0f
			&& DefaultInput.LookAxes.x == 0.0f && DefaultInput.LookAxes.y == 0.0f && DefaultInput.ZoomAxis == 0.0f
			&& !DefaultInput.bJumpRequested && !DefaultInput.bRunRequested, "未指定の操作量を安全な無入力として初期化する" );
		const FThirdPersonCharacter3DParams LegacyOrderedParams{ FVec3{ 0.0f, 0.5f, 0.0f }, FKinematicCharacterMovementParams3D{}, 4.0f, 540.0f, FCollisionShapeId3D{}, CSceneCollision3D::kAllLayers, FNodeOrbitCamera3DParams{} };
		Harness.Check( LegacyOrderedParams.MaximumTurnDegreesPerSecond == 540.0f && LegacyOrderedParams.CollisionMask == CSceneCollision3D::kAllLayers && LegacyOrderedParams.RunSpeedMultiplier == 1.75f, "従来順の位置指定初期化を保ち、走行倍率だけ末尾の既定値を使う" );

		TUniquePtr<CTestThirdPersonScene3D> Scene = MakeUnique<CTestThirdPersonScene3D>();
		CSceneCollision3D Collision{ Scene->Graph() };
		Harness.Check( AddFloor( *Scene, Collision ).IsValid(), "移動用の床を登録できる" );
		ANode* const Character = SpawnNode( *Scene, "ThirdPersonCharacter" );
		Harness.Check( Character != nullptr, "操作対象を置ける" );
		if ( Character == nullptr ) return;
		Character->SetPosition( FVec3{ 0.0f, 0.001f, 0.0f } );

		FThirdPersonCharacter3DParams Params;
		Params.Movement.GravityAcceleration = 0.0f;
		Params.CollisionMask = 0x1u;
		CThirdPersonCharacter3D Controller;
		Harness.Check( Controller.Bind( Collision, *Scene, *Character, Params ), "移動と追従カメラへ一括接続できる" );
		Harness.Check( Controller.IsBound() && Controller.Character() == Character, "一括接続状態を確認できる" );
		Harness.Check( !Scene->FreeCameraEnabled() && Scene->OrbitCameraOverrideActive(), "接続中は追従カメラを明示選択する" );

		FActionInput CurrentInput;
		CurrentInput.SetAxis( 1u, 1.0f );
		CurrentInput.SetAxis( 2u, 0.5f );
		const FThirdPersonCharacter3DUpdateResult Result = Controller.Update( CurrentInput, FActionInput{}, 0.5f );
		Harness.Check( Result.Succeeded(), "汎用入力から必須の4段階を1回で完了する" );
		Harness.Check( !Result.bAnimationWasBound && Result.AnimationSucceeded(), "任意アニメーションなしでも成功する" );
		CheckNear( Harness, Controller.OrbitCamera().State().yaw_radians, 45.0f * kDeg2Rad, "視点操作を移動より先に反映する" );

		const FVec3 Position = Character->World().position;
		CheckNear( Harness, Length( FVec2{ Position.x, Position.z } ), 2.0f, "更新後カメラ基準で最大速度の0.5秒分進む" );
		const FVec3 Velocity = Controller.Mover().Velocity();
		CheckNear( Harness, Length( FVec2{ Velocity.x, Velocity.z } ), 4.0f, "確定した水平速度を公開アダプターから読める" );
		const FVec3 Forward = Rotate( Character->World().rotation, FVec3::Forward() );
		const f32 FacingAlignment = Dot( Normalize( FVec3{ Forward.x, 0.0f, Forward.z } ), Normalize( FVec3{ Velocity.x, 0.0f, Velocity.z } ) );
		Harness.Check( FacingAlignment > 0.999f, "実際の移動方向へ向く" );
		CheckNear( Harness, Controller.OrbitCamera().State().target.x, Position.x, "移動後の注視点Xを追う" );
		CheckNear( Harness, Controller.OrbitCamera().State().target.y, Position.y + 1.4f, "移動後の注視点高さを追う" );
		CheckNear( Harness, Controller.OrbitCamera().State().target.z, Position.z, "移動後の注視点Zを追う" );
		Harness.Check( Controller.OrbitCamera().TryShakePreset( EShakePreset::HitImpact ), "公開カメラアダプターから揺れを加えられる" );

		FThirdPersonCharacter3DInput RunInput;
		RunInput.MoveAxes = FVec2{ 0.0f, 1.0f };
		RunInput.bRunRequested = true;
		const FThirdPersonCharacter3DUpdateResult RunResult = Controller.Update( RunInput, 0.25f );
		Harness.Check( RunResult.Succeeded(), "走行要求も同じ更新経路で適用する" );
		const FVec3 RunVelocity = Controller.Mover().Velocity();
		CheckNear( Harness, Length( FVec2{ RunVelocity.x, RunVelocity.z } ), 7.0f, "走行中は基本速度へ既定倍率を掛ける" );

		Controller.Unbind();
		Harness.Check( !Controller.IsBound() && Scene->FreeCameraEnabled() && !Scene->OrbitCameraOverrideActive(), "解除時に移動接続と場面カメラ設定を戻す" );
	}

	Harness.BeginSuite( "CThirdPersonCharacter3D / 不正入力と接続失敗を段階別に返す" );

	{
		TUniquePtr<CTestThirdPersonScene3D> Scene = MakeUnique<CTestThirdPersonScene3D>();
		CSceneCollision3D Collision{ Scene->Graph() };
		ANode* const Character = SpawnNode( *Scene, "SafeThirdPersonCharacter" );
		Harness.Check( Character != nullptr, "安全性確認用ノードを置ける" );
		if ( Character == nullptr ) return;

		CThirdPersonCharacter3D Controller;
		FThirdPersonCharacter3DParams BrokenParams;
		BrokenParams.MaximumMoveSpeed = std::numeric_limits<f32>::infinity();
		Harness.Check( !Controller.Bind( Collision, *Scene, *Character, BrokenParams ), "有限でない最大速度を接続前に拒否する" );
		BrokenParams.MaximumMoveSpeed = 4.0f;
		BrokenParams.RunSpeedMultiplier = 0.99f;
		Harness.Check( !Controller.Bind( Collision, *Scene, *Character, BrokenParams ), "1未満の走行倍率を接続前に拒否する" );
		Harness.Check( !Controller.IsBound() && Scene->FreeCameraEnabled() && !Scene->OrbitCameraOverrideActive(), "接続失敗で場面設定を変えない" );

		FThirdPersonCharacter3DParams Params;
		Params.Movement.GravityAcceleration = 0.0f;
		Harness.Check( Controller.Bind( Collision, *Scene, *Character, Params ), "正常設定なら接続できる" );
		Harness.Check( !Controller.Bind( Collision, *Scene, *Character, Params ), "明示解除前の再接続を拒否する" );
		const FVec3 PositionBefore = Character->World().position;
		FThirdPersonCharacter3DInput BrokenInput;
		BrokenInput.ZoomAxis = std::numeric_limits<f32>::quiet_NaN();
		const FThirdPersonCharacter3DUpdateResult Failed = Controller.Update( BrokenInput, 1.0f );
		Harness.Check( !Failed.bCameraInputApplied && !Failed.bMovementApplied && !Failed.bFacingApplied && !Failed.bCameraFollowApplied, "不正入力では全段階を実行しない" );
		CheckNear( Harness, Character->World().position.x, PositionBefore.x, "不正入力でノード位置を保つ" );

		Controller.Unbind();
		const FThirdPersonCharacter3DUpdateResult Unbound = Controller.Update( FThirdPersonCharacter3DInput{}, 0.0f );
		Harness.Check( !Unbound.Succeeded(), "未接続では更新しない" );
	}

	Harness.BeginSuite( "CThirdPersonCharacter3D / 任意アニメーションを移動状態へ接続する" );

	{
		TUniquePtr<CTestThirdPersonScene3D> Scene = MakeUnique<CTestThirdPersonScene3D>();
		CSceneCollision3D Collision{ Scene->Graph() };
		Harness.Check( AddFloor( *Scene, Collision ).IsValid(), "アニメーション確認用の床を登録できる" );
		ANode* const Character = SpawnNode( *Scene, "AnimatedThirdPersonCharacter" );
		Harness.Check( Character != nullptr, "アニメーション対象を置ける" );
		if ( Character == nullptr ) return;
		Character->SetPosition( FVec3{ 0.0f, 0.001f, 0.0f } );
		auto& Skin = Character->AddComponent<ASkinnedMeshComponent3D>();
		Skin.SetMeshAsset( MakeCharacterMesh() );

		FThirdPersonCharacter3DParams Params;
		Params.Movement.GravityAcceleration = 0.0f;
		Params.MaximumMoveSpeed = 6.0f;
		Params.CollisionMask = 0x1u;
		{
			CThirdPersonCharacter3D Controller;
			Harness.Check( Controller.Bind( Collision, *Scene, *Character, Params ), "アニメーション付き対象へ本体接続できる" );
			Harness.Check( Controller.Update( FThirdPersonCharacter3DInput{}, 0.0f ).Succeeded(), "床上の初期状態を確定できる" );
			Harness.Check( Controller.TryBindAnimation(), "対象ノードの4状態アニメーションへ任意接続できる" );
			Skin.OnUpdate( 0.30f );

			FThirdPersonCharacter3DInput WalkInput;
			WalkInput.MoveAxes = FVec2{ 0.0f, 0.5f };
			const FThirdPersonCharacter3DUpdateResult Walk = Controller.Update( WalkInput, 0.1f );
			Harness.Check( Walk.Succeeded() && Walk.bAnimationWasBound && Walk.bAnimationApplied && Walk.AnimationSucceeded(), "移動と歩き状態を同じ更新で反映する" );
			Harness.Check( Controller.Animator().CurrentState() == EAnimationGraphState::Walk, "水平速度から歩きを選ぶ" );

			FThirdPersonCharacter3DInput RunInput;
			RunInput.MoveAxes = FVec2{ 0.0f, 1.0f };
			const FThirdPersonCharacter3DUpdateResult DeferredRun = Controller.Update( RunInput, 0.1f );
			Harness.Check( DeferredRun.Succeeded(), "姿勢遷移中も移動とカメラの必須処理を完了する" );
			Harness.Check( DeferredRun.bAnimationWasBound, "姿勢遷移中もアニメーション接続を報告する" );
			Harness.Check( !DeferredRun.bAnimationApplied, "姿勢遷移中の走り要求を保留する" );
			Harness.Check( !DeferredRun.AnimationSucceeded(), "必須処理の成功とアニメーション保留を区別する" );

			Skin.OnUpdate( Controller.Animator().Profile().BlendSeconds + 0.01f );
			const FThirdPersonCharacter3DUpdateResult Run = Controller.Update( RunInput, 0.1f );
			Harness.Check( Run.Succeeded() && Run.bAnimationApplied, "既存遷移の完了後に走りを再試行する" );
			Harness.Check( Controller.Animator().CurrentState() == EAnimationGraphState::Run, "水平速度から走りを選ぶ" );
		}
		Harness.Check( Scene->FreeCameraEnabled() && !Scene->OrbitCameraOverrideActive(), "デストラクタで場面カメラ設定を戻す" );
	}
}
