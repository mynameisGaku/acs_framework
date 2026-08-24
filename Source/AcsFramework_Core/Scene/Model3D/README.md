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

// HDR自己発光は色と強度だけで作り、ACSのtonemapとbloomへ同じ経路で渡す
SpawnModel3D( FModel3DSpawnParams::FromEmissivePrimitive( EMeshPrimitive3D::Sphere, FVec3{ -2.0f, 1.0f, 0.0f }, FVec3{ 0.12f, 0.52f, 1.0f }, 4.0f ) );

// 車の塗装やラッカーのような透明な上塗りは、色と上塗り粗さだけで作る
SpawnModel3D( FModel3DSpawnParams::FromCoatedPrimitive( EMeshPrimitive3D::Sphere, FVec3{ 0.0f, 1.0f, 2.0f }, FVec3{ 0.78f, 0.10f, 0.06f }, 0.05f ) );

// 肌や蝋のように表面のすぐ下へ光が回る材質は、表面色と内部色だけで作る
SpawnModel3D( FModel3DSpawnParams::FromSubsurfacePrimitive( EMeshPrimitive3D::Sphere, FVec3{ 1.0f, 1.0f, 2.0f }, FVec3{ 0.82f, 0.46f, 0.34f }, FVec3{ 1.0f, 0.18f, 0.08f } ) );

// 布やベルベットのように輪郭へ柔らかく光を返す材質は、表面色と位置だけで作る
SpawnModel3D( FModel3DSpawnParams::FromFabricPrimitive( EMeshPrimitive3D::Sphere, FVec3{ 3.0f, 1.0f, 2.0f }, FVec3{ 0.16f, 0.28f, 0.68f } ) );

// ACS既定の二段影と縁光を使うイラスト調の形も、色と位置だけで作る
SpawnModel3D( FModel3DSpawnParams::FromToonPrimitive( EMeshPrimitive3D::Sphere, FVec3{ 2.0f, 1.0f, 2.0f }, FVec3{ 0.95f, 0.58f, 0.10f } ) );

// 素材を使わない直方体は、全寸法と中心位置だけで表示と箱型衝突を揃える
FCollidableModel3DSpawnResult SolidObstacle = SpawnBlock3D(
    FVec3{ 2.0f, 1.0f, 0.5f }, FVec3{ 4.0f, 0.5f, 0.0f }, 0x2u );

// 素材を使わない球は、半径と中心位置だけで表示と球型衝突を揃える
FCollidableModel3DSpawnResult SolidBall = SpawnSphere3D(
    0.75f, FVec3{ 2.0f, 0.75f, 0.0f }, 0x2u );

// 歩ける平面は広さだけで表示面と直下の厚み付き箱を揃える
const FCollidableModel3DSpawnResult SolidFloor = SpawnGround3D(
    FVec2{ 10.0f, 10.0f }, FVec3{}, 0x2u );

// 複数部品を同じ位置・向きで動かす場合は空の親を作る
ANode* const Character = SpawnNode3D( FStringView( "Character" ) );
if ( Character != nullptr ) SpawnModel3D(
    FModel3DSpawnParams::FromPrimitive( EMeshPrimitive3D::Cube, FVec3{} ), Character );

// 置いた後に動かす
Hero->Local().position.x += 1.0f;

