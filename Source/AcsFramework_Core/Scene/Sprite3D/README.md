# Scene/Sprite3D — 画像を3D空間へ置く

`CSprite3DSpawner`は画像名、位置、大きさを、ACSの`ASprite3DComponent`を持つノードへ
まとめる。画像のデコードは`CImageLibrary`、GPU画像の所有、深度判定、透過合成はACSが担う。

```cpp
CAssetLoaderSubsystem* const Assets = GetSubsystem<CAssetLoaderSubsystem>();
if ( Assets != nullptr )
{
    FSprite3DSpawnParams Marker = FSprite3DSpawnParams::FromImage(
        FStringView( "Textures/Marker.png" ), FVec3{ 0.0f, 2.0f, 3.0f }, FVec2{ 0.8f, 0.8f } );
    CSprite3DSpawner::SpawnInto( Graph(), Marker, Assets->Images() );
}
```

板はノードのローカルXY面に固定される。看板、カード、印、簡単な草木など、worldに向きが
ある画像へ使う。カメラへ自動で向くビルボードではない。向きは`RotationDeg`へ度で指定する。

`SpawnInto(..., CImageLibrary&)`は画像が未読込なら`TexturePath`から同期で読み、失敗時は
ノードを追加せず`nullptr`を返す。読込済みの`ImageAsset`を渡した場合は再読込しない。

画像板は通常のscene depthを読み、透過部分を捨ててHDRへ合成される。ポスト処理の前なので
露出とbloomの対象になる。色変更、UV切出し、カメラ追従は現時点の公開契約に含めない。
