# AcsFramework Core

「簡単に使える」を、既存ACSの機能を隠してしまうことではなく、所有者・配線・更新順を
ゲーム側が毎回書かなくてよいこととして扱う。

## 使い始める

共通の入口は `AcsFramework_Core/AcsFramework.h` である。ゲーム側は必要な型だけを使い、
実体の取得はシーンまたはアプリから `GetSubsystem<T>()` で行う。

```cpp
#include "AcsFramework_Core/AcsFramework.h"

void AMyScene::OnEnter() noexcept
{
    if ( CGameSettingsSubsystem* const Settings = GetSubsystem<CGameSettingsSubsystem>() )
    {
        Settings->SetFloat( FString( "Audio/Bgm" ), 0.8f );
    }

    if ( CAudioSubsystem* const Audio = GetSubsystem<CAudioSubsystem>() )
    {
        Audio->PlaySfx( FString( "Assets/Se/Decide.wav" ) );
    }
}
```

ゲーム固有のアプリは `CAcsFrameworkApp` を継承し、`InitialScene()` を実装する。共通の
起動・更新・描画・終了処理は基底側へ残す。

## 責務の境界

| 場所 | 持つもの | 代表例 |
|---|---|---|
| ACS engine | 部品の実装と低水準の所有 | `CAssetRegistry`、`CSettings`、`CAudioDirector` |
| `AcsFramework_Core` | ゲームから使う窓口、配線、寿命、更新順 | `CGameSettingsSubsystem`、`CAudioSubsystem` |
| ゲーム側 | ルール、画面固有の状態、製品固有のキー | `Audio/Bgm` の既定値、セーブの中身 |
| `DebugTop` | 開発中だけ使う診断値と操作画面 | `CDebugTopSettings` |

### Core の現在の契約

Core はゲーム側から使う窓口、共有機能の寿命、更新順、Engine への接続を提供する。
Subsystem は共有状態の所有と外部窓口に限定し、値、要求、変換、個別の処理は責務ごとの型が扱う。
利用側は `AcsFramework_Core/AcsFramework.h` を読み込み、GameInstance の Subsystem を取得して呼び出す。

Core はACSの部品を再実装しない。たとえば設定の値と検証付きファイル書き込みは
`CSettings`に任せ、Coreは文字列の寿命、ファイルの場所、遅延保存、終了時保存を引き受ける。
同じ理由で音声は`CAudioDirector`とXAudio2 backendを所有・配線するが、音声のミキシングや
アセット解決を複製しない。

### Text変換の契約

`AcsToWide`の配列版は変換成功時だけ出力を更新し、確保・入力長・変換数の失敗時は既存内容を保つ。
固定容量版はNULを含む容量を受け取り、容量不足や変換失敗時は有効なバッファの先頭をNULにする。
`AcsToUtf8`も変換成功時だけ出力を更新し、nullptr、空文字列、確保、変換数の失敗時は既存内容を保つ。

## GameInstanceサービス一覧

すべて `ESubsystemScope::GameInstance` に登録する。シーンを跨いでも同じ実体を参照する
ものだけをこの範囲へ置く。サブシステムでない型は、必要な場所が所有する値または補助型
として扱う。

| 型 | 役割 | 配線する相手 / 更新 |
|---|---|---|
| `CAppSubsystem` | 終了要求、FPS、実時間、フレーム数 | `CApplication` / なし |
| `CAssetLoaderSubsystem` | 非同期アセットの一括ロードと進捗 | `CAssetRegistry` / `Update()` |
| `CAudioSubsystem` | BGM、効果音、音量、ダッキング | `CApplication` / 実時間 `Update()` |
| `CEventSubsystem` | 型付き通知の購読と発行 | engine event broker / なし |
| `CFadeSubsystem` | 暗転・明転 | `CGame` / `Update()` 相当の呼出し |
| `CLoadingScreenSubsystem` | ロード中の表示 | loader、renderer / `Update()`・`Draw()` |
| `CPauseScreenSubsystem` | ポーズ表示 | time、renderer / `Update()`・`Draw()` |
| `CSaveSubsystem` | スロットの一覧、検証付き読み書き | `TSaveSlot` / 呼出し時 |
| `CSceneTravelSubsystem` | Change、Push、Pop、遷移演出 | `CGame` / `Update()` |
| `CScreenSubsystem` | 解像度、全画面、窓の操作 | `CApplication` / なし |
| `CGameSettingsSubsystem` | プレイヤー設定の保持と永続化 | `CSettings` / `Update()` |
| `CAppStateSubsystem` | シーンを跨ぐ型付き状態 | Core所有 / 呼出し時 |
| `CUiFontSubsystem` | UIフォントの遅延生成と保持 | `CRenderer` / `Acquire()` |
| `CTimeSubsystem` | pause理由、速度、fixed step | `CGame` / `Update()` |
| `CTimerSubsystem` | scaled / unscaled の待機処理 | engine timer / `Update()` |

`FEventSubscription`、`FInputRepeat`、`FSaveSlotInfo`、`FGameTimer`、`ESceneTransition`は共有状態を継続更新する
所有者ではないため、必要な利用側が値として持つ。

