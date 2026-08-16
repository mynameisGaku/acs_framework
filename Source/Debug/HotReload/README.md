# HotReload

走らせたままファイルを差し替え、その場で反映する。開発中の道具。

このファイルは**フォルダに何を置いてよいか**を決めるもの。迷ったらここに戻ること。

---

## エンジンから借りているもの

見張る仕組みは ACS が持っている。ここは**再実装しない**。

| ACS | 役割 |
|---|---|
| `CHotReloadWatcher` | フォルダ / ファイルの監視、まとめ (debounce)、変更の溜め込み |
| `FHotReloadEvent` | 変わったパス・時刻・消えたか |
| `EHotReloadResult` | 監視を足せなかった理由 |

エンジンには上限がある (パス 256、フォルダ 64、溜められる変更 1024)。

---

## 3 つに分けてある

「差し替えたのに反映されない」とき、どこで止まったかを切り分けられるようにする。

```
   見る場所を決める          拾う                      配る
  ┌──────────────────┐   ┌──────────────────┐   ┌────────────────────┐
  │CHotReloadWatchPlan│─▶ │CHotReloadWatcher  │─▶ │CHotReloadDispatcher │─▶ IHotReloadHandler
  │  (どこを)         │   │  (ACS。溜める)     │   │  (誰へ)             │      (何を作り直すか)
  └──────────────────┘   └──────────────────┘   └────────────────────┘
            ▲                       ▲                       ▲
            └───────────────────────┴───────────────────────┘
                        CHotReloadSubsystem (所有・寿命・毎フレームの順番)
```

`View/AHotReloadPage` はこの 3 段をそれぞれ数字で出す。見張り 0 なら場所が悪い、
拾えているのに配れていないなら引き受け手が居ない、と切り分けられる。

---

## フォルダに置いてよいもの

| 置き場所 | 置いてよいもの | 例 |
|---|---|---|
| 直下 | 見張りに関わる型 | `CHotReloadSubsystem`、`CHotReloadWatchPlan`、`CHotReloadDispatcher` |
| 直下 | 値型 | `FHotReloadWatchEntry` |
| 直下 | 差込口 | `IHotReloadHandler` |
| `Builtin/` | 同梱の引き受け手 | `CHotReloadLogHandler` |
| `View/` | デバッグメニューへの見せ方 | `AHotReloadPage` |

**何を作り直すかはここへ書かない。** アセットを持っている側が `IHotReloadHandler` を
実装する (テクスチャなら描画側、設定なら設定側)。

---

## 関数の分け方

| 役割 | 例 | 決まり |
|---|---|---|
| 流れ | `StartWatching` / `Update` / `OnDeinitialize` | 呼ぶ順番だけ |
| 収集 | `AddDirectory` / `AddFile` / `AddHandler` | 溜めるだけ |
| 実処理 | `ApplyTo` / `DispatchPending` / `DeliverOne` | 副作用ひとつ |
| 判定 | `CanHandle` / `IsWatching` / `IsValid` | `const noexcept` |

---

## 使い方

```cpp
// アプリの起動時に (既定では Assets の下を見る)
HotReload->StartWatchingDefaults();
HotReload->AddHandler( MakeUnique<CHotReloadLogHandler>( FString( ".png" ) ) );

// アプリの更新から。実時間で渡すこと
HotReload->Update( DeltaSeconds );
```

---

## 気をつけること

- **実時間で進める。** 止めて眺めながら絵を差し替えるのが普通の使い方なので、ゲーム時間
  (倍率つき) で進めるとポーズ中に反映されなくなる。
- 1 フレームで配るのは 32 件まで。まとめて大量に差し替えても、そのフレームが伸び切らない
  ようにしている (残りは次のフレームへ回る)。
- 引き受け手の寿命はサブシステムが持つ。`AddHandler` へ渡した後に自分で消さないこと。
- 見張り始めるのは 1 度だけ。二度目の `StartWatching` は何もしない (見張りを張り替えたい
  ときは、いったんアプリを畳むこと)。
