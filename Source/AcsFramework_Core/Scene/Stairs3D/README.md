# Stairs3D

階段を高さの違う衝突付き直方体へ手作業で分けず、段数と1段の寸法から1回で置く。

```cpp
// 原点からZ正方向へ、幅2m、踏面0.32m、段差0.18mの8段を置く。
FStairs3DSpawnResult Stairs = SpawnStairs3D(
    8u, 2.0f, 0.32f, 0.18f );
```

方向や見た目を変える場合は値を明示する。

```cpp
FStairs3DSpawnParams Stairs = FStairs3DSpawnParams::FromSteps(
    10u, 2.4f, 0.30f, 0.17f,
    FVec3{ 4.0f, 0.0f, 1.0f }, EStairs3DDirection::NegativeX );
Stairs.Color = FVec4{ 0.36f, 0.40f, 0.48f, 1.0f };
Stairs.Roughness = 0.72f;
Stairs.CollisionLayer = 0x2u;

FStairs3DSpawnResult Placed = SpawnStairs3D( Stairs );
```

`BottomEdgeCenter`は最下段手前の床上中心で、`Direction`は低い側から高い側へ伸びる軸方向である。
各段の奥行きは`StepDepth`、上面の高さ差は`StepHeight`になる。各直方体の下端を同じ床面へ揃えるため、
階段の下に移動体が入り込む隙間を残さない。X方向へ上る場合は幅をZ、Z方向では幅をXへ取る。

段数は1から256に制限する。0以下または有限でない寸法、派生する全奥行き・最上段位置が有限でない値、
範囲外の材質値、衝突しないレイヤー0は配置前に拒否する。

`CStairs3DSpawner`は既存の`CBlock3DSpawner`だけを合成する。途中の生成、衝突登録、結果確保に
失敗した場合は、それ以前の段を高い側から逆順に破棄予定へ戻して形状も外す。
場面途中で外す場合は`DestroyStairs3D`へ成功結果を渡す。全段が同じ場面で重複なく対になっている
ことを確認してから片付け、成功時だけ呼出側の結果を空にする。

階段全体を回転・拡縮する場合は同じ親ノードの下へ置ける。親を斜め回転または非一様拡縮すると、
各箱衝突は既存契約どおり表示を包む安全側のworld軸平行箱になる。

階段の前後を両端が開いた通路へ繋ぐ場合は[`SpawnCorridor3D`](../Corridor3D/README.md)へ内幅と長さを
渡すと、床と側壁2枚を同じ衝突付き配置契約で追加できる。
