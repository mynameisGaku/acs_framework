// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Camera3D/NodeOrbitCamera3D.h"
#include "Common/Test/TestHarness.h"

#include <cstring>
#include <limits>

namespace
{
	/** 軌道カメラの公開状態だけを検証する最小3D場面。 */
	class CTestOrbitScene3D final : public ALegacyScene3DAdapter
	{
	};

	/** 名前付きノードを場面へ置く。 */
	ANode* SpawnNode( CTestOrbitScene3D& Scene, const char* Name ) noexcept
	{
		const FScene3DSpawnResult Spawned = Scene.Graph().TrySpawn( FStringView( Name ) );
		return Spawned ? Spawned.Node : nullptr;
	}

	/** 小さな浮動小数誤差を許して比較する。 */
	void CheckNear( CTestHarness& Harness, f32 Actual, f32 Expected, const char* Label ) noexcept
	{
		const f32 Difference = Actual > Expected ? Actual - Expected : Expected - Actual;
		Harness.Check( Difference <= 1.0e-4f, Label );
	}

	/** 軌道カメラ状態の主要値が一致するならtrue。 */
	bool SameState( const COrbitCameraController3D::FOrbitCameraState3D& Left, const COrbitCameraController3D::FOrbitCameraState3D& Right ) noexcept
	{
		return Left.target.x == Right.target.x && Left.target.y == Right.target.y && Left.target.z == Right.target.z && Left.yaw_radians == Right.yaw_radians && Left.pitch_radians == Right.pitch_radians && Left.distance == Right.distance;
	}
}


