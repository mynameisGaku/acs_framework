// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Sample/Scene/Demo3DScene.h"

#include "AcsFramework_Core/Scene/Model3D/Model3DSpawner.h"

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
		CModel3DSpawner::SpawnInto( Root(), Ball );
	}

	// 回す立方体。動いていることと、面ごとの陰りの差が分かる。
	FModel3DSpawnParams Cube = FModel3DSpawnParams::FromPrimitive( EMeshPrimitive3D::Cube, FVec3{ 0.0f, 1.2f, -3.0f } );
	Cube.Scale = FVec3{ 1.4f, 1.4f, 1.4f };
	Cube.Color = FVec4{ 0.90f, 0.75f, 0.30f, 1.0f };
	Cube.Name = FStringView( "Spinner" );
	m_Spinner = CModel3DSpawner::SpawnInto( Root(), Cube );

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
	Clouds().RenderScale = 3.0f;   // 画面の 3/4 の寸法でトレースする (1.0 は 1/4)
	Clouds().BaseAltitude = 2600.0f;  // 低いと地平線を真横から貫いて、そこだけ粗く見える
	Clouds().TopAltitude = 5200.0f;

	// 大気が描く «地面» の色を、置いた床に寄せる。ここがずれると、地平線から下だけ
	// 別の場所の色になり、床の縁で色が切り替わって見える。
	Atmosphere().ground_albedo = FVec3{ 0.06f, 0.07f, 0.06f };

	// 全体が入る位置までカメラを引く。
	FrameScene();

	// カメラを固定する。自由カメラを入れたままだと矢印キーで画角が変わり、
	// 撮り比べたときに «何が変わったのか» が分からなくなる。
	SetFreeCameraEnabled( false );
	SetOrbit( FVec3{ 0.0f, 1.0f, 0.0f }, 0.0f, 0.32f, 14.0f );

}


void ADemo3DScene::OnUpdate( f32 DeltaSeconds ) noexcept
{
	ALegacyScene3DAdapter::OnUpdate( DeltaSeconds );

	if ( m_Spinner == nullptr ) return;

	m_SpinDegrees += kSpinSpeed * DeltaSeconds;
	if ( m_SpinDegrees >= 360.0f ) m_SpinDegrees -= 360.0f;

	m_Spinner->Local().rotation = FQuat::Euler( 0.0f, m_SpinDegrees * 0.01745329252f, 0.0f );
}
