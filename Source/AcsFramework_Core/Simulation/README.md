# Simulation

ゲームロジックを `State + Input + Time + Parameters → Next State + Events` の形で回す土台。

このファイルは**フォルダに何を置いてよいか**を決めるもの。迷ったらここに戻ること。

---

## 何のためにあるか

ロジックが Actor や現在時刻やシングルトンを直接触っていると、次のことが全部できなくなる。

| やりたいこと | 触っていると困る理由 |
|---|---|
| ロジックだけテストしたい | ゲームを起動しないと動かない |
| バグを再現したい | 同じ操作をしても同じ盤面にならない |
| Player と AI で同じロジックを使いたい | 入力の出どころがロジックに埋まっている |
| バランス調整で 1 万回回したい | 実時間でしか進まない |
| 処理落ちしても挙動を変えたくない | dt がフレームごとに違う |

このモジュールは、その 5 つが**同時に**成り立つ形をひとつ決めて、そこへ乗せるための土台を置く。

---

## エンジンから借りているもの

| ACS | 役割 |
|---|---|
| `acs::timing::CFixedStepClock` | 実時間 → 固定ステップの割り直し、snapshot / restore |
| `acs::game::FRandom` + `FRandomSnapshot` | 乱数と、その内部状態の写し取り |

`acs::game::CSaveArchive` はテープをファイルへ置くのに使う (一時ファイル → 差し替え + CRC)。

**使っていない ACS の部品と、その理由**

| ACS | 使わない理由 |
|---|---|
| `CInputRecorder` | キーとマウスの状態を記録するもので層が違う。キー割り当てを変えると再生が壊れる |
| `FInputMap` | 内部で `acs::CInput` を直接 poll する作りで**差し替えができない**。装置の無い場所で試せず、1 ティックぶんの値としても取り出せない。またシーンのサービス (`ESvc::Input`) なのでシーンを起動しないと触れない |

代わりに `Input/` が同じ役目を、**読む相手を引数で渡す**形で持つ。実機では `CDeviceActionReader`
(`acs::CInput` を呼ぶのはこのクラスだけ)、テストでは偽の装置を差せる。

---

## 流れ

```
装置 (キー / パッド)
        ↓ IActionDeviceReader     ← ここだけが acs::CInput を知る。テストでは偽物を差せる
CActionBindingTable               ← 「この操作はこのアクション」の対応表
        ├→ CActionInputTracker    ← 通常フレームの現在・前回・押下・解放
        └→ IActionInputSource (Player / AI / 台本)
        ↓ FActionInput            ← 装置ではなくアクション。だから Player と AI が同じ口を通る
CActionInputTape (記録 / 再生)     ← 種 + 入力列。これがあればバグを再現できる
        ↓ CReplayFile で保存・読込
CSimulationSubsystem ── CFixedStepDriver (ACS CFixedStepClock)
        ↓ FSimulationContext     └ CDeterministicRandom (ACS FRandom + 種と引いた回数)
   ISimulationRule (ゲーム側が実装)
        ↓ FSimulationEvent
CSimulationEventQueue → 読む側 (Actor / Audio / VFX)
```

**上から下へしか参照しない。** ルールはサブシステムを知らないし、入力元はルールを知らない。

---

## フォルダに置いてよいもの

| 置き場所 | 置いてよいもの | 例 |
|---|---|---|
| 直下 | 値型 | `FActionInput`、`FSimulationEvent`、`FSimulationContext` |
| 直下 | 差込口 | `IActionInputSource`、`ISimulationRule` |
| 直下 | 部品 | `CFixedStepDriver`、`CDeterministicRandom`、`CActionInputTape`、`CSimulationEventQueue`、`CReplayFile` |
| 直下 | 途中から始める | `CSimulationSnapshot`、`CSimulationSnapshotFile` |
| 直下 | 所有と順番 | `CSimulationSubsystem` |
| `Input/` | 装置 → アクションの変換と通常フレームの履歴 | `IActionDeviceReader`、`CActionBindingTable`、`CActionInputTracker`、`FActionKeyRebindState`、`FActionGamepadRebindState`、`CDeviceActionReader`、`CBoundActionSource` |
| `Test/` | ゲーム抜きで回す自己テスト | `SimulationDeterminismTest.cpp` |

**ゲームのルールはここへ置かない。** `ISimulationRule` を実装するのはゲーム側。

---

