// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Sample/Scene/Demo3DScene.h"

#include "AcsFramework_Core/Scene/Animation3D/AnimatedModel3DSpawner.h"
#include "AcsFramework_Core/Scene/Light3D/Light3DSpawner.h"
#include "AcsFramework_Core/Scene/Model3D/Model3DSpawner.h"
#include "AcsFramework_Core/Scene/Pick3D/ScenePicker.h"
#include "AcsFramework_Core/Scene/Sprite3D/Sprite3DSpawner.h"
#include "AcsFramework_Core/Scene/Water3D/Water3DSpawner.h"
#include "AcsFramework_Core/UI/WorldLabel3D/WorldLabel3DParams.h"

#include "AcsFramework_Core/Assets/AssetLoaderSubsystem.h"
#include "AcsFramework_Core/Audio/Spatial/SpatialAudioSubsystem.h"
#include "AcsFramework_Core/Settings/GameSettingsSubsystem.h"
#include "Common/Compat/AcsEnumReflection.h"

#include <cmath>
#include <cstring>

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

	/** 操作キャラクターと歩ける面・障害物を結ぶ衝突レイヤー。 */
	constexpr u32 kCharacterCollisionLayer = 0x1u;

	/** 初期画面でキャラクターと展示物を同時に見せる足元位置。 */
	constexpr FVec3 kCharacterStartPosition{ 0.0f, 0.001f, 4.2f };

	/** screenshotでも見つけやすいようにhit effectを繰り返す間隔。 */
	constexpr f32 kEffectRepeatSeconds = 1.25f;

	/** 回転するcubeの中心付近に置くeffect位置。 */
	constexpr FVec3 kEffectPosition{ 0.0f, 1.4f, -3.0f };

	/** Effekseer sampleの広さをACSのmeter尺度へ合わせる倍率。 */
	constexpr f32 kEffectScale = 0.35f;

	/** プレイヤーUIの左端。 */
	constexpr f32 kUiLeft = 24.0f;

	/** プレイヤーUIの上端。 */
	constexpr f32 kUiTop = 24.0f;

	/** プレイヤーUIのカード幅。 */
	constexpr f32 kUiWidth = 260.0f;

	/** プレイヤーUIのカード高さ。 */
	constexpr f32 kUiHeight = 696.0f;

	/** FXAA切り替えに使うアクション番号。 */
	constexpr u32 kFxaaActionIndex = 0u;

	/** 初回起動時のFXAA切り替えキー。 */
	constexpr EKey kDefaultFxaaKey = EKey::F;

	/** 設定ファイルへ保存するFXAAキーの名前。 */
	constexpr const char* kFxaaKeySetting = "Input.FxaaToggleKey";

	/** デモで操作するゲームパッド番号。 */
	constexpr u32 kGamepadPlayerIndex = 0u;

	/** 初回起動時の第三者視点ジャンプボタン。 */
	constexpr EGamepadButton kDefaultJumpButton = EGamepadButton::South;

	/** 初回起動時の第三者視点前後移動軸。 */
	constexpr EGamepadAxis kDefaultMoveAxis = EGamepadAxis::LeftY;

	/** 第三者視点のゲームパッド軸へ適用する遊び。 */
	constexpr f32 kGamepadDeadZone = 0.15f;

	/** 軸の選択操作とみなす最小絶対値。 */
	constexpr f32 kGamepadAxisCaptureThreshold = 0.65f;

	/** 新しい軸入力を待つ前に中立へ戻ったとみなす最大絶対値。 */
	constexpr f32 kGamepadAxisCenterThreshold = 0.25f;

	/** 設定ファイルへ保存するジャンプボタンの名前。 */
	constexpr const char* kJumpButtonSetting = "Input.ThirdPersonJumpGamepadButton";

	/** 設定ファイルへ保存する前後移動軸の名前。 */
	constexpr const char* kMoveAxisSetting = "Input.ThirdPersonMoveGamepadAxis";

	/** 左右定位デモで鳴らす、短いモノラル効果音。 */
	constexpr const char* kSpatialSoundAsset = "Audio/SpatialPulse.wav";

	/** 水面の中心。床より少し上げ、同一面のちらつきを避ける。 */
	constexpr FVec3 kWaterPosition{ 2.7f, 0.04f, -1.35f };

	/** 水面のX/Z方向の広さ。 */
	constexpr FVec2 kWaterSize{ 3.4f, 2.8f };

	/** 水面に次の波紋を作る間隔。 */
	constexpr f32 kWaterRippleRepeatSeconds = 1.8f;

	/** 毎回同じ順序で使う、水面中心からの波紋位置。 */
	constexpr FVec2 kWaterRippleOffsets[] =
	{
		FVec2{ -0.82f, -0.46f },
		FVec2{ 0.68f, 0.34f },
		FVec2{ -0.18f, 0.70f },
		FVec2{ 0.76f, -0.62f },
	};

	/** 波紋位置の数。 */
	constexpr usize kWaterRipplePointCount = sizeof( kWaterRippleOffsets ) / sizeof( kWaterRippleOffsets[0] );

	/** 聴く位置から音源を前へ離す距離。 */
	constexpr f32 kSpatialSoundForwardDistance = 4.0f;

	/** 聴く位置から音源を左右へ離す距離。 */
	constexpr f32 kSpatialSoundSideDistance = 4.0f;

	/** デモの天候が次の状態へ移り切るまでの秒数。 */
	constexpr f32 kWeatherTransitionSeconds = 2.5f;

	/** 実形状判定の線と命中箱を見せ続ける秒数。 */
	constexpr f32 kGeometryPickDebugSeconds = 2.5f;

	/** 見た目の差が読みやすい順に巡回する天候。 */
	constexpr EWeatherKind kDemoWeatherCycle[] =
	{
		EWeatherKind::Storm,
		EWeatherKind::Fog,
		EWeatherKind::Clear,
	};

	/** 巡回する天候の数。 */
	constexpr usize kDemoWeatherCount = sizeof( kDemoWeatherCycle ) / sizeof( kDemoWeatherCycle[0] );

	/**
	 * デモのFXAA操作へ使えるキーかを返す。
	 *
	 * @details Escapeは自由カメラの終了操作と、割り当て待ちの取消に使うため予約する。
	 */
	bool IsDemoFxaaKey( EKey Key ) noexcept
	{
		return FActionKeyRebindState::IsValidKey( Key ) && Key != EKey::Escape;
	}

	/**
	 * キー名をUI用のNUL終端文字列へ写す。
	 *
	 * @param Key 表示するキー。
	 * @return 「KEYBOARD: F」のような表示文字列。
	 */
	FString MakeKeyLabel( EKey Key ) noexcept
	{
		FString Label( "KEYBOARD: " );
		const AcsFw::FEnumNameView Name = AcsFw::EnumToString( Key );
		if ( Name.IsEmpty() )
		{
			Label.TryAppend( FStringView( "UNKNOWN" ) );
			return Label;
		}

		Label.TryAppend( FStringView( Name.Data, Name.Size ) );
		return Label;
	}

	/** ゲームパッドボタンをUI用の短い物理名へ変える。 */
	const char* GamepadButtonLabel( EGamepadButton Button ) noexcept
	{
		switch ( Button )
		{
		case EGamepadButton::South: return "SOUTH";
		case EGamepadButton::East: return "EAST";
		case EGamepadButton::West: return "WEST";
		case EGamepadButton::North: return "NORTH";
		case EGamepadButton::Up: return "DPAD UP";
		case EGamepadButton::Down: return "DPAD DOWN";
		case EGamepadButton::Left: return "DPAD LEFT";
		case EGamepadButton::Right: return "DPAD RIGHT";
		case EGamepadButton::LeftBumper: return "LEFT BUMPER";
		case EGamepadButton::RightBumper: return "RIGHT BUMPER";
		case EGamepadButton::LeftStick: return "LEFT STICK";
		case EGamepadButton::RightStick: return "RIGHT STICK";
		case EGamepadButton::Start: return "START";
		case EGamepadButton::Back: return "BACK";
		case EGamepadButton::Guide: return "GUIDE";
		default: return "UNKNOWN";
		}
	}

	/** ゲームパッド軸をUI用の短い物理名へ変える。 */
	const char* GamepadAxisLabel( EGamepadAxis Axis ) noexcept
	{
		switch ( Axis )
		{
		case EGamepadAxis::LeftX: return "LEFT X";
		case EGamepadAxis::LeftY: return "LEFT Y";
		case EGamepadAxis::RightX: return "RIGHT X";
		case EGamepadAxis::RightY: return "RIGHT Y";
		case EGamepadAxis::LeftTrigger: return "LEFT TRIGGER";
		case EGamepadAxis::RightTrigger: return "RIGHT TRIGGER";
		default: return "UNKNOWN";
		}
	}

	/** ジャンプボタンをUI用のNUL終端文字列へ写す。 */
	FString MakeJumpButtonLabel( EGamepadButton Button ) noexcept
	{
		FString Label( "JUMP PAD: " );
		Label.TryAppend( FStringView( GamepadButtonLabel( Button ) ) );
		return Label;
	}

	/** 前後移動軸をUI用のNUL終端文字列へ写す。 */
	FString MakeMoveAxisLabel( EGamepadAxis Axis ) noexcept
	{
		FString Label( "MOVE PAD: " );
		Label.TryAppend( FStringView( GamepadAxisLabel( Axis ) ) );
		return Label;
	}

	/** screenshot用hit effectの位置と倍率を揃える。 */
	FEffect3DPlayParams MakeDemoEffectParams() noexcept
	{
		FEffect3DPlayParams Params = FEffect3DPlayParams::At( kEffectPosition );
		Params.Scale = FVec3{ kEffectScale, kEffectScale, kEffectScale };
		return Params;
	}

	/** 天候種別をプレイヤーUI用の短い名前へ変える。 */
	const char* WeatherLabel( EWeatherKind Kind ) noexcept
	{
		switch ( Kind )
		{
		case EWeatherKind::Clear: return "CLEAR";
		case EWeatherKind::Cloudy: return "CLOUDY";
		case EWeatherKind::Rain: return "RAIN";
		case EWeatherKind::HeavyRain: return "HEAVY RAIN";
		case EWeatherKind::Snow: return "SNOW";
		case EWeatherKind::Storm: return "STORM";
		case EWeatherKind::Fog: return "FOG";
		case EWeatherKind::Sandstorm: return "SANDSTORM";
		default: return "UNKNOWN";
		}
	}

	/**
	 * 表示内容が変わったときだけUI文字列を置き換える。
	 *
	 * @param Layer 文字列を持つUI層。
	 * @param Handle 置き換える文字列またはボタンの識別子。
	 * @param Text 新しいNUL終端文字列。
	 */
	void SetUiTextIfChanged( CUiLayer& Layer, u32 Handle, const char* Text ) noexcept
	{
		if ( Text == nullptr ) return;
		const char* const CurrentText = Layer.Text( Handle );
		if ( CurrentText != nullptr && std::strcmp( CurrentText, Text ) == 0 ) return;
		Layer.SetText( Handle, Text );
	}

	/**
	 * デモ用の平面を識別子付きで置く。
	 *
	 * @param Graph 置くシーン。
	 * @param Position 平面の中心。
	 * @param Scale X/Z方向の広さ。
	 * @param Color 表面色。
	 * @param Roughness 表面の粗さ。
	 * @param Name ノード名。
	 * @return 置いた平面ノード。生成に失敗したらnullptr。
	 */
	ANode* SpawnDemoPlane( CSceneNodeGraph& Graph, FVec3 Position, FVec3 Scale, FVec4 Color,
		f32 Roughness, FStringView Name ) noexcept
	{
		FModel3DSpawnParams Plane = FModel3DSpawnParams::FromPrimitive( EMeshPrimitive3D::Plane, Position );
		Plane.Scale = Scale;
		Plane.Color = Color;
		Plane.Roughness = Roughness;
		Plane.bCastsShadow = false;
		Plane.Name = Name;
		return CModel3DSpawner::SpawnInto( Graph, Plane );
	}

	/**
	 * 描画平面の直下へ厚さ1の歩ける箱を登録する。
	 *
	 * @param Collision 登録先の場面衝突集合。
	 * @param Plane 描画平面ノード。
	 * @return 平面の現在拡縮に追従する箱を登録できたらtrue。
	 */
	bool TryAddWalkablePlane( CSceneCollision3D* Collision, ANode* Plane ) noexcept
	{
		return Collision != nullptr && Plane != nullptr && Collision->TryAddBox( *Plane, FVec3{ 0.0f, -0.5f, 0.0f }, FVec3{ 0.5f, 0.5f, 0.5f }, kCharacterCollisionLayer ).IsValid();
	}

	/**
	 * 素材ファイルなしで向きが読める操作キャラクターを置く。
	 *
	 * @param Graph 置く場面のノードグラフ。
	 * @return 足元原点の親ノード。全ての見た目を置けなければnullptr。
	 */
	ANode* SpawnThirdPersonCharacter( CSceneNodeGraph& Graph ) noexcept
	{
		const FScene3DSpawnResult RootSpawn = Graph.TrySpawn( FStringView( "ThirdPersonCharacter" ) );
		ANode* const Root = RootSpawn ? RootSpawn.Node : nullptr;
		if ( Root == nullptr ) return nullptr;
		Root->SetPosition( kCharacterStartPosition );
		Root->RotateDeg( FVec3{ 0.0f, 180.0f, 0.0f } );

		FModel3DSpawnParams Body = FModel3DSpawnParams::FromPrimitive( EMeshPrimitive3D::Cube, FVec3{ 0.0f, 0.72f, 0.0f } );
		Body.Scale = FVec3{ 0.72f, 1.12f, 0.46f };
		Body.Color = FVec4{ 0.10f, 0.56f, 0.78f, 1.0f };
		Body.Metallic = 0.18f;
		Body.Roughness = 0.28f;
		Body.Name = FStringView( "ThirdPersonBody" );

		FModel3DSpawnParams Head = FModel3DSpawnParams::FromPrimitive( EMeshPrimitive3D::Sphere, FVec3{ 0.0f, 1.52f, 0.0f } );
		Head.Scale = FVec3{ 0.48f, 0.48f, 0.48f };
		Head.Color = FVec4{ 0.72f, 0.90f, 0.96f, 1.0f };
		Head.Metallic = 0.08f;
		Head.Roughness = 0.20f;
		Head.Name = FStringView( "ThirdPersonHead" );

		FModel3DSpawnParams FacingMark = FModel3DSpawnParams::FromPrimitive( EMeshPrimitive3D::Sphere, FVec3{ 0.0f, 0.88f, 0.50f } );
		FacingMark.Scale = FVec3{ 0.18f, 0.18f, 0.18f };
		FacingMark.Color = FVec4{ 1.0f, 0.42f, 0.08f, 1.0f };
		FacingMark.Roughness = 0.24f;
		FacingMark.Name = FStringView( "ThirdPersonFacingMark" );

		const bool bComplete = CModel3DSpawner::SpawnInto( Graph, Body, Root ) != nullptr && CModel3DSpawner::SpawnInto( Graph, Head, Root ) != nullptr && CModel3DSpawner::SpawnInto( Graph, FacingMark, Root ) != nullptr;
		if ( bComplete ) return Root;

		Graph.Destroy( Root->Id() );
		return nullptr;
	}
}


