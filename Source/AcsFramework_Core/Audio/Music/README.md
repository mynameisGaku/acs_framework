# Music

状況に合わせて曲を切り替える。

このファイルは**フォルダに何を置いてよいか**を決めるもの。迷ったらここに戻ること。

---

## エンジンから借りているもの

| ACS | 役割 |
|---|---|
| `CMusicDirector` | 状態と強さから曲を選び、指定秒でつなぐ。差し込みの一音も溜める |
| `FMusicTrack` / `EMusicState` | 曲の情報と、状態の種類 |
| `CAudioDirector` | 実際に鳴らす側 (`CAudioSubsystem` が持っている) |

`CMusicDirector::SetAudioDirector` に鳴らす側を差すのが**唯一の接続点**。ここが繋がっていないと
状態だけが変わって音が出ない。

---

## 「決める側」と「鳴らす側」を分けてある

```
  ゲーム側 ──「いまは戦闘」── RequestState / IMusicStateSource
                                        │ 溜まる
                                        ▼
                              CMusicStateArbiter (1 つに決める)
                                        │ フレームに 1 回
                                        ▼
                              CMusicDirector (曲を選ぶ)  ──▶ CAudioDirector (鳴らす)
                                        │ 差し込みの一音
                                        ▼
                              CMusicStingerPump ──▶ CAudioSubsystem::PlaySfx
```

申告する側は**曲を知らない**。状態と強さを言うだけで、どの曲かは `CMusicTrackCatalog` が決める。

---

## フォルダに置いてよいもの

| 置き場所 | 置いてよいもの | 例 |
|---|---|---|
| 直下 | 曲の切り替えに関わる型 | `CMusicSubsystem`、`CMusicTrackCatalog`、`CMusicStateArbiter` |
| 直下 | 値型 | `FMusicStateRequest`、`EMusicPriority` |
| 直下 | 差込口 | `IMusicStateSource` |

**どの曲を使うかはここへ書かない。** ゲーム側が `RegisterTrack` で足す。

---

## 申告の 2 通り

| やり方 | 向く場面 |
|---|---|
| `RequestState()` を毎フレーム呼ぶ | 押しっぱなしの状態 (戦闘中である間ずっと) |
| `IMusicStateSource` を差す | 状態を持っているものがある場合 (聞かれたら今の値を答える) |

どちらも**そのフレームだけ有効**。言い続けなければ「申告なし」に戻る。競ったときは
`EMusicPriority` が大きいほうが勝ち、同じなら後から来たほうを採る。

---

## 使い方

```cpp
Music->RegisterTrack( EMusicState::Calm,   FString( "Assets/Bgm/Field.wav" ) );
Music->RegisterTrack( EMusicState::Combat, FString( "Assets/Bgm/Battle.wav" ) );
Music->RegisterTrack( EMusicState::Combat, FString( "Assets/Bgm/Battle_Hot.wav" ), 0.7f, 1.0f );

// 遊びの側から。強さ 0.7 以上なら上の «Hot» に切り替わる
Music->RequestState( FMusicStateRequest{ EMusicState::Combat, 0.8f, 1.5f, EMusicPriority::Gameplay } );
```

---

## 気をつけること

- **実時間で進める。** ゲームを止めても曲は流れ続ける (止めた «ゲームの中» ではなく
  «プレイヤーの手元» で鳴っているものなので)。
- 同じ状態を毎フレーム渡しても切り替えは始まり直さない。`CMusicSubsystem` が
  「変わったときだけ」エンジンへ伝えている。
- 曲のパスはエンジンが複製しない。`CMusicTrackCatalog` が名前プールへ写している。
- 差し込みの一音は取り出さないと次で上書きされて消える。取り出しは `Update` の中で
  毎フレーム行っている。