## 通常の場面で入力を読む

固定ステップを使わない場面では、`CActionInputTracker`を場面のfieldとして持つ。
現在値と前フレーム値を手で入れ替える必要はなく、実機、偽装置、完成済みのアクション入力を
同じ履歴判定へ渡せる。

```cpp
// OnEnterなどで1回
Input.GetBindings().BindKey( kActionJump, EKey::Space );
Input.GetBindings().BindGamepadButton( kActionJump, EGamepadButton::South );

// OnUpdateで1フレームに1回
Input.Update();
if ( Input.WasPressed( kActionJump ) ) Jump();
if ( Input.WasReleased( kActionJump ) ) StopCharging();
const f32 MoveX = Input.GetAxis( kAxisMoveX );
```

AIや入力再生なら`Input.Update( ActionInput )`、装置なしのテストなら
`Input.Update( FakeDeviceReader )`を使う。固定ステップでは1描画フレームに複数ティック進むことが
あるため、このトラッカーではなく`FSimulationContext::WasPressed` / `WasReleased`を使う。

---

## 途中から始める

最初から流し直すのでは足りない場面がある（長い記録の後半だけ見たい、バグの瞬間へ戻りたい、
1 フレーム戻して確かめたい）。続きから同じ道を進むには 3 つが要る。**1 つでも欠けると別の道になる。**

| 写すもの | 欠けるとどうなるか |
|---|---|
| 盤面 (`ISimulationRule::TrySaveState`) | そもそも別の局面から始まる |
| 時計 (`CFixedStepDriver`) | ティックがずれ、テープの読み出し位置が合わない |
| 乱数 (`CDeterministicRandom`) | 同じ入力でも違う出目になる |

`CSimulationSnapshot` がこの 3 つを 1 つに束ねる。テープは含めない（テープは「どう操作したか」、
スナップショットは「どうなっていたか」）。途中から再生するときは 2 つを組み合わせる。

```cpp
CSimulationSnapshot Snapshot;
Simulation->TryCaptureSnapshot( Snapshot );      // ここまでを覚える
...
Simulation->TryRestoreSnapshot( Snapshot );      // ここへ戻る (溜まったイベントは捨てられる)
CSimulationSnapshotFile::Save( Snapshot, FString( "Saved/Replay/bug.acssave" ) );
```

**規則が `TrySaveState` を実装していないと写せない**（既定は false）。実装するときは、
結果に影響する値を漏らさず入れること。時計と乱数は枠組みが別に写すので入れなくてよい。

戻す処理は「3 つとも戻せるか」を先に確かめてから実際に戻す。途中で失敗して一部だけ戻った
状態から進むと、原因の分からないずれ方をするため。

## ルールを書くときの約束

`ISimulationRule::AdvanceStep` の中で守ること。守れば、同じ入力列と同じ種から必ず同じ盤面になる。

- 結果に影響する値は `FSimulationContext` から取る（現在時刻・World・シングルトンを見ない）
- 経過時間は `Context.StepSeconds`（実時間の dt を持ち込まない）
- 乱数は `Context.Random`（自前の乱数を持たない）
- 音や絵は出さず `Context.Raise()` で「起きたこと」を置く

覗きたい値が出てきたら、それは実質的に入力である。`FSimulationContext` へ足すことを検討する。

---

## 使い方

```cpp
Simulation->Configure( 1.0 / 60.0 );
Simulation->SetRule( MakeUnique<CMyRule>() );

TUniquePtr<CBoundActionSource> Source = MakeUnique<CBoundActionSource>();
Source->GetTable().BindAxisKeys( 0u, EKey::A, EKey::D );
Source->GetTable().BindKey( kActionFire, EKey::Space );
Source->GetTable().BindGamepadButton( kActionFire, EGamepadButton::A );
Simulation->SetInputSource( Move( Source ) );

Simulation->StartRecording( 20260816u );

// 毎フレーム
Simulation->Update( DeltaSeconds );

for ( usize i = 0u; i < Simulation->GetEvents().Num(); ++i )
{
    const FSimulationEvent& Event = Simulation->GetEvents().Get( i );
    if ( Event.Id == kEventFired ) Audio->PlaySfx( FString( "Assets/Se/Fire.wav" ) );
}
Simulation->ClearEvents();
```

AI に差し替えるときは `SetInputSource` へ別の実装を渡すだけ。ルールは 1 文字も変えない。

