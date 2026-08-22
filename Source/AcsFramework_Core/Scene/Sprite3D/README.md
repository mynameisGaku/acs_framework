# Scene/Sprite3D — 画像を3D空間へ置く

`CSprite3DSpawner`は画像名、位置、大きさを、ACSの`ASprite3DComponent`を持つノードへ
まとめる。画像のデコードは`CImageLibrary`、GPU画像の所有、深度判定、透過合成はACSが担う。

```cpp
FSprite3DSpawnParams Marker = FSprite3DSpawnParams::FromImage(
    FStringView( "Textures/Marker.png" ), FVec3{ 0.0f, 2.0f, 3.0f }, FVec2{ 0.8f, 0.8f } );
SpawnImage3D( Marker );
```

`SpawnImage3D`は`AUi3DScene`の派生場面で使える。画像読込と識別子付きノード生成をまとめるだけで、
向き固定、深度判定、HDR合成は従来と同じACS経路を通る。`AUi3DScene`を使わないノード木では、
`CSprite3DSpawner::SpawnInto`へグラフと`CImageLibrary`を渡す。

板はノードのローカルXY面に固定される。看板、カード、印、簡単な草木など、worldに向きが
ある画像へ使い、向きは`RotationDeg`へ度で指定する。カメラへ自動で向ける場合は
`AUi3DScene::SpawnBillboard3D`または`Billboards().Track(...)`を使う。

`SpawnInto(..., CImageLibrary&)`は画像が未読込なら`TexturePath`から同期で読み、失敗時は
ノードを追加せず`nullptr`を返す。読込済みの`ImageAsset`を渡した場合は再読込しない。

画像板は通常のscene depthを読み、透過部分を捨ててHDRへ合成される。ポスト処理の前なので
露出とbloomの対象になる。色変更とUV切出しは現時点の公開契約に含めない。
