# 3Dシーン衝突

`CSceneCollision3D`は、ACSの`CCollisionWorld3D`へシーンノードを結び、ゲーム側で必要になる
「この球と重なる物」「この大きさで進んだとき最初に触れる物」をノードとして返す薄い窓口である。
`TryMoveCharacter()`は同期済み形状から球型キャラクターの次状態を求め、ノード反映は行わない。

```cpp
CSceneCollision3D Collision{ Graph() };
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

登録時の中心、半サイズ、半径はノードのローカル座標で指定する。問い合わせ前に位置、回転、拡縮を
自動同期するため、動くノードを毎フレーム登録し直す必要はない。ノードまたは祖先を無効にすると
判定対象から外れ、再び有効にすると元のレイヤーへ戻る。非表示は描画だけの状態なので衝突を残す。
シーン読み込みなどでグラフのルート実体が交換された場合は、別ノードへの誤接続を防ぐため全登録を
自動で外す。読み替え後のノードは改めて登録する。

1ノード1形状に限定する。複合形状が要る場合は子ノードへ形状を分けると、各部位を戻り値のノードで
区別できる。剛体、状態の所有、固定更新はこの型の責務に含めない。ノードへ移動を反映して速度と
接地状態を保持する場合は`CCharacterMover3D`を使う。
