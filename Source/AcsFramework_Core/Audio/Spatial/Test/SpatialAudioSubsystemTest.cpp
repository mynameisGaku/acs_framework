// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Audio/Spatial/SpatialAudioSubsystem.h"
#include "Common/Test/TestHarness.h"


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
}
