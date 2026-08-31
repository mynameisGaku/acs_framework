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
		├→ FActionAxisResponse         ← 1軸または2軸の遊びを除き応答曲線を適用
		├→ FActionChord                ← 必要操作と禁止操作から同時押しを判定
		├→ FActionCommandSequenceTracker ← 異なる操作の順番と間隔からコマンド完了を判定
		├→ FActionInputMask            ← 1つのゲーム状態で許可した入力だけを履歴ごと残す
		├→ FActionInputMaskStack       ← 入れ子の入力制限を後から重ねた順に戻す
		├→ FActionInputBuffer          ← 通常履歴または固定ステップ入力から押下を短時間保持
		├→ FActionHoldTracker          ← 通常履歴または固定ステップ入力から短押しと長押しを判定
		├→ FActionTapSequenceTracker   ← 押下間隔からダブルタップや複数回タップを判定
		└→ FGameplayCooldown           ← 使用成功から再使用可能までを明示時間で進める
        ↓ FSimulationEvent
CSimulationEventQueue → 読む側 (Actor / Audio / VFX)
```

**上から下へしか参照しない。** ルールはサブシステムを知らないし、入力元はルールを知らない。

---

## フォルダに置いてよいもの

| 置き場所 | 置いてよいもの | 例 |
|---|---|---|
| 直下 | 値型 | `FActionInput`、`FSimulationEvent`、`FSimulationContext`、`FGameplayCooldown` |
| 直下 | 差込口 | `IActionInputSource`、`ISimulationRule` |
| 直下 | 部品 | `CFixedStepDriver`、`CDeterministicRandom`、`CActionInputTape`、`CSimulationEventQueue`、`CReplayFile` |
| 直下 | 途中から始める | `CSimulationSnapshot`、`CSimulationSnapshotFile` |
| 直下 | 所有と順番 | `CSimulationSubsystem` |
| `Input/` | 装置 → アクションの変換、通常履歴、通常・固定の両方で使う局所入力判定 | `IActionDeviceReader`、`CActionBindingTable`、`CActionInputTracker`、`FActionInputMask`、`FActionInputMaskStack`、`FActionChord`、`FActionCommandSequenceTracker`、`FActionAxisResponse`、`FActionInputBuffer`、`FActionHoldTracker`、`FActionTapSequenceTracker`、`FActionKeyRebindState`、`FActionGamepadRebindState`、`CDeviceActionReader`、`CBoundActionSource` |
| `Test/` | ゲーム抜きで回す自己テスト | `SimulationDeterminismTest.cpp` |

**ゲームのルールはここへ置かない。** `ISimulationRule` を実装するのはゲーム側。

---

## 重み付き抽選を再現する

出現候補、演出候補、AIの選択肢などから比率で1つ選ぶ場合は、ゲーム規則の中で
`Context.Random->TryChooseWeightedIndex()`を使う。抽選対象そのものや結果の適用はゲーム側が持ち、
この関数は有限かつ0以上の重みから添字を1つ返すだけに留める。

```cpp
constexpr f32 Weights[] = { 1.0f, 3.0f, 6.0f };
usize ChosenIndex = 0u;
if ( Context.Random == nullptr
	|| !Context.Random->TryChooseWeightedIndex( Weights, 3u, ChosenIndex ) ) return;
```

成功時だけ32bit乱数を2個進め、53bit相当の離散点で選ぶため、同じ種と同じ重み列なら同じ添字列になる。
0の項目は選ばず、
空、負、非有限、全0の重みは、出力と乱数位置を変えずfalseにする。抽選結果へ影響する重みは
盤面または明示した設定として管理し、現在時刻やシングルトンから隠れて取得しないこと。

離散的な乱数なので、総重みに対する比率が`2^-53`（約`1.11e-16`）未満の項目には抽選点が
割り当たらない場合がある。そのような極端な比率は、重みを正規化または調整してから渡す。

---

## 配列を同じ順番でシャッフルする

出現順、問題順、巡回順などを毎回変えつつ記録再生できるようにする場合は、利用側が所有する配列を
`Context.Random->TryShuffle()`へ渡す。項目の意味や利用方法はゲーム側に残り、乱数は並びだけを変える。
候補から均等に1件だけ選ぶ場合は`TryChooseIndex()`へ件数を渡し、返った添字で利用側の配列を読む。

```cpp
u32 SpawnOrder[] = { 0u, 1u, 2u, 3u };
if ( Context.Random == nullptr
	|| !Context.Random->TryShuffle( SpawnOrder, 4u ) ) return;
