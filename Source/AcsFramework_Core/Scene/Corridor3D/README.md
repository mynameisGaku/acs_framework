# Corridor3D

床と左右の壁を個別に組まず、入口と出口が開いた歩ける3D通路を1回で置く。

```cpp
// 原点を入口として、Z正方向へ内幅3m、長さ10m、壁高3mの通路を置く。
FCorridor3DSpawnResult Corridor = SpawnCorridor3D( 3.0f, 10.0f, 3.0f );
```

方向や見た目を変える場合は値を明示する。

```cpp
FCorridor3DSpawnParams Corridor = FCorridor3DSpawnParams::FromDimensions(
    4.0f, 12.0f, 3.5f, FVec3{ 6.0f, 0.0f, 2.0f },
    ECorridor3DDirection::NegativeX );
Corridor.FloorColor = FVec4{ 0.20f, 0.24f, 0.30f, 1.0f };
Corridor.WallColor = FVec4{ 0.52f, 0.58f, 0.68f, 1.0f };
Corridor.CollisionLayer = 0x2u;

FCorridor3DSpawnResult Placed = SpawnCorridor3D( Corridor );

FCorridor3DSpawnParams Turned = FCorridor3DSpawnParams::FromDimensions(
    5.0f, 16.0f, 4.0f, FVec3{ 2.0f, 0.0f, 8.0f },
    ECorridor3DDirection::NegativeX );
Turned.FloorColor = FVec4{ 0.18f, 0.28f, 0.24f, 1.0f };
Turned.CollisionLayer = 0x4u;
const bool bUpdated = TryUpdateCorridor3D( Placed, Turned );
```

`EntranceCenter`は入口境界の床上中心で、`Direction`は入口から出口へ伸びる軸方向である。
`InnerWidth`は側壁の内面間、`Length`は入口境界から出口境界までの距離になる。床は両壁の外面まで
広げるため、壁の下に落下できる隙間を残さない。X方向へ伸ばす場合は通路幅をZ、Z方向では幅をXへ取る。

`TryUpdateCorridor3D`は床と側壁2枚の所有、形状対応、重複、共通親、全祖先、表示部品、新指定を
先に確認する。成功時も3ノードと3形状を作り直さないため、保持している番号を無効にせず、入口、
方向、内幅、長さ、壁高、材質、影、名前、衝突レイヤーを揃えて変更できる。別場面の結果、
共通親を失った構成、破棄予定ノード、不正値では何も変更しない。

`CCorridor3DSpawner`は既存の`CGround3DSpawner`と`CBlock3DSpawner`だけを合成する。3組の途中で
失敗した場合は、それ以前の部分を側壁から逆順に破棄予定へ戻して形状も外す。場面途中で外す場合は
`DestroyCorridor3D`へ成功結果を渡す。独自場面で更新する場合は`CCorridor3DSpawner::TryApplyTo`へ
生成時と同じグラフ、衝突集合、生成結果を渡す。全3組が同じ場面で重複なく対になっていることを
確認してから片付け、成功時だけ呼出側の結果を空にする。

通路全体を回転・拡縮する場合は同じ親ノードの下へ置ける。親を斜め回転または非一様拡縮すると、
各箱衝突は既存契約どおり表示を包む安全側のworld軸平行箱になる。

通路端に開口壁枠を置く場合は[`SpawnDoorway3D`](../Doorway3D/README.md)へ壁と開口の寸法を渡すと、
左右柱と上枠だけを追加し、通過部分の表示と衝突を空けられる。
