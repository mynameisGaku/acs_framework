# acs_framework

**3D のゲームを、DXLib と同等以下の手数で作り始められて、DXLib より綺麗に映る。**

描く力は [ACS](https://github.com/mynameisGaku/ArtsCommonSystem) が持っている。この枠組みが
足すのは**手数の少なさ**だけで、機能を作り直すことはしない。

現在は開発版`0.5.0-dev`。版の意味と互換性は[`Docs/VERSIONING.md`](Docs/VERSIONING.md)、
利用側へ影響する変更は[`CHANGELOG.md`](CHANGELOG.md)に記録する。
代表的な3D公開入口は[`Docs/PUBLIC_API.md`](Docs/PUBLIC_API.md)にまとめ、`RunRepoChecks.ps1`で監査する。

![3D デモ](Docs/demo3d.png)

上の絵はサンプル 1 本 (`Source/AcsFramework_Sample/Scene/Demo3DScene.cpp`) の
出力。**空・太陽・影・環境光・雲がすべて 1 つの太陽から繋がっている。** 空は物理ベースの
大気を焼いたもので、それがそのまま環境光にもなるので、空と光が食い違わない。

## 動かす

必要なもの: Windows x64 / Visual Studio 2026 (MSVC v145、C++20) / DirectX 12 が動くGPU。
保証範囲と対象外の環境は[`Docs/SUPPORTED_PLATFORMS.md`](Docs/SUPPORTED_PLATFORMS.md)にまとめている。

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
`X`では回転立方体を操作対象・衝突形状ごと破棄し、もう一度押すと3登録をまとめて再生成する。
往復する取り込みモデルが回転立方体の範囲へ入る、または出ると、同じ表示欄で近接トリガーの
進入・退出も確認できる。`V`を押すと1world単位の床グリッドと全コライダーを一括表示し、近接箱は
外側の水色から内側の橙色へ変わる。回転立方体には赤X・緑Y・青Zのローカル座標軸も重なり、
実形状、検出範囲、尺度、現在の向きを同時に比較できる。

> `FetchAcs.ps1` がまだ Release を落とせない段階なら、エンジンをローカルでビルドして
> `.\Tools\FetchAcs.ps1 -FromLocal C:\acs_dev` で持ってくる (`ThirdParty/acs/README.md`)。

## 何ができるか

| | |
|---|---|
| 3D を置く | `SpawnModel3D()` / `SpawnAnimatedModel3D()`と、それぞれの操作対象版・衝突版・操作＋衝突版、`SpawnBlock3D()` / `TryUpdateBlock3D()`、`SpawnSphere3D()`、`SpawnRoom3D()`、`SpawnCorridor3D()`、`SpawnBridge3D()`、`SpawnDoorway3D()`、`SpawnFence3D()`、`SpawnStairs3D()`、`SpawnStreetLamp3D()`、`DestroyCollidableModel3D()`、`SpawnNode3D()`、FBX の取り込み、材質 (metallic / roughness / HDR自己発光 / 布の毛羽反射 / 内部散乱) |
| 3D画像を置く | `SpawnImage3D()`の固定板、`SpawnBillboard3D()`のカメラ追従板、透過PNG、深度判定、HDR合成 |
| 3D を照らす | `SpawnLight3D()`の太陽・点光源、`SpawnLamp3D()`の見える発光球＋点光源、`SpawnStreetLamp3D()` / `TryUpdateStreetLamp3D()`の衝突付き金属ポスト＋発光球＋点光源、`SpawnStudioLightRig3D()` / `TryUpdateStudioLightRig3D()`の被写体用キー・フィル・リム |
| 3D 地面 | `SpawnGround3D()`、広さだけで置ける表示面と直下の厚み付き衝突 |
| 動かす | `SpawnThirdPersonCharacter3D()`でモデル生成・自己衝突・移動・向き・追従カメラを一括化。既存ノードには`BindThirdPersonCharacter3D()` |
| 操作を変える | UIでキーボード、ゲームパッドのボタン・軸を選び、自動保存して次回起動時に復元 |
| カメラで追う | `CNodeOrbitCamera3D`、人物の注視点追従、回転・距離操作、遮蔽物回避 |
| 見た目 | `TryApplyVisualPreset3D()`の3段階一括設定、物理大気・空気遠近・ボリューム雲・影・IBL・遮蔽 (SSAO)・間接光 (SSGI)・反射 (SSR)・霧・トーンマップ・TAA・輪郭補正 (FXAA) |
| 3D 天候 | `AWeather3DScene`、晴天・曇天・雨・雪・嵐・霧・砂嵐の滑らかな遷移 |
| 3D 水面 | `SpawnWater3D()`、屈折・反射・泡・動的な波紋 |
| 3D 演出 | `AEffect3DScene`、Effekseer、depth 遮蔽、HDR・bloom への自動合成 |
| 3D 音響 | `PlaySound3D()`、現在カメラ基準の距離減衰、モノラル効果音の左右定位 |
| 遊ぶ人向け UI | `AUi3DScene`、文字・ボタン・入力、ポスト処理後の鮮明なHUD合成 |
| 3D位置の文字 | `WorldLabels()`、ノード破棄と画面外を安全に扱う敵名・目的地表示 |
| 3Dデバッグ描画 | `DrawLine3D()`、`DrawArrow3D()`、`DrawAxes3D()`、`DrawGrid3D()`、`DrawCircle3D()`、`DrawCone3D()`、`DrawCylinder3D()`、`DrawBox3D()`、`DrawAabb3D()`、`DrawSphere3D()`、単体・レイヤー一括の`DrawCollisionShape3D()` / `DrawCollisionShapes3D()`、`DrawProximityTrigger3D()`、`DrawCheckpoint3D()`、深度を無視する1フレーム線 |
| 当てる | `MakeScreenRay3D()` / `Raycast3D()` / `PickScreen3D()`で球面や読み込みメッシュへ正確に当てる |
| 重なりと移動判定 | `CSceneCollision3D`、ノード追従、球・箱の重なり、球スイープ、レイヤー |
| 近づきを取る | `BindProximityTrigger3D()`、ノード追従する球・箱への進入・滞在・退出、レイヤー絞り込み |
| 通過を取る | `SpawnCheckpoint3D()`、指定した1形状の一度限り／再進入発火、`FCheckpointRoute3D`の順番・周回管理 |
| 土台 | 起動・場面遷移・アセット・音・セーブ・設定・入力再割り当て・多言語・決定性・開発支援 |

詳しくは [`Docs/ROADMAP.md`](Docs/ROADMAP.md)。**v1.0.0 で何を入れて何を入れないか**もそこに書いてある。
失敗時の出力、ログ、スレッド安全性、セーブ互換性の共通規則は
[`Docs/RUNTIME_CONTRACTS.md`](Docs/RUNTIME_CONTRACTS.md)を正本とする。
対応OS、コンパイラ、GPUとCIの検証範囲は
[`Docs/SUPPORTED_PLATFORMS.md`](Docs/SUPPORTED_PLATFORMS.md)を正本とする。

## 書き味

```cpp
// 半径と中心だけで、表示と球型衝突が揃った球を置く
FCollidableModel3DSpawnResult Ball = SpawnSphere3D(
    1.0f, FVec3{ 0.0f, 1.0f, 0.0f } );

// 色と強度だけで、bloomへ繋がる自己発光球を置く
SpawnModel3D( FModel3DSpawnParams::FromEmissivePrimitive( EMeshPrimitive3D::Sphere, FVec3{ 2, 1, 0 }, FVec3{ 0.1f, 0.5f, 1.0f }, 4.0f ) );

// 色と上塗り粗さだけで、車の塗装のような光沢コート球を置く
SpawnModel3D( FModel3DSpawnParams::FromCoatedPrimitive( EMeshPrimitive3D::Sphere, FVec3{ 0, 1, 2 }, FVec3{ 0.8f, 0.1f, 0.06f }, 0.05f ) );

// 色と位置だけで、磨き筋に沿って反射が伸びる金属球を置く
SpawnModel3D( FModel3DSpawnParams::FromBrushedMetalPrimitive( EMeshPrimitive3D::Sphere, FVec3{ -1, 1, 2 }, FVec3{ 0.58f, 0.64f, 0.72f } ) );

// 表面色と内部色だけで、肌や蝋のように光が回り込む球を置く
SpawnModel3D( FModel3DSpawnParams::FromSubsurfacePrimitive( EMeshPrimitive3D::Sphere, FVec3{ 1, 1, 2 }, FVec3{ 0.82f, 0.46f, 0.34f }, FVec3{ 1.0f, 0.18f, 0.08f } ) );

// 色と位置だけで、布やベルベットのように輪郭へ柔らかく光を返す球を置く
SpawnModel3D( FModel3DSpawnParams::FromFabricPrimitive( EMeshPrimitive3D::Sphere, FVec3{ 3, 1, 2 }, FVec3{ 0.16f, 0.28f, 0.68f } ) );

// 色と位置だけで、二段影と縁光を持つトゥーン球を置く
SpawnModel3D( FModel3DSpawnParams::FromToonPrimitive( EMeshPrimitive3D::Sphere, FVec3{ -2, 1, 2 }, FVec3{ 0.95f, 0.58f, 0.10f } ) );

// 8時から太陽、空、環境光を同じ時計で動かす (既定は実時間1分でゲーム内1時間)
EnableTimeOfDay3D( 8.0f );

// FBX を置く (Assets からの相対名)
FModel3DSpawnParams Model = FModel3DSpawnParams::FromMesh( FStringView( "Models/Robot.fbx" ), Position );
SpawnModel3D( Model );

// 広さだけで、表示面とその直下1mの歩ける衝突を同時に置く
FCollidableModel3DSpawnResult Ground = SpawnGround3D( FVec2{ 16.0f, 12.0f } );

// 全寸法と中心位置だけで、表示と箱型衝突が揃った壁を置く
FCollidableModel3DSpawnResult Wall = SpawnBlock3D(
    FVec3{ 4.0f, 2.0f, 0.5f }, FVec3{ 0.0f, 1.0f, 4.0f } );

// 形状番号を保ち、表示、位置、寸法、衝突レイヤーを片側だけずらさず更新する
FBlock3DSpawnParams MovedWall = FBlock3DSpawnParams::FromSize(
    FVec3{ 6.0f, 2.0f, 0.5f }, FVec3{ 1.0f, 1.0f, 4.0f } );
TryUpdateBlock3D( Wall, MovedWall );

// 内寸と壁高だけで、歩ける床と四方の壁をまとめて置く
FRoom3DSpawnResult Room = SpawnRoom3D( FVec2{ 12.0f, 8.0f }, 3.0f );

// 内幅と長さだけで、床と側壁を持つ両端が開いた通路を置く
FCorridor3DSpawnResult Corridor = SpawnCorridor3D(
    3.0f, 10.0f, 3.0f, FVec3{ 0.0f, 0.0f, 4.0f } );

// 幅、長さ、柵高だけで、歩ける床板と両側柵を持つ橋を置く
FBridge3DSpawnResult Bridge = SpawnBridge3D(
    3.0f, 10.0f, 1.15f, FVec3{ 0.0f, 1.0f, -5.0f } );

// 壁と開口の寸法だけで、見えない衝突を残さない出入口枠を置く
FDoorway3DSpawnResult Doorway = SpawnDoorway3D(
    4.0f, 3.0f, 1.2f, 2.2f, 0.25f, FVec3{ 0.0f, 0.0f, 14.0f } );

// 長さ、高さ、最大支柱間隔だけで、支柱と横桟を持つ衝突付き柵を置く
FFence3DSpawnResult Fence = SpawnFence3D(
    6.0f, 1.2f, 2.0f, FVec3{ 3.0f, 0.0f, -2.0f } );

// 段数と1段の寸法だけで、隙間のない衝突付き階段を置く
FStairs3DSpawnResult Stairs = SpawnStairs3D(
    8u, 2.0f, 0.32f, 0.18f, FVec3{ -2.0f, 0.0f, -3.0f } );

// 床位置だけで、衝突付き金属ポストと見える暖色ランプをまとめて置く
FStreetLamp3DSpawnResult StreetLamp = SpawnStreetLamp3D(
    FVec3{ 2.5f, 0.0f, -1.5f } );

// 床位置、高さ、発光と衝突を、生成番号を保ったまま街灯全体へ反映する
FStreetLamp3DSpawnParams TallerLamp = FStreetLamp3DSpawnParams::At(
    FVec3{ 3.0f, 0.0f, -1.5f } );
TallerLamp.PostHeight = 3.2f;
TryUpdateStreetLamp3D( StreetLamp, TallerLamp );

// 遮蔽、反射、間接光、bloom、露出、輪郭補正を標準品質へ揃える
TryApplyVisualPreset3D( EVisualPreset3D::Balanced );

// FBXを置くと同時に衝突と視線操作へ登録する。途中失敗時はモデルも形状も残らない
FCollidableModel3DSpawnResult Door = SpawnInteractableCollidableModel3D(
    FModel3DSpawnParams::FromMesh(
        FStringView( "Models/Door.fbx" ), FVec3{ 0.0f, 0.0f, 4.0f } ),
    FStringView( "E: OPEN" ), FCollisionShape3DParams::FromBounds( 0x2u ) );

// 全寸法と中心位置だけで壁を置く。表示と衝突は同じ寸法になる
const FCollidableModel3DSpawnResult SolidWall = SpawnBlock3D(
    FVec3{ 4.0f, 2.0f, 0.5f }, FVec3{ 4.0f, 1.0f, 0.0f }, 0x2u );

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

// 位置だけで、bloomする発光球と周囲を照らす点光源を同時に置く
FLamp3DSpawnResult Lamp = SpawnLamp3D( FVec3{ 1.6f, 2.1f, -0.8f } );

// 動かすときも発光球と点光源の位置・色を片側だけずらさない
FLamp3DParams MovedLamp = FLamp3DParams::At( FVec3{ 2.0f, 2.4f, 0.0f } );
MovedLamp.Color = FVec3{ 0.18f, 0.52f, 1.0f };
TryUpdateLamp3D( Lamp, MovedLamp );

// 中心、カメラ方向、半径だけで、太陽を保ったまま被写体用の3点照明を置く
FStudioLightRig3DSpawnResult StudioLights = SpawnStudioLightRig3D(
    FVec3{ 0.0f, 1.0f, 0.0f }, FVec3{ 0.0f, 0.0f, -1.0f }, 1.2f );

// 被写体やカメラが動いても、3灯の位置と見た目を片側だけずらさず追従させる
FStudioLightRig3DParams MovedStudioLights = FStudioLightRig3DParams::AroundSubject(
    FVec3{ 2.0f, 1.2f, -1.0f }, FVec3{ 1.0f, 0.0f, 0.0f }, 1.2f );
TryUpdateStudioLightRig3D( StudioLights, MovedStudioLights );

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

// 一括生成した扉は、操作案内と衝突形状も同じ呼び出しで直ちに外す
DestroyInteractableCollidableModel3D( Door );

// 通常の衝突付きモデルや地面も、ノードと形状を対のまま片付ける
DestroyCollidableModel3D( Ground );

// 左上を0、右下を1とした画面位置から、実際の3D表面へ当てる
const FSceneRay Ray = MakeScreenRay3D( FVec2{ MouseX / static_cast<f32>( W ), MouseY / static_cast<f32>( H ) } );
const FSceneRayHit Hit = Raycast3D( Ray );

// 当たり判定の線と箱を1フレーム表示する。残したい場合は毎フレーム呼ぶ
DrawGrid3D(); // world原点を中心に1単位刻みの水平グリッドを表示する
if ( Hit.IsHit() )
{
    DrawLine3D( Ray.Origin, Hit.Point, FVec4{ 0.2f, 0.95f, 1.0f, 1.0f } );
    DrawAabb3D( FAabb3::FromCenterExtents( Hit.Point, FVec3{ 0.08f, 0.08f, 0.08f } ),
        FVec4{ 1.0f, 0.62f, 0.12f, 1.0f } );
    DrawSphere3D( FSphere{ Hit.Point, 0.12f }, FVec4{ 1.0f, 0.28f, 0.78f, 1.0f } );
    DrawCircle3D( Hit.Point + Hit.Normal * 0.01f, Hit.Normal, 0.35f );
    DrawCone3D( Hit.Point, Hit.Normal, 0.75f, 0.28f );
    DrawCylinder3D( Hit.Point, Hit.Normal, 0.6f, 0.18f );
    DrawBox3D( Hit.Point, FQuat::Identity(), FVec3{ 0.2f, 0.2f, 0.2f } );
    DrawArrow3D( Hit.Point, Hit.Point + Hit.Normal * 0.75f );
    DrawAxes3D( Hit.Point );
}

// 場面所有の衝突集合へ形状を結び、現在位置へ自動追従させる
CSceneCollision3D& Collision = Collision3D();
Collision.TryAddBounds( *WallNode, 0x2u );
TArray<ANode*> Nearby;
Collision.TryOverlapSphere(
    FSphere{ HeroNode->World().position, 2.0f }, Nearby, HeroSpawn.Shape, 0x2u );
DrawCollisionShape3D( HeroSpawn.Shape ); // 登録形状を表示し続ける場合は毎フレーム呼ぶ
DrawCollisionShapes3D( 0x2u ); // レイヤー2の有効コライダーを一括表示する

// 扉へ追従する箱範囲へ人物が入った瞬間だけ処理する。Triggerは場面のメンバーとして保持する
CProximityTrigger3D DoorTrigger;
BindProximityTrigger3D(
    DoorTrigger, *Door.Node,
    FProximityTrigger3DParams::Box( FVec3{ 1.5f, 2.0f, 3.0f }, 0x1u ) );
FProximityTrigger3DUpdateResult Proximity;
if ( DoorTrigger.Update( Proximity ) && Proximity.DidEnter( HeroNode->Id() ) ) OpenDoor();
DrawProximityTrigger3D( DoorTrigger ); // 表示を続ける場合は毎フレーム呼ぶ

// GoalCheckpointとGoalは場面メンバー。人物形状がゴールへ入った瞬間だけ受け取る
CCheckpoint3D GoalCheckpoint;
FCheckpoint3DSpawnResult Goal = SpawnCheckpoint3D(
    GoalCheckpoint, HeroSpawn.Shape, FVec3{ 0.0f, 1.0f, 12.0f }, 2.0f, 0x1u );
FCheckpoint3DUpdateResult GoalState;
if ( GoalCheckpoint.Update( GoalState ) && GoalState.bActivatedThisUpdate ) SaveGoal();
DrawCheckpoint3D( GoalCheckpoint );

// 複数のCheckpointを通る順番と2周の完了を管理し、明示時間から区間タイムを測る
FCheckpointRoute3D Route;
Route.SetParams( FCheckpointRoute3DParams::ForCheckpoints( 3u, 2u ) );
FCheckpointRoute3DTimer RouteTimer;
RouteTimer.Start();
RouteTimer.Tick( DeltaSeconds );
FCheckpointRoute3DAdvanceResult RouteState;
if ( GoalState.bActivatedThisUpdate
    && Route.Advance( GoalIndex, RouteState ) )
{
    FCheckpointRoute3DTimingResult Timing;
    if ( RouteTimer.RecordAdvance( RouteState, Timing )
        && Timing.bRouteCompletedThisAdvance ) FinishRace( Timing.TotalElapsedSeconds );
}

// セーブ時は進行と計測を対で取得し、同じルート設定へ戻す
const FCheckpointRoute3DProgress SavedRoute = Route.CaptureProgress();
const FCheckpointRoute3DTimerState SavedTimer = RouteTimer.CaptureState();

// ロード時。Routeには保存時と同じ件数・周回数を設定しておく
Route.RestoreProgress( SavedRoute );
RouteTimer.RestoreState( SavedTimer );

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
.\Tools\RunCiChecks.ps1                    # CIと同じDebug/Release完全検証
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
Docs/                  ロードマップ、実行時契約、版管理、対応環境と画
```

各フォルダに README がある。**「なぜそうなっているか」はそちらに書いてある。**

## ライセンス

Apache-2.0 ([`LICENSE`](LICENSE))。ACS の配布物と同梱ライブラリのライセンスは
`ThirdParty/acs/Licenses/` に入る。
