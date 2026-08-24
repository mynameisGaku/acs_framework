# Bridge3D

`SpawnBridge3D`へ入口、幅、長さ、柵高を渡すと、歩ける床板と両側の柵をX/Zの正負4方向へ
一括配置する。

```cpp
FBridge3DSpawnResult Bridge = SpawnBridge3D(
    3.0f, 10.0f, 1.15f,
    FVec3{ 0.0f, 1.0f, -5.0f },
    EBridge3DDirection::PositiveZ );
```

入口と出口の支柱は、床板の端から支柱半幅だけ内側へ収める。両側の支柱間隔は
`MaximumPostSpacing`を超えないよう自動で増え、横桟数は`RailCount`で変更できる。

```cpp
FBridge3DSpawnParams BridgeParams = FBridge3DSpawnParams::FromDimensions(
    4.0f, 14.0f, 1.3f );
BridgeParams.MaximumPostSpacing = 1.8f;
BridgeParams.RailCount = 3u;
BridgeParams.DeckColor = FVec4{ 0.30f, 0.25f, 0.20f, 1.0f };
BridgeParams.RailingColor = FVec4{ 0.12f, 0.15f, 0.18f, 1.0f };
FBridge3DSpawnResult Placed = SpawnBridge3D( BridgeParams );
```

`CBridge3DSpawner`は既存の`CGround3DSpawner`と`CFence3DSpawner`だけを合成する。
生成途中で失敗した場合は正側柵、負側柵、床板の順に巻き戻す。場面途中で外す場合は
`DestroyBridge3D`へ成功結果を渡す。全パーツの場面所有とノード・形状番号の重複を先に確認するため、
別場面や壊れた結果では既存部分を変更しない。
