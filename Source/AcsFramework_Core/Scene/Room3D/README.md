# Room3D

床と四方の壁を個別に組まず、天井のない歩ける3D部屋を1回で置く。

```cpp
// 原点を床上面の中心として、内寸12m x 8m、壁高3mの部屋を置く。
FRoom3DSpawnResult Room = SpawnRoom3D( FVec2{ 12.0f, 8.0f }, 3.0f );
```

壁厚、床厚、見た目を変える場合は値を明示する。

```cpp
FRoom3DSpawnParams Room = FRoom3DSpawnParams::FromInnerSize(
    FVec2{ 12.0f, 8.0f }, 3.2f, FVec3{ 0.0f, -0.2f, 0.0f } );
Room.WallThickness = 0.3f;
Room.FloorThickness = 0.6f;
Room.FloorColor = FVec4{ 0.20f, 0.24f, 0.30f, 1.0f };
Room.WallColor = FVec4{ 0.52f, 0.58f, 0.68f, 1.0f };
Room.CollisionLayer = 0x2u;

FRoom3DSpawnResult Placed = SpawnRoom3D( Room );

// 配置済みの5形状を作り直さず、部屋全体を新しい寸法と見た目へ揃える。
Room.InnerSize = FVec2{ 16.0f, 10.0f };
Room.WallHeight = 3.8f;
Room.WallColor = FVec4{ 0.46f, 0.54f, 0.66f, 1.0f };
TryUpdateRoom3D( Placed, Room );
```

`InnerSize`は壁の内側で使えるX、Z方向の全幅である。床は両側の壁厚ぶん外周へ広げ、壁の中心は
内寸を狭めないよう外側へ壁厚半分ずらす。X方向の壁は内寸Z、Z方向の壁は壁を含む外周Xまで伸ばすため、
四隅に隙間を残さない。

`CRoom3DSpawner`は既存の`CGround3DSpawner`と`CBlock3DSpawner`だけを合成する。5組の途中で
生成または衝突登録に失敗した場合は、それ以前の組を逆順に破棄予定へ戻して形状も外す。
配置後の`TryUpdateRoom3D`は5組の所有関係、共通親、重複、破棄予定状態を全て先に検証し、
同じノードと形状番号のまま床上面、内寸、壁と床の寸法、見た目、衝突レイヤーを同期更新する。
場面途中で外す場合は`DestroyRoom3D`へ成功結果を渡す。全5組が同じ場面で重複なく対になっている
ことを確認してから片付け、成功時だけ呼出側の結果を空にする。

部屋同士を両端が開いた床で繋ぐ場合は[`SpawnCorridor3D`](../Corridor3D/README.md)へ内幅と長さを
渡すと、床と側壁2枚の生成・巻き戻し・一括破棄をまとめられる。

壁の一部を通過できる形へ置き換える場合は[`SpawnDoorway3D`](../Doorway3D/README.md)へ壁と開口の
寸法を渡すと、左右柱と上枠だけを置いて開口内の衝突を空けられる。

部屋内で高さを繋ぐ場合は[`SpawnStairs3D`](../Stairs3D/README.md)へ段数と1段の寸法を渡すと、
この部屋と同じ衝突付き直方体の契約で隙間のない階段を追加できる。

部屋全体を回転・拡縮する場合は同じ親ノードの下へ置ける。箱衝突は既存契約どおりworld軸平行箱へ
変換されるため、斜め回転や非一様拡縮では表示を包む安全側の範囲になる。
