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

WASDで移動、左Shiftで走行、矢印キーで視点、Spaceでジャンプ、Q/Eで距離を操作する。ゲームパッドなら
左スティックで移動、左スティック押込で走行、右スティックで視点、下側ボタンでジャンプ、
左右バンパーで距離を変える。

デモ中に `B` を押すと、カメラ追従する3D画像板を実行中に追加し、もう一度押すと安全に破棄する。
描画開始後の追加・削除でも、ACSのGPU資源同期を通ることを確認できる。

> `FetchAcs.ps1` がまだ Release を落とせない段階なら、エンジンをローカルでビルドして
> `.\Tools\FetchAcs.ps1 -FromLocal C:\acs_dev` で持ってくる (`ThirdParty/acs/README.md`)。

## 何ができるか

| | |
|---|---|
| 3D を置く | `SpawnNode3D()`、`SpawnModel3D()`、`SpawnInteractableModel3D()`、`SpawnCollidableModel3D()`、`SpawnAnimatedModel3D()`、`SpawnInteractableAnimatedModel3D()`、`SpawnCollidableAnimatedModel3D()`、FBX の取り込み、材質 (metallic / roughness) |
| 3D画像を置く | `SpawnImage3D()`の固定板、`SpawnBillboard3D()`のカメラ追従板、透過PNG、深度判定、HDR合成 |
| 3D を照らす | `SpawnLight3D()`、方向だけで置ける太陽、位置と距離だけで置ける点光源 |
| 動かす | `SpawnThirdPersonCharacter3D()`でモデル生成・自己衝突・移動・向き・追従カメラを一括化。既存ノードには`BindThirdPersonCharacter3D()` |
| 操作を変える | UIでキーボード、ゲームパッドのボタン・軸を選び、自動保存して次回起動時に復元 |
| カメラで追う | `CNodeOrbitCamera3D`、人物の注視点追従、回転・距離操作、遮蔽物回避 |
| 見た目 | 物理大気・空気遠近・ボリューム雲・影・IBL・遮蔽 (SSAO)・間接光 (SSGI)・反射 (SSR)・霧・トーンマップ・輪郭補正 (FXAA) |
| 3D 天候 | `AWeather3DScene`、晴天・曇天・雨・雪・嵐・霧・砂嵐の滑らかな遷移 |
| 3D 水面 | `SpawnWater3D()`、屈折・反射・泡・動的な波紋 |
| 3D 演出 | `AEffect3DScene`、Effekseer、depth 遮蔽、HDR・bloom への自動合成 |
| 3D 音響 | `PlaySound3D()`、現在カメラ基準の距離減衰、モノラル効果音の左右定位 |
| 遊ぶ人向け UI | `AUi3DScene`、文字・ボタン・入力、ポスト処理後の鮮明なHUD合成 |
| 3D位置の文字 | `WorldLabels()`、ノード破棄と画面外を安全に扱う敵名・目的地表示 |
| 3Dデバッグ描画 | `DrawLine3D()`、`DrawAabb3D()`、`DrawSphere3D()`、深度を無視して常に確認できる1フレーム線 |
| 当てる | `MakeScreenRay3D()` / `Raycast3D()` / `PickScreen3D()`で球面や読み込みメッシュへ正確に当てる |
| 重なりと移動判定 | `CSceneCollision3D`、ノード追従、球・箱の重なり、球スイープ、レイヤー |
| 土台 | 起動・場面遷移・アセット・音・セーブ・設定・入力再割り当て・多言語・決定性・開発支援 |

詳しくは [`Docs/ROADMAP.md`](Docs/ROADMAP.md)。**v1.0.0 で何を入れて何を入れないか**もそこに書いてある。

## 書き味

