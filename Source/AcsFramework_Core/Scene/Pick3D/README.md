# Scene/Pick3D — 「そこに何があるか」を訊く

線を飛ばして、当たったノードを返す。**クリックした物を取る**のと、**足元に何があるか調べる**
のが主な用途。

## 使う

```cpp
// マウスの位置から
const FSceneRay Ray = FSceneRay::FromScreen( Camera, MouseX, MouseY, Width, Height );
const FSceneRayHit Hit = CScenePicker::RaycastGeometry( *this, Ray );
if ( Hit.IsHit() )
{
    Hit.Node->SetName( FStringView( "掴んだ" ) );
}

// 足元
const FSceneRayHit Ground = CScenePicker::RaycastGeometry( *this, FSceneRay::Down( Position ) );
```

大まかな境界箱だけでよい場合は`Raycast( Root(), Ray )`。重なっている境界箱を全部欲しいときは
`RaycastAll`で、手前から順に並ぶ。

## この層が足しているもの

木の走査と形状判定はACSが持っている。Frameworkが足すのは3つ。

1. **画面から線を作る。** `FSceneRay::FromScreen`で座標と距離を揃える
2. **場面をそのまま渡せる。** ノードグラフと識別子の扱いを呼び出し側へ漏らさない
3. **結果を1つの型へ揃える。** ノード、距離、世界座標の点と法線を`FSceneRayHit`で返す

毎回書くには長いが、書き方は 1 通りしかない。だから窓口にした。

## 箱と実形状を使い分ける

`RaycastGeometry`は、球の丸い表面や読み込みメッシュの三角形まで判定し、実表面の位置と法線を
返す。階層の移動・回転・非一様な拡大もACS側で処理する。

`Raycast`は従来どおり境界箱だけを見る。取りこぼしにくく安いので、大まかな候補抽出や
`RaycastAll`が必要な場面に向く。球の角を掠めると実際には触れていなくても当たる。

回転していると箱は実際より大きくなる (軸に沿った箱で包み直すため)。45 度回した細長い板が
いちばんずれる。**取りこぼすよりは大きめに拾う方がまし**、という判断で作ってある
(掴めないより、掴みすぎの方が気付ける)。

読み込みメッシュの実形状判定は、境界箱で候補を絞った後に三角形を調べる。高ポリゴンモデルへ
毎フレーム大量に線を飛ばす用途では、ACSの`CMeshCollider`を所有者側で構築し、BVHを再利用する。

## 見えないものは当たらない

`IsVisible()` か `IsEnabled()` が false のノードは、その子ごと飛ばす。
**画面から消したものを «掴めてしまう» のがいちばん困る**ため。

## 落とし穴

- **`MaxDistance` を無限にしない。** 遠くの物まで拾うと、画面外の物を掴んだことになる
- 境界箱の`Raycast`は厚み0の板へごく薄い幅を足す。`RaycastGeometry`は有限平面そのものを見る
- `Raycast`の`Normal`は箱の面、`RaycastGeometry`では実表面の向きになる
- `Hit.Node` は所有しない。この記録を跨いでノードを消さないこと

## ファイル

| | |
|---|---|
| `SceneRay.h` / `.cpp` | 線。画面の位置から作る `FromScreen` を含む |
| `SceneRayHit.h` | 当たった記録 |
| `ScenePicker.h` / `.cpp` | 木を辿って答える |
| `Test/ScenePickerTest.cpp` | 手前、表示状態、球面、読み込みメッシュの三角形を検証 |
