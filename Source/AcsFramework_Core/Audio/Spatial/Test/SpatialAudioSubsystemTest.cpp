// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Audio/AudioSubsystem.h"
#include "AcsFramework_Core/Audio/Spatial/SpatialAudioSubsystem.h"
#include "Common/Test/TestHarness.h"

namespace
{
	/** 3Dカメラから聴取位置を作る単体検証用の最小場面。 */
	class CTestSpatialAudioScene3D final : public ALegacyScene3DAdapter
	{
	};
}


void RunSpatialAudioSubsystemTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "CSpatialAudioSubsystem / Engine発行の音源番号を所有する" );

	CSpatialAudioSubsystem Spatial;
	const u32 SourceId = Spatial.AcquireSource( FVec3{ 2.0f, 0.0f, 4.0f }, FVec3::Zero(), 12.0f, EAttenuationCurve::Linear );
	Harness.Check( SourceId != 0u, "音源番号を登録できる" );
	Harness.CheckEqualU64( Spatial.GetSourceCount(), 1u, "登録直後からEngineの音源として数えられる" );

	Spatial.UpdateSource( SourceId, FVec3{ -2.0f, 0.0f, 4.0f } );
	Harness.CheckEqualU64( Spatial.GetSourceCount(), 1u, "位置更新では音源を増減させない" );

	Spatial.ReleaseSource( SourceId );
	Harness.CheckEqualU64( Spatial.GetSourceCount(), 0u, "解除した音源をEngineに残さない" );
	Spatial.ReleaseSource( SourceId );
	Harness.CheckEqualU64( Spatial.GetSourceCount(), 0u, "解除済み番号を再度渡しても数を壊さない" );

	Harness.BeginSuite( "CSpatialListenerBinder / 3Dカメラから聴取位置を作る" );

	{
		TUniquePtr<CTestSpatialAudioScene3D> Scene = MakeUnique<CTestSpatialAudioScene3D>();
		Scene->SetOrbit( FVec3{ 2.0f, 1.5f, -3.0f }, 0.35f, 0.2f, 7.0f );
		FAudioListener Listener;
		Harness.Check( CSpatialListenerBinder::TryMakeFromCamera( *Scene, Listener ), "軌道カメラを聴取位置へ変換できる" );

		COrbitCameraController3D::FOrbitCameraFixedStepSnapshot3D Snapshot;
		Harness.Check( Scene->TryCaptureOrbitCameraSnapshot( Snapshot ), "比較用の軌道カメラ状態を取得できる" );
		const FVec3 ExpectedForward = Normalize( Snapshot.current.target - Scene->Camera().Eye() );
		Harness.CheckNearF32( Listener.position.x, Scene->Camera().Eye().x, 0.0001f, "聴取位置Xをカメラへ合わせる" );
		Harness.CheckNearF32( Listener.position.y, Scene->Camera().Eye().y, 0.0001f, "聴取位置Yをカメラへ合わせる" );
		Harness.CheckNearF32( Listener.position.z, Scene->Camera().Eye().z, 0.0001f, "聴取位置Zをカメラへ合わせる" );
		Harness.CheckNearF32( Listener.forward.x, ExpectedForward.x, 0.0001f, "前方向Xを注視点へ向ける" );
		Harness.CheckNearF32( Listener.forward.y, ExpectedForward.y, 0.0001f, "前方向Yを注視点へ向ける" );
		Harness.CheckNearF32( Listener.forward.z, ExpectedForward.z, 0.0001f, "前方向Zを注視点へ向ける" );

		Scene->Camera().SetLookAt( FVec3{ 1.0e30f, 0.0f, 0.0f }, FVec3::Zero() );
		FAudioListener Preserved;
		Preserved.position = FVec3{ 8.0f, 7.0f, 6.0f };
		FAudioListener Rejected = Preserved;
		Harness.Check( !CSpatialListenerBinder::TryMakeFromCamera( *Scene, Rejected ), "方向計算の範囲を超えるカメラ位置を拒否する" );
		Harness.Check( Rejected.position.x == Preserved.position.x && Rejected.position.y == Preserved.position.y && Rejected.position.z == Preserved.position.z, "失敗時は出力を変更しない" );
	}

	Harness.BeginSuite( "CSpatialAudioSubsystem / 再生直前の聴取位置を反映する" );

	{
		CSpatialAudioSubsystem ImmediateSpatial;
		CAudioSubsystem SilentAudio;
		ImmediateSpatial.Bind( SilentAudio );

		FAudioListener Listener;
		Listener.position = FVec3{ 8.0f, 0.0f, 0.0f };
		ImmediateSpatial.SetListener( Listener );

		FSpatialPlayRequest Request;
		Request.AssetPath = FString( "Audio/SpatialPulse.wav" );
		Request.Position = FVec3{ 4.0f, 0.0f, 0.0f };
		Harness.Check( !ImmediateSpatial.PlayOnce( Request ), "音声出力が無い場合は再生失敗を返す" );
		Harness.CheckNearF32( ImmediateSpatial.GetLastPan(), -1.0f, 0.0001f, "同じ呼出しで設定した聴取位置から左右を求める" );
		Harness.CheckEqualU64( ImmediateSpatial.GetSourceCount(), 0u, "再生失敗後も一時音源を残さない" );
	}
}
