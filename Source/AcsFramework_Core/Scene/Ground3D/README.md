# Ground3D

表示面と歩ける厚みを別々に組まず、1回で3D地面を置く。

```cpp
// 原点へ16m x 12mの地面を置く。上面から下へ既定の1mぶん衝突を持つ。
FCollidableModel3DSpawnResult Ground = SpawnGround3D( FVec2{ 16.0f, 12.0f } );
```

見た目や厚みを変える場合は値を明示する。

```cpp
FGround3DSpawnParams Ground = FGround3DSpawnParams::FromSize(
    FVec2{ 16.0f, 12.0f }, FVec3{ 0.0f, -0.2f, 0.0f } );
Ground.Thickness = 0.4f;
Ground.Color = FVec4{ 0.30f, 0.34f, 0.40f, 1.0f };
Ground.Roughness = 0.72f;
Ground.CollisionLayer = 0x2u;

const FCollidableModel3DSpawnResult Placed = SpawnGround3D( Ground );

FGround3DSpawnParams Wider = FGround3DSpawnParams::FromSize(
    FVec2{ 24.0f, 16.0f }, FVec3{ 0.0f, -0.4f, 0.0f } );
Wider.Thickness = 0.8f;
Wider.Color = FVec4{ 0.24f, 0.31f, 0.22f, 1.0f };
const bool bUpdated = TryUpdateGround3D( Placed, Wider );
```

`Position`は表示上面の中心で、箱型衝突はそこから下へ`Thickness`だけ伸びる。平面と箱は同じ
ノード尺度を使うため、`Size`を変えても表示範囲と歩ける範囲がずれない。0の寸法、有限でない値、
範囲外の材質値、衝突しないレイヤー0は配置前に拒否する。

`TryUpdateGround3D`は生成時のノードと形状番号の対応、新指定、表示部品を先に確認する。成功時も
衝突形状を作り直さないため、保持している形状番号を無効にせず、上面位置、広さ、厚み、材質、影、
衝突レイヤーを揃えて変更できる。別場面の結果、破棄予定ノード、不正値では何も変更しない。

`AUi3DScene`を使わない独自場面では、`CGround3DSpawner::SpawnInto`へ場面グラフと
`CSceneCollision3D`を渡す。更新には`CGround3DSpawner::TryApplyTo`へ同じ2つと生成結果を渡す。
生成後の衝突登録に失敗した場合はノードも巻き戻される。
場面途中で外す場合は結果を`DestroyCollidableModel3D`へ渡し、表示ノードと歩ける箱を同じ
呼出しで片付ける。

四方の壁も同時に必要な場合は[`SpawnRoom3D`](../Room3D/README.md)へ内寸と壁高を渡す。
床を壁の外周まで広げ、5組の途中失敗と一括破棄もまとめて扱う。

両端を開けて左右の壁だけを置く場合は[`SpawnCorridor3D`](../Corridor3D/README.md)へ内幅と長さを渡す。
入口から出口までの床を壁外面へ広げ、3組の途中失敗と一括破棄もまとめて扱う。
