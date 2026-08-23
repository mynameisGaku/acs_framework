# 3D見た目プリセット

`TryApplyVisualPreset3D`は、ACSが持つ遮蔽、画面空間反射、画面空間間接光、bloom、露出、
色調、TAAまたはFXAAを一つの選択へまとめる。描画機能やGPU資源はFrameworkへ複製せず、
`AUi3DScene`が所有するACS設定へ値を渡すだけの窓口である。

```cpp
// 多くの3D場面で最初に選ぶ設定。
TryApplyVisualPreset3D( EVisualPreset3D::Balanced );

// 場面固有の距離だけ、適用後に上書きできる。
GlobalIllumination().MaxDistance = 8.0f;
```

| 値 | 主な違い | 向く場面 |
|---|---|---|
| `Performance` | 反射と間接光を切り、軽い遮蔽とFXAAを使う | GPU負荷を抑えたい場面 |
| `Balanced` | 遮蔽、控えめな反射と間接光、FXAAを使う | 通常のゲームプレイ |
| `Cinematic` | 強い反射と間接光、TAA、映画調の仕上げを使う | 撮影、会話、余裕のあるGPU |

切替は原子的で、未知の列挙値では既存設定を一つも変えず`false`を返す。実行中に切り替えても、
ACSが毎フレーム更新するGPUテクスチャ参照、カメラ行列、経過時間は維持する。TAAが利用できない
場合に備え、全プリセットでFXAAを代替処理として有効にしている。

個別の画作りが必要なら、適用後に`AmbientOcclusion()`、`Reflections()`、
`GlobalIllumination()`、`PostParams()`を直接調整する。プリセットは高度な窓口を隠さない。
