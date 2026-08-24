# Block3D

壁、足場、箱型障害物を立方体表示と箱型衝突へ別々に組まず、1回で置く。

```cpp
// 中心(0, 1, 4)へ、幅4m、高さ2m、奥行き0.5mの壁を置く。
FCollidableModel3DSpawnResult Wall = SpawnBlock3D(
    FVec3{ 4.0f, 2.0f, 0.5f }, FVec3{ 0.0f, 1.0f, 4.0f } );
```

見た目や向きを変える場合は値を明示する。

```cpp
FBlock3DSpawnParams Block = FBlock3DSpawnParams::FromSize(
    FVec3{ 3.0f, 0.4f, 2.0f }, FVec3{ 2.0f, 1.5f, 0.0f } );
Block.RotationDeg = FVec3{ 0.0f, 30.0f, 0.0f };
Block.Color = FVec4{ 0.24f, 0.32f, 0.48f, 1.0f };
Block.Roughness = 0.42f;
Block.CollisionLayer = 0x2u;

const FCollidableModel3DSpawnResult Platform = SpawnBlock3D( Block );
```

`Position`は直方体の中心、`Size`はX、Y、Z方向の全寸法である。表示用の`Cube`とローカル半寸法
0.5の箱は同じノード尺度を使うため、回転前の寸法を別々に書く必要がない。箱衝突はFrameworkの
既存契約どおりworld軸平行箱へ変換される。90度単位の回転では表示範囲と一致し、それ以外の角度では
回転後の表示を包む安全側の範囲になる。精密な斜め形状が必要な場合は実形状判定を使う。

0以下または有限でない寸法、有限でない位置・回転、範囲外の材質値、衝突しないレイヤー0は配置前に
拒否する。

`AUi3DScene`を使わない独自場面では、`CBlock3DSpawner::SpawnInto`へ場面グラフと
`CSceneCollision3D`を渡す。生成後の衝突登録に失敗した場合はノードも巻き戻される。
場面途中で外す場合は結果を`DestroyCollidableModel3D`へ渡し、表示ノードと箱を同じ呼出しで片付ける。

読み込みモデルやトゥーン、自己発光、光沢コートなど高度な見た目が必要な物は、
`SpawnCollidableModel3D`と`FModel3DSpawnParams`を使う。

球型障害物は[`SpawnSphere3D`](../Sphere3D/README.md)へ半径と中心位置を渡すと、表示と球衝突を
同じ半径へ揃えられる。

床と四方の壁を毎回組む用途では[`SpawnRoom3D`](../Room3D/README.md)を使う。内寸と壁高から
この直方体4個と地面1個を配置し、途中失敗と一括破棄まで扱う。

両端が開いた通路では[`SpawnCorridor3D`](../Corridor3D/README.md)を使う。内幅と長さから
この直方体2個と地面1個を4方向へ配置し、途中失敗と一括破棄まで扱う。

開口のある壁では[`SpawnDoorway3D`](../Doorway3D/README.md)を使う。壁と開口の寸法から
この直方体3個を左右柱と上枠へ分け、通過部分に衝突を残さず途中失敗と一括破棄まで扱う。

高さの違う段を毎回組む用途では[`SpawnStairs3D`](../Stairs3D/README.md)を使う。段数と1段の寸法から
隙間のない直方体列を4方向へ配置し、途中失敗と一括破棄まで扱う。
