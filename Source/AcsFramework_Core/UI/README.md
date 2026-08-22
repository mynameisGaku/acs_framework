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

同じ基底は、3Dの見える物を少ない手数で置く窓口も持つ。`SpawnModel3D`はプリミティブまたは静的
モデル、`SpawnCollidableModel3D`は静的モデルと衝突形状の一括生成、`SpawnImage3D`は向き固定の
画像板、`SpawnBillboard3D`はカメラ追従画像板、
`SpawnAnimatedModel3D`は骨付きモデル、`SpawnLight3D`は太陽または点光源、`SpawnWater3D`は
水面を扱う。パスを渡した場合だけ場面共通のasset窓口で読み、読込済みassetはそのまま使う。

```cpp
SpawnModel3D( FModel3DSpawnParams::FromMesh(
    FStringView( "Models/House.fbx" ), FVec3{ 0.0f, 0.0f, 4.0f } ) );
SpawnImage3D( FSprite3DSpawnParams::FromImage(
    FStringView( "Textures/Sign.png" ), FVec3{ 0.0f, 2.0f, 3.0f }, FVec2{ 1.2f, 0.6f } ) );
SpawnLight3D( FLight3DSpawnParams::Sun( FVec3{ -0.47f, 0.58f, 0.66f } ) );

const FCollidableModel3DSpawnResult Wall = SpawnCollidableModel3D(
    FModel3DSpawnParams::FromMesh(
        FStringView( "Models/Wall.fbx" ), FVec3{ 0.0f, 0.0f, 8.0f } ),
    FCollisionShape3DParams::FromBounds( 0x2u ) );

FWater3DSpawnParams Water;
Water.Position = FVec3{ 2.5f, 0.1f, -1.0f };
SpawnWater3D( Water );

PlaySound3D( FStringView( "Audio/Hit.wav" ), FVec3{ 1.0f, 0.5f, 3.0f } );

const FSceneRayHit Picked = PickScreen3D( FVec2{ 0.5f, 0.5f }, 100.0f );
```

`PlaySound3D`は現在カメラを聴取位置へ同期してから、その瞬間の距離と左右位置で短い効果音を
鳴らす。持続する音源を個別管理する場合は`RefreshSpatialAudioListener`をカメラ更新後に呼び、
`CSpatialAudioSubsystem`の`AcquireSource`と`PlayFromSource`を使う。

`Collision3D()`はこの場面専用の3D衝突集合を返す。ノードへ球・箱・描画境界を登録すると、
問い合わせ時に現在位置へ同期し、破棄済みノードと場面終了時の登録を自動で外す。場面側で
`CSceneCollision3D{ Graph() }`を別途所有する必要はない。

`SpawnCollidableModel3D`はモデル生成とこの衝突登録を一括で行い、ノードと形状番号を返す。
描画境界、明示箱、明示球を選べ、登録できなければ生成ノードも破棄予定へ戻す。厚さのない
`Plane`を床にする場合は`FCollisionShape3DParams::FromBox`で歩ける厚みを明示する。

`BindThirdPersonCharacter3D`へ呼出側所有の制御と自場面ノードを渡すと、この衝突集合と現在場面の
追従カメラへまとめて接続する。低水準の`Controller.Bind( Collision3D(), *this, Node, Params )`を
派生場面へ繰り返し書く必要はない。

`PickScreen3D`は左上を0、右下を1とした画面位置から現在カメラの線を作り、この場面で最前面の
実形状を返す。判定線もデバッグ表示などへ使う場合は`MakeScreenRay3D`で作り、`Raycast3D`へ渡す。

返ったノードは`RotateDeg`、`MoveToward`、`LookAt`で動かせる。不要になったら
`DestroyNode3D( Node )`へ生ポインタを渡す。自場面のノードだけを破棄予定にし、成功時は
呼出側のポインタも`nullptr`へ戻す。ビルボードやワールドラベルの追従は世代付き識別子で
自動的に外れるため、追従レイヤーを先に手作業で掃除する必要はない。

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
