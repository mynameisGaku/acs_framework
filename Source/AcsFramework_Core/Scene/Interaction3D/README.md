# 3D視線フォーカス

画面中央などから実形状へ線を飛ばし、登録対象へ入った、離れた、切り替わった、決定した、を
世代付きノードIDで返す。人物の見た目を複数の子ノードへ分けても、親だけ登録すればよい。

`AUi3DScene`派生では場面接続を自動で行う。

```cpp
FModel3DSpawnParams DoorParams = FModel3DSpawnParams::FromMesh(
    FStringView( "Models/Door.fbx" ), FVec3{ 0.0f, 0.0f, 4.0f } );
ANode* const Door = SpawnInteractableModel3D(
    DoorParams, FStringView( "E: OPEN" ), FVec3{ 0.0f, 2.0f, 0.0f } );

// 既存の親ノードや複数形状を1対象へまとめる場合は個別登録も使える。
ANode* const GroupedNpc = SpawnNode3D( FStringView( "GroupedNpc" ) );
ANode* const NpcBody = GroupedNpc != nullptr ? SpawnModel3D(
    FModel3DSpawnParams::FromPrimitive(
        EMeshPrimitive3D::Cube, FVec3{ 0.0f, 1.0f, 5.0f } ), GroupedNpc ) : nullptr;
if ( NpcBody != nullptr ) InteractionFocus().RegisterTarget(
    *GroupedNpc, FStringView( "E: TALK" ), FVec3{ 0.0f, 2.0f, 0.0f } );

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

`SpawnInteractableModel3D`は静的モデル、`SpawnInteractableAnimatedModel3D`は骨付きモデルの
読み込み、生成、操作対象登録を1回で行う。対象登録まで完了できなければ生成ノードも破棄予定へ
戻すため、「見えるが操作できない」半端なモデルを成功として残さない。

扉や押せる箱のように衝突も必要なら、`SpawnInteractableCollidableModel3D`または
`SpawnInteractableCollidableAnimatedModel3D`を使う。モデル生成、衝突形状登録、操作対象登録を
1回で行い、成功したノードと形状番号を`FCollidableModel3DSpawnResult`で返す。最後の操作登録に
失敗した場合も衝突形状を外してからノードを破棄予定へ戻す。

実行中に消す場合は、通常版の結果を`DestroyInteractableModel3D`、衝突付き結果を
`DestroyInteractableCollidableModel3D`へ渡す。ノード破棄予約、現在の操作案内、対象登録、任意の
衝突形状を同じ呼び出しで外し、成功時は呼出側のポインタまたは結果も空に戻す。

## 分解

`AdvanceInteractionFocus3D(State, Input)`は値だけを受け取り、次の対象とイベントだけを返す。
入力装置、時刻、場面、描画へ触れないため、対象切替と決定規則を単体で再現できる。

`CInteractionFocus3D`は薄い場面アダプターで、次だけを行う。

1. `FSceneRay::FromNormalizedScreen`で正規化画面位置から有限レイを作る
2. `CScenePicker::RaycastGeometry`で最前面の実形状だけを取る
3. 命中した子から登録済み祖先を探す
4. 純粋遷移へ候補を渡し、フォーカス中だけ`CWorldLabel3DLayer`へ操作案内を置く

`CInteractableModel3DSpawner`も状態を持たず、既存の静的・骨付きモデル生成器を呼んだ後に
`CInteractionFocus3D::RegisterTarget`へ渡すだけの接続層とする。衝突付きでは既存生成器が返した
形状番号も受け取り、後段失敗と明示破棄の解除順だけを追加する。

`AUi3DScene`は描画直前に現在の有効対象だけをACSの`SetSelectionHighlight`へ渡す。
輪郭マスク、手前の物による遮蔽、ポスト処理での合成はACSが持ち、Frameworkは
`FInteractionHighlight3DParams`の検証と場面寿命に合わせた解除だけを行う。
`InteractionHighlightParams().bEnabled = false`で輪郭だけを切っても、判定、案内、照準は残る。

未登録の最前面形状は遮蔽物になる。壁越しに奥の対象を拾わない。ノード破棄とscene内容差し替えは
`FNodeId`とroot同一性で検出し、古い対象や案内を再利用しない。対象自身だけでなく祖先が
破棄予定になった場合も、次の`UpdateInteractionFocus`で対象登録と案内を同時に外す。

`UpdateInteractionFocus`は自動では呼ばれない。入力の押した瞬間を作品側で決めて、カメラ更新後に
1フレーム1回呼ぶ。これによりUI入力中の抑止や、キーボードとゲームパッドの割り当て方を作品側で
自由に保てる。