```

末尾から未確定範囲を1項目ずつ選ぶFisher-Yates法を使い、32bitの出目を棄却法で範囲へ移すため、
配列位置に剰余由来の偏りを作らない。同じ種、同じ乱数位置、同じ項目数なら同じ順番になる。
成功時の乱数消費は通常1交換位置につき1個で、境界を公平にするため棄却した場合だけ増える。
実際の消費数は`GetDrawCount()`へ全て反映される。空配列はnullでも成功し、1件以下では乱数を進めない。
1件以上のnullまたは`u32`で表せない件数は、配列と乱数位置を変えずfalseにする。
null以外のポインターが指定件数ぶん有効であることは、通常の配列APIと同じく呼出側が保証する。

---

## 3D球へ均等に散らす

エフェクト、出現位置、探索方向などを球状に散らす場合は、原点相対の位置を
`TryPointOnSphere3D()`または`TryPointInSphere3D()`で作り、利用側の中心位置へ加える。

```cpp
FVec3 Offset{};
if ( Context.Random == nullptr
	|| !Context.Random->TryPointInSphere3D( 5.0f, Offset ) ) return;
const FVec3 SpawnPosition = EffectCenter + Offset;
```

球面はY成分を面積に対して一様に選び、Y軸まわりの角度と組み合わせるため、極付近へ点が
集中しない。球内部はさらに単位乱数の立方根を半径へ掛け、中心寄りではなく体積に対して一様にする。
球面は成功ごとに32bit乱数を2個、球内部は3個進める。同じ種と乱数位置なら同じ点になり、
snapshotへ戻した後も再生できる。半径0は原点を乱数なしで返し、負または非有限の半径は
出力と乱数位置を変えずfalseにする。

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

### ゲーム状態ごとに入力を絞る

メニュー、会話、演出、操作不能中に一部の操作だけを止める場合は`FActionInputMask`を局所設定として
持つ。既定は全許可、`None()`は全禁止なので、必要な操作だけを短く列挙できる。複数の制限は
`Intersect()`で重ねると、両方が許可した操作だけが残る。

```cpp
FActionInputMask DialogueInput = FActionInputMask::None();
DialogueInput.SetActionEnabled( kActionAdvanceText, true );
DialogueInput.SetActionEnabled( kActionPause, true );

FActionInput CurrentInput;
FActionInput PreviousInput;
DialogueInput.ApplyHistory( Input, CurrentInput, PreviousInput );
if ( CurrentInput.IsDown( kActionAdvanceText )
	&& !PreviousInput.IsDown( kActionAdvanceText ) ) AdvanceText();
```

現在と前回を同じマスクで変換するため、禁止中から再許可した時点でボタンを押しっぱなしでも
新しい押下を合成しない。固定ステップでは`ApplyHistory( Context.Input,
Context.PreviousInput, CurrentInput, PreviousInput )`を使い、変換後の2値を入力バッファ、長押し、
複数回タップなどへそのまま渡す。許可bitを保存する場合は`GetActionMask()` / `GetAxisMask()`で取得し、
読み込み時に`TrySetMasks()`へまとめて渡すと、未使用の軸bitを拒否して設定を原子的に保てる。

ポーズ中に確認画面を開くように制限の開始と終了が入れ子になる場合は、`FActionInputMaskStack`へ
各状態のmaskを`Push()`する。全層が許可した操作だけが残り、確認画面、ポーズの順に`Pop()`すると
直前の制限へ戻る。層が空なら全許可になり、誤った9層目や空の`Pop()`では現在状態を変えない。

```cpp
InputMasks.Push( PauseInput );
InputMasks.Push( ConfirmDialogInput );
InputMasks.ApplyHistory( Input, CurrentInput, PreviousInput );

InputMasks.Pop(); // 確認画面を閉じ、ポーズ用の制限へ戻す
```

開始と終了が入れ子にならない独立した制限には、従来どおり`Intersect()`で明示的に合成する。
`CaptureState()` / `RestoreState()`は下層からの順番と許可bitをまとめ、未使用層や不正な軸bitを
持つ保存値を拒否して現在のstackを保つ。

### アクションの同時押しを判定する

構えながら攻撃、修飾操作付きショートカット、複数ボタン入力には`FActionChord`を設定値として持つ。
必要操作を1つ以上追加し、必要なら押されていてはいけない操作も指定する。入力装置のキー名ではなく
アクション番号を使うため、キーボード、ゲームパッド、AI、再生入力で同じゲーム規則を使える。

```cpp
FActionChord FocusAttack{ kActionAttack };
FocusAttack.RequireAction( kActionFocus );
FocusAttack.ForbidAction( kActionPause );