void RunNodeOrbitCamera3DTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "CNodeOrbitCamera3D / ノードを見ながら回転と距離を操作する" );

	{
		TUniquePtr<CTestOrbitScene3D> Scene = MakeUnique<CTestOrbitScene3D>();
		Scene->SetOrbit( FVec3{ 8.0f, 2.0f, -3.0f }, 0.3f, 0.2f, 9.0f );
		// 復元順序の崩れも検出できるよう、一つ前と現在を異なる状態にする。
		COrbitCameraController3D::FOrbitCameraFixedStepSnapshot3D ExpectedBeforeBind;
		Harness.Check( Scene->TryCaptureOrbitCameraSnapshot( ExpectedBeforeBind ), "接続前の軌道状態候補を取得できる" );
		ExpectedBeforeBind.previous.target = FVec3{ 7.0f, 1.0f, -2.0f };
		ExpectedBeforeBind.previous.yaw_radians = 0.1f;
		ExpectedBeforeBind.previous.pitch_radians = 0.15f;
		ExpectedBeforeBind.previous.distance = 8.0f;
		Harness.Check( Scene->TryRestoreOrbitCameraSnapshot( ExpectedBeforeBind ), "異なる補間区間を接続前へ設定できる" );
		COrbitCameraController3D::FOrbitCameraFixedStepSnapshot3D BeforeBind;
		Harness.Check( Scene->TryCaptureOrbitCameraSnapshot( BeforeBind ), "接続前の軌道状態を取得できる" );
		const auto ObstructionBefore = Scene->OrbitCameraObstructionSettings();

		ANode* const Target = SpawnNode( *Scene, "CameraTarget" );
		Harness.Check( Target != nullptr, "追従ノードを置ける" );
		if ( Target == nullptr ) return;
		Target->SetPosition( FVec3{ 2.0f, 0.0f, 3.0f } );

		CNodeOrbitCamera3D Camera;
		Harness.Check( Camera.Bind( *Scene, *Target ), "既定値で追従を始められる" );
		Harness.Check( Camera.IsBound() && Camera.Target() == Target, "接続先を保持する" );
		Harness.Check( !Scene->FreeCameraEnabled() && Scene->OrbitCameraActive() && Scene->OrbitCameraOverrideActive(), "既定自由操作を止めて軌道カメラを明示選択する" );
		Harness.Check( Scene->OrbitCameraObstructionSettings().Enabled, "遮蔽物回避を有効にする" );
		CheckNear( Harness, Camera.State().target.x, 2.0f, "初期注視点X" );
		CheckNear( Harness, Camera.State().target.y, 1.4f, "ローカル高さを初期注視点へ反映する" );
		CheckNear( Harness, Camera.State().target.z, 3.0f, "初期注視点Z" );

		Target->SetPosition( FVec3{ 4.0f, 0.0f, 5.0f } );
		Harness.Check( Camera.Update( FVec2{ 1.0f, 0.0f }, 1.0f, 0.5f ), "追従しながら右回転して近づける" );
		CheckNear( Harness, Camera.State().target.x, 4.0f, "移動後の注視点Xを追う" );
		CheckNear( Harness, Camera.State().target.y, 1.4f, "移動後も注視点高さを保つ" );
		CheckNear( Harness, Camera.State().target.z, 5.0f, "移動後の注視点Zを追う" );
		CheckNear( Harness, Camera.State().yaw_radians, 90.0f * kDeg2Rad, "0.5秒分だけ右へ回る" );
		CheckNear( Harness, Camera.State().distance, 3.0f, "現在距離の半分まで近づく" );

		COrbitCameraController3D::FOrbitCameraFixedStepSnapshot3D Applied;
		Harness.Check( Scene->TryCaptureOrbitCameraSnapshot( Applied ), "場面へ反映した軌道状態を取得できる" );
		Harness.Check( SameState( Applied.current, Camera.State() ), "場面と追従器の状態が一致する" );

		// 同じ場面へ再接続しても、最初の接続前状態を復元基準として保つ。
		FNodeOrbitCamera3DParams ReboundParams;
		ReboundParams.bAvoidObstructions = false;
		Harness.Check( Camera.Bind( *Scene, *Target, ReboundParams ), "同じ場面へ設定を変えて再接続できる" );
		Harness.Check( !Scene->OrbitCameraObstructionSettings().Enabled, "再接続した遮蔽物回避設定を反映する" );

		Camera.Unbind();
		Harness.Check( !Camera.IsBound() && Scene->FreeCameraEnabled() && Scene->OrbitCameraActive() && !Scene->OrbitCameraOverrideActive(), "同じ場面への再接続後も元の自由操作と自動選択を戻す" );
		const auto ObstructionAfter = Scene->OrbitCameraObstructionSettings();
		Harness.Check( ObstructionAfter.Enabled == ObstructionBefore.Enabled && ObstructionAfter.TargetClearance == ObstructionBefore.TargetClearance && ObstructionAfter.CameraClearance == ObstructionBefore.CameraClearance && ObstructionAfter.ProbeRadius == ObstructionBefore.ProbeRadius && ObstructionAfter.RecoverySharpness == ObstructionBefore.RecoverySharpness, "解除時に遮蔽物回避設定を戻す" );
		COrbitCameraController3D::FOrbitCameraFixedStepSnapshot3D AfterUnbind;
		Harness.Check( Scene->TryCaptureOrbitCameraSnapshot( AfterUnbind ) && SameState( AfterUnbind.previous, BeforeBind.previous ) && SameState( AfterUnbind.current, BeforeBind.current ), "解除時に接続前の軌道状態を戻す" );

		Scene->SetOrbitCameraActive( true );
		{
			CNodeOrbitCamera3D ScopedCamera;
			Harness.Check( ScopedCamera.Bind( *Scene, *Target ), "明示軌道選択中にも一時接続できる" );
		}
		Harness.Check( Scene->OrbitCameraOverrideActive(), "デストラクタで接続前の明示軌道選択を戻す" );
	}

	Harness.BeginSuite( "CNodeOrbitCamera3D / 別場面への再接続で両方の設定を保つ" );

	{
		TUniquePtr<CTestOrbitScene3D> FirstScene = MakeUnique<CTestOrbitScene3D>();
		TUniquePtr<CTestOrbitScene3D> SecondScene = MakeUnique<CTestOrbitScene3D>();
		ANode* const FirstTarget = SpawnNode( *FirstScene, "FirstCameraTarget" );
		ANode* const SecondTarget = SpawnNode( *SecondScene, "SecondCameraTarget" );
		Harness.Check( FirstTarget != nullptr && SecondTarget != nullptr, "2場面へ追従ノードを置ける" );
		if ( FirstTarget == nullptr || SecondTarget == nullptr ) return;

		CNodeOrbitCamera3D Camera;
		Harness.Check( Camera.Bind( *FirstScene, *FirstTarget ), "最初の場面へ接続できる" );
		Harness.Check( Camera.Bind( *SecondScene, *SecondTarget ), "次の場面へ再接続できる" );
		Harness.Check( FirstScene->FreeCameraEnabled() && FirstScene->OrbitCameraActive() && !FirstScene->OrbitCameraOverrideActive() && !FirstScene->OrbitCameraObstructionSettings().Enabled, "最初の場面を接続前へ戻す" );
		Harness.Check( !SecondScene->FreeCameraEnabled() && SecondScene->OrbitCameraActive() && SecondScene->OrbitCameraOverrideActive() && SecondScene->OrbitCameraObstructionSettings().Enabled, "次の場面の設定を所有する" );
		Camera.Unbind();
		Harness.Check( SecondScene->FreeCameraEnabled() && SecondScene->OrbitCameraActive() && !SecondScene->OrbitCameraOverrideActive() && !SecondScene->OrbitCameraObstructionSettings().Enabled, "次の場面も解除時に戻す" );
	}

	Harness.BeginSuite( "CNodeOrbitCamera3D / 不正な再接続で既存接続を保つ" );

	{
		TUniquePtr<CTestOrbitScene3D> Scene = MakeUnique<CTestOrbitScene3D>();
		TUniquePtr<CTestOrbitScene3D> OtherScene = MakeUnique<CTestOrbitScene3D>();
		ANode* const Target = SpawnNode( *Scene, "SafeCameraTarget" );
		ANode* const OtherTarget = SpawnNode( *OtherScene, "BrokenCameraTarget" );
		Harness.Check( Target != nullptr && OtherTarget != nullptr, "安全性確認用ノードを置ける" );
		if ( Target == nullptr || OtherTarget == nullptr ) return;

		CNodeOrbitCamera3D Camera;
		Harness.Check( Camera.Bind( *Scene, *Target ), "既存接続を作れる" );
		Harness.Check( !Camera.Bind( *OtherScene, *Target ), "別場面が所有する追従ノードを拒否する" );
		FNodeOrbitCamera3DParams Broken;
		Broken.MinimumDistance = 8.0f;
		Broken.MaximumDistance = 2.0f;
		Harness.Check( !Camera.Bind( *OtherScene, *OtherTarget, Broken ), "逆転した距離範囲を拒否する" );
		Broken = FNodeOrbitCamera3DParams{};
		Broken.LocalTargetOffset.x = std::numeric_limits<f32>::infinity();
		Harness.Check( !Camera.Bind( *OtherScene, *OtherTarget, Broken ), "有限でない注視点を拒否する" );
		Broken = FNodeOrbitCamera3DParams{};
		Broken.PitchLimitDegrees = 89.0f;
		Broken.InitialPitchDegrees = 88.0f;
		Harness.Check( !Camera.Bind( *OtherScene, *OtherTarget, Broken ), "場面が保持できない上下角度を拒否する" );
		Harness.Check( Camera.Target() == Target && !Scene->FreeCameraEnabled() && Scene->OrbitCameraActive() && Scene->OrbitCameraOverrideActive(), "再接続失敗で既存接続を保つ" );
		Harness.Check( OtherScene->FreeCameraEnabled() && OtherScene->OrbitCameraActive() && !OtherScene->OrbitCameraOverrideActive() && !OtherScene->OrbitCameraObstructionSettings().Enabled, "再接続失敗で次の場面を変えない" );

		const auto StateBefore = Camera.State();
		Harness.Check( !Camera.Update( FVec2{}, 0.0f, std::numeric_limits<f32>::infinity() ), "有限でない時刻を拒否する" );
		Harness.Check( SameState( Camera.State(), StateBefore ), "更新失敗で状態を保つ" );
	}

	Harness.BeginSuite( "CNodeOrbitCamera3D / 接続前の明示カメラ選択を復元する" );

	{
		TUniquePtr<CTestOrbitScene3D> Scene = MakeUnique<CTestOrbitScene3D>();
		ANode* const Target = SpawnNode( *Scene, "AuthoredCameraTarget" );
		ANode* const CameraNode = SpawnNode( *Scene, "AuthoredCamera" );
		Harness.Check( Target != nullptr && CameraNode != nullptr, "追従先と配置済みカメラを置ける" );
		if ( Target == nullptr || CameraNode == nullptr ) return;

		auto& CameraComponent = CameraNode->AddComponent<ACameraComponent3D>();
		FScene3DCameraState AuthoredState;
		AuthoredState.IsAuthored = true;
		AuthoredState.IsActivePreferred = true;
		std::memcpy( AuthoredState.StableId, "framework.camera", 17u );
		Harness.Check( CameraComponent.TrySetAuthoredState( AuthoredState ), "配置済みカメラ状態を設定できる" );
		Harness.Check( Scene->SetActiveCamera( "framework.camera" ), "実行時に置いたカメラを安定識別子で明示選択できる" );
		Harness.Check( Scene->AuthoredCameraOverrideActive() && !Scene->OrbitCameraOverrideActive(), "接続前の明示選択を確認できる" );

		CNodeOrbitCamera3D Camera;
		Harness.Check( Camera.Bind( *Scene, *Target ), "追従カメラへ一時的に切り替えられる" );
		Harness.Check( Scene->OrbitCameraOverrideActive() && !Scene->AuthoredCameraOverrideActive(), "接続中は軌道カメラだけを明示選択する" );
		Camera.Unbind();
		Harness.Check( Scene->AuthoredCameraOverrideActive() && !Scene->OrbitCameraOverrideActive() && Scene->AuthoredCamera() != nullptr && std::strcmp( Scene->AuthoredCamera()->StableId, "framework.camera" ) == 0, "解除時に元の配置済みカメラを明示選択し直す" );
	}
}
