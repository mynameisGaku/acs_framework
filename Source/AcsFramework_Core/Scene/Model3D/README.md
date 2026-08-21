# Model3D

3D の見えるものをシーンへ置く。

このファイルは**フォルダに何を置いてよいか**を決めるもの。迷ったらここに戻ること。

---

## この枠組みが何のために在るか

ACS は 3D を描く力を一式持っている（PBR・影・IBL・空・水・SSS・ポスト処理）。
足りないのは**手数の少なさ**だけ。ここは「モデルを置く」を数行で済ませるための層。

| ACS | 役割 |
|---|---|
| `ANode` + `AMeshComponent3D` | 見えるもの 1 つ（形・モデルの場所・色・影） |
| `EMeshPrimitive3D` | 素材が無くても試せる形（Cube / Sphere / Plane / Mesh） |
| `ALegacyScene3DAdapter` | 上を実際に描く側。エディタで作った `.acscene` も読める |

**描画を自前で書かない。** ここが作るのはノードと部品だけで、描くのは向こうの仕事。

---

## 使い方

```cpp
// 置く
ANode* const Hero = CModel3DSpawner::SpawnInto( Scene.Graph(),
    FModel3DSpawnParams::FromMesh( FStringView( "hero.mdl" ), FVec3{ 0.0f, 0.0f, 5.0f } ) );

// 素材が無くても試せる
FModel3DSpawnParams Ball = FModel3DSpawnParams::FromPrimitive( EMeshPrimitive3D::Sphere, FVec3{ 2.0f, 0.0f, 0.0f } );
Ball.Color = FVec4{ 1.0f, 0.2f, 0.2f, 1.0f };
CModel3DSpawner::SpawnInto( Scene.Graph(), Ball );

// 置いた後に動かす
Hero->Local().position.x += 1.0f;
```

シーンの物は `Scene.Graph()` へ置く。これにより有効な `FNodeId` が付き、当たり判定、
波紋、識別子による破棄へ同じノードを渡せる。`ANode&` を受ける従来の多重定義は、
シーン外の一時的なノード木や既存コードとの互換用であり、識別子は発行しない。

何も書かなければ「原点に、等倍で、白い立方体を、影を落とす形で」置く。
**要るところだけ書けばよい**ようにしてある。

---

## フォルダに置いてよいもの

| 置き場所 | 置いてよいもの | 例 |
|---|---|---|
| 直下 | 何をどこへ置くかの指定 | `FModel3DSpawnParams` |
| 直下 | 置く処理 | `CModel3DSpawner` |

カメラ・ライティング・アニメーション・当たり判定はここではない。
骨付きモデルは`Scene/Animation3D`、実形状への線判定は`Scene/Pick3D`へ分けてある。

---

## 気をつけること

- **置けたのに見えない、を作らせない。** 大きさに 0 が入っている場合と、モデルを指しているのに
  場所が空の場合は、置く前に弾いて `nullptr` を返す。失敗も何も起きないのが一番追いにくい。
- **失敗したら親には何も足さない。** 半端なノードがシーンに残らないようにする。
- **向きは度で受ける。** 書く人が度で考えるため。中でラジアンへ直す
  （ACS の保存形式も度なので、往復しても崩れない）。
- 負の倍率は**鏡写しとして通す**。0 だけを弾く。
- **名前はシーンを保存すると消える。** 名前で探す作りにしないこと
  （`Scene/Snapshot/README.md` を見ること）。
