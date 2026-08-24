# Doorway3D

開口のある壁を左右柱と上枠へ手作業で分けず、壁と開口の寸法から1回で置く。

```cpp
// 原点を壁下辺の中心として、幅4m、高さ3mの壁へ幅1.2m、高さ2.2mの開口を作る。
FDoorway3DSpawnResult Doorway = SpawnDoorway3D(
    4.0f, 3.0f, 1.2f, 2.2f );
```

向き、厚み、開口の横位置、見た目を変える場合は値を明示する。

```cpp
FDoorway3DSpawnParams Doorway = FDoorway3DSpawnParams::FromOpening(
    5.0f, 3.5f, 1.5f, 2.4f, FVec3{ 4.0f, 0.0f, 1.0f },
    EDoorway3DOrientation::AlongZ );
Doorway.WallThickness = 0.3f;
Doorway.OpeningCenterOffset = 0.6f;
Doorway.Color = FVec4{ 0.52f, 0.58f, 0.68f, 1.0f };
Doorway.CollisionLayer = 0x2u;

FDoorway3DSpawnResult Placed = SpawnDoorway3D( Doorway );
```

`BottomCenter`は壁全体の下辺中央で、`Orientation`は壁幅を伸ばす軸である。開口は床から始まり、
`OpeningCenterOffset`が0なら中央に置く。左右柱は壁上端まで伸ばし、上枠は開口幅の真上だけを埋めるため、
開口部分には見えない箱型衝突を残さない。

`CDoorway3DSpawner`は既存の`CBlock3DSpawner`だけを合成する。3組の途中で失敗した場合は、
それ以前の部分を上枠側から逆順に破棄予定へ戻して形状も外す。場面途中で外す場合は
`DestroyDoorway3D`へ成功結果を渡す。全3組が同じ場面で重複なく対になっていることを確認してから
片付け、成功時だけ呼出側の結果を空にする。

この機能は開口壁枠だけを作り、扉板、開閉、鍵、操作対象は固定しない。作品に必要なら開口の親ノードへ
任意のモデルや`SpawnInteractableCollidableModel3D`を追加する。

開口の左右へ支柱と横桟を続ける場合は[`SpawnFence3D`](../Fence3D/README.md)を必要な長さへ分けて置く。
