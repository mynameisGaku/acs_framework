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
// AUi3DSceneの派生場面なら、読み込みも含めて置く
ANode* const Hero = SpawnModel3D(
    FModel3DSpawnParams::FromMesh( FStringView( "Models/Hero.fbx" ), FVec3{ 0.0f, 0.0f, 5.0f } ) );

// 素材が無くても試せる
FModel3DSpawnParams Ball = FModel3DSpawnParams::FromPrimitive( EMeshPrimitive3D::Sphere, FVec3{ 2.0f, 0.0f, 0.0f } );
Ball.Color = FVec4{ 1.0f, 0.2f, 0.2f, 1.0f };
SpawnModel3D( Ball );

// 別の立体に見た目と衝突形状の両方が必要なら、一括生成結果を受け取る
FModel3DSpawnParams Obstacle = FModel3DSpawnParams::FromPrimitive(
    EMeshPrimitive3D::Cube, FVec3{ 4.0f, 0.5f, 0.0f } );
const FCollidableModel3DSpawnResult SolidObstacle = SpawnCollidableModel3D(
    Obstacle, FCollisionShape3DParams::FromBounds( 0x2u ) );

// 厚さのない描画面には歩ける箱を明示する
FModel3DSpawnParams Floor = FModel3DSpawnParams::FromPrimitive( EMeshPrimitive3D::Plane, FVec3{} );
Floor.Scale = FVec3{ 10.0f, 1.0f, 10.0f };
const FCollidableModel3DSpawnResult SolidFloor = SpawnCollidableModel3D(
    Floor, FCollisionShape3DParams::FromBox(
        FVec3{ 0.0f, -0.5f, 0.0f }, FVec3{ 0.5f, 0.5f, 0.5f }, 0x2u ) );

// 置いた後に動かす
Hero->Local().position.x += 1.0f;
```

`AUi3DScene`を使わないノード木では、従来どおり`CModel3DSpawner::SpawnInto`へグラフと、
パス読込時だけ`CModelLibrary`を渡す。`SpawnModel3D`はこの生成器と場面共通のasset窓口を結ぶ薄い
アダプターであり、別のモデル管理は持たない。

`SpawnCollidableModel3D`は同じ読込経路でモデルを置き、`CSceneCollision3D`へ形状を登録する。
成功時は`FCollidableModel3DSpawnResult`の`Node`と`Shape`を両方返す。登録に失敗した生成ノードは
破棄予定へ戻すため、「見えるが当たらない」半端な配置を成功として残さない。

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
| 直下 | 生成と衝突登録の結果 | `FCollidableModel3DSpawnResult` |
| 直下 | 置く処理 | `CModel3DSpawner` |

カメラ・ライティング・アニメーション・当たり判定はここではない。
骨付きモデルは`Scene/Animation3D`、実形状への線判定は`Scene/Pick3D`へ分けてある。

---

## 気をつけること

- **置けたのに見えない、を作らせない。** 大きさに 0 が入っている場合と、モデルを指しているのに
  場所も読込済みモデルも無い場合は、置く前に弾いて `nullptr` を返す。失敗も何も起きないのが一番追いにくい。
- **失敗したら親には何も足さない。** 半端なノードがシーンに残らないようにする。
- **向きは度で受ける。** 書く人が度で考えるため。中でラジアンへ直す
  （ACS の保存形式も度なので、往復しても崩れない）。
- 負の倍率は**鏡写しとして通す**。0 だけを弾く。
- **名前はシーンを保存すると消える。** 名前で探す作りにしないこと
  （`Scene/Snapshot/README.md` を見ること）。
