// SPDX-License-Identifier: Apache-2.0

#include "AcsFramework_Core/AcsFramework.h"

#include "Common/Test/TestHarness.h"

/**
 * 利用側が共通ヘッダーだけを読み込んでも、3D公開入口の型が解決できることを確かめる。
 *
 * @param Harness 単体テストの結果を集める土台。
 */
void RunPublicApiHeaderTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "AcsFramework.h / 利用側の共通includeだけで公開APIを解決する" );

	const FVec3 Position{};
	const FCollidableModel3DSpawnResult EmptyResult{};
	const FCheckpoint3DParams CheckpointParams = FCheckpoint3DParams::Around( 2.0f );
	const FCheckpoint3DSpawnResult EmptyCheckpoint{};
	const EVisualPreset3D Preset = EVisualPreset3D::Balanced;

	Harness.Check( Position.x == 0.0f, "ACSの基本型が解決できる" );
	Harness.Check( !EmptyResult.Succeeded(), "3D生成結果が解決できる" );
	Harness.Check( CheckpointParams.IsValid() && !EmptyCheckpoint.Succeeded(),
		"3Dチェックポイントの設定と生成結果が解決できる" );
	Harness.Check( Preset == EVisualPreset3D::Balanced, "3D見た目設定が解決できる" );
}
