# ノード追従3Dカメラ

`CNodeOrbitCamera3D`は、人物などのノードを注視点として追いながら、明示した操作量で水平・上下・
距離を変える軌道カメラである。ACSの軌道計算と場面描画形状による遮蔽物回避をそのまま使う。

```cpp
CNodeOrbitCamera3D CharacterCamera;
CharacterCamera.Bind( *this, *Hero );

// xは右回転、yは見下ろす回転。Zoomは正で近づく。
CharacterCamera.Update( FVec2{ LookX, LookY }, Zoom, DeltaSeconds );

// 爆発や被弾ではACSの組み込みプリセットを1回加えるだけで揺れる。
CharacterCamera.TryShakePreset( EShakePreset::ExplosionLarge );
```

既定値はノード原点から1.4m上を、20度見下ろした6m後方から見る。距離、回転速度、注視点の
ローカル位置は`FNodeOrbitCamera3DParams`で変更できる。操作量はACS側で`[-1, 1]`へ制限される。

接続中は、場面のWASD・矢印キーによる既定自由カメラを止め、軌道カメラを明示選択する。
`Unbind()`またはデストラクタで、自由操作、配置済みカメラを含む明示選択、遮蔽物回避設定を
接続前へ戻す。

入力装置と経過時間は内部取得しない。キーボード、ゲームパッド、AI、再生処理のどれでも、同じ
操作量を`Update()`へ渡せる。場面と追従ノードは所有しないため、この型より長く生存させる。

揺れも経過時間を内部取得せず、`Update()`へ渡した秒数だけ進んで減衰する。任意値は
`TryAddShake(FShakeParams)`で指定でき、不正値では現在の揺れを変えない。`StopShake()`は表示位置を
基準の追従位置へ即座に戻す。揺れはeyeと注視点を同量だけ平行移動するため、操作方向は変わらない。
