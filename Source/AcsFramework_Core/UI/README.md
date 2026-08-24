# 3D場面のプレイヤーUI

`AUi3DScene` は、ACSの軽量な `CUiLayer` を3D場面の寿命へ結び付ける。
初期化、入力、更新、終了、HUD描画を基底が引き受けるため、派生側は表示物を追加して結果を読む。

```cpp
class AGameScene : public AUi3DScene
{
public:
	void OnEnter() noexcept override
	{
		AUi3DScene::OnEnter();
		m_StartButton = Ui().AddButton( "START", FVec2{ 32, 88 }, FVec2{ 180, 44 } );
	}

	void OnUpdate( f32 DeltaSeconds ) noexcept override
	{
		AUi3DScene::OnUpdate( DeltaSeconds );
		if ( Ui().ConsumeButtonPress( m_StartButton ) ) StartGame();
	}

private:
	u32 m_StartButton = 0;
};
```

`AEffect3DScene` は `AUi3DScene` を継承するため、Effekseerを使う場面でも同じ `Ui()` を使える。
UIはHDR描画、トーンマップ、TAAまたはFXAAが終わった後のLDR画面へ重ねる。文字やボタンは
bloomや輪郭補正の影響を受けず、3D場面より手前、ポーズ・デバッグ・ロード表示より奥に出る。

同じ場面から`TryApplyVisualPreset3D( EVisualPreset3D::Balanced )`を呼ぶと、遮蔽、反射、
間接光、bloom、露出、輪郭補正を標準品質へ一括設定できる。`Performance`は高価な反射と
間接光を切り、`Cinematic`はTAAと強めの画面空間効果を使う。適用後も`PostParams()`などを
直接調整でき、未知の値では既存設定を変更しない。

同じ基底は、3Dの見える物を少ない手数で置く窓口も持つ。`SpawnNode3D`は複数の見た目をまとめる
空ノード、`SpawnModel3D`はプリミティブまたは静的モデル、`SpawnInteractableModel3D`は静的モデルと
視線操作登録、`SpawnCollidableModel3D`は静的モデルと衝突、
`SpawnInteractableCollidableModel3D`は静的モデルと衝突と視線操作の一括生成、
`SpawnImage3D`は向き固定の画像板、`SpawnBillboard3D`はカメラ追従画像板、
`SpawnAnimatedModel3D`は骨付きモデル、`SpawnInteractableAnimatedModel3D`は骨付きモデルと視線操作登録、
`SpawnCollidableAnimatedModel3D`は骨付きモデルと衝突、
`SpawnInteractableCollidableAnimatedModel3D`は骨付きモデルと衝突と視線操作、
`SpawnThirdPersonCharacter3D`は静的または骨付きモデルと自己衝突、移動、追従カメラ、任意アニメーション、
`SpawnGround3D`は表示面と直下の厚み付き箱、`SpawnBlock3D`は同寸法の立方体表示と箱型衝突、
`SpawnSphere3D`は同半径の球表示と球型衝突、
`SpawnRoom3D`は歩ける床と四方の壁、
`SpawnStairs3D`は共通底面から積み上がる隙間のない階段、
`SpawnLight3D`は太陽または点光源、
`SpawnWater3D`は水面を扱う。パスを渡した場合だけ場面共通の
asset窓口で読み、読込済みassetはそのまま使う。通常の衝突付き生成結果は
`DestroyCollidableModel3D`へ渡すと、ノードと形状を対のまま片付けられる。

