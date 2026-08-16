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
        ↓
IActionInputSource (Player / AI / 台本)
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
| 直下 | 所有と順番 | `CSimulationSubsystem` |
| `Input/` | 装置 → アクションの変換 | `IActionDeviceReader`、`CActionBindingTable`、`CDeviceActionReader`、`CBoundActionSource` |
| `Test/` | ゲーム抜きで回す自己テスト | `SimulationDeterminismTest.cpp` |

**ゲームのルールはここへ置かない。** `ISimulationRule` を実装するのはゲーム側。

---

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

---

## 自己テスト

```powershell
.\Tools\RunSimulationDeterminismTest.ps1
```

Simulation の .cpp と `Test/SimulationDeterminismTest.cpp` だけをコンパイルして走らせる。
見ているのは 4 つ。

1. 記録 → バイト列へ保存 → 読み込み → 再生で、位置・発射回数・乱数を引いた回数・イベント列が完全に一致する
2. 種を変えると結果が変わる（テストが常に通るだけの形になっていないことの確認）
3. 割り当て表が、装置なしで解決できる（偽の装置を差す。両押しは打ち消して 0）
4. 記録 → **ファイル**へ保存 → 読み込み → 再生でも完全に一致する

2026-08-16 時点の結果: 600 ステップ / テープ 337 件・8,112 バイト / 4 項目とも PASS。

---

## 気をつけること

- **ステップの途中でイベントを読まない。** 1 フレーム進め終えてからまとめて読み、読んだら捨てる。
- 再生中は入力元を見ない。テープに無いティックは中立で進む（記録の終端より先まで回した場合）。
- テープは「変わったティックだけ」を残す。600 ステップで 337 件だったのは入力が毎ステップは
  変わらないため。
- `Update()` に渡すのはゲーム時間（倍率を掛けた後）。止めれば盤面も止まる。
- 大量実行は `AdvanceSteps()` を使う。実時間を介さずステップ数だけ進む。
