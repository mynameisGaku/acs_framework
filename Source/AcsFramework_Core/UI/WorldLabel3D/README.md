# 3Dワールドラベル

敵名、会話対象、目的地などを3D位置へ追従させ、ポスト処理後の読みやすい文字として重ねる。
`AUi3DScene`、`AEffect3DScene`、`AWeather3DScene`の派生場面なら追加の初期化は要らない。

```cpp
FWorldLabel3DParams Label;
Label.Text = FStringView( "PLAYER" );
Label.WorldOffset = FVec3{ 0.0f, 2.15f, 0.0f };
WorldLabels().AddNodeLabel( *PlayerNode, Label );
```

`CWorldLabel3DLayer`はノードを所有しない。場面の`FNodeId`を描画ごとに解決し、scene読込で
グラフのrootが差し替わった場合は古い登録を全て消すため、別ノードへ誤追従しない。ノード自身または祖先が
無効、非表示、破棄予定ならラベルも隠れる。カメラ後方、画面外、`MaximumDistance`より遠い位置も
描画しない。

固定位置は`AddWorldLabel`、文字変更は`SetText`、一時的な表示切替は`SetVisible`を使う。
`TryProjectLabel`は同じ安全確認を通したpixel位置を返すため、作品固有のアイコンやゲージを描く
アダプターにも利用できる。

ラベルはdepth textureで遮蔽しない。壁越し表示を避ける必要がある作品は、`CScenePicker`でカメラから
対象まで線を飛ばし、その結果を`SetVisible`へ渡す。
