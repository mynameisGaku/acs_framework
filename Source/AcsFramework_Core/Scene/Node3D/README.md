# Node3D world移動

`TryTranslateNodeWorld3D`は、親ノードが移動・回転・拡縮していても、対象ノードを指定したworld量だけ
移動する。ACSの`ANode::Translate`は親座標系の移動なので、worldのX/Y/Zへそのまま動かしたい場合に
使い分ける。

```cpp
TryTranslateNodeWorld3D( *MovingNode, FVec3{ 0.0f, 0.0f, Speed * DeltaSeconds } );
```

位置以外の状態も同時に確定してから適用したい処理は、
`TryCalculateLocalPositionAfterWorldTranslation3D`でローカル位置だけを先に計算できる。どちらも
ノード、親、時刻を所有しない。破棄予定ノード、有限でない移動量、逆変換できない親Transformでは
失敗し、計算出力またはノード位置を変更しない。
