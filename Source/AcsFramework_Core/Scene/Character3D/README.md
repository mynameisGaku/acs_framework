# 第三者視点3Dキャラクター操作

`CThirdPersonCharacter3D`は、カメラ基準の移動、ジャンプ、移動方向への向き変更、ノード追従カメラを
1回の`Update()`へまとめる。新しくモデルから始める場合は`AUi3DScene::SpawnThirdPersonCharacter3D`が
静的または骨格モデルの生成、自己衝突登録、移動、追従カメラを1回で接続する。骨格モデルに
待機・歩き・走り・ジャンプが揃っていれば、その移動連動再生も同時に接続する。

```cpp
CThirdPersonCharacter3D HeroController;
FAnimatedModel3DSpawnParams Hero = FAnimatedModel3DSpawnParams::FromModel(
	FStringView( "Models/Hero.fbx" ), FVec3{} );
FThirdPersonCharacter3DSpawnParams Setup;
Setup.Control.CollisionMask = 0x1u;
Setup.Collision = FCollisionShape3DParams::FromSphere(
	Setup.Control.LocalCollisionCenter, Setup.Control.Movement.Radius, 0x2u );
FThirdPersonCharacter3DSpawnResult Spawned =
	SpawnThirdPersonCharacter3D( HeroController, Hero, Setup );

CActionBindingTable ActionBindings;
const FThirdPersonCharacter3DActionSet Actions = FThirdPersonCharacter3DActionSet::WithRunAction();
FThirdPersonCharacter3DControlPreset{}.TryBuildBindings( ActionBindings, Actions );
FActionInput PreviousInput;

// 固定更新ごとに装置を明示的に読み、前回入力と一緒に渡す。
const FActionInput CurrentInput = ActionBindings.Resolve( DeviceReader );
const FThirdPersonCharacter3DUpdateResult Result = HeroController.Update( CurrentInput, PreviousInput, DeltaSeconds, Actions );
PreviousInput = CurrentInput;
```

`SpawnThirdPersonCharacter3D`は生成した形状番号を`Control.SelfShape`へ必ず設定するため、呼出側が
同じ番号を写す必要はない。モデル生成、形状登録、移動またはカメラ接続の途中で失敗した場合は、
形状とノードを両方巻き戻す。骨格モデルの必須処理が成功し、4状態のクリップだけが不足した場合は
`Succeeded()`をtrueのまま保ち、`bAnimationBound`をfalseにして`InitialAnimation`の再生を続ける。
場面の途中で消す場合は`DestroyThirdPersonCharacter3D(HeroController, Spawned)`へ生成結果を渡す。
ノードだけを先に破棄して制御へ非所有参照を残さず、自己形状を外して生成結果も空へ戻す。
既に組み立てた複合ノードを使う場合は、従来の`BindThirdPersonCharacter3D`で個別に接続できる。

既定の`FThirdPersonCharacter3DActionSet`は、軸0/1を左右・前後移動、軸2/3を左右・上下視点、
アクション0をジャンプ、1/2をカメラの近接・遠隔へ割り当て、走行は無効にして従来入力との互換性を保つ。
`WithRunAction()`で作るとアクション3を走行へ明示追加できる。番号を変えたい場合は
`FThirdPersonCharacter3DActionSet`の値を変更して`Update()`の第4引数へ渡す。ジャンプは現在と前回の
差から押した瞬間だけ発生し、押し続けたまま着地しても自動で再ジャンプしない。

`FThirdPersonCharacter3DControlPreset`は、WASD、矢印、Space、E/Qと、左右スティック、下側ボタン、
左右バンパーを上記の番号へまとめて割り当てる。走行を有効にした`ActionSet`も渡すと、左Shiftと
左スティック押込を追加する。走行中は`FThirdPersonCharacter3DParams::RunSpeedMultiplier`を
基本速度へ掛ける。別のキー配置が必要なら、既存の
`CActionBindingTable`へ個別に追加する。プリセットは入力装置を読まず、作成先の表だけを置き換える。
`AcsFramework_Sample/Scene/Demo3DScene.cpp`では、素材不要のキャラクター、床・水底・障害物の衝突、
この既定操作、追従カメラまでを実際の場面寿命へ接続している。

入力装置と時刻は内部取得しない。AIや記録再生から直接操作する場合は、従来どおり
`FThirdPersonCharacter3DInput`を組み立てる`Update()`も使える。`FThirdPersonCharacter3DUpdateResult`は
視点、移動、向き、追従点、
任意アニメーションの各段階を分けて返すため、途中まで反映された失敗を隠さない。`Mover()`、
`OrbitCamera()`、`Animator()`から個別機能も操作でき、たとえば被弾時は
`HeroController.OrbitCamera().TryShakePreset(EShakePreset::HitImpact)`で揺らせる。

## 移動だけを個別に使う

`CCharacterMover3D`は、希望する世界X/Z速度とジャンプ要求をACSの球型移動計算へ渡し、
成功した移動だけをシーンノードへ反映する。重力、床への接地、壁沿いの移動、天井、
初期貫通の解消を1回の`Move()`で扱える。

```cpp
CSceneCollision3D& Collision = Collision3D();
Collision.TryAddBox( *Floor, FVec3{}, FVec3{ 10.0f, 0.5f, 10.0f }, 0x1u );

CCharacterMover3D HeroMover;
HeroMover.Bind( Collision, *Hero, FVec3{ 0.0f, 0.5f, 0.0f } );
HeroMover.SetCollisionFilter( {}, 0x1u );

HeroMover.MoveFromCamera( Camera(), FVec2{ MoveX, MoveForward }, 4.0f, bJumpPressed, DeltaSeconds );
HeroMover.TurnTowardMovement( 540.0f, DeltaSeconds );
HeroAnimator.Update( FCharacterAnimation3DInput{ Length( FVec2{ HeroMover.Velocity().x, HeroMover.Velocity().z } ), HeroMover.IsGrounded() } );
```

`Bind()`は球中心のローカル位置を受け取る。足元をノード原点にする場合は、Yへ球半径を指定する。
ノードが親を持つ場合も、計算結果の世界移動量を親座標へ戻してからローカル位置へ反映する。
`MoveFromCamera()`へ画面の左右・前後操作量と最大速度を渡すと、カメラの上下角を除いた向きへ
変換する。斜め入力は長さ1へ制限するので、前後移動より速くならない。世界X/Z速度を直接決める
AIや再生処理は`Move()`を使う。

`TurnTowardMovement()`は直前の実速度へ世界Y軸回りで最短回転する。1秒あたりの最大角度を渡すため、
小さな値なら重く、大きな値なら素早く向きを変える。停止中は現在の向きを保つ。

キャラクター自身を`CSceneCollision3D`へ登録した場合は、その形状を`SetCollisionFilter()`へ渡す。
無効な形状を渡すと自己除外を行わない。外部から瞬間移動させた後に速度と接地状態も消したい場合は、
`ResetMotion()`で現在位置を読み直す。

固定更新の時刻、入力の押下判定、ノードと衝突集合の寿命、描画とアニメーションは呼出側が所有する。
移動失敗時はノード、速度、接地状態、直前結果を変更しない。