void ADemo3DScene::OnEnter() noexcept
{
	AWeather3DScene::OnEnter();
	m_bInteractionRequested = false;
	m_GeometryPickDebugRemainingSeconds = 0.0f;
	FInteractionFocus3DParams InteractionParams;
	InteractionParams.MaximumDistance = 8.0f;
	if ( !InteractionFocus().SetParams( InteractionParams ) ) ACS_LOG_WARN( "Demo3D: 視線フォーカス距離を設定できなかった" );
	m_Spinner = nullptr;
	m_Mover = nullptr;
	m_WaterSurfaceId = FNodeId{};
	m_ThirdPersonCharacter.Unbind();
	m_CharacterCollision = MakeUnique<CSceneCollision3D>( Graph() );
	m_CharacterNode = nullptr;
	m_CharacterActionBindings.Clear();
	m_PreviousCharacterInput = FActionInput{};

	// 磨いた床。水面の直下だけ穴を空け、屈折が極浅い床を拾って白くならないようにする。
	constexpr FVec4 FloorColor{ 0.55f, 0.56f, 0.58f, 1.0f };
	ANode* const FloorLeft = SpawnDemoPlane( Graph(), FVec3{ -1.75f, 0.0f, 0.0f }, FVec3{ 5.5f, 1.0f, kFloorSize },
		FloorColor, 0.10f, FStringView( "FloorLeft" ) );
	ANode* const FloorFront = SpawnDemoPlane( Graph(), FVec3{ 2.7f, 0.0f, -3.625f }, FVec3{ 3.4f, 1.0f, 1.75f },
		FloorColor, 0.10f, FStringView( "FloorFront" ) );
	ANode* const FloorBack = SpawnDemoPlane( Graph(), FVec3{ 2.7f, 0.0f, 2.275f }, FVec3{ 3.4f, 1.0f, 4.45f },
		FloorColor, 0.10f, FStringView( "FloorBack" ) );
	ANode* const FloorRight = SpawnDemoPlane( Graph(), FVec3{ 4.45f, 0.0f, 0.0f }, FVec3{ 0.10f, 1.0f, kFloorSize },
		FloorColor, 0.10f, FStringView( "FloorRight" ) );

	// 水底。水面から十分離し、ACSの吸収と散乱が色として現れる深さを作る。
	ANode* const PoolBottom = SpawnDemoPlane( Graph(), FVec3{ kWaterPosition.x, -0.76f, kWaterPosition.z },
		FVec3{ kWaterSize.x, 1.0f, kWaterSize.y }, FVec4{ 0.08f, 0.16f, 0.20f, 1.0f },
		0.82f, FStringView( "PoolBottom" ) );

	// 描画面と同じ拡縮を持つ箱を直下へ置き、床の穴と水底の深さも実際の移動へ反映する。
	const bool bFloorLeftWalkable = TryAddWalkablePlane( m_CharacterCollision.Get(), FloorLeft );
	const bool bFloorFrontWalkable = TryAddWalkablePlane( m_CharacterCollision.Get(), FloorFront );
	const bool bFloorBackWalkable = TryAddWalkablePlane( m_CharacterCollision.Get(), FloorBack );
	const bool bFloorRightWalkable = TryAddWalkablePlane( m_CharacterCollision.Get(), FloorRight );
	const bool bPoolBottomWalkable = TryAddWalkablePlane( m_CharacterCollision.Get(), PoolBottom );
	const bool bCharacterWorldReady = bFloorLeftWalkable && bFloorFrontWalkable && bFloorBackWalkable && bFloorRightWalkable && bPoolBottomWalkable;

	// ACSの屈折、反射、泡、動的波紋を使う水面。位置と広さを決めるだけで置ける。
	FWater3DSpawnParams Water;
	Water.Position = kWaterPosition;
	Water.Size = kWaterSize;
	Water.ShallowColor = FVec3{ 0.035f, 0.42f, 0.58f };
	Water.DeepColor = FVec3{ 0.006f, 0.045f, 0.14f };
	Water.Roughness = 0.18f;
	Water.NormalStrength = 0.70f;
	Water.WaveAmplitude = 0.075f;
	Water.WaveScale = 1.05f;
	Water.RefractionStrength = 0.45f;
	Water.FoamIntensity = 0.32f;
	Water.Name = FStringView( "DemoWater" );
	m_WaterSurfaceId = FNodeId{};
	if ( ANode* const WaterSurface = CWater3DSpawner::SpawnInto( Graph(), Water ) )
	{
		m_WaterSurfaceId = WaterSurface->Id();
	}

	// 水面を横切る形を置き、接触部の泡と前後関係を見えるようにする。
	FModel3DSpawnParams WaterStone = FModel3DSpawnParams::FromPrimitive( EMeshPrimitive3D::Sphere,
		FVec3{ kWaterPosition.x, 0.34f, kWaterPosition.z } );
	WaterStone.Scale = FVec3{ 0.82f, 0.82f, 0.82f };
	WaterStone.Color = FVec4{ 0.32f, 0.36f, 0.39f, 1.0f };
	WaterStone.Roughness = 0.72f;
	WaterStone.Name = FStringView( "WaterStone" );
	ANode* const WaterStoneNode = CModel3DSpawner::SpawnInto( Graph(), WaterStone );
	if ( m_CharacterCollision && WaterStoneNode != nullptr ) m_CharacterCollision->TryAddBounds( *WaterStoneNode, kCharacterCollisionLayer );

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
		ANode* const BallNode = CModel3DSpawner::SpawnInto( Graph(), Ball );
		if ( m_CharacterCollision && BallNode != nullptr ) m_CharacterCollision->TryAddBounds( *BallNode, kCharacterCollisionLayer );
	}

	// 回す立方体。動いていることと、面ごとの陰りの差が分かる。
	FModel3DSpawnParams Cube = FModel3DSpawnParams::FromPrimitive( EMeshPrimitive3D::Cube, FVec3{ 0.0f, 1.2f, -3.0f } );
	Cube.Scale = FVec3{ 1.4f, 1.4f, 1.4f };
	Cube.Color = FVec4{ 0.90f, 0.75f, 0.30f, 1.0f };
	Cube.Metallic = 1.0f;      // 金属。拡散反射が消えるので、環境光と反射が要る
	Cube.Roughness = 0.28f;
	Cube.Name = FStringView( "Spinner" );
	m_Spinner = CModel3DSpawner::SpawnInto( Graph(), Cube );
	if ( m_CharacterCollision && m_Spinner != nullptr ) m_CharacterCollision->TryAddBounds( *m_Spinner, kCharacterCollisionLayer );

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
		m_Mover = CModel3DSpawner::SpawnInto( Graph(), Model, Assets->Models() );
		if ( m_CharacterCollision && m_Mover != nullptr ) m_CharacterCollision->TryAddBounds( *m_Mover, kCharacterCollisionLayer );

		// 透過PNGをカメラへ向く3D板として置く。画像読込、ノード、追従登録を1回へまとめる。
		FSprite3DSpawnParams ImageMarker = FSprite3DSpawnParams::FromImage(
			FStringView( "circle.png" ), FVec3{ -3.4f, 2.45f, 2.4f }, FVec2{ 0.72f, 0.72f } );
		ImageMarker.Name = FStringView( "ImageMarker" );
		if ( SpawnBillboard3D( ImageMarker ) == nullptr )
			ACS_LOG_WARN( "Demo3D: カメラ追従の3D画像マーカーを配置できなかった" );

		// 骨で動くモデル。読み込み、部品追加、最初のクリップ再生までを1回で行う。
		// **骨の入っていないFBXを渡すと読めない。** そのときは1行出て、何も置かれない。
		FAnimatedModel3DSpawnParams Animated = FAnimatedModel3DSpawnParams::FromModel(
			FStringView( "Models/SkinnedAnimated.fbx" ), FVec3{ 3.6f, 0.2f, 1.4f } );
		Animated.Scale = FVec3{ 0.02f, 0.02f, 0.02f };   // 書き出し単位がセンチメートル
		Animated.Color = FVec3{ 0.72f, 0.78f, 0.86f };
		Animated.Name = FStringView( "Animated" );
		CAnimatedModel3DSpawner::SpawnInto( Graph(), Animated, Assets->Models() );
	}

	// 太陽。
	//
	// 向きは «面から光源へ向かう» 側。正規化したY成分0.58は仰角約36度を表す。
	//
	// 方向を選んだ理由は2つあって、どちらも «床に何が映るか» で決まる。
	//
	// - **高い太陽は床に映らない。** 見下ろしている床の鏡像は «水平よりやや上» を向くので、
	//   仰角 60 度の太陽の像は画面の外へ行く。低くするほど像が手前へ降りてくる
	// - **方位が合っていないと映らない。** X/Zの比は、手前左にglareが来て白飛びしない程度に
	//   カメラ正面から外してある
	//
	// 以前は真上に近い位置に置いていたので «床に太陽が映らない» ように見えていた。
	// 描けていなかったのではなく、映る場所が画面の外だった。
	// 以前のEuler角と同じ方向を、光の意味に合う「面から太陽へ向かう方向」で直接渡す。
	// ノード作成、回転への変換、部品の追加は配置窓口がまとめる。
	CLight3DSpawner::SpawnInto( Graph(), FLight3DSpawnParams::Sun( FVec3{ -0.472623f, 0.581683f, 0.662021f } ) );

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
	// 同じ雲を環境光へ焼き、曇った方向から届く光も物体の陰影へ反映する。
	Clouds().bAffectEnvironmentLighting = true;
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

	// トーンマップ後の輪郭に軽いアンチエイリアスを掛け、斜め線のギザギザを抑える。
	// このデモはTAAを使わないため、時間方向の履歴を持たないFXAAを仕上げに使う。
	PostParams().fxaa_enabled = true;

	// 遊ぶ人向けUI。初期化、入力、更新、終了、ポスト処理後の描画はAUi3DSceneが受け持つ。
	// ここでは表示物を置き、あとでボタンの結果を読むだけにする。
	Ui().AddText( "ACS 3D", FVec2{ kUiLeft + 16.0f, kUiTop + 14.0f } );
	Ui().AddText( "WASD + PAD: THIRD PERSON", FVec2{ kUiLeft + 16.0f, kUiTop + 42.0f } );
	m_FxaaToggleButton = Ui().AddButton( "TOGGLE FXAA", FVec2{ kUiLeft + 16.0f, kUiTop + 76.0f }, FVec2{ kUiWidth - 32.0f, 44.0f } );
	m_FxaaStatusText = Ui().AddText( "FXAA: ON", FVec2{ kUiLeft + 16.0f, kUiTop + 132.0f } );
	m_FxaaRebindButton = Ui().AddButton( "CHANGE FXAA KEY", FVec2{ kUiLeft + 16.0f, kUiTop + 158.0f }, FVec2{ kUiWidth - 32.0f, 44.0f } );
	m_FxaaKeyText = Ui().AddText( "KEYBOARD: F", FVec2{ kUiLeft + 16.0f, kUiTop + 216.0f } );
	m_GamepadJumpRebindButton = Ui().AddButton( "CHANGE JUMP PAD", FVec2{ kUiLeft + 16.0f, kUiTop + 246.0f }, FVec2{ kUiWidth - 32.0f, 44.0f } );
	m_GamepadJumpText = Ui().AddText( "JUMP PAD: SOUTH", FVec2{ kUiLeft + 16.0f, kUiTop + 304.0f } );
	m_GamepadMoveRebindButton = Ui().AddButton( "CHANGE MOVE AXIS", FVec2{ kUiLeft + 16.0f, kUiTop + 330.0f }, FVec2{ kUiWidth - 32.0f, 44.0f } );
	m_GamepadMoveText = Ui().AddText( "MOVE PAD: LEFT Y", FVec2{ kUiLeft + 16.0f, kUiTop + 388.0f } );
	m_SpatialSoundButton = Ui().AddButton( "PLAY 3D SOUND: LEFT", FVec2{ kUiLeft + 16.0f, kUiTop + 414.0f }, FVec2{ kUiWidth - 32.0f, 44.0f } );
	m_SpatialSoundStatusText = Ui().AddText( "HEADPHONES RECOMMENDED", FVec2{ kUiLeft + 16.0f, kUiTop + 472.0f } );
	m_WeatherButton = Ui().AddButton( "SET WEATHER: STORM", FVec2{ kUiLeft + 16.0f, kUiTop + 498.0f }, FVec2{ kUiWidth - 32.0f, 44.0f } );
	m_WeatherStatusText = Ui().AddText( "WEATHER: CLEAR", FVec2{ kUiLeft + 16.0f, kUiTop + 556.0f } );
	m_GeometryPickButton = Ui().AddButton( "PICK EXACT SHAPE", FVec2{ kUiLeft + 16.0f, kUiTop + 582.0f }, FVec2{ kUiWidth - 32.0f, 44.0f } );
	m_GeometryPickStatusText = Ui().AddText( "PICK: READY", FVec2{ kUiLeft + 16.0f, kUiTop + 640.0f } );
	m_InteractionStatusText = Ui().AddText( "FOCUS: LOOK AT PLAYER", FVec2{ kUiLeft + 16.0f, kUiTop + 666.0f } );
	m_bNextSpatialSoundRight = false;
	m_NextWeatherIndex = 0u;
	SetWeather( EWeatherKind::Clear, 0.0f );
	SetWeatherWindDirection( FVec2{ 0.92f, 0.38f } );
	RefreshWeatherText();

	// 設定は整数として保存し、EKeyの実キー範囲へ戻せる値だけを採用する。
	CGameSettingsSubsystem* const Settings = GetSubsystem<CGameSettingsSubsystem>();
	const i32 StoredKeyValue = Settings != nullptr
		? Settings->GetInt( FString( kFxaaKeySetting ), static_cast<i32>( kDefaultFxaaKey ) )
		: static_cast<i32>( kDefaultFxaaKey );
	EKey LoadedKey = static_cast<EKey>( StoredKeyValue );
	if ( !IsDemoFxaaKey( LoadedKey ) )
	{
		LoadedKey = kDefaultFxaaKey;
		if ( Settings != nullptr ) Settings->SetInt( FString( kFxaaKeySetting ), static_cast<i32>( LoadedKey ) );
	}

	m_ActionBindings.Clear();
	m_PreviousActionInput = FActionInput{};
	m_bSuppressBoundActionPress = false;
	m_bSuppressJumpButtonUntilReleased = false;
	m_bSuppressMoveAxisUntilCentered = false;
	m_bRestoreFreeCameraAfterUpdate = false;
	m_FxaaKeyRebind.SetCurrentKey( LoadedKey );
	if ( !m_ActionBindings.ReplaceKeyBinding( kFxaaActionIndex, LoadedKey ) )
	{
		ACS_LOG_WARN( "Demo3D: FXAAキー割り当ての初期化に失敗" );
	}
	RefreshFxaaKeyText();

	const i32 StoredJumpValue = Settings != nullptr
		? Settings->GetInt( FString( kJumpButtonSetting ), static_cast<i32>( kDefaultJumpButton ) )
		: static_cast<i32>( kDefaultJumpButton );
	const bool bStoredJumpValid = StoredJumpValue >= 0 && StoredJumpValue < static_cast<i32>( EGamepadButton::_Count );
	EGamepadButton LoadedJumpButton = bStoredJumpValid ? static_cast<EGamepadButton>( StoredJumpValue ) : kDefaultJumpButton;
	if ( !bStoredJumpValid )
	{
		LoadedJumpButton = kDefaultJumpButton;
		if ( Settings != nullptr ) Settings->SetInt( FString( kJumpButtonSetting ), static_cast<i32>( LoadedJumpButton ) );
	}

	const i32 StoredMoveValue = Settings != nullptr
		? Settings->GetInt( FString( kMoveAxisSetting ), static_cast<i32>( kDefaultMoveAxis ) )
		: static_cast<i32>( kDefaultMoveAxis );
	const bool bStoredMoveValid = StoredMoveValue >= 0 && StoredMoveValue < static_cast<i32>( EGamepadAxis::_Count );
	EGamepadAxis LoadedMoveAxis = bStoredMoveValid ? static_cast<EGamepadAxis>( StoredMoveValue ) : kDefaultMoveAxis;
	if ( !bStoredMoveValid )
	{
		LoadedMoveAxis = kDefaultMoveAxis;
		if ( Settings != nullptr ) Settings->SetInt( FString( kMoveAxisSetting ), static_cast<i32>( LoadedMoveAxis ) );
	}

	m_JumpGamepadRebind.SetCurrentButton( LoadedJumpButton );
	m_MoveGamepadRebind.SetCurrentAxis( LoadedMoveAxis );
	RefreshGamepadRebindText();

	// 反射。磨いた床と金属に、画面に映っているものを映す。
	// **画面に映っていないものは映せない** ので、切っておく方が素直な場面もある。
	Reflections().Intensity = 0.9f;

	// 大気が描く «地面» の色を、置いた床に寄せる。ここがずれると、地平線から下だけ
	// 別の場所の色になり、床の縁で色が切り替わって見える。
	Atmosphere().ground_albedo = FVec3{ 0.06f, 0.07f, 0.06f };

	// 遠くの物と水面を、同じ太陽で計算した大気へ距離に応じて馴染ませる。
	// world単位はキャラクター半径と同じメートルなので、追加の倍率指定は要らない。
	SetAerialPerspectiveEnabled( true );

	// 全体が入る位置までカメラを引く。
	FrameScene();

	// 第三者視点の接続に失敗しても見回せるよう、復元先には従来の自由カメラを用意する。
	//
	// 切ると画角が動かなくなる。**撮り比べるときは切ること。** 入れたままだと撮影中の
	// キー入力で画角が変わり、«何が変わったのか» が分からない画が並ぶ。
	SetFreeCameraEnabled( true );
	SetOrbit( FVec3{ 0.0f, 1.0f, 0.0f }, 0.0f, 0.32f, 14.0f );
	if ( !bCharacterWorldReady || !TryInitializeThirdPersonCharacter() )
	{
		ACS_LOG_WARN( "Demo3D: 第三者視点キャラクターの初期化に失敗。自由カメラで継続" );
	}
	RefreshSpatialAudioListener();

	// 素材名と明示した置き方だけで3D effectを出す。renderer準備前なら基底が開始まで保持する。
	m_EffectElapsedSeconds = 0.0f;
	PlayEffect3D( FStringView( "Effects/hit.efkefc" ), MakeDemoEffectParams() );

	// 最初の画面から波紋が見えるように1つ作り、その後は明示秒で同じ列を繰り返す。
	m_WaterRippleElapsedSeconds = 0.0f;
	m_WaterRippleIndex = 0u;
	AddDemoWaterRipple();
}


