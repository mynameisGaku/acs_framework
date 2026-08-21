# Spatial

場所のある効果音を、3Dカメラの聴取位置・距離・左右方向へ結び付ける。

このファイルは**フォルダに何を置いてよいか**を決めるもの。迷ったらここに戻ること。

---

## エンジンから借りているもの

| ACS | 役割 |
|---|---|
| `CSpatialAudio` | 音源番号を発行し、聴取位置と音源位置から距離減衰と左右位置を計算する |
| `FAudioListener` | 聴く位置・前・上を表す |
| `CAudioDirector` | voiceを開始し、音量・左右位置・再生速度を出力先へ渡す |

`CAudioSubsystem`はこの3つをゲームから少ない手数で使うためのアダプターであり、計算式や
voiceの所有を複製しない。

---

## 反映されるもの

| 項目 | 状態 |
|---|---|
| 距離減衰 | `MaxDistance`と`AttenuationCurve`に従って反映する |
| 左右位置 | モノラル素材へ反映する。-1が左、+1が右 |
| 再生速度 | `Pitch`を反映する |

XAudio2の左右位置はモノラル素材向け。ステレオ・多チャンネル素材では元のチャンネル行列を
保ち、音量と再生速度だけを更新する。位置のある短い効果音はモノラルで用意する。

---

## 状態・計算・副作用を分ける

```text
ANode / 3D camera -> CSpatialListenerBinder -> FAudioListener
                                                |
FSpatialPlayRequest -> CSpatialAudio -----------+-> ComputeSpatialSfxMix
                         (音源番号と位置)                 |
                                                          v
                  CSpatialSfxRouter -> CAudioSubsystem -> CAudioDirector
                     (再生の判断)       (素材名の解決)      (voice出力)
```

`ComputeSpatialSfxMix`は同じ入力から同じ音量・左右位置・可聴判定を返す純粋計算。素材解決と
実際の再生はアダプター側へ分け、窓や音声出力がなくても境界値を単体テストできる。

`CSpatialSourceRegistry`は従来公開した汎用番号管理として残しているが、このサブシステムの
音源番号には使わない。`CSpatialAudio`が発行した番号だけが距離計算の有効な入力になる。

---

## 2通りの鳴らし方

| やり方 | 向く場面 |
|---|---|
| `PlayOnce` | 着弾・足音など、その瞬間の場所から1回だけ鳴らす |
| `AcquireSource` -> `UpdateSource` -> `PlayFromSource` -> `ReleaseSource` | 同じ物体から銃声などを繰り返し鳴らす |

どちらも、再生を始めた瞬間の距離と左右位置をvoiceへ設定する。すでに鳴っている長い音を
移動へ追従させるAPIではない。`PlayOnce`は音源番号を登録して鳴らし、直後に解除する。

---

## フォルダに置いてよいもの

| 置き場所 | 置いてよいもの | 例 |
|---|---|---|
| 直下 | 場所のある音に関わる型 | `CSpatialAudioSubsystem`、`CSpatialSfxRouter` |
| 直下 | 値と純粋計算 | `FSpatialPlayRequest`、`FSpatialSfxMix` |
| 直下 | 聴取位置の作り方 | `CSpatialListenerBinder` |
| `Test/` | 出力なしで動く決定論的テスト | `SpatialSfxMixTest.cpp` |

---

## 使い方

```cpp
Spatial->SetListenerNode( PlayerNode );

FSpatialPlayRequest Request;
Request.AssetPath = FString( "Audio/SpatialPulse.wav" );
Request.Position = HitPoint;
Request.MaxDistance = 20.0f;
Spatial->PlayOnce( Request );
```

素材名は`Assets`からの相対名。`Assets/Audio/...`と`Audio/...`のどちらも受け付け、実行時の
作業フォルダに依存せず`CAssetRoot`から解決する。

---

## 診断値

- `GetLastVolume()`と`GetLastPan()`で直近の計算結果を確認できる。
- 遠すぎてvoiceを使わなかった回数は`GetSkippedCount()`。
- 素材や出力先の問題で開始できなかった回数は`GetFailedCount()`。
- 音源番号0は無効。解除後の番号は再生へ使わない。