### 音声の契約

`CAudioSubsystem`はGameInstanceの寿命で音の取りまとめと音の出力先を所有し、`Bind`でアセットレジストリと出力先を接続する。
出力先の初期化に失敗した場合は`false`を返し、呼出し側を止めずに無音で継続する。音声名と音量状態は保持する。
`Update`は実時間で音声を進め、終了時は音を止めて非所有参照を外し、出力先を終了する。
ゲーム時間が停止しても実時間で進むため、止める場合は`Pause()`を明示して呼び出す。
全体、BGM、効果音の3種類の音量を永続化する場合は、GameSettingsへの接続側が担当する。

## 起動・更新・描画の契約

`CAcsFrameworkApp`の処理順は意図的に固定する。新しい常駐機能を追加するときは、ここへ
置く理由と、停止中も動くべきかを先に決める。

```text
OnStart
  CGame::OnStart
  Fade / SceneTravel / AssetRegistry / Screen / App / Event / AppState を配線
  Save / GameSettings を設定
  Audio を asset registry と player settings へ接続
  Time / AssetLoader を配線

OnUpdate(real dt)
  DebugTop の入力と時間の理由
  Time の反映 → シーン更新（停止中は呼ばない）
  AssetLoader → LoadingScreen → SceneTravel
  Timer（ゲーム時間倍率を受ける）→ Audio（実時間）
  GameSettings（手が止まったら保存）→ PauseScreen（実時間）

OnRender
  UI font → scene → pause → DebugTop → loading screen

OnShutdown
  dirty な GameSettings を保存 → CGame::OnShutdown
```

シーン停止中も進めるものは実時間側へ置く。音声、暗転、ロード表示、ポーズ表示をゲーム
時間へ繋ぐと、止めた瞬間に演出まで止まるためである。

## 設定とデバッグ設定を混ぜない

`CGameSettingsSubsystem`は製品としてプレイヤーが決める値を扱い、既定の保存先は
`Saved/GameSettings.acscfg`である。キー文字列はCoreが安定した領域へ写してから
`CSettings`へ渡す。`CDebugTopSettings`は開発中の診断・調整値なので、同じキーを共有しない。

設定を追加するときは次の順にする。

1. 読めない場合の既定値を決める。
2. 起動時に`Get*`して実行中の部品へ反映する。
3. UIの変更時に`Set*`する。
4. 変更のたびに`Save()`せず、`Update()`または終了処理へ任せる。

## 新しい機能を追加する判断

次の条件を全て満たす場合だけGameInstanceサブシステムを追加する。

- 複数のシーンまたは複数の利用者が同じ状態を共有する。
- 明確なownerと寿命があり、起動・終了または毎フレームの処理を持つ。
- ACSにある部品を所有・配線することで、ゲーム側の定型コードを減らせる。
- 失敗時にゲームを落とさず、既定動作または明確な戻り値へ退避できる。

単純な計算、決定論的な値、1つの画面だけの状態はサブシステムにしない。まず既存の
`F`値型、`CApplication`、`CGame`、`FScene`、`FSceneServices`、engineのregistryや
managerを使い、Coreに必要な責任だけを置く。

## アセット読み込み

`CAssetLoaderSubsystem`はGameInstanceの寿命に合わせてregistryと1件の
`FAssetLoadBatch`を保持し、`Update()`から読み込み結果を通知する窓口である。
同時に複数のownerが同じbatchを使う契約ではなく、現在の1件だけを観測する。
`FAssetLoadBatch`はBeginへ渡したpathの順序と件数を保つ。path本文は最大259 UTF-16 code unitで、終端NUL込み容量は260である。これを超えるpath、開始できないfuture、結果エラーを該当indexの失敗として完了させる。空入力は即完了し、
確保失敗時も入力件数、進捗1、失敗状態、空のasset結果を保つ。

1件のbatchだけが現在の観測対象となる。新しいBeginは旧callbackとfutureの観測を外し、
旧workerはEngine側で継続し得るが新batchへ影響しない。Cancelはcallbackを解除し、
scene終了時には利用側がCancelとLoadingScreenの追従解除を行う。

`FAssetLoadRequest`はLoaderと組み合わせて使う非所有の値で、発行元識別子と世代が完全一致する場合だけ現在要求として扱う。
`BeginRequest`は有効な要求を返し、発行元または世代を確保できない場合は無効値を返して既存の読み込み結果を変えない。
`CancelRequest`は処理中の現在要求だけを解除し、完了済みや古い要求では結果を保持する。`FollowRequest`と`UnfollowRequest`も要求を完全一致で照合し、シーン終了時は追従解除と読み込み取消を行う。
`CLoadingScreenSubsystem`は追従区間中に読み込み窓口を参照するため、解除または自動完了までGameInstanceと読み込み窓口が生存する前提で使う。
既存のBeginは要求を発行せず、GetCurrentRequestは無効値を返す。既存のFollowはLoaderの最新batchを追従し、要求APIは発行元と世代で追従対象を分離する。