```cpp
SpawnModel3D( FModel3DSpawnParams::FromMesh(
    FStringView( "Models/House.fbx" ), FVec3{ 0.0f, 0.0f, 4.0f } ) );
SpawnInteractableCollidableModel3D(
    FModel3DSpawnParams::FromMesh(
        FStringView( "Models/Door.fbx" ), FVec3{ 0.0f, 0.0f, 6.0f } ),
    FStringView( "E: OPEN" ), FCollisionShape3DParams::FromBounds( 0x2u ) );
SpawnImage3D( FSprite3DSpawnParams::FromImage(
    FStringView( "Textures/Sign.png" ), FVec3{ 0.0f, 2.0f, 3.0f }, FVec2{ 1.2f, 0.6f } ) );
SpawnGround3D( FVec2{ 16.0f, 12.0f } );
SpawnBlock3D( FVec3{ 4.0f, 2.0f, 0.5f }, FVec3{ 0.0f, 1.0f, 4.0f } );
FCollidableModel3DSpawnResult Ball = SpawnSphere3D( 0.8f, FVec3{ 2.0f, 0.8f, 2.0f } );
FRoom3DSpawnResult Room = SpawnRoom3D( FVec2{ 12.0f, 8.0f }, 3.0f );
FStairs3DSpawnResult Stairs = SpawnStairs3D(
    8u, 2.0f, 0.32f, 0.18f, FVec3{ -2.0f, 0.0f, -3.0f } );
SpawnLight3D( FLight3DSpawnParams::Sun( FVec3{ -0.47f, 0.58f, 0.66f } ) );

ANode* const Vehicle = SpawnNode3D( FStringView( "Vehicle" ) );
if ( Vehicle != nullptr ) SpawnModel3D(
    FModel3DSpawnParams::FromPrimitive( EMeshPrimitive3D::Cube, FVec3{} ), Vehicle );

FCollidableModel3DSpawnResult Wall = SpawnBlock3D(
    FVec3{ 5.0f, 2.5f, 0.4f }, FVec3{ 0.0f, 1.25f, 8.0f }, 0x2u );

const FCollidableModel3DSpawnResult Enemy = SpawnCollidableAnimatedModel3D(
    FAnimatedModel3DSpawnParams::FromModel(
        FStringView( "Models/Enemy.fbx" ), FVec3{ 2.0f, 0.0f, 6.0f } ),
    FCollisionShape3DParams::FromSphere( FVec3{ 0.0f, 0.9f, 0.0f }, 0.45f, 0x2u ) );

FWater3DSpawnParams Water;
Water.Position = FVec3{ 2.5f, 0.1f, -1.0f };
SpawnWater3D( Water );

PlaySound3D( FStringView( "Audio/Hit.wav" ), FVec3{ 1.0f, 0.5f, 3.0f } );

const FSceneRayHit Picked = PickScreen3D( FVec2{ 0.5f, 0.5f }, 100.0f );
DestroyCollidableModel3D( Wall );
DestroyCollidableModel3D( Ball );
DestroyRoom3D( Room );
DestroyStairs3D( Stairs );
```

`PlaySound3D`は現在カメラを聴取位置へ同期してから、その瞬間の距離と左右位置で短い効果音を
鳴らす。持続する音源を個別管理する場合は`RefreshSpatialAudioListener`をカメラ更新後に呼び、
`CSpatialAudioSubsystem`の`AcquireSource`と`PlayFromSource`を使う。

`Collision3D()`はこの場面専用の3D衝突集合を返す。ノードへ球・箱・描画境界を登録すると、
問い合わせ時に現在位置へ同期し、破棄済みノードと場面終了時の登録を自動で外す。場面側で
`CSceneCollision3D{ Graph() }`を別途所有する必要はない。

呼出側所有の`CProximityTrigger3D`を`BindProximityTrigger3D`へ渡すと、基準ノードへ追従する
球または箱の範囲をこの場面の衝突集合へ接続できる。毎更新の`Update`は、対象レイヤーに入った、滞在中、
出たノードを別々の世代付き識別子配列で返す。扉や会話などの反応は場面側で決める。

```cpp
const FCollisionShapeId3D DoorShape = Collision3D().TryAddBounds( *DoorNode, DoorLayer );
CProximityTrigger3D DoorTrigger;
BindProximityTrigger3D(
    DoorTrigger, *DoorNode,
    FProximityTrigger3DParams::Box( FVec3{ 1.5f, 2.0f, 3.0f }, PlayerLayer ) );

FProximityTrigger3DUpdateResult Result;
if ( DoorTrigger.Update( Result ) && Result.DidEnter( PlayerNode->Id() ) ) OpenDoor();
DrawCollisionShape3D( DoorShape ); // 登録した衝突形状を表示し続ける場合
DrawCollisionShapes3D( DoorLayer ); // 同じレイヤーの有効形状を一括表示する場合
DrawProximityTrigger3D( DoorTrigger ); // 判定範囲を表示し続ける場合
DrawArrow3D( DoorNode->World().position, DoorNode->World().position + FVec3::Up() ); // 方向を表示する場合
DrawAxes3D( DoorNode->World().position, DoorNode->World().rotation ); // ローカル座標軸を表示する場合
DrawGrid3D(); // world原点を中心に水平グリッドを表示する場合
DrawCircle3D( DoorNode->World().position, FVec3::Up(), 0.5f ); // 水平面の向きと半径を輪で表示する場合
DrawCone3D( DoorNode->World().position, Rotate( DoorNode->World().rotation, FVec3::Forward() ), 2.0f, 0.6f ); // 正面範囲を表示する場合
DrawCylinder3D( DoorNode->World().position, FVec3::Up(), 2.0f, 0.4f ); // 回転軸や円柱範囲を表示する場合
DrawBox3D( DoorNode->World().position, DoorNode->World().rotation, FVec3{ 0.6f, 1.0f, 0.2f } ); // 回転した局所範囲を表示する場合
```

