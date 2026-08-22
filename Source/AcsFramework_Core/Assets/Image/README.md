# Assets/Image — 3Dで使う画像を読む

`CImageLibrary`は`Assets`からの相対名をACSの`CAssetRegistry`へ渡し、CPU側の
`AImageAsset`を返す。デコードや同一パスのキャッシュは作り直さない。

```cpp
CAssetLoaderSubsystem* const Assets = GetSubsystem<CAssetLoaderSubsystem>();
TSharedPtr<AAsset> Marker = Assets != nullptr
    ? Assets->Images().Load( FStringView( "Textures/Marker.png" ) )
    : TSharedPtr<AAsset>();
```

対応形式はACSの標準画像ローダと同じで、`png`、`jpg`、`jpeg`、`bmp`、`tga`、`gif`、
`hdr`、`pic`、`pnm`、`ppm`、`pgm`、`psd`。ゲーム用素材は透過を保てる`png`を薦める。

絶対パスと`..`は`CAssetRoot`が拒む。ファイルが無い、形式が違う、画像としてデコード
できない場合は空を返し、理由をログへ1行残す。
