# 3Dキャラクター移動

`CCharacterMover3D`は、希望する世界X/Z速度とジャンプ要求をACSの球型移動計算へ渡し、
成功した移動だけをシーンノードへ反映する。重力、床への接地、壁沿いの移動、天井、
初期貫通の解消を1回の`Move()`で扱える。

```cpp
CSceneCollision3D Collision{ Graph() };
Collision.TryAddBox( *Floor, FVec3{}, FVec3{ 10.0f, 0.5f, 10.0f }, 0x1u );

CCharacterMover3D HeroMover;
HeroMover.Bind( Collision, *Hero, FVec3{ 0.0f, 0.5f, 0.0f } );
HeroMover.SetCollisionFilter( {}, 0x1u );

HeroMover.Move( FVec2{ MoveX * 4.0f, MoveZ * 4.0f }, bJumpPressed, DeltaSeconds );
HeroAnimator.Update( FCharacterAnimation3DInput{ Length( FVec2{ HeroMover.Velocity().x, HeroMover.Velocity().z } ), HeroMover.IsGrounded() } );
```

`Bind()`は球中心のローカル位置を受け取る。足元をノード原点にする場合は、Yへ球半径を指定する。
ノードが親を持つ場合も、計算結果の世界移動量を親座標へ戻してからローカル位置へ反映する。

キャラクター自身を`CSceneCollision3D`へ登録した場合は、その形状を`SetCollisionFilter()`へ渡す。
無効な形状を渡すと自己除外を行わない。外部から瞬間移動させた後に速度と接地状態も消したい場合は、
`ResetMotion()`で現在位置を読み直す。

固定更新の時刻、入力の押下判定、ノードと衝突集合の寿命、描画とアニメーションは呼出側が所有する。
移動失敗時はノード、速度、接地状態、直前結果を変更しない。