`DrawCollisionShape3D`は`Collision3D()`へ登録済みで現在問い合わせ対象の形状番号だけを受け付け、
判定と同じworld球またはworld軸平行箱を既存デバッグ線へ一括登録する。
`DrawCollisionShapes3D`は同じ処理をレイヤーマスクへ一致する全有効形状へ行い、表示できた形状数を返す。
`DrawProximityTrigger3D`はこの場面へ接続済みのトリガーだけを受け付け、判定と同じworld球または
world軸平行箱を既存デバッグ線へ一括登録する。線は次の透明3D描画後に消える。
`DrawArrow3D`は胴体1本と矢尻4本を原子的に登録し、法線、移動方向、光の向きを素材なしで表示する。
`DrawAxes3D`は指定回転のローカルX、Y、Zを赤、緑、青の3本の矢印として原子的に登録する。
`DrawGrid3D`は中心のyを高さとして水平XZ面を等分し、X・Z方向の全線を原子的に登録する。
`DrawCircle3D`は指定world法線へ直交する円を作り、接触面、半径、効果範囲を少ない線で表示する。
`DrawCone3D`は指定world方向へ底面円と4本の側線を伸ばし、視野、範囲、ノード正面を表示する。
`DrawCylinder3D`は指定world軸へ両端円と4本の側線を置き、回転軸、体積、センサー範囲を表示する。
`DrawBox3D`は中心、回転、半サイズから向き付きの12辺を作り、回転ノードの局所範囲を表示する。

`SpawnCollidableModel3D`はモデル生成とこの衝突登録を一括で行い、ノードと形状番号を返す。
描画境界、明示箱、明示球を選べ、登録できなければ生成ノードも破棄予定へ戻す。厚さのない
`Plane`を床にする場合は`FCollisionShape3DParams::FromBox`で歩ける厚みを明示する。
素材を使わない壁、足場、箱型障害物は`SpawnBlock3D`へ全寸法と中心位置を渡すと、表示と衝突の
寸法を別々に書かずに済む。既定外の色、向き、材質は`FBlock3DSpawnParams`で変更する。
球型障害物は`SpawnSphere3D`へ半径と中心位置を渡すと、表示と球衝突の半径を別々に書かずに済む。
既定外の色と材質は`FSphere3DSpawnParams`で変更する。
床と四方の壁を持つ天井なし空間は`SpawnRoom3D`へ内寸と壁高を渡すと5組をまとめて置ける。
途中失敗は既生成分まで巻き戻し、`DestroyRoom3D`は全5組を検証してから片付ける。
衝突付き階段は`SpawnStairs3D`へ段数、幅、踏面奥行き、段差を渡すと、下へ隙間を残さない段を
XZの正負4方向へ置ける。最大256段の途中失敗は高い側から巻き戻し、`DestroyStairs3D`は全段の
所有関係と重複を検証してから片付ける。
`SpawnCollidableAnimatedModel3D`は同じ失敗時巻き戻しを骨付きモデルへ適用し、初期animation再生も
成功したノードだけを返す。大きく姿勢が変わる人物には、読込時の境界より明示箱または明示球を使う。
通常モデル、骨付きモデル、地面、直方体、球の一括生成結果は`DestroyCollidableModel3D`へ渡すと、ノードと
形状が同じ登録対であることを確認してから両方を片付け、成功時だけ結果を空に戻す。

