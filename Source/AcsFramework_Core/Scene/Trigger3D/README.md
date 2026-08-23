# 3D近接トリガー

`CProximityTrigger3D`は、1個の場面ノードへ追従する球または箱の範囲について、衝突ノードが今回入った、
中にいる、出た、を世代付き`FNodeId`で返す。扉の自動開閉、チェックポイント、会話開始範囲、
危険区域など、物理反発を起こさないゲーム判定に使う。

```cpp
CProximityTrigger3D DoorTrigger;
BindProximityTrigger3D(
    DoorTrigger, *Door,
    FProximityTrigger3DParams::Box( FVec3{ 1.5f, 2.0f, 3.0f }, PlayerLayer ) );

FProximityTrigger3DUpdateResult Result;
if ( DoorTrigger.Update( Result ) && Result.DidEnter( Player->Id() ) )
{
    OpenDoor();
}
```

利用側がトリガーを所有し、`AUi3DScene::BindProximityTrigger3D`は場面所有のノードグラフと
`CSceneCollision3D`へ接続するだけに留める。対象は既に衝突集合へ登録されている必要がある。
基準ノード自身は結果から外し、対象レイヤーは`CollisionMask`で絞る。
球は`Around( Radius, Mask )`、箱は`Box( HalfSize, Mask )`で作る。`LocalCenter`を変えると
基準ノードからの位置をずらせる。箱の8隅へ現在Transformを適用し、回転後の形を包むworld軸平行箱で
問い合わせるため、ACSの既存箱衝突と同じ結果になる。

最初の成功更新では現在範囲内の全対象が進入になる。同じ状態の次更新では進入・退出は空になり、
移動、無効化、破棄、レイヤー変更で範囲外になれば退出へ移る。`ResetState`を呼ぶと、次回に現在対象を
改めて進入として受け取れる。コールバック、入力、描画、時間進行は持たないため、成立したイベントの用途は
ゲーム側で決める。