// 衝突付き結果はノードと形状を同じ呼出しで片付ける
DestroyCollidableModel3D( SolidObstacle );
```

`AUi3DScene`を使わないノード木では、従来どおり`CModel3DSpawner::SpawnInto`へグラフと、
パス読込時だけ`CModelLibrary`を渡す。`SpawnModel3D`はこの生成器と場面共通のasset窓口を結ぶ薄い
アダプターであり、別のモデル管理は持たない。

`SpawnCollidableModel3D`は同じ読込経路でモデルを置き、`CSceneCollision3D`へ形状を登録する。
成功時は`FCollidableModel3DSpawnResult`の`Node`と`Shape`を両方返す。登録に失敗した生成ノードは
破棄予定へ戻すため、「見えるが当たらない」半端な配置を成功として残さない。
通常の衝突付き結果は`DestroyCollidableModel3D`へ渡すと、ノードと形状の対応を検証したうえで
両方を片付け、成功時だけ呼出側の結果を空へ戻す。
素材を使わない直方体は[`SpawnBlock3D`](../Block3D/README.md)、球は
[`SpawnSphere3D`](../Sphere3D/README.md)、歩ける表示面は[`SpawnGround3D`](../Ground3D/README.md)を
使うと、表示と衝突の寸法を1個の設定へまとめられる。
床と四方の壁をまとめる場合は[`SpawnRoom3D`](../Room3D/README.md)が5組の生成と巻き戻しを扱う。
両端が開いた通路は[`SpawnCorridor3D`](../Corridor3D/README.md)が床と側壁2枚の生成、巻き戻し、
一括破棄を扱う。
開口のある壁枠は[`SpawnDoorway3D`](../Doorway3D/README.md)が左右柱と上枠の生成、巻き戻し、
一括破棄を扱い、通過部分に箱型衝突を残さない。
衝突付き階段は[`SpawnStairs3D`](../Stairs3D/README.md)が段数と1段の寸法から直方体列を組み、
全段の生成、巻き戻し、一括破棄を扱う。
同じ単一モデルを視線操作へも登録する場合は`SpawnInteractableCollidableModel3D`を使うと、
3処理の途中失敗をまとめて巻き戻せる。
場面途中では結果を`DestroyInteractableCollidableModel3D`へ渡し、形状と操作対象ごと安全に消せる。

`SpawnNode3D`は`AUi3DScene`のグラフへ空ノードを置く。複数の`SpawnModel3D`へ同じ親を渡せば、
人物、車、武器などを1個の親Transformで動かせる。途中失敗時は親を`DestroyNode3D`へ渡す。

`FromEmissivePrimitive`は表面色と自己発光色を同じRGBへ揃え、HDR強度を設定する。強度が1を
超える部分はACSのtonemapで表示域へ収まり、場面のbloomが有効なら周囲へ光が広がる。自己発光は
照明そのものではないため、周囲を直接照らす必要がある場合は`SpawnLight3D`も併用する。
外部モデルを発光させる場合は`EmissiveColor`と`EmissiveStrength`を配置前に直接指定できる。

`FromCoatedPrimitive`は表面色を揃え、`Clearcoat`を1、指定した`ClearcoatRoughness`を上塗り層へ
設定する。通常の`Roughness`は元の面、`ClearcoatRoughness`はその上へ重なる透明層だけを変える。
車の塗装、ラッカー、濡れた面のように細い反射を残したい場合に使う。外部モデルでは2値を直接
指定でき、どちらも0から1の有限値だけを受け付ける。

`FromSubsurfacePrimitive`は表面色、内部を通って見える色、光の回り込み量をACSのPBR材質へ渡す。
肌、蝋、乳白素材のように影の境目を柔らかく見せたい場合に使う。描画器や画面用の一時資源は
Frameworkへ複製せず、ACSが必要な場面だけ有効にする内部散乱経路をそのまま利用する。外部モデルでは
`SubsurfaceColor`と`SubsurfaceStrength`を直接指定でき、どちらも0から1の有限値だけを受け付ける。

`FromFabricPrimitive`は粗い非金属面と、視線に近い輪郭へ返る毛羽の反射をACSのPBR材質へ渡す。
布、フェルト、ベルベットを色と位置だけで始められ、毛羽色には同じ表面色を使う。外部モデルでは
`SheenColor`、`SheenStrength`、`SheenRoughness`を直接指定でき、すべて0から1の有限値だけを
受け付ける。光源、環境光、実際の反射計算はFrameworkへ複製せずACSへ任せる。

`FromToonPrimitive`は`bToonShading`を有効にし、ACS既定の二段影、縁光、段階的な反射を使う。
細かなトゥーン値をFramework側へ複製せず、ACSが調整した既定値をそのまま利用する。PBR専用の
上塗りはトゥーン陰影では使われないため、光沢コートと同時に指定しない。

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
- 自己発光色は各成分0から1、強度は0から10だけを受け付ける。壊れたHDR値を描画へ渡さない。
- 上塗り強度と上塗り粗さは0から1の有限値だけを受け付ける。元の面の粗さとは別に扱う。
- 毛羽の色、強さ、粗さは0から1の有限値だけを受け付ける。強さ0なら従来のPBRになる。
- 内部色と光の回り込み量は0から1の有限値だけを受け付ける。強度0なら従来と同じ不透明PBRになる。
- **名前はシーンを保存すると消える。** 名前で探す作りにしないこと
  （`Scene/Snapshot/README.md` を見ること）。
