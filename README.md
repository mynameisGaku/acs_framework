# acs_framework

**3D のゲームを、DXLib と同等以下の手数で作り始められて、DXLib より綺麗に映る。**

描く力は [ACS](https://github.com/mynameisGaku/ArtsCommonSystem) が持っている。この枠組みが
足すのは**手数の少なさ**だけで、機能を作り直すことはしない。

![3D デモ](Docs/demo3d.png)

上の絵はサンプル 1 本 (`Source/AcsFramework_Sample/Scene/Demo3DScene.cpp`) の
出力。**空・太陽・影・環境光・雲がすべて 1 つの太陽から繋がっている。** 空は物理ベースの
大気を焼いたもので、それがそのまま環境光にもなるので、空と光が食い違わない。

## 動かす

必要なもの: Windows / Visual Studio 2022 以降 (C++20) / DirectX 12 が動く GPU。

```powershell
git clone https://github.com/mynameisGaku/acs_framework
cd acs_framework
.\Tools\FetchAcs.ps1                        # ACS の配布物を ThirdParty\acs へ
msbuild acs_framework.vcxproj /p:Configuration=Release /p:Platform=x64
.\x64\Release\acs_framework.exe
```

矢印キーで見回し、WASD で寄る。

> `FetchAcs.ps1` がまだ Release を落とせない段階なら、エンジンをローカルでビルドして
> `.\Tools\FetchAcs.ps1 -FromLocal C:\acs_dev` で持ってくる (`ThirdParty/acs/README.md`)。

## 何ができるか

| | |
|---|---|
| 3D を置く | `CModel3DSpawner`、FBX の取り込み、材質 (metallic / roughness) |
| 3D を照らす | `CLight3DSpawner`、方向だけで置ける太陽、位置と距離だけで置ける点光源 |
| 動かす | `CCharacterMover3D`による重力・接地・壁沿い移動・ジャンプ、姿勢を滑らかに繋ぐ`CCharacterAnimator3D` |
| カメラで追う | `CNodeOrbitCamera3D`、人物の注視点追従、回転・距離操作、遮蔽物回避 |
| 見た目 | 物理大気・ボリューム雲・影・IBL・遮蔽 (SSAO)・間接光 (SSGI)・反射 (SSR)・霧・トーンマップ・輪郭補正 (FXAA) |
| 3D 天候 | `AWeather3DScene`、晴天・曇天・雨・雪・嵐・霧・砂嵐の滑らかな遷移 |
| 3D 水面 | `CWater3DSpawner`、屈折・反射・泡・動的な波紋 |
| 3D 演出 | `AEffect3DScene`、Effekseer、depth 遮蔽、HDR・bloom への自動合成 |
| 3D 音響 | `CSpatialAudioSubsystem`、距離減衰、モノラル効果音の左右定位 |
| 遊ぶ人向け UI | `AUi3DScene`、文字・ボタン・入力、ポスト処理後の鮮明なHUD合成 |
| 当てる | 画面から線を飛ばし、球面や読み込みメッシュの三角形へ正確に当てる (`CScenePicker`) |
| 重なりと移動判定 | `CSceneCollision3D`、ノード追従、球・箱の重なり、球スイープ、レイヤー |
| 土台 | 起動・場面遷移・アセット・音・セーブ・設定・入力再割り当て・多言語・決定性・開発支援 |

詳しくは [`Docs/ROADMAP.md`](Docs/ROADMAP.md)。**v1.0.0 で何を入れて何を入れないか**もそこに書いてある。

## 書き味

```cpp
// 置く
FModel3DSpawnParams Ball = FModel3DSpawnParams::FromPrimitive( EMeshPrimitive3D::Sphere, FVec3{ 0, 1, 0 } );
Ball.Roughness = 0.2f;
CModel3DSpawner::SpawnInto( Graph(), Ball );

// FBX を置く (Assets からの相対名)
FModel3DSpawnParams Model = FModel3DSpawnParams::FromMesh( FStringView( "Models/Robot.fbx" ), Position );
CModel3DSpawner::SpawnInto( Graph(), Model, Assets->Models() );

// 面から太陽へ向かう方向だけで、影とPBRへ繋がる平行光を置く
CLight3DSpawner::SpawnInto( Graph(), FLight3DSpawnParams::Sun( FVec3{ -0.47f, 0.58f, 0.66f } ) );

// 骨付きFBXを読み、置き、Idleを再生する
FAnimatedModel3DSpawnParams Hero = FAnimatedModel3DSpawnParams::FromModel(
    FStringView( "Models/Hero.fbx" ), Position );
Hero.InitialAnimation = FStringView( "Idle" );
ANode* const HeroNode = CAnimatedModel3DSpawner::SpawnInto( Graph(), Hero, Assets->Models() );

// HeroAnimatorはキャラクターと同じ場所で所有し、毎フレーム速度と接地状態だけを渡す
CCharacterAnimator3D HeroAnimator;
if ( HeroNode != nullptr ) HeroAnimator.Bind( *HeroNode );
HeroAnimator.Update( FCharacterAnimation3DInput{ HorizontalSpeed, bGrounded } );

// 動かす
Node->RotateDeg( FVec3{ 0, 90.0f * DeltaSeconds, 0 } );
Node->MoveToward( Target, Speed * DeltaSeconds );
Node->LookAt( Target );

// 画面上の位置から、実際の3D表面へ当てる
const FSceneRayHit Hit = CScenePicker::RaycastGeometry(
    *this, FSceneRay::FromScreen( Camera, MouseX, MouseY, W, H ) );

// ノードへ衝突形状を結び、現在位置へ自動追従させる
CSceneCollision3D Collision{ Graph() };
const FCollisionShapeId3D PlayerShape = Collision.TryAddSphere(
    *HeroNode, FVec3{ 0.0f, 0.9f, 0.0f }, 0.45f, 0x1u );
Collision.TryAddBounds( *WallNode, 0x2u );
TArray<ANode*> Nearby;
Collision.TryOverlapSphere(
    FSphere{ HeroNode->World().position, 2.0f }, Nearby, PlayerShape, 0x2u );

// 球中心と希望する世界X/Z速度だけで、床・壁・重力・ジャンプをノードへ反映する
CCharacterMover3D HeroMover;
HeroMover.Bind( Collision, *HeroNode, FVec3{ 0.0f, 0.45f, 0.0f } );
HeroMover.SetCollisionFilter( PlayerShape, 0x2u );
HeroMover.MoveFromCamera(
    Camera(), FVec2{ MoveX, MoveForward }, 4.0f, bJumpPressed, DeltaSeconds );
HeroMover.TurnTowardMovement( 540.0f, DeltaSeconds );

// 人物の少し上を追い、明示した視点操作で回る。間の壁には自動で寄る
CNodeOrbitCamera3D HeroCamera;
HeroCamera.Bind( *this, *HeroNode );
HeroCamera.Update( FVec2{ LookX, LookY }, Zoom, DeltaSeconds );

// 反射・屈折・泡を持つ水面を置き、波紋を起こす
FWater3DSpawnParams Water;
Water.Position = FVec3{ 2.5f, 0.1f, -1.0f };
ANode* const Surface = CWater3DSpawner::SpawnInto( Graph(), Water );
if ( Surface != nullptr ) AddWaterDisturbance( Surface->Id(), Water.Position );

// 近くの物から跳ね返る色を足す
GlobalIllumination().Intensity = 0.75f;

// TAAを使わない場面の斜め線を滑らかにする
PostParams().fxaa_enabled = true;

// 雲、雲影、霧、空色、環境光を2.5秒で嵐へ揃える (classの基底はAWeather3DScene)
SetWeather( EWeatherKind::Storm, 2.5f );

// 3D effectを出す (AWeather3DSceneにもAEffect3DSceneが含まれる)
PlayEffect3D( FStringView( "Effects/hit.efkefc" ), FVec3{ 0, 1, 0 } );

// カメラの左から3D効果音を鳴らす
FSpatialPlayRequest Sound;
Sound.AssetPath = FString( "Audio/SpatialPulse.wav" );
Sound.Position = CameraPosition - CameraRight * 4.0f;
SpatialAudio->PlayOnce( Sound );

// 遊ぶ人向けのボタンを置く (AUi3DSceneとAEffect3DSceneのどちらでも使える)
const u32 StartButton = Ui().AddButton( "START", FVec2{ 32, 88 }, FVec2{ 180, 44 } );
```

## 素材

`Assets/` に置く。モデルは **FBX**。詳しくは [`Assets/README.md`](Assets/README.md)。

## 確かめる

```powershell
.\Tools\RunUnitTests.ps1                    # 窓も音も要らない単体テスト
.\Tools\RunSimulationDeterminismTest.ps1    # 記録と再生が一致するか
.\Tools\CaptureApp.ps1 -Out shot.png        # 起動して 1 枚撮って閉じる
```

**3D はテストだけでは分からない。** 光の向きを間違えても、影のパスが抜けても、行列が
転置していても、全部コンパイルは通ってテストも緑になる。出るのは「画がおかしい」だけ。
だから最後の 1 つがある。

## 構成

```
Source/
  AcsFramework_Core/   土台 (App / Assets / Audio / Scene / Text / Simulation ...)
  AcsFramework_Sample/ サンプルの場面
  Common/              どの層からも使う小物
  Debug/               開発支援 (DevConsole / HotReload / Perf / DebugTop)
  EntryPoint/          入口
Assets/                素材
Tools/                 ビルド・テスト・撮影・配布物の取得
Docs/                  ロードマップと画
```

各フォルダに README がある。**「なぜそうなっているか」はそちらに書いてある。**

## ライセンス

Apache-2.0 ([`LICENSE`](LICENSE))。ACS の配布物と同梱ライブラリのライセンスは
`ThirdParty/acs/Licenses/` に入る。
