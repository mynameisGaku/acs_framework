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

	/** 往復させる速さ (world 単位 / 秒)。 */
	constexpr f32 kMoveSpeed = 1.8f;

	/** screenshotでも見つけやすいようにhit effectを繰り返す間隔。 */
	constexpr f32 kEffectRepeatSeconds = 1.25f;

	/** 回転するcubeの中心付近に置くeffect位置。 */
	constexpr FVec3 kEffectPosition{ 0.0f, 1.4f, -3.0f };

	/** Effekseer sampleの広さをACSのmeter尺度へ合わせる倍率。 */
	constexpr f32 kEffectScale = 0.35f;

	/** screenshot用hit effectの位置と倍率を揃える。 */
	FEffect3DPlayParams MakeDemoEffectParams() noexcept
	{
		FEffect3DPlayParams Params = FEffect3DPlayParams::At( kEffectPosition );
		Params.Scale = FVec3{ kEffectScale, kEffectScale, kEffectScale };
		return Params;
	}
}


void ADemo3DScene::OnEnter() noexcept
{
	AEffect3DScene::OnEnter();

	// 床。大きく平たい板 1 枚。影の落ち方が見えるように置く。
	FModel3DSpawnParams Floor = FModel3DSpawnParams::FromPrimitive( EMeshPrimitive3D::Plane, FVec3{ 0.0f, 0.0f, 0.0f } );
	Floor.Scale = FVec3{ kFloorSize, 1.0f, kFloorSize };
	Floor.Color = FVec4{ 0.55f, 0.56f, 0.58f, 1.0f };
	Floor.Roughness = 0.10f;   // 磨いた床。低いほど反射 (SSR) が乗る
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
		Model.Color = FVec4{ 0.92f, 0.62f, 0.28f, 1.0f };
		Model.Roughness = 0.40f;
		Model.Name = FStringView( "ImportedModel" );
		m_Mover = CModel3DSpawner::SpawnInto( Root(), Model, Assets->Models() );

		// 骨で動くモデル。読み口が別なので、置き方も別 (部品を自分で付ける)。
		// **骨の入っていない FBX を渡すと読めない。** そのときは 1 行出て、何も置かれない。
		TSharedPtr<ASkinnedMeshAsset> Skinned =
			Assets->Models().LoadSkinned( FStringView( "Models/SkinnedAnimated.fbx" ) );
		if ( Skinned )
		{
			TObjectPtr<ANode> Node = NewObject<ANode>();
			Node->SetName( FStringView( "Animated" ) );
			Node->SetPosition( FVec3{ 3.6f, 0.2f, 1.4f } );
			Node->SetScale( 0.02f );   // 書き出し単位がセンチメートル

			ASkinnedMeshComponent3D& Skin = Node->AddComponent<ASkinnedMeshComponent3D>();
			Skin.SetMeshAsset( Skinned );
			Skin.SetColor( FVec3{ 0.72f, 0.78f, 0.86f } );
			Skin.Play( 0u );

			Root().AddChild( Move( Node ) );
		}
	}

	// 太陽。
	//
	// 向きは «面から光源へ向かう» 側で、無回転だと真上。**第 1 引数は «真上から何ラジアン
	// 倒すか»** であって仰角ではない。0.95 で真上から 54 度、つまり仰角 36 度。
	//
	// 角度を選んだ理由は 2 つあって、どちらも «床に何が映るか» で決まる。
	//
	// - **高い太陽は床に映らない。** 見下ろしている床の鏡像は «水平よりやや上» を向くので、
	//   仰角 60 度の太陽の像は画面の外へ行く。低くするほど像が手前へ降りてくる
	// - **方位が合っていないと映らない。** カメラの正面と太陽の方位がずれていると、
	//   像は床の左右どちらかの外へ外れる。-0.62 は、手前左に glare が来て
	//   白飛びしない程度に外した位置
	//
	// 以前は真上に近い位置に置いていたので «床に太陽が映らない» ように見えていた。
	// 描けていなかったのではなく、映る場所が画面の外だった。
	TObjectPtr<ANode> SunNode = NewObject<ANode>();
	SunNode->SetName( FStringView( "Sun" ) );
	SunNode->Local().rotation = FQuat::Euler( 0.95f, -0.62f, 0.0f );

	ALightComponent3D& Sun = SunNode->AddComponent<ALightComponent3D>();
	Sun.SetLightKind( ELight3DKind::Directional );
	Sun.SetColor( FVec3{ 1.0f, 0.96f, 0.90f } );
	Sun.SetIntensity( 1.6f );
	Root().AddChild( Move( SunNode ) );

	// 点光源は置かない。
	//
	// 以前は «平行光源だけだと陰が硬い» ので 1 灯足していたが、いまは空を焼いた環境光 (IBL)
	// が陰の側を持ち上げている。**役目が重なったうえ、害の方が大きかった。**
	// 磨いた床に点光源の鏡像が «見えない電球» の白い点として写り込み、太陽とも反射とも
	// 辻褄の合わない光として見えていた。

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

	// 間接光。赤い球や黄色い立方体で跳ね返った色を、近くの床や物へ薄く回す。
	// 画面外は探せないので、広げすぎずこの場面の物同士が届く距離に留める。
	GlobalIllumination().Intensity = 0.75f;
	GlobalIllumination().MaxDistance = 5.0f;

	// 反射。磨いた床と金属に、画面に映っているものを映す。
	// **画面に映っていないものは映せない** ので、切っておく方が素直な場面もある。
	Reflections().Intensity = 0.9f;

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

	// 素材名と明示した置き方だけで3D effectを出す。renderer準備前なら基底が開始まで保持する。
	m_EffectElapsedSeconds = 0.0f;
	PlayEffect3D( FStringView( "Effects/hit.efkefc" ), MakeDemoEffectParams() );
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
	AEffect3DScene::OnUpdate( DeltaSeconds );
	ReportFrameTime( DeltaSeconds );

	// 準備中に再生要求を溜めず、短いhit素材を1つずつ確認できる間隔で繰り返す。
	if ( Effects3D().IsReady() )
	{
		m_EffectElapsedSeconds += DeltaSeconds;
		if ( m_EffectElapsedSeconds >= kEffectRepeatSeconds )
		{
			m_EffectElapsedSeconds -= kEffectRepeatSeconds;
			PlayEffect3D( FStringView( "Effects/hit.efkefc" ), MakeDemoEffectParams() );
		}
	}

	// 回す。度のまま足せる (ラジアンへ直す必要は無い)。
	if ( m_Spinner != nullptr ) m_Spinner->RotateDeg( FVec3{ 0.0f, kSpinSpeed * DeltaSeconds, 0.0f } );

	// 取り込んだモデルを 2 点のあいだで往復させ、進む先を向かせる。
	// **«動かす» のに要るのはこれだけ**、というのを見せるための最小の動き。
	if ( m_Mover == nullptr ) return;

	if ( m_Mover->MoveToward( m_MoveTarget, kMoveSpeed * DeltaSeconds ) )
	{
		m_MoveTarget.z = -m_MoveTarget.z;
	}
	m_Mover->LookAt( m_MoveTarget );
}