`SpawnInteractableModel3D`と`SpawnInteractableAnimatedModel3D`は、モデル生成後に
`InteractionFocus()`へ操作案内と対象を登録する。対象登録に失敗した場合は生成ノードも破棄予定へ
戻すため、個別の巻き戻しを書く必要はない。既存の複合ノードを1対象として扱う場合は
`InteractionFocus().RegisterTarget(...)`を直接使う。

衝突も必要な単一モデルには`SpawnInteractableCollidableModel3D`または
`SpawnInteractableCollidableAnimatedModel3D`を使う。モデル生成、場面所有の衝突集合、視線操作を
同じノードへ接続し、最後の対象登録に失敗しても形状とノードを両方巻き戻す。
場面途中では`DestroyInteractableModel3D`または`DestroyInteractableCollidableModel3D`へ生成結果を
渡すと、操作案内と登録を直ちに外し、衝突付きなら形状も外してから呼出側の結果を空に戻す。

`SpawnNode3D`は見た目を持たない世代付きノードを作る。車体と車輪、人物の胴体と頭などを
同じ親の下へ`SpawnModel3D( Params, Parent )`で置くと、親の移動・回転だけで全体を動かせる。
生成に失敗した複合物は親を`DestroyNode3D`へ渡せば、子も含めて破棄予定へ戻せる。

`BindThirdPersonCharacter3D`へ呼出側所有の制御と自場面ノードを渡すと、この衝突集合と現在場面の
追従カメラへまとめて接続する。低水準の`Controller.Bind( Collision3D(), *this, Node, Params )`を
派生場面へ繰り返し書く必要はない。新しく単一モデルから作る場合は
`SpawnThirdPersonCharacter3D`へ制御、モデル、設定を渡せば、自己形状番号の反映と失敗時の
形状・ノード巻き戻しまで一括で行う。場面途中の破棄は`DestroyThirdPersonCharacter3D`が
ノード破棄予約、制御解除、自己形状解除、生成結果の無効化を同じ呼び出しで行う。

`PickScreen3D`は左上を0、右下を1とした画面位置から現在カメラの線を作り、この場面で最前面の
実形状を返す。判定線もデバッグ表示などへ使う場合は`MakeScreenRay3D`で作り、`Raycast3D`へ渡す。

返ったノードは`RotateDeg`、`MoveToward`、`LookAt`で動かせる。不要になったら
`DestroyNode3D( Node )`へ生ポインタを渡す。自場面のノードだけを破棄予定にし、成功時は
呼出側のポインタも`nullptr`へ戻す。ビルボードやワールドラベルの追従は世代付き識別子で
自動的に外れる。視線操作対象も次の`UpdateInteractionFocus`で破棄予定の祖先ごと外れるため、
各レイヤーを先に手作業で掃除する必要はない。

3Dノードへ文字を追従させる場合は`WorldLabels()`を使う。場面グラフへの接続、現在カメラからの
射影、ノード破棄と表示状態の確認を基底が受け持つ。

```cpp
FWorldLabel3DParams Label;
Label.Text = FStringView( "BOSS" );
Label.WorldOffset = FVec3{ 0.0f, 2.4f, 0.0f };
WorldLabels().AddNodeLabel( *BossNode, Label );
```

詳しい表示範囲と固定位置の使い方は`UI/WorldLabel3D/README.md`を参照する。

`InteractionFocus()`へ3D操作対象を登録すると、判定位置には照準も自動表示される。対象を
捉えたときの色や寸法は`InteractionReticleParams()`で変更できる。詳しくは
`UI/InteractionReticle3D/README.md`を参照する。捉えたmesh subtreeには奥行きを守る選択輪郭も
自動で付き、色、強さ、pixel幅は`InteractionHighlightParams()`で変更できる。

`CUiLayer` は固定位置の文字とボタンを少ない手数で置く入口である。アンカー、入力欄、スライダー、
チェックボックス、データ結合が必要な画面はACSの `AWidget`、`AAnchorPanel`、`AButton`、
`ATextInput` などを直接使う。

Demo3Dの「CHANGE FXAA KEY」は、ボタンで `FActionKeyRebindState` の入力待ちを始め、次の
`KeyPressed`を割り当て表へ適用する例になっている。確定キーは`CGameSettingsSubsystem`へ保存し、
次回起動時に復元する。UI層には設定や入力状態を持たせず、表示とクリックだけを任せる。
