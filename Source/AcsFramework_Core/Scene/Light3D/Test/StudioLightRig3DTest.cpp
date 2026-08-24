// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Light3D/StudioLightRig3DSpawner.h"
#include "Common/Test/TestHarness.h"

#include <limits>

namespace
{
	/** 置いたノードから光の部品を取り出す。 */
	ALightComponent3D* LightOf( ANode* Node ) noexcept
	{
		return Node != nullptr ? Node->GetComponent<ALightComponent3D>() : nullptr;
	}

	/** 3成分を小さな浮動小数誤差を許して比較する。 */
	void CheckVector( CTestHarness& Harness, FVec3 Actual,
		FVec3 Expected, const char* Label ) noexcept
	{
		constexpr f32 kTolerance = 0.0001f;
		Harness.Check( LengthSq( Actual - Expected )
			< kTolerance * kTolerance, Label );
	}
}


void RunStudioLightRig3DTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "FStudioLightRig3DParams / 被写体から3灯を決める" );

	{
		const FStudioLightRig3DParams Params =
			FStudioLightRig3DParams::AroundSubject(
				FVec3{ 1.0f, 2.0f, 3.0f },
				FVec3{ 0.0f, 0.5f, -2.0f }, 2.0f );
		FLight3DSpawnParams Key;
		FLight3DSpawnParams Fill;
		FLight3DSpawnParams Rim;
		Harness.Check( Params.IsValid(), "中心、見る方向、半径だけで有効になる" );
		Harness.Check( Params.TryBuildLights( Key, Fill, Rim ),
			"3灯の配置指定を一括計算できる" );
		Harness.Check( Key.Kind == ELight3DKind::Point
			&& Fill.Kind == ELight3DKind::Point
			&& Rim.Kind == ELight3DKind::Point,
			"既存の太陽を置き換えない点光源だけを作る" );
		Harness.Check( Key.Name == FStringView( "StudioKeyLight" )
			&& Fill.Name == FStringView( "StudioFillLight" )
			&& Rim.Name == FStringView( "StudioRimLight" ),
			"3つの役割をノード名で区別できる" );
		CheckVector( Harness, Key.Position,
			FVec3{ -1.8f, 5.4f, -0.6f },
			"キーをカメラ左上へ置く" );
		CheckVector( Harness, Fill.Position,
			FVec3{ 4.0f, 3.6f, -0.2f },
			"フィルをカメラ右側へ置く" );
		CheckVector( Harness, Rim.Position,
			FVec3{ 2.0f, 5.2f, 6.4f },
			"リムを被写体背面上側へ置く" );
		Harness.CheckNearF32( Key.Range, 9.0f, 0.0001f,
			"被写体半径から3灯共通の到達距離を決める" );
		Harness.Check( Key.Intensity > Fill.Intensity
			&& Rim.Intensity > Fill.Intensity,
			"フィルを主光と輪郭光より弱く保つ" );
	}

	{
		const FStudioLightRig3DParams Params =
			FStudioLightRig3DParams::AroundSubject(
				FVec3{}, FVec3{ 4.0f, 2.0f, 0.0f }, 1.0f );
		FLight3DSpawnParams Key;
		FLight3DSpawnParams Fill;
		FLight3DSpawnParams Rim;
		Harness.Check( Params.TryBuildLights( Key, Fill, Rim ),
			"別方向のカメラへ照明配置を回せる" );
		CheckVector( Harness, Key.Position,
			FVec3{ 1.8f, 1.7f, -1.4f },
			"+Xカメラでもキーを左上へ回す" );
		CheckVector( Harness, Fill.Position,
			FVec3{ 1.6f, 0.8f, 1.5f },
			"+Xカメラでもフィルを右へ回す" );
		CheckVector( Harness, Rim.Position,
			FVec3{ -1.7f, 1.6f, 0.5f },
			"+Xカメラでもリムを背面へ回す" );
	}

	Harness.BeginSuite( "FStudioLightRig3DParams / 不正値では出力を変えない" );

	{
		const f32 QuietNaN = std::numeric_limits<f32>::quiet_NaN();
		const f32 Maximum = std::numeric_limits<f32>::max();
		FStudioLightRig3DParams Vertical =
			FStudioLightRig3DParams::AroundSubject(
				FVec3{}, FVec3{ 0.0f, 1.0f, 0.0f }, 1.0f );
		FStudioLightRig3DParams ZeroRadius = Vertical;
		ZeroRadius.ViewDirectionToCamera = FVec3{ 0.0f, 0.0f, -1.0f };
		ZeroRadius.SubjectRadius = 0.0f;
		FStudioLightRig3DParams BrokenColor = ZeroRadius;
		BrokenColor.SubjectRadius = 1.0f;
		BrokenColor.FillColor.x = -0.1f;
		FStudioLightRig3DParams BrokenCenter = BrokenColor;
		BrokenCenter.FillColor.x = 0.5f;
		BrokenCenter.SubjectCenter.x = QuietNaN;
		FStudioLightRig3DParams Overflow = BrokenColor;
		Overflow.FillColor.x = 0.5f;
		Overflow.SubjectRadius = Maximum;

		Harness.Check( !Vertical.IsValid(), "高さ成分だけの見る方向を拒否する" );
		Harness.Check( !ZeroRadius.IsValid(), "0以下の被写体半径を拒否する" );
		Harness.Check( !BrokenColor.IsValid(), "負の灯色を拒否する" );
		Harness.Check( !BrokenCenter.IsValid(), "有限でない中心を拒否する" );
		Harness.Check( !Overflow.IsValid(), "派生位置または距離が溢れる指定を拒否する" );

		FLight3DSpawnParams Key = FLight3DSpawnParams::Sun(
			FVec3{ 1.0f, 1.0f, 0.0f } );
		FLight3DSpawnParams Fill = Key;
		FLight3DSpawnParams Rim = Key;
		Harness.Check( !Vertical.TryBuildLights( Key, Fill, Rim ),
			"不正方向の変換は失敗する" );
		Harness.Check( Key.Kind == ELight3DKind::Directional
			&& Fill.Kind == ELight3DKind::Directional
			&& Rim.Kind == ELight3DKind::Directional,
			"失敗時は3つの出力を変更しない" );
	}

	Harness.BeginSuite( "CStudioLightRig3DSpawner / 3灯を同じ親へ一括配置する" );

	{
		CSceneNodeGraph Graph;
		const FScene3DSpawnResult Parent = Graph.TrySpawn(
			FStringView( "SubjectRoot" ) );
		if ( Parent.Node != nullptr )
			Parent.Node->SetPosition( FVec3{ 10.0f, 0.0f, -4.0f } );
		const FStudioLightRig3DParams Params =
			FStudioLightRig3DParams::AroundSubject(
				FVec3{ 0.0f, 1.0f, 0.0f },
				FVec3{ 0.0f, 0.0f, -1.0f }, 1.5f );
		FStudioLightRig3DSpawnResult Spawned =
			CStudioLightRig3DSpawner::SpawnInto(
				Graph, Params, Parent.Node );

		Harness.Check( Spawned.Succeeded(), "キー、フィル、リムを全て置ける" );
		Harness.CheckEqualU64( Graph.RegisteredCount(), 5u,
			"root、親、3灯だけを登録する" );
		Harness.Check( Spawned.KeyLight() != nullptr
			&& Spawned.FillLight() != nullptr
			&& Spawned.RimLight() != nullptr,
			"生成番号から生存中の3灯を取れる" );
		Harness.Check( Spawned.KeyLight() != nullptr
			&& Spawned.KeyLight()->Parent() == Parent.Node
			&& Spawned.FillLight()->Parent() == Parent.Node
			&& Spawned.RimLight()->Parent() == Parent.Node,
			"3灯へ同じ親変形を適用する" );
		Harness.Check( LightOf( Spawned.KeyLight() ) != nullptr
			&& LightOf( Spawned.FillLight() ) != nullptr
			&& LightOf( Spawned.RimLight() ) != nullptr,
			"3灯へACSの光部品を付ける" );

		ANode* const Key = Spawned.KeyLight();
		ANode* const Fill = Spawned.FillLight();
		ANode* const Rim = Spawned.RimLight();
		CSceneNodeGraph WrongGraph;
		Harness.Check( !CStudioLightRig3DSpawner::Destroy(
			WrongGraph, Spawned ), "別場面からの一括破棄を拒否する" );
		Harness.Check( Spawned.Succeeded()
			&& Key != nullptr && !Key->IsPendingDestroy(),
			"別場面による失敗時は結果と3灯を変えない" );
		Harness.Check( CStudioLightRig3DSpawner::Destroy(
			Graph, Spawned ), "生成時の場面から3灯を一括破棄できる" );
		Harness.Check( Spawned.IsEmpty(), "破棄成功時は生成結果を空にする" );
		Harness.Check( Key != nullptr && Key->IsPendingDestroy()
			&& Fill != nullptr && Fill->IsPendingDestroy()
			&& Rim != nullptr && Rim->IsPendingDestroy(),
			"3灯を全て破棄予定へ移す" );
		Graph.ResolveStructuralChanges();
		Harness.CheckEqualU64( Graph.RegisteredCount(), 2u,
			"確定後はrootと指定親だけを残す" );
	}

	Harness.BeginSuite( "CStudioLightRig3DSpawner / 被写体に合わせて3灯を同期更新する" );

	{
		CSceneNodeGraph Graph;
		const FScene3DSpawnResult Parent = Graph.TrySpawn(
			FStringView( "SubjectRoot" ) );
		FStudioLightRig3DSpawnResult Spawned =
			CStudioLightRig3DSpawner::SpawnInto(
				Graph, FStudioLightRig3DParams::AroundSubject(
					FVec3{ 0.0f, 1.0f, 0.0f },
					FVec3{ 0.0f, 0.0f, -1.0f }, 1.0f ), Parent.Node );
		const FNodeId OriginalKeyId = Spawned.KeyLightId();
		const FNodeId OriginalFillId = Spawned.FillLightId();
		const FNodeId OriginalRimId = Spawned.RimLightId();

		FStudioLightRig3DParams Updated =
			FStudioLightRig3DParams::AroundSubject(
				FVec3{ 2.0f, 1.5f, -3.0f },
				FVec3{ 1.0f, 0.2f, 0.0f }, 2.0f );
		Updated.KeyColor = FVec3{ 1.0f, 0.70f, 0.45f };
		Updated.KeyIntensity = 2.6f;
		Updated.FillColor = FVec3{ 0.30f, 0.55f, 1.0f };
		Updated.FillIntensity = 0.9f;
		Updated.RimColor = FVec3{ 1.0f, 0.35f, 0.18f };
		Updated.RimIntensity = 1.8f;
		Updated.RangeScale = 5.0f;
		FLight3DSpawnParams ExpectedKey;
		FLight3DSpawnParams ExpectedFill;
		FLight3DSpawnParams ExpectedRim;
		Harness.Check( Updated.TryBuildLights(
			ExpectedKey, ExpectedFill, ExpectedRim ),
			"更新後の3灯指定を計算できる" );

		Harness.Check( CStudioLightRig3DSpawner::TryApplyTo(
			Graph, Spawned, Updated ),
			"有効な新指定を3灯へ一括反映できる" );
		Harness.Check( Spawned.KeyLightId() == OriginalKeyId
			&& Spawned.FillLightId() == OriginalFillId
			&& Spawned.RimLightId() == OriginalRimId,
			"更新しても3灯の世代付き番号を保つ" );
		Harness.CheckEqualU64( Graph.RegisteredCount(), 5u,
			"更新で3灯を作り直さない" );

		ANode* const Key = Spawned.KeyLight();
		ANode* const Fill = Spawned.FillLight();
		ANode* const Rim = Spawned.RimLight();
		CheckVector( Harness,
			Key != nullptr ? Key->Local().position : FVec3{},
			ExpectedKey.Position, "キーを新しい被写体の左上へ移す" );
		CheckVector( Harness,
			Fill != nullptr ? Fill->Local().position : FVec3{},
			ExpectedFill.Position, "フィルを新しい被写体の右側へ移す" );
		CheckVector( Harness,
			Rim != nullptr ? Rim->Local().position : FVec3{},
			ExpectedRim.Position, "リムを新しい被写体の背面へ移す" );
		Harness.Check( Key != nullptr && Key->Parent() == Parent.Node
			&& Fill != nullptr && Fill->Parent() == Parent.Node
			&& Rim != nullptr && Rim->Parent() == Parent.Node,
			"更新後も3灯の共通親を保つ" );
		Harness.Check( Key != nullptr
			&& Key->Name() == FStringView( "StudioKeyLight" )
			&& Fill != nullptr
			&& Fill->Name() == FStringView( "StudioFillLight" )
			&& Rim != nullptr
			&& Rim->Name() == FStringView( "StudioRimLight" ),
			"更新後も3灯の役割名を保つ" );

		ALightComponent3D* const KeyLight = LightOf( Key );
		ALightComponent3D* const FillLight = LightOf( Fill );
		ALightComponent3D* const RimLight = LightOf( Rim );
		FPointLight KeyOutput{};
		FPointLight FillOutput{};
		FPointLight RimOutput{};
		Harness.Check( KeyLight != nullptr && KeyLight->FillPoint( KeyOutput )
			&& FillLight != nullptr && FillLight->FillPoint( FillOutput )
			&& RimLight != nullptr && RimLight->FillPoint( RimOutput ),
			"更新後の3灯を描画値へ変換できる" );
		Harness.CheckNearF32( KeyOutput.range, ExpectedKey.Range, 0.0001f,
			"キーを新しい到達距離へ更新する" );
		Harness.CheckNearF32( FillOutput.color.z,
			ExpectedFill.Color.z * ExpectedFill.Intensity, 0.0001f,
			"フィルを新しい色と強さへ更新する" );
		Harness.CheckNearF32( RimOutput.color.x,
			ExpectedRim.Color.x * ExpectedRim.Intensity, 0.0001f,
			"リムを新しい色と強さへ更新する" );
	}

	Harness.BeginSuite( "CStudioLightRig3DSpawner / 更新失敗時は3灯を変えない" );

	{
		CSceneNodeGraph Graph;
		const FScene3DSpawnResult Parent = Graph.TrySpawn(
			FStringView( "SubjectRoot" ) );
		const FScene3DSpawnResult OtherParent = Graph.TrySpawn(
			FStringView( "OtherRoot" ) );
		const FStudioLightRig3DParams Initial =
			FStudioLightRig3DParams::AroundSubject(
				FVec3{ 0.0f, 1.0f, 0.0f },
				FVec3{ 0.0f, 0.0f, -1.0f }, 1.0f );
		FStudioLightRig3DSpawnResult Spawned =
			CStudioLightRig3DSpawner::SpawnInto(
				Graph, Initial, Parent.Node );
		ANode* const Key = Spawned.KeyLight();
		ANode* const Fill = Spawned.FillLight();
		ANode* const Rim = Spawned.RimLight();
		FLight3DSpawnParams InitialKey;
		FLight3DSpawnParams InitialFill;
		FLight3DSpawnParams InitialRim;
		Harness.Check( Initial.TryBuildLights(
			InitialKey, InitialFill, InitialRim ),
			"失敗時比較用の初期3灯を計算できる" );

		FStudioLightRig3DParams Updated =
			FStudioLightRig3DParams::AroundSubject(
				FVec3{ 6.0f, 2.0f, -4.0f },
				FVec3{ 1.0f, 0.0f, 0.0f }, 2.5f );
		FStudioLightRig3DParams Invalid = Updated;
		Invalid.RimIntensity = -1.0f;
		Harness.Check( !CStudioLightRig3DSpawner::TryApplyTo(
			Graph, Spawned, Invalid ), "不正な新指定を拒否する" );
		CheckVector( Harness,
			Key != nullptr ? Key->Local().position : FVec3{},
			InitialKey.Position, "不正入力ではキーを変えない" );
		CheckVector( Harness,
			Fill != nullptr ? Fill->Local().position : FVec3{},
			InitialFill.Position, "不正入力ではフィルを変えない" );
		CheckVector( Harness,
			Rim != nullptr ? Rim->Local().position : FVec3{},
			InitialRim.Position, "不正入力ではリムを変えない" );
		FPointLight KeyAfterInvalid{};
		FPointLight FillAfterInvalid{};
		FPointLight RimAfterInvalid{};
		Harness.Check( LightOf( Key ) != nullptr
			&& LightOf( Key )->FillPoint( KeyAfterInvalid )
			&& LightOf( Fill ) != nullptr
			&& LightOf( Fill )->FillPoint( FillAfterInvalid )
			&& LightOf( Rim ) != nullptr
			&& LightOf( Rim )->FillPoint( RimAfterInvalid ),
			"不正入力後も3灯を描画値へ変換できる" );
		Harness.CheckNearF32( KeyAfterInvalid.range,
			InitialKey.Range, 0.0001f,
			"不正入力ではキーの到達距離を変えない" );
		Harness.CheckNearF32( FillAfterInvalid.color.z,
			InitialFill.Color.z * InitialFill.Intensity, 0.0001f,
			"不正入力ではフィルの色と強さを変えない" );
		Harness.CheckNearF32( RimAfterInvalid.color.x,
			InitialRim.Color.x * InitialRim.Intensity, 0.0001f,
			"不正入力ではリムの色と強さを変えない" );

		CSceneNodeGraph WrongGraph;
		Harness.Check( !CStudioLightRig3DSpawner::TryApplyTo(
			WrongGraph, Spawned, Updated ),
			"別場面からの更新を拒否する" );
		CheckVector( Harness,
			Key != nullptr ? Key->Local().position : FVec3{},
			InitialKey.Position, "別場面ではキーを変えない" );

		if ( Rim != nullptr && OtherParent.Node != nullptr )
			Rim->Reparent( *OtherParent.Node );
		Graph.ResolveStructuralChanges();
		Harness.Check( Rim != nullptr && Rim->Parent() == OtherParent.Node,
			"共通親を崩す更新検証を準備できる" );
		Harness.Check( !CStudioLightRig3DSpawner::TryApplyTo(
			Graph, Spawned, Updated ),
			"3灯が共通親を失った更新を拒否する" );
		CheckVector( Harness,
			Fill != nullptr ? Fill->Local().position : FVec3{},
			InitialFill.Position, "共通親不一致ではフィルを変えない" );
		if ( Rim != nullptr && Parent.Node != nullptr )
			Rim->Reparent( *Parent.Node );
		Graph.ResolveStructuralChanges();
		Harness.Check( Rim != nullptr && Rim->Parent() == Parent.Node,
			"リムを生成時の共通親へ戻せる" );

		Harness.Check( Graph.Destroy( Spawned.KeyLightId() ),
			"破棄予定の灯による更新検証を準備できる" );
		Harness.Check( !CStudioLightRig3DSpawner::TryApplyTo(
			Graph, Spawned, Updated ),
			"キーが破棄予定なら3灯更新を拒否する" );
		CheckVector( Harness,
			Fill != nullptr ? Fill->Local().position : FVec3{},
			InitialFill.Position, "破棄予定による失敗でもフィルを変えない" );
		CheckVector( Harness,
			Rim != nullptr ? Rim->Local().position : FVec3{},
			InitialRim.Position, "破棄予定による失敗でもリムを変えない" );
		FPointLight FillAfterStructuralFailures{};
		FPointLight RimAfterStructuralFailures{};
		Harness.Check( LightOf( Fill ) != nullptr
			&& LightOf( Fill )->FillPoint( FillAfterStructuralFailures )
			&& LightOf( Rim ) != nullptr
			&& LightOf( Rim )->FillPoint( RimAfterStructuralFailures ),
			"構成不正による失敗後も残る2灯を描画値へ変換できる" );
		Harness.CheckNearF32( FillAfterStructuralFailures.range,
			InitialFill.Range, 0.0001f,
			"構成不正ではフィルの到達距離を変えない" );
		Harness.CheckNearF32( RimAfterStructuralFailures.color.x,
			InitialRim.Color.x * InitialRim.Intensity, 0.0001f,
			"構成不正ではリムの色と強さを変えない" );
		Harness.Check( !CStudioLightRig3DSpawner::TryApplyTo(
			Graph, FStudioLightRig3DSpawnResult{}, Updated ),
			"空の生成結果を拒否する" );
	}

	Harness.BeginSuite( "CStudioLightRig3DSpawner / 個別破棄後も残りを片付ける" );

	{
		CSceneNodeGraph Graph;
		const FStudioLightRig3DParams Params;
		FStudioLightRig3DSpawnResult Spawned =
			CStudioLightRig3DSpawner::SpawnInto( Graph, Params );
		Harness.Check( Spawned && Graph.Destroy( Spawned.KeyLightId() ),
			"キーライトだけを先に破棄予定へ移せる" );
		Harness.Check( Spawned.KeyLight() == nullptr,
			"破棄予定の灯を生存中として返さない" );
		Harness.Check( CStudioLightRig3DSpawner::Destroy(
			Graph, Spawned ), "残るフィルとリムも一括で片付けられる" );
		Graph.ResolveStructuralChanges();
		Harness.CheckEqualU64( Graph.RegisteredCount(), 1u,
			"個別破棄を含めて3灯を残さない" );
	}

	Harness.BeginSuite( "CStudioLightRig3DSpawner / 無効親と入力を半端に残さない" );

	{
		CSceneNodeGraph OwnerGraph;
		const FScene3DSpawnResult ForeignParent = OwnerGraph.TrySpawn(
			FStringView( "ForeignParent" ) );
		CSceneNodeGraph TargetGraph;
		const u32 BeforeCount = TargetGraph.RegisteredCount();
		const FStudioLightRig3DSpawnResult ForeignResult =
			CStudioLightRig3DSpawner::SpawnInto(
				TargetGraph, FStudioLightRig3DParams{}, ForeignParent.Node );
		Harness.Check( ForeignResult.IsEmpty(), "別場面の親を拒否する" );
		Harness.CheckEqualU64( TargetGraph.RegisteredCount(), BeforeCount,
			"無効親では1灯も残さない" );

		FStudioLightRig3DParams Invalid;
		Invalid.RimIntensity = -1.0f;
		const FStudioLightRig3DSpawnResult InvalidResult =
			CStudioLightRig3DSpawner::SpawnInto( TargetGraph, Invalid );
		Harness.Check( InvalidResult.IsEmpty(), "不正な灯設定を拒否する" );
		Harness.CheckEqualU64( TargetGraph.RegisteredCount(), BeforeCount,
			"不正入力では1灯も残さない" );
	}
}
