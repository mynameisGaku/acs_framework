# Scene/Pick3D — 「そこに何があるか」を訊く

線を飛ばして、当たったノードを返す。**クリックした物を取る**のと、**足元に何があるか調べる**
のが主な用途。

## 使う

```cpp
// マウスの位置から
const FSceneRay Ray = FSceneRay::FromScreen( Camera, MouseX, MouseY, Width, Height );
const FSceneRayHit Hit = CScenePicker::Raycast( Root(), Ray );
if ( Hit.IsHit() )
{
    Hit.Node->SetName( FStringView( "掴んだ" ) );
}

// 足元
const FSceneRayHit Ground = CScenePicker::Raycast( Root(), FSceneRay::Down( Position ) );
```

重なっているものを全部欲しいときは `RaycastAll`。手前から順に並ぶ。

## この層が足しているもの

判定そのものは ACS が持っている (`RaycastAabb`)。ここが足すのは 3 つ。

1. **木を辿る。** 根から下の全ノードを見る
2. **境界を世界へ移す。** 各ノードの `World()` を掛け、8 隅を包み直す
3. **いちばん手前を選ぶ**

毎回書くには長いが、書き方は 1 通りしかない。だから窓口にした。

## 精度は «箱まで»

**モデルの三角形とは判定しない。** 球の角を掠めると、実際には当たっていなくても当たる。

掴む・調べる・大まかに拾う用途にはこれで足りる。厳密に要るなら、ここで候補を絞ってから
ACS の `CMeshCollider` (BVH 付き) を使うこと。

回転していると箱は実際より大きくなる (軸に沿った箱で包み直すため)。45 度回した細長い板が
いちばんずれる。**取りこぼすよりは大きめに拾う方がまし**、という判断で作ってある
(掴めないより、掴みすぎの方が気付ける)。

## 見えないものは当たらない

`IsVisible()` か `IsEnabled()` が false のノードは、その子ごと飛ばす。
**画面から消したものを «掴めてしまう» のがいちばん困る**ため。

## 落とし穴

- **`MaxDistance` を無限にしない。** 遠くの物まで拾うと、画面外の物を掴んだことになる
- 板 (`Plane`) は厚みが 0。内部でごく薄い厚みを足しているので、真上から落とした線でも当たる
- `FSceneRayHit::Normal` は**箱の面の向き**で、モデル表面の向きではない
- `Hit.Node` は所有しない。この記録を跨いでノードを消さないこと

## ファイル

| | |
|---|---|
| `SceneRay.h` / `.cpp` | 線。画面の位置から作る `FromScreen` を含む |
| `SceneRayHit.h` | 当たった記録 |
| `ScenePicker.h` / `.cpp` | 木を辿って答える |
| `Test/ScenePickerTest.cpp` | 手前が返るか、消したものを飛ばすか、板に当たるか |