void ADemo3DScene::OnExit() noexcept
{
	m_JumpGamepadRebind.CancelCapture();
	m_MoveGamepadRebind.CancelCapture();
	m_bSuppressJumpButtonUntilReleased = false;
	m_bSuppressMoveAxisUntilCentered = false;
	m_ThirdPersonCharacter.Unbind();
	m_CharacterActionBindings.Clear();
	m_PreviousCharacterInput = FActionInput{};
	m_GeometryPickDebugRemainingSeconds = 0.0f;
	m_CharacterNode = nullptr;
	m_CharacterCollision.Reset();
	m_Spinner = nullptr;
	m_Mover = nullptr;
	m_WaterSurfaceId = FNodeId{};
	m_bInteractionRequested = false;
	AWeather3DScene::OnExit();
}


bool ADemo3DScene::TryInitializeThirdPersonCharacter() noexcept
{
	if ( !m_CharacterCollision || m_ThirdPersonCharacter.IsBound() ) return false;

	const FThirdPersonCharacter3DActionSet Actions = FThirdPersonCharacter3DActionSet::WithRunAction();
	CActionBindingTable BuiltBindings;
	if ( !FThirdPersonCharacter3DControlPreset{}.TryBuildBindings( BuiltBindings, Actions ) ) return false;

	if ( !BuiltBindings.ReplaceGamepadButtonBinding( Actions.JumpAction, m_JumpGamepadRebind.CurrentButton(), kGamepadPlayerIndex ) || !BuiltBindings.ReplaceGamepadAxisBinding( Actions.MoveForwardAxis, m_MoveGamepadRebind.CurrentAxis(), kGamepadPlayerIndex, kGamepadDeadZone, 1.0f ) ) return false;

	ANode* const Character = SpawnThirdPersonCharacter( Graph() );
	if ( Character == nullptr ) return false;

	FThirdPersonCharacter3DParams Params;
	Params.LocalCollisionCenter = FVec3{ 0.0f, 0.52f, 0.0f };
	Params.Movement.Radius = 0.52f;
	Params.MaximumMoveSpeed = 3.6f;
	Params.CollisionMask = kCharacterCollisionLayer;
	Params.Camera.LocalTargetOffset = FVec3{ 0.0f, 1.05f, 0.0f };
	Params.Camera.InitialYawDegrees = 180.0f;
	Params.Camera.InitialPitchDegrees = 18.0f;
	Params.Camera.InitialDistance = 6.8f;
	Params.Camera.MinimumDistance = 2.0f;
	Params.Camera.MaximumDistance = 14.0f;
	Params.Camera.TargetClearance = 2.5f;
	if ( !m_ThirdPersonCharacter.Bind( *m_CharacterCollision, *this, *Character, Params ) )
	{
		Graph().Destroy( Character->Id() );
		return false;
	}

	const FThirdPersonCharacter3DUpdateResult InitialUpdate = m_ThirdPersonCharacter.Update( FActionInput{}, FActionInput{}, 0.0f, Actions );
	if ( !InitialUpdate.Succeeded() )
	{
		m_ThirdPersonCharacter.Unbind();
		Graph().Destroy( Character->Id() );
		return false;
	}

	m_CharacterNode = Character;
	m_CharacterActionBindings = Move( BuiltBindings );
	m_PreviousCharacterInput = FActionInput{};
	FWorldLabel3DParams CharacterLabel;
	CharacterLabel.Text = FStringView( "PLAYER" );
	CharacterLabel.WorldOffset = FVec3{ 0.0f, 2.15f, 0.0f };
	CharacterLabel.TextColor = FVec4{ 0.72f, 0.92f, 1.0f, 1.0f };
	WorldLabels().AddNodeLabel( *Character, CharacterLabel );

	FWorldLabel3DParams InteractionLabel;
	InteractionLabel.Text = FStringView( "ENTER: INTERACT" );
	InteractionLabel.WorldOffset = FVec3{ 0.0f, 2.55f, 0.0f };
	InteractionLabel.TextColor = FVec4{ 1.0f, 0.86f, 0.35f, 1.0f };
	InteractionLabel.MaximumDistance = 8.0f;
	if ( !InteractionFocus().RegisterTarget( *Character, InteractionLabel ) ) ACS_LOG_WARN( "Demo3D: 操作キャラクターを視線対象へ登録できなかった" );
	return true;
}