バグの入力列を残すには `GetTape().TrySaveToBuffer()`。再現するには読み込んで `StartReplay()`。

### キーボード割り当てを変える

`FActionKeyRebindState` は「次のキーを待つ・確定する・取り消す」だけを持つ。UI、実機入力、
設定保存を知らないので、押下開始の列を直接渡してゲームを起動せず試せる。

```cpp
FActionKeyRebindState Rebind;
Rebind.SetCurrentKey( EKey::F );
Bindings.ReplaceKeyBinding( kActionFire, Rebind.CurrentKey() );

Rebind.BeginCapture( EKey::Escape );
const auto Result = Rebind.HandlePressedKey( Event.key.key );
if ( Result == FActionKeyRebindState::EResult::Applied )
{
    Bindings.ReplaceKeyBinding( kActionFire, Rebind.CurrentKey() );
    Settings->SetInt( FString( "Input.FireKey" ), static_cast<i32>( Rebind.CurrentKey() ) );
}
```

設定から戻す値は `FActionKeyRebindState::IsValidKey` で検証してから使う。`CActionBindingTable` の
`ReplaceKeyBinding` は同じアクションのキーボード割り当てだけを1つへまとめ、ゲームパッドの
割り当てを維持する。Demo3Dではこの形でFXAA操作を変更し、自動保存される設定へ値を残している。

### ゲームパッド割り当てを変える

`FActionGamepadRebindState`は実機を読まず、明示されたボタンまたは軸だけで状態を進める。
`CDeviceActionReader`が押下開始のボタンと、指定しきい値以上で最も大きく動いた軸を読む薄い
境界になる。割り当て確定に使った操作をゲームへ重ねて渡さない処理と、設定保存は呼び出し側が担う。

```cpp
FActionGamepadRebindState Rebind;
Rebind.SetCurrentButton( EGamepadButton::South );
Rebind.BeginButtonCapture();

EGamepadButton Pressed = EGamepadButton::_Count;
if ( Device.TryReadPressedGamepadButton( 0u, Pressed )
    && Rebind.HandlePressedButton( Pressed ) == FActionGamepadRebindState::EResult::Applied )
{
    Bindings.ReplaceGamepadButtonBinding( kActionJump, Rebind.CurrentButton(), 0u );
}
```

軸は`BeginAxisCapture`と`HandleActiveAxis`を使う。`ReplaceGamepadButtonBinding`と
`ReplaceGamepadAxisBinding`は同じアクションまたは軸番号、同じプレイヤーの重複だけを1つへ
まとめ、キーボードと別プレイヤーを維持する。Demo3Dではジャンプボタンと前後移動軸をUIから
選び、確定値を自動保存する。

---

## 自己テスト

```powershell
.\Tools\RunSimulationDeterminismTest.ps1
```

Simulation の .cpp と `Test/SimulationDeterminismTest.cpp` だけをコンパイルして走らせる。
見ているのは 7 つ。

1. 記録 → バイト列へ保存 → 読み込み → 再生で、位置・発射回数・乱数を引いた回数・イベント列が完全に一致する
2. 種を変えると結果が変わる（テストが常に通るだけの形になっていないことの確認）
3. 割り当て表が、装置なしで解決できる（偽の装置を差す。両押しは打ち消して 0）
4. 記録 → **ファイル**へ保存 → 読み込み → 再生でも完全に一致する
5. 途中で写して戻すと、そこから先が 1 回目とまったく同じ道を進む
6. 通しで回した結果と、途中から回した結果の盤面が一致する
7. 写したものを**ファイル**へ置いて戻しても、同じ続きになる

2026-08-16 時点の結果: 600 ステップ / テープ 337 件・8,112 バイト /
スナップショット tick 300・盤面 8 バイト / 7 項目とも PASS。

---

## 気をつけること

- **ステップの途中でイベントを読まない。** 1 フレーム進め終えてからまとめて読み、読んだら捨てる。
- 再生中は入力元を見ない。テープに無いティックは中立で進む（記録の終端より先まで回した場合）。
- テープは「変わったティックだけ」を残す。600 ステップで 337 件だったのは入力が毎ステップは
  変わらないため。
- `Update()` に渡すのはゲーム時間（倍率を掛けた後）。止めれば盤面も止まる。
- 大量実行は `AdvanceSteps()` を使う。実時間を介さずステップ数だけ進む。
