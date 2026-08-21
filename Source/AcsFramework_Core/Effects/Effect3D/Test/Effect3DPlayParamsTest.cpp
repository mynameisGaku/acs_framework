// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Effects/Effect3D/Effect3DHandle.h"
#include "AcsFramework_Core/Effects/Effect3D/Effect3DPlayParams.h"
#include "Common/Test/TestHarness.h"

#include <limits>

void RunEffect3DPlayParamsTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "FEffect3DPlayParams / 位置だけで再生できる" );

	{
		const FEffect3DPlayParams Defaults;
		Harness.Check( Defaults.IsValid(), "既定値がそのまま使える" );
		Harness.CheckEqualF32( Defaults.Scale.x, 1.0f, "既定は等倍" );
		Harness.CheckEqualF32( Defaults.Speed, 1.0f, "既定は素材どおりの速度" );

		const FEffect3DPlayParams AtPosition = FEffect3DPlayParams::At( FVec3{ 1.0f, 2.0f, 3.0f } );
		Harness.CheckEqualF32( AtPosition.Position.y, 2.0f, "位置だけを短く指定できる" );
		Harness.Check( AtPosition.IsValid(), "位置指定もそのまま使える" );
	}

	Harness.BeginSuite( "FEffect3DPlayParams / 見えない指定を再生前に弾く" );

	{
		FEffect3DPlayParams ZeroScale;
		ZeroScale.Scale.y = 0.0f;
		Harness.Check( !ZeroScale.IsValid(), "0 倍は弾く" );

		FEffect3DPlayParams Mirrored;
		Mirrored.Scale.x = -1.0f;
		Harness.Check( Mirrored.IsValid(), "負の倍率は鏡写しとして通す" );

		FEffect3DPlayParams Stopped;
		Stopped.Speed = 0.0f;
		Harness.Check( !Stopped.IsValid(), "止まった速度は弾く" );

		FEffect3DPlayParams BeforeStart;
		BeforeStart.StartFrame = -1;
		Harness.Check( !BeforeStart.IsValid(), "負の開始位置は弾く" );

		FEffect3DPlayParams NotANumber;
		NotANumber.Position.x = std::numeric_limits<f32>::quiet_NaN();
		Harness.Check( !NotANumber.IsValid(), "NaN は弾く" );

		FEffect3DPlayParams BrokenRotation;
		BrokenRotation.RotationDeg.z = std::numeric_limits<f32>::infinity();
		Harness.Check( !BrokenRotation.IsValid(), "無限大の向きは弾く" );
	}

	Harness.BeginSuite( "FEffect3DHandle / backend の番号を漏らさない" );

	{
		FEffect3DHandle Handle;
		Harness.Check( !Handle.IsValid(), "既定は無効" );

		Handle = FEffect3DHandle::FromValue( 42u );
		Harness.Check( Handle.IsValid(), "発行値があれば有効" );
		Harness.CheckEqualU64( Handle.Value(), 42u, "発行値を保つ" );

		Handle.Reset();
		Harness.Check( !Handle.IsValid(), "明示的に無効へ戻せる" );
	}
}
