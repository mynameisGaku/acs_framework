// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Light3D/Lamp3DSpawner.h"
#include "Common/Test/TestHarness.h"

#include <limits>

namespace
{
	/** 置いたノードから見た目の部品を取り出す。 */
	AMeshComponent3D* MeshOf_Internal( ANode* Node ) noexcept
	{
		return Node != nullptr ? Node->GetComponent<AMeshComponent3D>() : nullptr;
	}

	/** 置いたノードから光の部品を取り出す。 */
	ALightComponent3D* LightOf_Internal( ANode* Node ) noexcept
	{
		return Node != nullptr ? Node->GetComponent<ALightComponent3D>() : nullptr;
	}

	/** 3成分を小さな浮動小数誤差を許して比較する。 */
	void CheckVector_Internal( CTestHarness& Harness, FVec3 Actual,
		FVec3 Expected, const char* Label ) noexcept
	{
		constexpr f32 kTolerance = 0.0001f;
		Harness.Check( LengthSq( Actual - Expected )
			< kTolerance * kTolerance, Label );
	}
}


void RunLamp3DTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "FLamp3DParams / 発光球と点光源を同じ指定から作る" );

	{
		FLamp3DParams Params = FLamp3DParams::At(
			FVec3{ 1.0f, 2.5f, -3.0f } );
		Params.Radius = 0.25f;
		Params.Color = FVec3{ 0.20f, 0.55f, 1.0f };
		Params.EmissiveStrength = 5.0f;
		Params.LightIntensity = 2.5f;
		Params.Range = 7.0f;
		Params.BulbName = FStringView( "BlueBulb" );
		Params.LightName = FStringView( "BlueLight" );

		FModel3DSpawnParams Bulb;
		FLight3DSpawnParams Light;
		Harness.Check( Params.IsValid(), "位置と見た目から有効なランプになる" );
		Harness.Check( Params.TryBuildParts( Bulb, Light ),
			"発光球と点光源の指定を一括計算できる" );
		Harness.Check( Bulb.Primitive == EMeshPrimitive3D::Sphere,
			"光源本体は球になる" );
		CheckVector_Internal( Harness, Bulb.Position, Params.Position,
			"発光球へ指定位置を渡す" );
		CheckVector_Internal( Harness, Bulb.Scale,
			FVec3{ 0.5f, 0.5f, 0.5f },
			"ローカル直径1の球を指定半径へ合わせる" );
		Harness.Check( !Bulb.bCastsShadow,
			"小さな発光球自身は影を落とさない" );
		Harness.CheckEqualF32( Bulb.Roughness, 0.35f,
			"発光していない部分にも柔らかな艶を残す" );
		CheckVector_Internal( Harness, Bulb.EmissiveColor, Params.Color,
			"発光球へ共有色を渡す" );
		Harness.CheckEqualF32( Bulb.EmissiveStrength, 5.0f,
			"発光球へHDR強度を渡す" );
		Harness.Check( Bulb.Name == FStringView( "BlueBulb" ),
			"発光球名を渡す" );

		Harness.Check( Light.Kind == ELight3DKind::Point,
			"周囲を照らす側は点光源になる" );
		CheckVector_Internal( Harness, Light.Position, Params.Position,
			"点光源を発光球と同じ位置へ置く" );
		CheckVector_Internal( Harness, Light.Color, Params.Color,
			"点光源へ発光球と同じ色を渡す" );
		Harness.CheckEqualF32( Light.Intensity, 2.5f,
			"点光源へ照明強度を渡す" );
		Harness.CheckEqualF32( Light.Range, 7.0f,
			"点光源へ到達距離を渡す" );
		Harness.Check( Light.Name == FStringView( "BlueLight" ),
			"点光源名を渡す" );
	}

	Harness.BeginSuite( "FLamp3DParams / 不正値では出力を変えない" );

	{
		const f32 QuietNaN = std::numeric_limits<f32>::quiet_NaN();
		const f32 Maximum = std::numeric_limits<f32>::max();
		FLamp3DParams InvalidPosition;
		InvalidPosition.Position.x = QuietNaN;
		FLamp3DParams ZeroRadius;
		ZeroRadius.Radius = 0.0f;
		FLamp3DParams NegativeColor;
		NegativeColor.Color.y = -0.1f;
		FLamp3DParams TooBrightColor;
		TooBrightColor.Color.z = 1.01f;
		FLamp3DParams InvalidEmission;
		InvalidEmission.EmissiveStrength = 10.01f;
		FLamp3DParams NegativeLight;
		NegativeLight.LightIntensity = -0.1f;
		FLamp3DParams ZeroRange;
		ZeroRange.Range = 0.0f;
		FLamp3DParams OverflowRadius;
		OverflowRadius.Radius = Maximum;

		Harness.Check( !InvalidPosition.IsValid(), "有限でない位置を拒否する" );
		Harness.Check( !ZeroRadius.IsValid(), "0以下の半径を拒否する" );
		Harness.Check( !NegativeColor.IsValid(), "負の共有色を拒否する" );
		Harness.Check( !TooBrightColor.IsValid(),
			"自己発光材質の上限を超える共有色を拒否する" );
		Harness.Check( !InvalidEmission.IsValid(),
			"HDR自己発光の上限を超える強度を拒否する" );
		Harness.Check( !NegativeLight.IsValid(), "負の照明強度を拒否する" );
		Harness.Check( !ZeroRange.IsValid(), "0以下の到達距離を拒否する" );
		Harness.Check( !OverflowRadius.IsValid(),
			"直径への変換で溢れる半径を拒否する" );

		FModel3DSpawnParams Bulb = FModel3DSpawnParams::FromPrimitive(
			EMeshPrimitive3D::Plane, FVec3{ 8.0f, 7.0f, 6.0f } );
		Bulb.Name = FStringView( "KeepBulb" );
		FLight3DSpawnParams Light = FLight3DSpawnParams::Sun(
			FVec3{ 1.0f, 1.0f, 0.0f } );
		Light.Name = FStringView( "KeepLight" );
		Harness.Check( !InvalidPosition.TryBuildParts( Bulb, Light ),
			"不正な指定の変換は失敗する" );
		Harness.Check( Bulb.Primitive == EMeshPrimitive3D::Plane
			&& Bulb.Name == FStringView( "KeepBulb" )
			&& Light.Kind == ELight3DKind::Directional
			&& Light.Name == FStringView( "KeepLight" ),
			"失敗時は2つの出力を変更しない" );
	}

	Harness.BeginSuite( "CLamp3DSpawner / 見える発光球と点光源を一括配置する" );

	{
		CSceneNodeGraph Graph;
		const FScene3DSpawnResult Parent = Graph.TrySpawn(
			FStringView( "LampRoot" ) );
		FLamp3DParams Params = FLamp3DParams::At(
			FVec3{ -1.0f, 2.0f, 3.0f } );
		Params.Radius = 0.20f;
		FLamp3DSpawnResult Spawned = CLamp3DSpawner::SpawnInto(
			Graph, Params, Parent.Node );

		ANode* const Bulb = Spawned.Bulb();
		ANode* const LightNode = Spawned.Light();
		AMeshComponent3D* const Mesh = MeshOf_Internal( Bulb );
		ALightComponent3D* const Light = LightOf_Internal( LightNode );
		Harness.Check( Spawned.Succeeded(),
			"発光球と点光源を両方置ける" );
		Harness.CheckEqualU64( Graph.RegisteredCount(), 4u,
			"root、指定親、発光球、点光源だけを登録する" );
		Harness.Check( Bulb != nullptr && LightNode != nullptr
			&& Bulb->Parent() == Parent.Node
			&& LightNode->Parent() == Parent.Node,
			"2ノードへ同じ親変形を適用する" );
		Harness.Check( Bulb != nullptr
			&& Bulb->Name() == FStringView( "LampBulb" )
			&& LightNode != nullptr
			&& LightNode->Name() == FStringView( "LampLight" ),
			"2ノードを役割名で区別できる" );
		CheckVector_Internal( Harness,
			Bulb != nullptr ? Bulb->Local().position : FVec3{},
			Params.Position, "発光球を指定位置へ置く" );
		CheckVector_Internal( Harness,
			LightNode != nullptr ? LightNode->Local().position : FVec3{},
			Params.Position, "点光源を発光球と同じ位置へ置く" );
		Harness.Check( Mesh != nullptr
			&& Mesh->Primitive() == EMeshPrimitive3D::Sphere
			&& !Mesh->CastsShadow(),
			"影を落とさない球表示を付ける" );
		Harness.Check( Mesh != nullptr && Mesh->MaterialLoaded(),
			"発光球のPBR材質を確定する" );
		if ( Mesh != nullptr && Mesh->MaterialLoaded() )
		{
			CheckVector_Internal( Harness, Mesh->Material().pbr.emissive,
				Params.Color, "発光材質へ共有色を渡す" );
			Harness.CheckEqualF32( Mesh->Material().pbr.emissiveStrength,
				Params.EmissiveStrength, "発光材質へHDR強度を渡す" );
		}
		Harness.Check( Light != nullptr
			&& Light->LightKind() == ELight3DKind::Point,
			"ACSの点光源部品を付ける" );
		if ( Light != nullptr )
		{
			FPointLight Output{};
			Harness.Check( Light->FillPoint( Output ),
				"点光源を描画値へ変換できる" );
			Harness.CheckNearF32( Output.range, Params.Range, 0.0001f,
				"点光源の到達距離を描画へ渡す" );
			Harness.CheckNearF32( Output.color.x,
				Params.Color.x * Params.LightIntensity, 0.0001f,
				"点光源の共有色へ照明強度を掛ける" );
		}

		CSceneNodeGraph WrongGraph;
		Harness.Check( !CLamp3DSpawner::Destroy( WrongGraph, Spawned ),
			"別場面からの一括破棄を拒否する" );
		Harness.Check( Spawned.Succeeded()
			&& Bulb != nullptr && !Bulb->IsPendingDestroy()
			&& LightNode != nullptr && !LightNode->IsPendingDestroy(),
			"別場面による失敗時は結果と2ノードを変えない" );
		Harness.Check( CLamp3DSpawner::Destroy( Graph, Spawned ),
			"生成時の場面からランプを一括破棄できる" );
		Harness.Check( Spawned.IsEmpty(),
			"破棄成功時は生成結果を空にする" );
		Harness.Check( Bulb != nullptr && Bulb->IsPendingDestroy()
			&& LightNode != nullptr && LightNode->IsPendingDestroy(),
			"発光球と点光源を両方破棄予定へ移す" );
		Graph.ResolveStructuralChanges();
		Harness.CheckEqualU64( Graph.RegisteredCount(), 2u,
			"確定後はrootと指定親だけを残す" );
	}

	Harness.BeginSuite( "CLamp3DSpawner / 個別破棄後も残りを片付ける" );

	{
		CSceneNodeGraph Graph;
		FLamp3DSpawnResult Spawned = CLamp3DSpawner::SpawnInto(
			Graph, FLamp3DParams{} );
		Harness.Check( Spawned && Graph.Destroy( Spawned.BulbId() ),
			"発光球だけを先に破棄予定へ移せる" );
		Harness.Check( Spawned.Bulb() == nullptr,
			"破棄予定の発光球を生存中として返さない" );
		Harness.Check( CLamp3DSpawner::Destroy( Graph, Spawned ),
			"残る点光源も一括で片付けられる" );
		Graph.ResolveStructuralChanges();
		Harness.CheckEqualU64( Graph.RegisteredCount(), 1u,
			"個別破棄を含めてランプを残さない" );
	}

	Harness.BeginSuite( "CLamp3DSpawner / 無効親と入力を半端に残さない" );

	{
		CSceneNodeGraph OwnerGraph;
		const FScene3DSpawnResult ForeignParent = OwnerGraph.TrySpawn(
			FStringView( "ForeignParent" ) );
		CSceneNodeGraph TargetGraph;
		const u32 BeforeCount = TargetGraph.RegisteredCount();
		const FLamp3DSpawnResult ForeignResult = CLamp3DSpawner::SpawnInto(
			TargetGraph, FLamp3DParams{}, ForeignParent.Node );
		Harness.Check( ForeignResult.IsEmpty(), "別場面の親を拒否する" );
		Harness.CheckEqualU64( TargetGraph.RegisteredCount(), BeforeCount,
			"無効親では発光球も点光源も残さない" );

		FLamp3DParams Invalid;
		Invalid.Range = -1.0f;
		const FLamp3DSpawnResult InvalidResult = CLamp3DSpawner::SpawnInto(
			TargetGraph, Invalid );
		Harness.Check( InvalidResult.IsEmpty(), "不正なランプ設定を拒否する" );
		Harness.CheckEqualU64( TargetGraph.RegisteredCount(), BeforeCount,
			"不正入力では発光球も点光源も残さない" );
	}
}
