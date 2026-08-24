# Sphere3D

球型の障害物や目印を、球表示と球型衝突へ別々に組まず、半径1個で置く。

```cpp
// 中心(0, 1.2, 3)へ、表示と衝突が半径1.2mの球を置く。
FCollidableModel3DSpawnResult Ball = SpawnSphere3D(
    1.2f, FVec3{ 0.0f, 1.2f, 3.0f } );
```

見た目を変える場合は値を明示する。

```cpp
FSphere3DSpawnParams Sphere = FSphere3DSpawnParams::FromRadius(
    0.8f, FVec3{ 2.0f, 0.8f, 0.0f } );
Sphere.Color = FVec4{ 0.18f, 0.48f, 0.92f, 1.0f };
Sphere.Metallic = 0.12f;
Sphere.Roughness = 0.24f;
Sphere.CollisionLayer = 0x2u;

const FCollidableModel3DSpawnResult Placed = SpawnSphere3D( Sphere );
```

ACSの球プリミティブと明示球衝突はどちらもローカル半径0.5である。`CSphere3DSpawner`は
指定半径の2倍をノードの均一尺度へ設定するため、root直下または均一拡縮の親では表示半径と
衝突半径が一致する。非一様な親拡縮では表示が楕円体になり、衝突は既存契約どおり最大軸で
外接する安全側の球になる。

0以下または有限でない半径、直径へ変換できない過大値、有限でない位置、範囲外の材質値、
衝突しないレイヤー0は配置前に拒否する。

`AUi3DScene`を使わない独自場面では、`CSphere3DSpawner::SpawnInto`へ場面グラフと
`CSceneCollision3D`を渡す。生成後の衝突登録に失敗した場合はノードも巻き戻される。
場面途中で外す場合は結果を`DestroyCollidableModel3D`へ渡し、表示ノードと球を同じ呼出しで片付ける。

外部モデル、トゥーン、自己発光、光沢コートなど高度な見た目が必要な物は、
`SpawnCollidableModel3D`と`FModel3DSpawnParams`を使う。
