// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Sample/Scene/Demo3DScene.h"

#include "AcsFramework_Core/Scene/Model3D/Model3DSpawner.h"

#include "AcsFramework_Core/Assets/AssetLoaderSubsystem.h"

namespace
{
	/** 床の広さ。広げすぎると FrameScene がカメラを引きすぎ、物が豆粒になる。 */
	constexpr f32 kFloorSize = 9.0f;

	/** 並べる物の間隔。 */
	constexpr f32 kSpacing = 2.4f;

	/** 回す速さ (度 / 秒)。 */
	constexpr f32 kSpinSpeed = 30.0f;
}


void ADemo3DScene::OnEnter() noexcept
{
	ALegacyScene3DAdapter::OnEnter();

	// 床。大きく平たい板 1 枚。影の落ち方が見えるように置く。
	FModel3DSpawnParams Floor = FModel3DSpawnParams::FromPrimitive( EMeshPrimitive3D::Plane, FVec3{ 0.0f, 0.0f, 0.0f } );
	Floor.Scale = FVec3{ kFloorSize, 1.0f, kFloorSize };
	Floor.Color = FVec4{ 0.55f, 0.56f, 0.58f, 1.0f };
	Floor.Roughness = 0.14f;   // 磨いた床。低いほど反射 (SSR) が乗る
	Floor.bCastsShadow = false;
	Floor.Name = FStringView( "Floor" );
	CModel3DSpawner::SpawnInto( Root(), Floor );

	// 並べた球。色違いで陰りの出方を見比べられるようにする。
	// 立方体より手前 (z = +1.6) へ置く。同じ列だと真ん中の球が立方体に隠れる。
	const FVec4 Colors[] =
	{
		FVec4{ 0.85f, 0.25f, 0.22f, 1.0f },
		FVec4{ 0.25f, 0.70f, 0.40f, 1.0f },
		FVec4{ 0.25f, 0.45f, 0.90f, 1.0f },
	};

	for ( usize Index = 0u; Index < 3u; ++Index )
	{
		const f32 X = ( static_cast<f32>( Index ) - 1.0f ) * kSpacing;

		FModel3DSpawnParams Ball = FModel3DSpawnParams::FromPrimitive( EMeshPrimitive3D::Sphere, FVec3{ X, 1.0f, 1.6f } );
		Ball.Color = Colors[Index];
		// 粗さを 3 つで振る。同じ色でも «艶» が違うと材質の違いとして読める。
		Ball.Roughness = 0.12f + static_cast<f32>( Index ) * 0.34f;
		CModel3DSpawner::SpawnInto( Root(), Ball );
	}

	// 回す立方体。動いていることと、面ごとの陰りの差が分かる。
	FModel3DSpawnParams Cube = FModel3DSpawnParams::FromPrimitive( EMeshPrimitive3D::Cube, FVec3{ 0.0f, 1.2f, -3.0f } );
	Cube.Scale = FVec3{ 1.4f, 1.4f, 1.4f };
	Cube.Color = FVec4{ 0.90f, 0.75f, 0.30f, 1.0f };
	Cube.Metallic = 1.0f;      // 金属。拡散反射が消えるので、環境光と反射が要る
	Cube.Roughness = 0.28f;
	Cube.Name = FStringView( "Spinner" );
	m_Spinner = CModel3DSpawner::SpawnInto( Root(), Cube );

	// Assets に置いた FBX。**置き場からモデルを読む道が通っていることの確認**でもある。
	// 読めなければ置かずに nullptr が返り、理由が 1 行出る (黙って消えない)。
	if ( CAssetLoaderSubsystem* const Assets = GetSubsystem<CAssetLoaderSubsystem>() )
	{
		FModel3DSpawnParams Model =
			FModel3DSpawnParams::FromMesh( FStringView( "Models/MergedSphere.fbx" ), FVec3{ -3.4f, 1.0f, 2.4f } );
		Model.Scale = FVec3{ 1.4f, 1.4f, 1.4f };
		Model.RotationDeg = FVec3{ 0.0f, -35.0f, 0.0f };
		Model.Color = FVec4{ 0.92f, 0.62f, 0.28f, 1.0f };
		Model.Roughness = 0.40f;
		Model.Name = FStringView( "ImportedModel" );
		CModel3DSpawner::SpawnInto( Root(), Model, Assets->Models() );
	}

	// 太陽。斜め上から差す。ここを消すとエンジン既定の太陽に落ちる。
	//
	// 向きは «面から光源へ向かう» 側なので、無回転だと真上に太陽がある。そのままだと
	// 陰影が平坦なので、X まわりに -35 度・Y まわりに -30 度だけ倒して斜めから差させる。
	TObjectPtr<ANode> SunNode = NewObject<ANode>();
	SunNode->SetName( FStringView( "Sun" ) );
	SunNode->Local().rotation = FQuat::Euler( -0.611f, -0.524f, 0.0f );

	ALightComponent3D& Sun = SunNode->AddComponent<ALightComponent3D>();
	Sun.SetLightKind( ELight3DKind::Directional );
	Sun.SetColor( FVec3{ 1.0f, 0.96f, 0.90f } );
	Sun.SetIntensity( 1.6f );
	Root().AddChild( Move( SunNode ) );

	// 手前を持ち上げる点光源。平行光源だけだと陰が硬いので、差が出るように 1 灯足す。
	TObjectPtr<ANode> FillNode = NewObject<ANode>();
	FillNode->SetName( FStringView( "Fill" ) );
	FillNode->Local().position = FVec3{ 2.5f, 2.0f, 3.5f };

	ALightComponent3D& Fill = FillNode->AddComponent<ALightComponent3D>();
	Fill.SetLightKind( ELight3DKind::Point );
	Fill.SetColor( FVec3{ 0.55f, 0.70f, 1.0f } );
	Fill.SetIntensity( 2.0f );
	Fill.SetRange( 14.0f );
	Root().AddChild( Move( FillNode ) );

	// 本物の雲を出す。太陽の側が明るく、縁が光る。
	//
	// 濃さは既定 (1.6) より上げてある。薄いと光が素通りするので、位相も消散も多重散乱も
	// 効く相手が無く、**灰色の靄にしかならない**。厚みがあって初めて上面と底面に差が出る。
	Clouds().Coverage = 0.68f;
	Clouds().Density = 2.8f;
	Clouds().BaseAltitude = 2600.0f;  // 低いと地平線を真横から貫いて、そこだけ粗く見える
	Clouds().TopAltitude = 5200.0f;

	// 上に薄い高い雲を敷く。1 枚だけだと空の «高さ» が読めない。
	// 同じレイで両方を通るので、2 倍にはならない。
	Clouds().UpperLayer.BaseAltitude = 7400.0f;
	Clouds().UpperLayer.TopAltitude = 9200.0f;

	// トレースの解像度。**雲の値段はほぼここで決まる。**
	//
	// 上を向いて画面全部が雲になる最悪の構図を Release で測ると (1296x759):
	//
	//   雲なし  3.4 ms   /  1/4 (1.0)  13.7 ms
	//   1/2 (2.0) 30.0 ms  /  3/4 (3.0)  40.6 ms
	//
	// ここは**見え方の摘みであって、速さの摘みではない**。重いなら描き方を速くする話で、
	// 粗く描いて誤魔化す話ではない。3.0 のまま置く。
	Clouds().RenderScale = 3.0f;

	// 遮蔽。物と床の接するところを締める。半径は場面の大きさに合わせる
	// (ここは球の直径が 1 前後なので 0.5)。
	AmbientOcclusion().Intensity = 1.0f;
	AmbientOcclusion().Radius = 0.5f;

	// 反射。磨いた床と金属に、画面に映っているものを映す。
	// **画面に映っていないものは映せない** ので、切っておく方が素直な場面もある。
	Reflections().Intensity = 0.6f;

	// 大気が描く «地面» の色を、置いた床に寄せる。ここがずれると、地平線から下だけ
	// 別の場所の色になり、床の縁で色が切り替わって見える。
	Atmosphere().ground_albedo = FVec3{ 0.06f, 0.07f, 0.06f };

	// 全体が入る位置までカメラを引く。
	FrameScene();

	// 見回せるようにしておく。矢印キーで回り、WASD で寄る。
	//
	// 切ると画角が動かなくなる。**撮り比べるときは切ること。** 入れたままだと撮影中の
	// キー入力で画角が変わり、«何が変わったのか» が分からない画が並ぶ。
	SetFreeCameraEnabled( true );
	SetOrbit( FVec3{ 0.0f, 1.0f, 0.0f }, 0.0f, 0.32f, 14.0f );
}


void ADemo3DScene::ReportFrameTime( f32 DeltaSeconds ) noexcept
{
	m_FrameTimeAccum += DeltaSeconds;
	++m_FrameCount;

	if ( m_FrameTimeAccum < 1.0f ) return;

	const f32 Milliseconds = m_FrameTimeAccum * 1000.0f / static_cast<f32>( m_FrameCount );
	ACS_LOG_INFO( "Demo3D: %.2f ms/frame (%u fps)",
		static_cast<f64>( Milliseconds ), m_FrameCount );

	m_FrameTimeAccum = 0.0f;
	m_FrameCount = 0u;
}


void ADemo3DScene::OnUpdate( f32 DeltaSeconds ) noexcept
{
	ALegacyScene3DAdapter::OnUpdate( DeltaSeconds );
	ReportFrameTime( DeltaSeconds );

	if ( m_Spinner == nullptr ) return;

	m_SpinDegrees += kSpinSpeed * DeltaSeconds;
	if ( m_SpinDegrees >= 360.0f ) m_SpinDegrees -= 360.0f;

	m_Spinner->Local().rotation = FQuat::Euler( 0.0f, m_SpinDegrees * 0.01745329252f, 0.0f );
}
