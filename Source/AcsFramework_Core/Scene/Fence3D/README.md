# Fence3D

支柱位置と横桟を手作業で並べず、長さと高さから衝突付きの柵を1回で置く。

```cpp
// 原点の始点支柱からZ正方向へ、長さ6m、高さ1.2m、最大2m間隔の柵を置く。
FFence3DSpawnResult Fence = SpawnFence3D(
    6.0f, 1.2f, 2.0f );
```

方向、支柱や横桟の寸法、本数、見た目を変える場合は値を明示する。

```cpp
FFence3DSpawnParams Fence = FFence3DSpawnParams::FromDimensions(
    5.0f, 1.4f, FVec3{ 4.0f, 0.0f, 1.0f },
    EFence3DDirection::NegativeX );
Fence.MaximumPostSpacing = 1.5f;
Fence.PostThickness = 0.18f;
Fence.RailCount = 3u;
Fence.RailHeight = 0.10f;
Fence.RailThickness = 0.08f;
Fence.Color = FVec4{ 0.34f, 0.22f, 0.12f, 1.0f };
Fence.Roughness = 0.76f;
Fence.CollisionLayer = 0x2u;

FFence3DSpawnResult Placed = SpawnFence3D( Fence );
```

`StartPostBottomCenter`は始点支柱の底面中央で、`Length`は始点と終点の支柱中心間距離である。
必要区間数は`Length / MaximumPostSpacing`を切り上げて決めるため、両端を含む支柱は常に等間隔になり、
指定した最大間隔を超えない。横桟は両端支柱の中心間を繋ぎ、底面と上端の間へ等間隔で置く。

区間数は最大256、横桟数は最大8に制限する。支柱が重なる間隔、横桟同士が重なる高さ、0以下または
有限でない寸法、派生する終点が有限でない値、範囲外の材質値、衝突しないレイヤー0は配置前に拒否する。

`CFence3DSpawner`は既存の`CBlock3DSpawner`だけを合成する。配列確保、生成、衝突登録の途中で
失敗した場合は、それ以前の横桟と支柱を生成の逆順で破棄予定へ戻して形状も外す。
場面途中で外す場合は`DestroyFence3D`へ成功結果を渡す。全要素が同じ場面で重複なく対になっている
ことを確認してから片付け、成功時だけ呼出側の結果を空にする。

柵はXまたはZの軸方向へ置く。斜めの親回転も指定できるが、箱衝突は既存契約どおり表示を包む
world軸平行箱になる。開閉する門や通過口が必要な場合は柵を必要な長さへ分け、間へ
[`SpawnDoorway3D`](../Doorway3D/README.md)や作品固有の操作対象を置く。
