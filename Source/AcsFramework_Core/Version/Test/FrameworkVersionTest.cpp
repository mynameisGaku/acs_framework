// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Version/FrameworkVersion.h"
#include "Common/Test/TestHarness.h"

void RunFrameworkVersionTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "FFrameworkVersion / 現在版と互換性境界を返す" );

	Harness.CheckEqualU64( kAcsFrameworkVersion.Major, 0u, "現在の主版" );
	Harness.CheckEqualU64( kAcsFrameworkVersion.Minor, 5u, "現在の副版" );
	Harness.CheckEqualU64( kAcsFrameworkVersion.Patch, 0u, "現在の修正版" );
	Harness.Check( kAcsFrameworkVersion.bPreRelease, "現在は開発版" );
	Harness.Check( FStringView( kAcsFrameworkVersionText ) == FStringView( "0.5.0-dev" ), "文字列版を返す" );

	Harness.Check( kAcsFrameworkVersion.IsAtLeast( FFrameworkVersion{ 0u, 4u, 9u, false } ), "新しい数値部は以前の正式版以上" );
	Harness.Check( kAcsFrameworkVersion.IsAtLeast( FFrameworkVersion{ 0u, 5u, 0u, true } ), "同じ開発版以上" );
	Harness.Check( !kAcsFrameworkVersion.IsAtLeast( FFrameworkVersion{ 0u, 5u, 0u, false } ), "同じ数値部の正式版には届かない" );
	Harness.Check( !kAcsFrameworkVersion.IsAtLeast( FFrameworkVersion{ 0u, 5u, 1u, true } ), "新しい修正版には届かない" );
	Harness.Check( !kAcsFrameworkVersion.HasStableApi(), "v1.0.0前は安定APIを名乗らない" );
	Harness.Check( FFrameworkVersion{ 1u, 0u, 0u, false }.HasStableApi(), "v1.0.0正式版から安定API" );
}
