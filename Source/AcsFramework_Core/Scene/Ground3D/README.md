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
```

`Position`は表示上面の中心で、箱型衝突はそこから下へ`Thickness`だけ伸びる。平面と箱は同じ
ノード尺度を使うため、`Size`を変えても表示範囲と歩ける範囲がずれない。0の寸法、有限でない値、
範囲外の材質値、衝突しないレイヤー0は配置前に拒否する。

`AUi3DScene`を使わない独自場面では、`CGround3DSpawner::SpawnInto`へ場面グラフと
`CSceneCollision3D`を渡す。生成後の衝突登録に失敗した場合はノードも巻き戻される。
場面途中で外す場合は結果を`DestroyCollidableModel3D`へ渡し、表示ノードと歩ける箱を同じ
呼出しで片付ける。
