// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Audio/Spatial/SpatialSfxMix.h"
#include "AcsFramework_Core/Audio/Spatial/SpatialPlayRequest.h"
#include "Common/Test/TestHarness.h"

#include <limits>


void RunSpatialSfxMixTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "FSpatialSfxMix / 3D位置から音量と左右位置を決める" );

	CSpatialAudio Spatial;
	FAudioListener Listener;
	Listener.position = FVec3::Zero();
	Listener.forward = FVec3::Forward();
	Listener.up = FVec3::Up();
	Spatial.SetListener( Listener );

	const u32 RightSource = Spatial.RegisterSource( FVec3{ 10.0f, 0.0f, 0.0f }, 20.0f, EAttenuationCurve::Linear );
	const FSpatialSfxMix RightMix = ComputeSpatialSfxMix( Spatial, RightSource, 0.8f );
	Harness.CheckNearF32( RightMix.Volume, 0.4f, 0.0001f, "半分の距離減衰へ基準音量を掛ける" );
	Harness.CheckNearF32( RightMix.Pan, 1.0f, 0.0001f, "右の音源は右から聞こえる" );
	Harness.Check( RightMix.bAudible, "しきい値より大きい音は鳴らす" );

	const u32 LeftSource = Spatial.RegisterSource( FVec3{ -4.0f, 0.0f, 0.0f }, 20.0f, EAttenuationCurve::Linear );
	const FSpatialSfxMix LeftMix = ComputeSpatialSfxMix( Spatial, LeftSource, 1.0f );
	Harness.CheckNearF32( LeftMix.Pan, -1.0f, 0.0001f, "左の音源は左から聞こえる" );

	const FSpatialSfxMix MutedMix = ComputeSpatialSfxMix( Spatial, RightSource, 0.8f, 0.5f );
	Harness.Check( !MutedMix.bAudible, "しきい値以下ならvoiceを使わない" );

	Harness.Check( !ComputeSpatialSfxMix( Spatial, 0u, 1.0f ).bAudible, "無効番号は鳴らさない" );
	Harness.Check( !ComputeSpatialSfxMix( Spatial, RightSource, -1.0f ).bAudible, "不正な基準音量は鳴らさない" );
	Harness.Check( !ComputeSpatialSfxMix( Spatial, RightSource, 1.0f, -1.0f ).bAudible, "不正なしきい値は鳴らさない" );

	Spatial.RemoveSource( RightSource );
	const FSpatialSfxMix ReleasedMix = ComputeSpatialSfxMix( Spatial, RightSource, 1.0f );
	Harness.CheckEqualF32( ReleasedMix.Volume, 0.0f, "返した音源番号には音量を残さない" );
	Harness.Check( !ReleasedMix.bAudible, "返した音源番号は鳴らさない" );

	Harness.BeginSuite( "FSpatialPlayRequest / 再生前に不正値を拒否する" );

	FSpatialPlayRequest Request;
	Request.AssetPath = FString( "Audio/SpatialPulse.wav" );
	Harness.Check( Request.IsValid(), "既定値と素材名があれば有効" );

	Request.BaseVolume = std::numeric_limits<f32>::quiet_NaN();
	Harness.Check( !Request.IsValid(), "有限でない基準音量は拒否する" );
	Request.BaseVolume = 1.0f;
	Request.MaxDistance = 0.0f;
	Harness.Check( !Request.IsValid(), "0以下の最大距離は拒否する" );
	Request.MaxDistance = 20.0f;
	Request.Pitch = std::numeric_limits<f32>::infinity();
	Harness.Check( !Request.IsValid(), "有限でない再生速度は拒否する" );
	Request.Pitch = 1.0f;
	Request.AttenuationCurve = static_cast<EAttenuationCurve>( 255u );
	Harness.Check( !Request.IsValid(), "未定義の距離曲線は拒否する" );
}