Input.Update();
if ( FocusAttack.WasActivated( Input ) ) BeginFocusAttack();
if ( FocusAttack.WasDeactivated( Input ) ) CancelFocusAttack();
```

必要操作をどの順で押しても、最後の必要操作を押した更新だけ`WasActivated()`がtrueになる。
禁止操作を離しただけでは有効化せず、必要操作をいったん離して押し直すまで待つ。禁止操作を押すか
必要操作を離すと`WasDeactivated()`がtrueになる。固定ステップでは
`WasActivated( Context.Input, Context.PreviousInput )`を使う。`FActionInputMask`で変換した履歴も
同じ入口へ渡せる。設定を保存する場合は2つのmaskを取得し、`TrySetMasks()`で重なりを検証して戻す。

### 異なるアクションを順番に判定する

方向操作から攻撃へ繋ぐ技、決まった順番の連続操作、短いメニューコマンドには
`FActionCommandSequenceTracker`を操作対象のfieldとして持つ。入力装置のキー名ではなく
アクション番号を並べるため、キーボード、ゲームパッド、AI、再生入力で同じ列を使える。

```cpp
constexpr u32 RollCommandActions[] = {
	kActionDown, kActionForward, kActionDodge };
FActionCommandSequenceTracker RollCommand{ RollCommandActions, 0.22f };

Input.Update();
RollCommand.Update( Input, DeltaSeconds );
if ( RollCommand.WasCompleted() ) StartRoll();
```

列に含まれない操作は途中入力を壊さない。列内の順番違いでも、直近の押下が新しい列の先頭と
重なるぶんは残して続ける。同じ更新で列内操作が複数押された場合は順番を推測しない。
固定ステップでは`Update( Context.Input, Context.PreviousInput, Context.StepSeconds )`を使う。
同じ入力標本を複数回渡さず、通常フレームまたは固定ステップごとに1回だけ更新する。
`CaptureState()` / `RestoreState()`で設定、途中位置、経過秒、今回完了結果をまとめて保存できる。

### 入力を取りこぼさない

着地、硬直終了、対象への接近など、操作を受理できる瞬間が少し後に来る場合は
`FActionInputBuffer`を操作対象のfieldとして持つ。入力を押しっぱなしにしても再装填せず、
`Consume()`に成功した1回だけ実行する。

```cpp
// field。猶予は用途ごとに決める
FActionInputBuffer InputBuffer{ 0.12f };

// OnUpdate
Input.Update();
InputBuffer.Update( Input, DeltaSeconds );
if ( CanDodge() && InputBuffer.Consume( kActionDodge ) ) Dodge();
```

固定ステップへ組み込む場合は`InputBuffer.Update( Context.Input, Context.PreviousInput,
Context.StepSeconds )`を使う。`CSimulationSubsystem`のスナップショットは現在・前回の入力履歴も
復元するため、長押しを新しい押下として二重発火させない。バッファの残り時間はゲーム規則が持つ
盤面なので、途中状態から再現する場合は`CaptureState()`で`FActionInputBufferState`を取得し、
`ISimulationRule`の保存状態へ各項目を含める。読み込み時は`RestoreState()`へ渡すと、有限性と
残り時間の矛盾を検証し、失敗時に現在のバッファを変えず復元できる。

```cpp
const FActionInputBufferState SavedInputBuffer = InputBuffer.CaptureState();
// SavedInputBufferの各項目を、ゲーム規則の盤面と一緒に保存する

if ( !InputBuffer.RestoreState( SavedInputBuffer ) ) return false;
```

### アナログ軸の遊びを除く

ゲームパッドの中心ずれを止めながら、移動、視点、照準を最大値まで使う場合は
`FActionAxisResponse`を設定値として持つ。内側の遊びを単に0へ切るのではなく、残った範囲を
0から1へ詰め直す。2軸版は入力方向を変えずに長さへ適用するため、斜め入力も歪まない。

```cpp
FActionAxisResponse MoveResponse;
MoveResponse.InnerDeadZone = 0.15f;
MoveResponse.OuterDeadZone = 0.02f;
MoveResponse.ResponseExponent = 1.4f;

CActionInputTracker Input;
Input.GetBindings().BindGamepadAxis(
	kMoveXAxis, EGamepadAxis::LeftX, 0u, 0.0f );
Input.GetBindings().BindGamepadAxis(
	kMoveYAxis, EGamepadAxis::LeftY, 0u, 0.0f );

Input.Update();
FVec2 MoveAxes;
if ( !MoveResponse.TryApplyRadial(
		Input.GetCurrentInput(), kMoveXAxis, kMoveYAxis, MoveAxes ) ) return;
```

円形応答へ渡すゲームパッド2軸は、`BindGamepadAxis`の`DeadZone`を`0.0f`にする。
既定の`0.15f`を残すと各成分が円形応答より先に個別で0へ切られ、斜め方向を復元できない。
キーから同じ軸へ加える割り当ては最大値を返すため、この指定の影響を受けない。

入力値が1を超えた場合は方向を保って長さ1へ止める。設定値、入力値、軸番号が不正なら
出力を変えずfalseを返すため、最後に有効だった操作量を明示的に維持できる。

### 短押しと長押しを分ける

チャージ、長押しインタラクト、短押しと長押しで異なる操作には`FActionHoldTracker`を
操作対象のfieldとして持つ。押している間の進行率、閾値へ届いた1回、短押しで離した1回、
長押し後に離した1回を同じ値状態から取得できる。

```cpp
FActionHoldTracker InteractHold{ 0.4f };

