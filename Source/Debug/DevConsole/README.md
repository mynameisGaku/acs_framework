# DevConsole

打ち込んだ 1 行でゲームを操る。開発中の道具。

このファイルは**フォルダに何を置いてよいか**を決めるもの。迷ったらここに戻ること。

---

## エンジンから借りているもの

解釈と実行は ACS が持っている。ここは**再実装しない**。

| ACS | 役割 |
|---|---|
| `CDevConsole` | コマンド登録、行の切り分け、実行、履歴、記録 (100 行で古いものから落ちる) |
| `CommandFn` / `FConsoleArg` | コマンドの関数型と、切り分けた引数 |

**画面も自前で作らない。** 文字の打ち込み・描画・キーの取り回しは DebugTop が持っている
ので、`View/` はその**ページとして**振る舞う。専用のオーバーレイを作ると、フォントの用意から
入力の奪い合いまで二重になる。

---

## 向きを逆にしてある

コマンドの中身をコンソール側に集めると、そこが全機能への参照置き場になる。逆にする。

```
  各機能 ──「自分にはこれができる」──▶ IConsoleCommandProvider
                                            │ ProvideConsoleCommands
                                            ▼
                                   CConsoleCommandRegistrar ──▶ CDevConsole
```

コンソールは**何が登録されているかを知らない**。どの機能を積むかを決めるのはアプリ
(`CAcsFrameworkApp::InitialScene`)。ゲーム側も同じ形で自分のコマンドを足せる。

---

## 層と、依存してよい向き

```
          ┌────────────────────────────────┐
  入口 ─▶ │ CDevConsoleSubsystem            │  所有・寿命・登録の入口
          └───┬─────────────┬──────────┬────┘
              ▼             ▼          ▼
  CConsoleCommandRegistrar  CConsoleLogTail   TArray<IConsoleCommandProvider>
       (名前を写して登録)      (記録を写す)        (申告するものの寿命)

  CConsoleArgumentReader … コマンドの中から使う。誰にも依存しない
```

`View/` と `Builtin/` は上記すべてを知ってよいが、**上記から View / Builtin を参照しない**。

---

## フォルダに置いてよいもの

| 置き場所 | 置いてよいもの | 例 |
|---|---|---|
| 直下 | コンソールそのものに関わる型 | `CDevConsoleSubsystem`、`CConsoleCommandRegistrar` |
| 直下 | 引数と記録の扱い | `CConsoleArgumentReader`、`CConsoleLogTail` |
| 直下 | 差込口 | `IConsoleCommandProvider` |
| `Builtin/` | 同梱の既製コマンド (= 差込口の利用者) | `CConsoleCommandsApp`、`CConsoleCommandsAudio`、`CConsoleCommandsPerf` |
| `View/` | デバッグメニューへの見せ方 | `ADevConsolePage`、`CConsoleLogRow` |

ゲーム固有のコマンドはここへ置かない。ゲーム側で `IConsoleCommandProvider` を実装する。

---

## 関数の分け方

コマンドは 2 段になる。エンジンから呼ばれる **入口** (`static On*`) は自分自身へ渡し直す
だけで、**中身** (`Quit` / `PlayBgm` など) はメンバ関数に置く。入口に処理を書くと、
`void*` からの復元と本題が混ざって読めなくなる。

| 役割 | 例 | 決まり |
|---|---|---|
| 入口 | `OnQuit` / `OnPlayBgm` | `static`。復元して渡すだけ |
| 実処理 | `Quit` / `PlayBgm` / `ReportFps` | 副作用ひとつ。失敗は記録へ書く |
| 収集 | `AddProvider` | 受け取って寿命を持つ |
| 写し取り | `CaptureLogTail` | 1 フレームに 1 度だけ |

---

## 使い方

```cpp
// アプリの起動時に。どの機能を積むかはアプリが決める
Console->AddProvider( MakeUnique<CConsoleCommandsAudio>( *Console, *Audio ) );

// ゲーム側から直接流すこともできる
Console->Execute( FString( "audio.volume 0.5" ) );
```

同梱コマンド: `app.quit` / `app.fps` / `audio.bgm <パス>` / `audio.bgmstop` /
`audio.volume <0..1>` / `perf.frame` / `perf.list`

---

## 気をつけること

- **名前と説明文はエンジンが複製しない。** `CConsoleCommandRegistrar` が名前プールへ
  写してから渡している。`CDevConsole::RegisterCommand` を直接呼ばないこと。
- **登録は取り消せない。** エンジン側に解除の口がない。申告するものの寿命はサブシステムが
  持つので、`AddProvider` へ渡した後に自分で消さないこと。
- 記録は 100 行で古いものから落ちる (エンジン側の固定上限)。
- `CDevConsole` は行数を返さないので、`CConsoleLogTail` が端から数えている。行ごとに
  数え直さないよう、写し取りは 1 フレームに 1 度だけにすること。
