# 3D視線フォーカス

画面中央などから実形状へ線を飛ばし、登録対象へ入った、離れた、切り替わった、決定した、を
世代付きノードIDで返す。人物の見た目を複数の子ノードへ分けても、親だけ登録すればよい。

`AUi3DScene`派生では場面接続を自動で行う。

```cpp
InteractionFocus().RegisterTarget( *Door, FStringView( "E: OPEN" ), FVec3{ 0.0f, 2.0f, 0.0f } );

// 省略時も、捉えた対象には照準と奥行きを守る選択輪郭が自動で付く。
InteractionHighlightParams().Color = FVec3{ 0.3f, 1.0f, 0.7f };
InteractionHighlightParams().ThicknessPixels = 2.0f;

const FInteractionFocus3DUpdateResult Result = UpdateInteractionFocus( bInteractPressed );
if ( Result.Activated() )
{
	ANode* const Target = Graph().Get( Result.ActivatedNode );
	// 作品側の扉、会話、取得処理を呼ぶ。
}
```

## 分解

`AdvanceInteractionFocus3D(State, Input)`は値だけを受け取り、次の対象とイベントだけを返す。
入力装置、時刻、場面、描画へ触れないため、対象切替と決定規則を単体で再現できる。

`CInteractionFocus3D`は薄い場面アダプターで、次だけを行う。

1. `FSceneRay::FromNormalizedScreen`で正規化画面位置から有限レイを作る
2. `CScenePicker::RaycastGeometry`で最前面の実形状だけを取る
3. 命中した子から登録済み祖先を探す
4. 純粋遷移へ候補を渡し、フォーカス中だけ`CWorldLabel3DLayer`へ操作案内を置く

`AUi3DScene`は描画直前に現在の有効対象だけをACSの`SetSelectionHighlight`へ渡す。
輪郭マスク、手前の物による遮蔽、ポスト処理での合成はACSが持ち、Frameworkは
`FInteractionHighlight3DParams`の検証と場面寿命に合わせた解除だけを行う。
`InteractionHighlightParams().bEnabled = false`で輪郭だけを切っても、判定、案内、照準は残る。

未登録の最前面形状は遮蔽物になる。壁越しに奥の対象を拾わない。ノード破棄とscene内容差し替えは
`FNodeId`とroot同一性で検出し、古い対象や案内を再利用しない。

`UpdateInteractionFocus`は自動では呼ばれない。入力の押した瞬間を作品側で決めて、カメラ更新後に
1フレーム1回呼ぶ。これによりUI入力中の抑止や、キーボードとゲームパッドの割り当て方を作品側で
自由に保てる。