Input.Update();
if ( !InteractHold.Update( Input, kActionInteract, DeltaSeconds ) ) return;
if ( InteractHold.WasTapped() ) Inspect();
if ( InteractHold.WasThresholdReached() ) BeginInteraction();
DrawHoldGauge( InteractHold.GetProgress() );
```

固定ステップでは`Update( Context.Input, Context.PreviousInput, kActionInteract,
Context.StepSeconds )`を使う。閾値を押下中に変更しても、現在の押下は開始時の値を保ち、次の
押下から新しい値を使う。途中状態を再現する場合は`CaptureState()`で得た
`FActionHoldTrackerState`の各項目を`ISimulationRule`の盤面へ含め、読み込み時に
`RestoreState()`へ渡す。追跡中の秒、操作番号、開始時閾値も戻るため、復元後の到達tickがずれない。

### ダブルタップや複数回タップを判定する

同じ操作を短い間隔で2回押す回避や走行切り替え、3回以上の特殊入力には
`FActionTapSequenceTracker`を操作対象のfieldとして持つ。押した瞬間だけを数えるため、
ボタンを押し続けても回数は増えない。必要回数へ届いた更新だけ`WasCompleted()`がtrueになる。

```cpp
FActionTapSequenceTracker DodgeTaps{ 2u, 0.25f };

Input.Update();
if ( !DodgeTaps.Update( Input, kActionDodge, DeltaSeconds ) ) return;
if ( DodgeTaps.WasCompleted() ) Dodge();
```

固定ステップでは`Update( Context.Input, Context.PreviousInput, kActionDodge,
Context.StepSeconds )`を使う。最大間隔を超えてから押すと、古い列を捨て、その押下を新しい1回目として
数える。待機中に`Configure()`しても現在の列は開始時の回数と間隔を保ち、次の列から新設定になる。
途中状態を再現する場合は`CaptureState()`で得た`FActionTapSequenceTrackerState`をゲーム規則の盤面へ
含め、`RestoreState()`へ渡す。途中回数、直前の押下からの時間、開始時設定と操作番号をまとめて戻せる。

### 再使用までの待ち時間を扱う

攻撃、回避、能力、操作装置などを連続使用させたくない場合は`FGameplayCooldown`を対象のfieldとして
持つ。最初は使用可能で、実行条件を満たしたときに`TryUse()`がtrueなら処理を行う。待機中の
`TryUse()`は残り時間を巻き戻さずfalseになるため、入力を押し続けても再使用時刻は後ろへずれない。

```cpp
FGameplayCooldown DashCooldown{ 0.8f };

// 固定ステップごと
DashCooldown.Update( Context.StepSeconds );
if ( Context.WasPressed( kActionDash ) && DashCooldown.TryUse() ) Dash();
DrawCooldown( DashCooldown.GetProgress() );
```

`Update()`へ渡したゲーム時間だけ進むため、固定ステップ、記録再生、時間停止の規則を利用側で
揃えられる。callbackを予約する`CTimerSubsystem`とは異なり、`CaptureState()` / `RestoreState()`で
開始時設定、経過秒、今回の完了通知まで盤面へ含められる。待機中に秒数を変更しても現在の待機は
開始時設定を保ち、次の`TryUse()`から新しい値を使う。`WasCompleted()`は完了した`Update()`から
次の有効な`Update()`まで保持されるため、同じ固定ステップ内ですぐ再使用しても完了通知を読める。

---

## 途中から始める

最初から流し直すのでは足りない場面がある（長い記録の後半だけ見たい、バグの瞬間へ戻りたい、
1 フレーム戻して確かめたい）。続きから同じ道を進むには 4 つが要る。**1 つでも欠けると別の道になる。**

| 写すもの | 欠けるとどうなるか |
|---|---|
| 盤面 (`ISimulationRule::TrySaveState`) | そもそも別の局面から始まる |
| 時計 (`CFixedStepDriver`) | ティックがずれ、テープの読み出し位置が合わない |
| 乱数 (`CDeterministicRandom`) | 同じ入力でも違う出目になる |
| 入力履歴 (`FActionInput`) | 長押しを新しい押下と誤認し、操作が二重に発火する |

`CSimulationSnapshot` がこの 4 つを 1 つに束ねる。テープは含めない（テープは「どう操作したか」、
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

戻す処理は「4 つとも戻せるか」を先に確かめてから実際に戻す。途中で失敗して一部だけ戻った
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
