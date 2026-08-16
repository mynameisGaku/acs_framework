# Spatial

場所のある音を扱う。

このファイルは**フォルダに何を置いてよいか**を決めるもの。迷ったらここに戻ること。

---

## エンジンから借りているもの

| ACS | 役割 |
|---|---|
| `CSpatialAudio` | 聴く位置と音源の位置から、距離による小ささと左右の向きを計算する |
| `FAudioListener` | 聴く位置・前・上 |
| `CAudioDirector` | 実際に鳴らす側 (`CAudioSubsystem` が持っている) |

---

## いまできること・できないこと

| | 状態 |
|---|---|
| 距離で小さくなる | **効く** (`ComputeAttenuatedVolume` を音量へ掛けている) |
| 左右から聞こえる | **効かない**。計算はできるが渡す口がない |

この世代の `CAudioDirector::PlaySfx` が受け取るのは音量だけで、pan を渡せない。
`GetLastPan()` で計算結果だけは見られるようにしてある (無音のまま「効いているつもり」に
ならないため)。エンジン側へ口を足す提案は `C:\dev\acs_temp_doc\0001-audio-pan-for-spatial.md`。

---

## 4 つに分けてある

```
  ANode ──▶ CSpatialListenerBinder ──▶ FAudioListener ──▶ CSpatialAudio
             (聴く位置を作る)                                  ▲
                                                              │ 位置
  CSpatialSourceRegistry (番号を配る) ──────────────────────────┘
                                                              │ 音量
  FSpatialPlayRequest ──▶ CSpatialSfxRouter ────────────────────┴──▶ CAudioSubsystem
                          (小さくして流す)
```

---

## 鳴らしっぱなしと、一度きり

| やり方 | 向く場面 |
|---|---|
| `AcquireSource` → 毎フレーム `UpdateSource` → `ReleaseSource` | 火・滝など、そこに在り続けるもの |
| `PlayOnce` | 着弾・足音など、その瞬間だけのもの |

`PlayOnce` は番号を借りて鳴らし、すぐ返す。音量はその瞬間の距離で決まるので、鳴り終わりを
待つ必要がない。待って番号を抱えると「鳴っていないのに数だけ増える」ことになる。

---

## フォルダに置いてよいもの

| 置き場所 | 置いてよいもの | 例 |
|---|---|---|
| 直下 | 場所のある音に関わる型 | `CSpatialAudioSubsystem`、`CSpatialSourceRegistry` |
| 直下 | 値型 | `FSpatialPlayRequest` |
| 直下 | 聴く位置の作り方 | `CSpatialListenerBinder` |
| 直下 | 鳴らす側への受け渡し | `CSpatialSfxRouter` |

---

## 使い方

```cpp
Spatial->SetListenerNode( PlayerNode );   // 聴く位置がこのノードを追いかける

FSpatialPlayRequest Request;
Request.AssetPath = FString( "Assets/Se/Hit.wav" );
Request.Position = HitPoint;
Spatial->PlayOnce( Request );
```

---

## 気をつけること

- **実時間で進める。** 音は止めない側なので、他の音と同じ扱いにする。
- 聴く位置は「シーンが動いた後」の位置を使いたいので、曲より後に更新している。
- 番号 0 は「無効」。`AcquireSource` が 0 を返したら借りられていない。
- 遠すぎて聞こえない音は鳴らさない (声を無駄に使わないため)。数は `GetSkippedCount()`。
