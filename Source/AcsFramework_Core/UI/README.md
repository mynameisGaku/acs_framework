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
`UI/InteractionReticle3D/README.md`を参照する。

`CUiLayer` は固定位置の文字とボタンを少ない手数で置く入口である。アンカー、入力欄、スライダー、
チェックボックス、データ結合が必要な画面はACSの `AWidget`、`AAnchorPanel`、`AButton`、
`ATextInput` などを直接使う。

Demo3Dの「CHANGE FXAA KEY」は、ボタンで `FActionKeyRebindState` の入力待ちを始め、次の
`KeyPressed`を割り当て表へ適用する例になっている。確定キーは`CGameSettingsSubsystem`へ保存し、
次回起動時に復元する。UI層には設定や入力状態を持たせず、表示とクリックだけを任せる。