void ADemo3DScene::UpdateThirdPersonCharacter( f32 DeltaSeconds ) noexcept
{
	if ( !m_ThirdPersonCharacter.IsBound() || m_CharacterNode == nullptr ) return;

	FActionInput CurrentInput = m_CharacterActionBindings.Resolve( m_ActionReader );
	bool bSuppressInput = IsInputCaptureActive() || m_bSuppressBoundActionPress;
	if ( m_bSuppressJumpButtonUntilReleased )
	{
		if ( m_ActionReader.IsGamepadButtonDown( kGamepadPlayerIndex, m_JumpGamepadRebind.CurrentButton() ) ) bSuppressInput = true;
		else m_bSuppressJumpButtonUntilReleased = false;
	}
	if ( m_bSuppressMoveAxisUntilCentered )
	{
		const f32 MoveValue = m_ActionReader.GetGamepadAxis( kGamepadPlayerIndex, m_MoveGamepadRebind.CurrentAxis() );
		if ( std::isfinite( MoveValue ) && std::abs( MoveValue ) >= kGamepadDeadZone ) bSuppressInput = true;
		else m_bSuppressMoveAxisUntilCentered = false;
	}
	if ( bSuppressInput ) CurrentInput = FActionInput{};
	const FThirdPersonCharacter3DActionSet Actions = FThirdPersonCharacter3DActionSet::WithRunAction();
	m_ThirdPersonCharacter.Update( CurrentInput, m_PreviousCharacterInput, DeltaSeconds, Actions );
	m_PreviousCharacterInput = CurrentInput;
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
	AWeather3DScene::OnUpdate( DeltaSeconds );
	ReportFrameTime( DeltaSeconds );
	RefreshWeatherText();

	// Escapeの押下フレームを基底場面が処理し終えてから、入力待ち前の自由カメラ状態へ戻す。
	if ( m_bRestoreFreeCameraAfterUpdate )
	{
		SetFreeCameraEnabled( m_bFreeCameraWasEnabledBeforeCapture );
		m_bRestoreFreeCameraAfterUpdate = false;
	}

	// 既に待機中の実機入力を先に処理する。UIを決定した同じ押下を割り当てへ使わないため、
	// 新しい待機の開始はこの処理より後に行う。
	UpdateGamepadRebinding();
	const bool bFxaaRebindPressed = Ui().ConsumeButtonPress( m_FxaaRebindButton );
	const bool bJumpRebindPressed = Ui().ConsumeButtonPress( m_GamepadJumpRebindButton );
	const bool bMoveRebindPressed = Ui().ConsumeButtonPress( m_GamepadMoveRebindButton );

	// キー変更ボタンは状態を開始するだけ。次のキーはOnEventで明示的に1件ずつ処理する。
	if ( !IsInputCaptureActive() && bFxaaRebindPressed && m_FxaaKeyRebind.BeginCapture( EKey::Escape ) )
	{
		m_bFreeCameraWasEnabledBeforeCapture = FreeCameraEnabled();
		SetFreeCameraEnabled( false );
		RefreshFxaaKeyText();
	}
	if ( !IsInputCaptureActive() && bJumpRebindPressed && m_JumpGamepadRebind.BeginButtonCapture() )
	{
		m_bFreeCameraWasEnabledBeforeCapture = FreeCameraEnabled();
		SetFreeCameraEnabled( false );
		RefreshGamepadRebindText();
	}
	if ( !IsInputCaptureActive() && bMoveRebindPressed && m_MoveGamepadRebind.BeginAxisCapture() )
	{
		m_bFreeCameraWasEnabledBeforeCapture = FreeCameraEnabled();
		SetFreeCameraEnabled( false );
		RefreshGamepadRebindText();
	}

	const FActionInput CurrentActionInput = m_ActionBindings.Resolve( m_ActionReader );
	bool bBoundActionPressed = !IsInputCaptureActive()
		&& CurrentActionInput.IsDown( kFxaaActionIndex )
		&& !m_PreviousActionInput.IsDown( kFxaaActionIndex );
	m_PreviousActionInput = CurrentActionInput;
	UpdateThirdPersonCharacter( DeltaSeconds );
	RefreshSpatialAudioListener();

	const bool bActivateInteraction = m_bInteractionRequested && !IsInputCaptureActive();
	m_bInteractionRequested = false;
	const FInteractionFocus3DUpdateResult InteractionResult = UpdateInteractionFocus( bActivateInteraction );
	if ( InteractionResult.Activated() )
	{
		SetUiTextIfChanged( Ui(), m_InteractionStatusText, "INTERACT: PLAYER" );
	}
	else if ( InteractionResult.FocusChanged() )
	{
		SetUiTextIfChanged( Ui(), m_InteractionStatusText, InteractionResult.FocusedNode.IsValid() ? "FOCUS: ENTER TO INTERACT" : "FOCUS: LOOK AT PLAYER" );
	}

	// 割り当て確定に使った同じ押下ではFXAAを切り替えない。次の押下から通常操作になる。
	if ( m_bSuppressBoundActionPress )
	{
		bBoundActionPressed = false;
		m_bSuppressBoundActionPress = false;
	}

	// ボタンのクリックは1回だけ消費される。状態と表示を同じ場所で確定させる。
	if ( Ui().ConsumeButtonPress( m_FxaaToggleButton ) || bBoundActionPressed )
	{
		SetFxaaEnabled( !PostParams().fxaa_enabled );
	}

	if ( Ui().ConsumeButtonPress( m_SpatialSoundButton ) ) PlaySpatialDemoSound();
	if ( Ui().ConsumeButtonPress( m_WeatherButton ) ) AdvanceDemoWeather();
	if ( Ui().ConsumeButtonPress( m_GeometryPickButton ) ) PickVisibleGeometry_Internal();
	DrawGeometryPickDebug_Internal( DeltaSeconds );

	if ( m_WaterSurfaceId.IsValid() )
	{
		m_WaterRippleElapsedSeconds += DeltaSeconds;
		if ( m_WaterRippleElapsedSeconds >= kWaterRippleRepeatSeconds )
		{
			m_WaterRippleElapsedSeconds -= kWaterRippleRepeatSeconds;
			AddDemoWaterRipple();
		}
	}

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


void ADemo3DScene::OnEvent( const FEvent& Event ) noexcept
{
	if ( Event.type == EEventType::KeyPressed && Event.key.key == EKey::Enter && !IsInputCaptureActive() ) m_bInteractionRequested = true;

	if ( Event.type == EEventType::KeyPressed && Event.key.key == EKey::Escape )
	{
		const bool bCancelledButton = m_JumpGamepadRebind.CancelCapture();
		const bool bCancelledAxis = m_MoveGamepadRebind.CancelCapture();
		if ( bCancelledButton || bCancelledAxis )
		{
			// 基底場面が同じEscapeを終了操作へ使い終えるまで自由カメラを止めておく。
			m_bRestoreFreeCameraAfterUpdate = true;
			RefreshGamepadRebindText();
		}
	}

	if ( m_FxaaKeyRebind.IsCapturing() && Event.type == EEventType::KeyPressed )
	{
		const EKey PreviousKey = m_FxaaKeyRebind.CurrentKey();
		const FActionKeyRebindState::EResult Result = m_FxaaKeyRebind.HandlePressedKey( Event.key.key );

		if ( Result == FActionKeyRebindState::EResult::Applied )
		{
			const EKey AppliedKey = m_FxaaKeyRebind.CurrentKey();
			if ( !IsDemoFxaaKey( AppliedKey ) || !m_ActionBindings.ReplaceKeyBinding( kFxaaActionIndex, AppliedKey ) )
			{
				m_FxaaKeyRebind.SetCurrentKey( PreviousKey );
				ACS_LOG_WARN( "Demo3D: FXAAキー割り当てを適用できなかった" );
			}
			else
			{
				if ( CGameSettingsSubsystem* const Settings = GetSubsystem<CGameSettingsSubsystem>() )
				{
					Settings->SetInt( FString( kFxaaKeySetting ), static_cast<i32>( AppliedKey ) );
				}
				m_bSuppressBoundActionPress = true;
			}
		}

		if ( Result == FActionKeyRebindState::EResult::Applied || Result == FActionKeyRebindState::EResult::Cancelled )
		{
			// OnUpdateの基底処理まで自由カメラを止め、取消のEscapeを終了操作として再利用させない。
			m_bRestoreFreeCameraAfterUpdate = true;
		}

		RefreshFxaaKeyText();
	}

	AEffect3DScene::OnEvent( Event );
}


void ADemo3DScene::SetFxaaEnabled( bool bEnabled ) noexcept
{
	PostParams().fxaa_enabled = bEnabled;
	Ui().SetText( m_FxaaStatusText, bEnabled ? "FXAA: ON" : "FXAA: OFF" );
}


void ADemo3DScene::RefreshFxaaKeyText() noexcept
{
	if ( m_FxaaKeyRebind.IsCapturing() )
	{
		SetUiTextIfChanged( Ui(), m_FxaaRebindButton, "PRESS A KEY..." );
		SetUiTextIfChanged( Ui(), m_FxaaKeyText, "ESC: CANCEL" );
		return;
	}

	SetUiTextIfChanged( Ui(), m_FxaaRebindButton, "CHANGE FXAA KEY" );
	const FString KeyLabel = MakeKeyLabel( m_FxaaKeyRebind.CurrentKey() );
	SetUiTextIfChanged( Ui(), m_FxaaKeyText, KeyLabel.Data() );
}


void ADemo3DScene::RefreshGamepadRebindText() noexcept
{
	if ( m_JumpGamepadRebind.IsCapturing() )
	{
		SetUiTextIfChanged( Ui(), m_GamepadJumpRebindButton, "PRESS PAD BUTTON..." );
		SetUiTextIfChanged( Ui(), m_GamepadJumpText, "ESC: CANCEL" );
	}
	else
	{
		SetUiTextIfChanged( Ui(), m_GamepadJumpRebindButton, "CHANGE JUMP PAD" );
		const FString JumpLabel = MakeJumpButtonLabel( m_JumpGamepadRebind.CurrentButton() );
		SetUiTextIfChanged( Ui(), m_GamepadJumpText, JumpLabel.Data() );
	}

	if ( m_MoveGamepadRebind.IsCapturing() )
	{
		SetUiTextIfChanged( Ui(), m_GamepadMoveRebindButton, "MOVE ANY PAD AXIS" );
		SetUiTextIfChanged( Ui(), m_GamepadMoveText, "ESC: CANCEL" );
	}
	else
	{
		SetUiTextIfChanged( Ui(), m_GamepadMoveRebindButton, "CHANGE MOVE AXIS" );
		const FString MoveLabel = MakeMoveAxisLabel( m_MoveGamepadRebind.CurrentAxis() );
		SetUiTextIfChanged( Ui(), m_GamepadMoveText, MoveLabel.Data() );
	}
}


void ADemo3DScene::UpdateGamepadRebinding() noexcept
{
	const FThirdPersonCharacter3DActionSet Actions;
	if ( m_JumpGamepadRebind.CaptureKind() == FActionGamepadRebindState::ECaptureKind::Button )
	{
		EGamepadButton PressedButton = EGamepadButton::_Count;
		if ( !m_ActionReader.TryReadPressedGamepadButton( kGamepadPlayerIndex, PressedButton ) ) return;

		const EGamepadButton PreviousButton = m_JumpGamepadRebind.CurrentButton();
		if ( m_JumpGamepadRebind.HandlePressedButton( PressedButton ) != FActionGamepadRebindState::EResult::Applied ) return;
		const EGamepadButton AppliedButton = m_JumpGamepadRebind.CurrentButton();
		if ( !m_CharacterActionBindings.ReplaceGamepadButtonBinding( Actions.JumpAction, AppliedButton, kGamepadPlayerIndex ) )
		{
			m_JumpGamepadRebind.SetCurrentButton( PreviousButton );
			ACS_LOG_WARN( "Demo3D: ジャンプボタン割り当てを適用できなかった" );
		}
		else
		{
			if ( CGameSettingsSubsystem* const Settings = GetSubsystem<CGameSettingsSubsystem>() ) Settings->SetInt( FString( kJumpButtonSetting ), static_cast<i32>( AppliedButton ) );
			m_bSuppressJumpButtonUntilReleased = true;
		}
		SetFreeCameraEnabled( m_bFreeCameraWasEnabledBeforeCapture );
		RefreshGamepadRebindText();
		return;
	}

	if ( m_MoveGamepadRebind.CaptureKind() != FActionGamepadRebindState::ECaptureKind::Axis ) return;
	if ( !m_MoveGamepadRebind.IsAxisCaptureReady() )
	{
		if ( m_ActionReader.AreGamepadAxesCentered( kGamepadPlayerIndex, kGamepadAxisCenterThreshold ) ) m_MoveGamepadRebind.ConfirmAxesCentered();
		return;
	}

	EGamepadAxis ActiveAxis = EGamepadAxis::_Count;
	if ( !m_ActionReader.TryReadActiveGamepadAxis( kGamepadPlayerIndex, kGamepadAxisCaptureThreshold, ActiveAxis ) ) return;

	const EGamepadAxis PreviousAxis = m_MoveGamepadRebind.CurrentAxis();
	if ( m_MoveGamepadRebind.HandleActiveAxis( ActiveAxis ) != FActionGamepadRebindState::EResult::Applied ) return;
	const EGamepadAxis AppliedAxis = m_MoveGamepadRebind.CurrentAxis();
	if ( !m_CharacterActionBindings.ReplaceGamepadAxisBinding( Actions.MoveForwardAxis, AppliedAxis, kGamepadPlayerIndex, kGamepadDeadZone, 1.0f ) )
	{
		m_MoveGamepadRebind.SetCurrentAxis( PreviousAxis );
		ACS_LOG_WARN( "Demo3D: 前後移動軸割り当てを適用できなかった" );
	}
	else
	{
		if ( CGameSettingsSubsystem* const Settings = GetSubsystem<CGameSettingsSubsystem>() ) Settings->SetInt( FString( kMoveAxisSetting ), static_cast<i32>( AppliedAxis ) );
		m_bSuppressMoveAxisUntilCentered = true;
	}
	SetFreeCameraEnabled( m_bFreeCameraWasEnabledBeforeCapture );
	RefreshGamepadRebindText();
}


bool ADemo3DScene::IsInputCaptureActive() const noexcept
{
	return m_FxaaKeyRebind.IsCapturing() || m_JumpGamepadRebind.IsCapturing() || m_MoveGamepadRebind.IsCapturing();
}


bool ADemo3DScene::TryMakeCameraAudioListener( FAudioListener& OutListener ) const noexcept
{
	OutListener.position = Camera().Eye();

	if ( const FScene3DCameraState* const Authored = AuthoredCamera() )
	{
		if ( LengthSq( Authored->Forward ) <= 0.000001f || LengthSq( Authored->Up ) <= 0.000001f ) return false;
		OutListener.forward = Normalize( Authored->Forward );
		OutListener.up = Normalize( Authored->Up );
		return true;
	}

	COrbitCameraController3D::FOrbitCameraFixedStepSnapshot3D Snapshot;
	if ( !TryCaptureOrbitCameraSnapshot( Snapshot ) ) return false;

	const FVec3 Forward = Snapshot.current.target - OutListener.position;
	if ( LengthSq( Forward ) <= 0.000001f ) return false;

	OutListener.forward = Normalize( Forward );
	OutListener.up = FVec3::Up();
	return true;
}


void ADemo3DScene::RefreshSpatialAudioListener() noexcept
{
	FAudioListener Listener;
	if ( !TryMakeCameraAudioListener( Listener ) ) return;

	if ( CSpatialAudioSubsystem* const Spatial = GetSubsystem<CSpatialAudioSubsystem>() ) Spatial->SetListener( Listener );
}


void ADemo3DScene::PlaySpatialDemoSound() noexcept
{
	CSpatialAudioSubsystem* const Spatial = GetSubsystem<CSpatialAudioSubsystem>();
	FAudioListener Listener;
	if ( Spatial == nullptr || !TryMakeCameraAudioListener( Listener ) )
	{
		Ui().SetText( m_SpatialSoundStatusText, "SOUND: UNAVAILABLE" );
		return;
	}

	const FVec3 RightDirection = Cross( Listener.up, Listener.forward );
	if ( LengthSq( RightDirection ) <= 0.000001f )
	{
		Ui().SetText( m_SpatialSoundStatusText, "SOUND: UNAVAILABLE" );
		return;
	}
	const FVec3 Right = Normalize( RightDirection );

	Spatial->SetListener( Listener );
	FSpatialPlayRequest Request;
	Request.AssetPath = FString( kSpatialSoundAsset );
	Request.Position = Listener.position + Listener.forward * kSpatialSoundForwardDistance + Right * ( m_bNextSpatialSoundRight ? kSpatialSoundSideDistance : -kSpatialSoundSideDistance );
	Request.BaseVolume = 0.85f;
	Request.MaxDistance = 18.0f;

	if ( !Spatial->PlayOnce( Request ) )
	{
		Ui().SetText( m_SpatialSoundStatusText, "SOUND: UNAVAILABLE" );
		return;
	}

	Ui().SetText( m_SpatialSoundStatusText, m_bNextSpatialSoundRight ? "PLAYED: RIGHT" : "PLAYED: LEFT" );
	m_bNextSpatialSoundRight = !m_bNextSpatialSoundRight;
	Ui().SetText( m_SpatialSoundButton, m_bNextSpatialSoundRight ? "PLAY 3D SOUND: RIGHT" : "PLAY 3D SOUND: LEFT" );
}


void ADemo3DScene::AddDemoWaterRipple() noexcept
{
	if ( !m_WaterSurfaceId.IsValid() ) return;

	const FVec2 Offset = kWaterRippleOffsets[m_WaterRippleIndex % kWaterRipplePointCount];
	m_WaterRippleIndex = ( m_WaterRippleIndex + 1u ) % kWaterRipplePointCount;
	const FVec3 Point{ kWaterPosition.x + Offset.x, kWaterPosition.y, kWaterPosition.z + Offset.y };
	AddWaterDisturbance( m_WaterSurfaceId, Point, 0.24f, 0.30f );
}


void ADemo3DScene::AdvanceDemoWeather() noexcept
{
	const EWeatherKind NextWeather = kDemoWeatherCycle[m_NextWeatherIndex % kDemoWeatherCount];
	if ( !SetWeather( NextWeather, kWeatherTransitionSeconds ) ) return;

	m_NextWeatherIndex = ( m_NextWeatherIndex + 1u ) % kDemoWeatherCount;
	RefreshWeatherText();
}


void ADemo3DScene::RefreshWeatherText() noexcept
{
	const EWeatherKind NextWeather = kDemoWeatherCycle[m_NextWeatherIndex % kDemoWeatherCount];
	FString ButtonText( "SET WEATHER: " );
	ButtonText.TryAppend( FStringView( WeatherLabel( NextWeather ) ) );
	SetUiTextIfChanged( Ui(), m_WeatherButton, ButtonText.Data() );

	FString StatusText( "WEATHER: " );
	if ( Weather().TransitionT() < 1.0f )
	{
		StatusText.TryAppend( FStringView( WeatherLabel( Weather().CurrentWeather() ) ) );
		StatusText.TryAppend( FStringView( " -> " ) );
		StatusText.TryAppend( FStringView( WeatherLabel( Weather().TargetWeather() ) ) );
	}
	else
	{
		StatusText.TryAppend( FStringView( WeatherLabel( Weather().CurrentWeather() ) ) );
	}
	SetUiTextIfChanged( Ui(), m_WeatherStatusText, StatusText.Data() );
}


void ADemo3DScene::PickVisibleGeometry_Internal() noexcept
{
	m_GeometryPickDebugRemainingSeconds = 0.0f;
	/** 3D操作の照準と同じ、左上を0・右下を1とする画面位置。 */
	const FVec2 ScreenPosition = InteractionFocus().Params().ScreenPosition;
	/** 2x2画面へ正規化位置を写し、現在見えている方向へ作る判定線。 */
	const FSceneRay Ray = FSceneRay::FromScreen(
		Camera(), ScreenPosition.x * 2.0f, ScreenPosition.y * 2.0f, 2u, 2u, 100.0f );
	/** 判定線上で最初に見つかった実際の3D表面。 */
	const FSceneRayHit Hit = CScenePicker::RaycastGeometry( *this, Ray );
	if ( !Hit.IsHit() )
	{
		Ui().SetText( m_GeometryPickStatusText, "PICK: MISS" );
		return;
	}

	Ui().SetText( m_GeometryPickStatusText, "PICK: VISIBLE SURFACE" );
	m_GeometryPickDebugStart = Ray.Origin;
	m_GeometryPickDebugEnd = Hit.Point;
	m_GeometryPickDebugRemainingSeconds = kGeometryPickDebugSeconds;
	FEffect3DPlayParams Marker = FEffect3DPlayParams::At( Hit.Point + Hit.Normal * 0.03f );
	Marker.Scale = FVec3{ 0.18f, 0.18f, 0.18f };
	PlayEffect3D( FStringView( "Effects/hit.efkefc" ), Marker );
}


void ADemo3DScene::DrawGeometryPickDebug_Internal( f32 DeltaSeconds ) noexcept
{
	if ( m_GeometryPickDebugRemainingSeconds <= 0.0f ) return;

	/** 視線方向の判定線が投影で点に潰れても、元のレイを確認できる登録結果。 */
	const bool bRayQueued = DrawLine3D( m_GeometryPickDebugStart, m_GeometryPickDebugEnd, FVec4{ 0.20f, 0.95f, 1.0f, 1.0f } );
	/** どのカメラ角度でも命中点を見分けられる各軸の半分の長さ。 */
	const FVec3 AxisExtent{ 0.42f, 0.42f, 0.42f };
	/** 命中点を通るworld X軸線の登録結果。 */
	const bool bXAxisQueued = DrawLine3D( m_GeometryPickDebugEnd - FVec3{ AxisExtent.x, 0.0f, 0.0f }, m_GeometryPickDebugEnd + FVec3{ AxisExtent.x, 0.0f, 0.0f }, FVec4{ 1.0f, 0.24f, 0.18f, 1.0f } );
	/** 命中点を通るworld Y軸線の登録結果。 */
	const bool bYAxisQueued = DrawLine3D( m_GeometryPickDebugEnd - FVec3{ 0.0f, AxisExtent.y, 0.0f }, m_GeometryPickDebugEnd + FVec3{ 0.0f, AxisExtent.y, 0.0f }, FVec4{ 0.34f, 1.0f, 0.24f, 1.0f } );
	/** 命中点を通るworld Z軸線の登録結果。 */
	const bool bZAxisQueued = DrawLine3D( m_GeometryPickDebugEnd - FVec3{ 0.0f, 0.0f, AxisExtent.z }, m_GeometryPickDebugEnd + FVec3{ 0.0f, 0.0f, AxisExtent.z }, FVec4{ 0.22f, 0.52f, 1.0f, 1.0f } );
	/** 命中点を囲む確認箱の登録結果。 */
	const bool bHitBoxQueued = DrawAabb3D( FAabb3::FromCenterExtents( m_GeometryPickDebugEnd, FVec3{ 0.22f, 0.22f, 0.22f } ), FVec4{ 1.0f, 0.62f, 0.12f, 1.0f } );
	/** 命中点を球形の接触範囲として読む3方向円の登録結果。 */
	const bool bHitSphereQueued = DrawSphere3D( FSphere{ m_GeometryPickDebugEnd, 0.32f }, FVec4{ 1.0f, 0.28f, 0.78f, 1.0f } );
	if ( !bRayQueued || !bXAxisQueued || !bYAxisQueued || !bZAxisQueued || !bHitBoxQueued || !bHitSphereQueued )
	{
		m_GeometryPickDebugRemainingSeconds = 0.0f;
		ACS_LOG_WARN( "Demo3D: 実形状判定の3Dデバッグ線を登録できなかった" );
		return;
	}

	if ( std::isfinite( DeltaSeconds ) && DeltaSeconds > 0.0f ) m_GeometryPickDebugRemainingSeconds -= DeltaSeconds;
}


void ADemo3DScene::OnDrawHud( FRenderContext& Context, CSpriteBatch& Sprites ) noexcept
{
	// 薄い影、半透明の面、アクセント線の3枚だけで、3Dから読み分けられるカードにする。
	Sprites.DrawRect( kUiLeft + 4.0f, kUiTop + 6.0f, kUiWidth, kUiHeight, FVec4{ 0.0f, 0.0f, 0.0f, 0.32f } );
	Sprites.DrawRect( kUiLeft, kUiTop, kUiWidth, kUiHeight, FVec4{ 0.035f, 0.055f, 0.09f, 0.90f } );
	Sprites.DrawRect( kUiLeft, kUiTop, 4.0f, kUiHeight, FVec4{ 0.22f, 0.58f, 1.0f, 1.0f } );

	AUi3DScene::OnDrawHud( Context, Sprites );
}