```cpp
// 置く
FModel3DSpawnParams Ball = FModel3DSpawnParams::FromPrimitive( EMeshPrimitive3D::Sphere, FVec3{ 0, 1, 0 } );
Ball.Roughness = 0.2f;
SpawnModel3D( Ball );

// FBX を置く (Assets からの相対名)
FModel3DSpawnParams Model = FModel3DSpawnParams::FromMesh( FStringView( "Models/Robot.fbx" ), Position );
SpawnModel3D( Model );

// FBXを置くと同時に視線操作へ登録する。登録失敗時は生成モデルも残らない
ANode* const Door = SpawnInteractableModel3D(
    FModel3DSpawnParams::FromMesh(
        FStringView( "Models/Door.fbx" ), FVec3{ 0.0f, 0.0f, 4.0f } ),
    FStringView( "E: OPEN" ) );

// 別の立体を置くと同時に描画境界を衝突へ登録する。両方成功した結果だけが返る
FModel3DSpawnParams Wall = FModel3DSpawnParams::FromPrimitive(
    EMeshPrimitive3D::Cube, FVec3{ 4.0f, 0.5f, 0.0f } );
const FCollidableModel3DSpawnResult SolidWall = SpawnCollidableModel3D(
    Wall, FCollisionShape3DParams::FromBounds( 0x2u ) );

// 厚さのないPlaneは、歩ける厚みをローカル箱で明示する
const FCollidableModel3DSpawnResult Floor = SpawnCollidableModel3D(
    FModel3DSpawnParams::FromPrimitive( EMeshPrimitive3D::Plane, FVec3{} ),
    FCollisionShape3DParams::FromBox(
        FVec3{ 0.0f, -0.5f, 0.0f }, FVec3{ 0.5f, 0.5f, 0.5f }, 0x2u ) );

// 複数の見た目を1個として動かす空ノードを作り、その下へモデルを置く
ANode* const Robot = SpawnNode3D( FStringView( "Robot" ) );
if ( Robot != nullptr ) SpawnModel3D(
    FModel3DSpawnParams::FromPrimitive( EMeshPrimitive3D::Cube, FVec3{} ), Robot );

// 透過PNGを固定向きの3D板として置く
FSprite3DSpawnParams Marker = FSprite3DSpawnParams::FromImage(
    FStringView( "Textures/Marker.png" ), FVec3{ 0, 2, 3 }, FVec2{ 0.8f, 0.8f } );
SpawnImage3D( Marker );

// AUi3DSceneなら、同じ指定をカメラへ向く3D板として1回で置ける
SpawnBillboard3D( Marker );

// 面から太陽へ向かう方向だけで、影とPBRへ繋がる平行光を置く
SpawnLight3D( FLight3DSpawnParams::Sun( FVec3{ -0.47f, 0.58f, 0.66f } ) );

// 骨付きFBXを読み、自己衝突、移動、追従カメラ、4状態アニメーションへ1回で繋ぐ
CThirdPersonCharacter3D HeroController;
FAnimatedModel3DSpawnParams Hero = FAnimatedModel3DSpawnParams::FromModel(
    FStringView( "Models/Hero.fbx" ), Position );
Hero.InitialAnimation = FStringView( "Idle" );
FThirdPersonCharacter3DSpawnParams HeroSetup;
HeroSetup.Control.LocalCollisionCenter = FVec3{ 0.0f, 0.9f, 0.0f };
HeroSetup.Control.Movement.Radius = 0.45f;
HeroSetup.Control.CollisionMask = 0x2u;
HeroSetup.Collision = FCollisionShape3DParams::FromSphere(
    FVec3{ 0.0f, 0.9f, 0.0f }, 0.45f, 0x1u );
FThirdPersonCharacter3DSpawnResult HeroSpawn =
    SpawnThirdPersonCharacter3D( HeroController, Hero, HeroSetup );
ANode* const HeroNode = HeroSpawn.Node;

// 動かす
Node->RotateDeg( FVec3{ 0, 90.0f * DeltaSeconds, 0 } );
Node->MoveToward( Target, Speed * DeltaSeconds );
Node->LookAt( Target );

// 消す。成功時はNodeもnullptrになるので、破棄予定ノードを触り続けない
DestroyNode3D( Node );

// 左上を0、右下を1とした画面位置から、実際の3D表面へ当てる
const FSceneRay Ray = MakeScreenRay3D( FVec2{ MouseX / static_cast<f32>( W ), MouseY / static_cast<f32>( H ) } );
const FSceneRayHit Hit = Raycast3D( Ray );

// 当たり判定の線と箱を1フレーム表示する。残したい場合は毎フレーム呼ぶ
if ( Hit.IsHit() )
{
    DrawLine3D( Ray.Origin, Hit.Point, FVec4{ 0.2f, 0.95f, 1.0f, 1.0f } );
    DrawAabb3D( FAabb3::FromCenterExtents( Hit.Point, FVec3{ 0.08f, 0.08f, 0.08f } ),
        FVec4{ 1.0f, 0.62f, 0.12f, 1.0f } );
    DrawSphere3D( FSphere{ Hit.Point, 0.12f }, FVec4{ 1.0f, 0.28f, 0.78f, 1.0f } );
}

// 場面所有の衝突集合へ形状を結び、現在位置へ自動追従させる
CSceneCollision3D& Collision = Collision3D();
Collision.TryAddBounds( *WallNode, 0x2u );
TArray<ANode*> Nearby;
Collision.TryOverlapSphere(
    FSphere{ HeroNode->World().position, 2.0f }, Nearby, HeroSpawn.Shape, 0x2u );

// 接続済みキャラクターを、既定の入力割り当てから1回進める
CActionBindingTable ActionBindings;
const FThirdPersonCharacter3DActionSet Actions = FThirdPersonCharacter3DActionSet::WithRunAction();
FThirdPersonCharacter3DControlPreset{}.TryBuildBindings( ActionBindings, Actions );
FActionInput PreviousCharacterInput;
const FActionInput CharacterInput = ActionBindings.Resolve( InputReader );
HeroController.Update( CharacterInput, PreviousCharacterInput, DeltaSeconds, Actions );
PreviousCharacterInput = CharacterInput;
HeroController.OrbitCamera().TryShakePreset( EShakePreset::HitImpact ); // 被弾時

// ノードの頭上へ、ポスト処理でぼけない文字を追従させる
FWorldLabel3DParams PlayerLabel;
PlayerLabel.Text = FStringView( "PLAYER" );
PlayerLabel.WorldOffset = FVec3{ 0.0f, 2.15f, 0.0f };
WorldLabels().AddNodeLabel( *HeroNode, PlayerLabel );

// 反射・屈折・泡を持つ水面を置き、波紋を起こす
FWater3DSpawnParams Water;
Water.Position = FVec3{ 2.5f, 0.1f, -1.0f };
ANode* const Surface = SpawnWater3D( Water );
if ( Surface != nullptr ) AddWaterDisturbance( Surface->Id(), Water.Position );

// 近くの物から跳ね返る色を足す
GlobalIllumination().Intensity = 0.75f;

// TAAを使わない場面の斜め線を滑らかにする
PostParams().fxaa_enabled = true;

// 雲、雲影、霧、空色、環境光を2.5秒で嵐へ揃える (classの基底はAWeather3DScene)
SetWeather( EWeatherKind::Storm, 2.5f );

// 遠くの3D物体と水面を物理大気へ馴染ませる (world単位はメートル)
SetAerialPerspectiveEnabled( true );

// 3D effectを出す (AWeather3DSceneにもAEffect3DSceneが含まれる)
PlayEffect3D( FStringView( "Effects/hit.efkefc" ), FVec3{ 0, 1, 0 } );

// 現在カメラの左から3D効果音を鳴らす
PlaySound3D( FStringView( "Audio/SpatialPulse.wav" ), CameraPosition - CameraRight * 4.0f );

// 遊ぶ人向けのボタンを置く (AUi3DSceneとAEffect3DSceneのどちらでも使える)
const u32 StartButton = Ui().AddButton( "START", FVec2{ 32, 88 }, FVec2{ 180, 44 } );

// 場面の途中で消す場合も、操作と自己形状を残さず結果まで空にする
DestroyThirdPersonCharacter3D( HeroController, HeroSpawn );
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
