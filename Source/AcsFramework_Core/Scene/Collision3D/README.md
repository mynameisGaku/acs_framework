# 3Dシーン衝突

`CSceneCollision3D`は、ACSの`CCollisionWorld3D`へシーンノードを結び、ゲーム側で必要になる
「この球と重なる物」「この大きさで進んだとき最初に触れる物」をノードとして返す薄い窓口である。
`TryMoveCharacter()`は同期済み形状から球型キャラクターの次状態を求め、ノード反映は行わない。

```cpp
CSceneCollision3D& Collision = Collision3D();
const FCollisionShapeId3D PlayerShape = Collision.TryAddSphere(
    *Player, FVec3{ 0.0f, 0.9f, 0.0f }, 0.45f, 0x1u );
Collision.TryAddBounds( *Wall, 0x2u );

TArray<ANode*> Nearby;
Collision.TryOverlapSphere(
    FSphere{ Player->World().position, 2.0f }, Nearby, PlayerShape, 0x2u );

FSceneSweepHit3D Hit;
Collision.TrySweepSphere(
    FSceneRay::FromDirection( Player->World().position, MoveDirection, MoveDistance ),
    0.45f, Hit, PlayerShape, 0x2u );
```

`AUi3DScene`の`Collision3D()`は場面グラフへ接続済みで、場面終了時に全登録を自動で外す。
`AUi3DScene`を使わない独自の所有者では、従来どおり`CSceneCollision3D{ Graph }`を直接所有できる。
ローカル形状を問い合わせへ使う場合は、公開アダプターの`TryMakeWorldBox`または
`TryMakeWorldSphere`で現在Transformを反映できる。失敗時は呼出側の出力を変更しない。
登録後の形状は`TryGetWorldShape`で同じ変換結果を読み取れる。戻り値の`Kind`が球と箱を区別し、
`Layer`と`bQueryable`が登録レイヤーと現在の問い合わせ対象状態を分けて示す。ノードを無効にしても
形状調整用のworld値は取得できるが、実際の判定対象ではないことを`bQueryable == false`で確認できる。
進入、滞在、退出を前回との差として受け取る用途は、同じ集合を使う
[`CProximityTrigger3D`](../Trigger3D/README.md)へ任せる。

`FCollisionShape3DParams`は描画境界、明示箱、明示球とレイヤーを1個の値へまとめる。
`Collision.TryAdd( Node, Params )`で個別登録でき、`AUi3DScene::SpawnCollidableModel3D`へ渡すと
モデル生成と同時に登録する。後者は登録失敗時に生成ノードも破棄予定へ戻し、成功時はノードと
形状番号を`FCollidableModel3DSpawnResult`で返す。
さらに視線操作も必要な単一モデルは`SpawnInteractableCollidableModel3D`で3処理を一括化できる。
実行中の破棄は、その結果を`DestroyInteractableCollidableModel3D`へ渡すと形状も直ちに外れる。

```cpp
const FCollidableModel3DSpawnResult Wall = SpawnCollidableModel3D(
    WallModel, FCollisionShape3DParams::FromBounds( 0x2u ) );

const FCollidableModel3DSpawnResult Floor = SpawnCollidableModel3D(
    FloorPlane, FCollisionShape3DParams::FromBox(
        FVec3{ 0.0f, -0.5f, 0.0f }, FVec3{ 0.5f, 0.5f, 0.5f }, 0x2u ) );
```

登録時の中心、半サイズ、半径はノードのローカル座標で指定する。問い合わせ前に位置、回転、拡縮を
自動同期するため、動くノードを毎フレーム登録し直す必要はない。ノードまたは祖先を無効にすると
判定対象から外れ、再び有効にすると元のレイヤーへ戻る。非表示は描画だけの状態なので衝突を残す。
シーン読み込みなどでグラフのルート実体が交換された場合は、別ノードへの誤接続を防ぐため全登録を
自動で外す。読み替え後のノードは改めて登録する。

1ノード1形状に限定する。複合形状が要る場合は子ノードへ形状を分けると、各部位を戻り値のノードで
区別できる。剛体、状態の所有、固定更新はこの型の責務に含めない。ノードへ移動を反映して速度と
接地状態を保持する場合は`CCharacterMover3D`を使う。
