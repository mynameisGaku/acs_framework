# AcsFramework Core

「簡単に使える」を、既存ACSの機能を隠してしまうことではなく、所有者・配線・更新順を
ゲーム側が毎回書かなくてよいこととして扱う。

## 使い始める

共通の入口は `AcsFramework_Core/AcsFramework.h` である。ゲーム側は必要な型だけを使い、
実体の取得はシーンまたはアプリから `GetSubsystem<T>()` で行う。

ゲーム固有のアプリは `CAcsFrameworkApp` を継承し、`CreateInitialScene()` だけを override する。
`InitialScene()` はサービス配線を完了してからこの hook を呼び、`OnStart()` は基底の起動処理へ
渡すため上書きできない。hook が空の `TUniquePtr<AScene>` を返した場合は代替シーンを作らず、`CGame` の既存の
null から終了する契約へ渡す。既定の Framework exe は `CAcsFrameworkApp` と `ABootScene` を使い、
ゲーム側の EntryPoint は `ACS_DEFINE_MAIN(CMyApp)` を定義して既定の EntryPoint.cpp を target へ入れない。
共通の起動・更新・描画・終了処理は基底側へ残す。

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
`CSettings`に任せる。`FGameSettingsStore`は値と安定した文字列領域を所有し、
`CGameSettingsSubsystem`はファイルの場所、遅延保存、警告、終了時保存を引き受ける。
同じ理由で音声は`CAudioDirector`とXAudio2 backendを所有・配線するが、音声のミキシングや
アセット解決を複製しない。

所有型だけが値を発行または更新する場合も`friend`へ依存しない。値側は`protected`の
`Foo_Internal`操作だけを持ち、所有型の`private`アダプターが必要な操作だけを再公開する。
別の局所型から世代付き状態を扱う場合は、`CLoadingScreenSubsystem::FScopeAdapter`のような
検証付きアダプターを返し、状態fieldそのものは`private`に保つ。

3Dエフェクトはscene固有のカメラ・HDR描画先・depth・終了順へ結び付くため、GameInstanceの
Subsystemにはしない。`AEffect3DScene`がscene単位で`CEffect3DPlayer`を所有し、ACSは外部描画を
挿入できる透明3DパスとD3D12借用契約だけを持つ。Effekseer固有の読込・再生・描画はframework
側の非公開実装へ閉じる。

3D水面もsceneのノード、カメラ、背景、depthへ結び付くためSubsystemにはしない。
`FWater3DSpawnParams`は描画器なしで検証できる値、`CWater3DSpawner`はACSの平面メッシュと
`AWaterSurface3DComponent`を識別子付きノードへ接続する状態なしのアダプターとする。

3D地面も局所的な生成処理なのでSubsystemにはしない。`FGround3DSpawnParams`は上面位置、広さ、
厚み、材質、衝突レイヤーを検証し、`CGround3DSpawner`は既存のモデル生成と場面衝突登録だけを
組み合わせる。表示面と直下の箱は同じノード尺度を使い、`TryApplyTo`は所有と形状対応を先に検証し、
形状番号を保って表示、広さ、厚み、レイヤーを同期更新する。後段失敗時は生成ノードも巻き戻す。

3D直方体も局所的な生成処理なのでSubsystemにはしない。`FBlock3DSpawnParams`は中心位置、回転、
XYZ全寸法、材質、衝突レイヤーを検証し、`CBlock3DSpawner`は既存の立方体生成と明示箱登録だけを
組み合わせる。表示と衝突は同じノード尺度を使い、回転時の衝突は既存契約どおりworld軸平行箱へ
安全側に包む。`TryApplyTo`は所有と形状対応を先に検証し、形状番号を保って表示、変形、レイヤーを
同期更新する。後段失敗時は生成ノードも巻き戻す。

3D球も局所的な生成処理なのでSubsystemにはしない。`FSphere3DSpawnParams`は中心位置、半径、
材質、衝突レイヤーと派生直径を検証し、`CSphere3DSpawner`は既存の球生成と明示球登録だけを
組み合わせる。ローカル半径0.5の表示と衝突へ同じ均一尺度を使い、非一様な親拡縮では既存契約
どおり最大軸の外接球へ安全側に広げる。`TryApplyTo`は所有と形状対応を先に検証し、形状番号を
保って表示、半径、レイヤーを同期更新する。後段失敗時は生成ノードも巻き戻す。

天井なし3D部屋も局所的な定型配置なのでSubsystemにはしない。`FRoom3DSpawnParams`は床上面位置、
内寸、壁と床の寸法、材質、衝突レイヤーに加え、計算後の外寸と中心が有限か検証する。
`CRoom3DSpawner`は地面1個と直方体4個を合成し、途中失敗時は既生成分を逆順に巻き戻す。
一括破棄は全5組の場面所有、ノードと形状の対応、重複不在を先に確認する。

両端が開いた3D通路も局所的な定型配置なのでSubsystemにはしない。`FCorridor3DSpawnParams`は入口境界、
XZの正負4方向、内幅、長さ、壁と床の寸法、材質、衝突レイヤーと派生する出口・外寸を検証する。
`CCorridor3DSpawner`は既存の地面1個と直方体2個だけを合成し、途中失敗時は既生成分を逆順に巻き戻す。
一括破棄では全3組の所有関係とノード・形状の重複を先に検証する。

両側柵付き3D橋も局所的な定型配置なのでSubsystemにはしない。`FBridge3DSpawnParams`は入口境界、
XZの正負4方向、床板幅と長さ、柵寸法、材質、衝突レイヤーから既存の地面1個と柵2組を計算する。
始終端の支柱は床板からはみ出さない位置へ収める。`CBridge3DSpawner`は既存生成器だけを合成し、
途中失敗を逆順に巻き戻す。一括破棄では全パーツの所有関係と重複を先に検証する。

3D出入口枠も局所的な定型配置なのでSubsystemにはしない。`FDoorway3DSpawnParams`は壁下辺中央、
X/Zの壁幅軸、壁と開口の寸法、開口横ずらし、材質、衝突レイヤーと派生する左右柱幅・上枠高を検証する。
`CDoorway3DSpawner`は既存の衝突付き直方体3個だけを合成し、開口部分へ表示も衝突も置かない。
途中失敗は逆順に巻き戻し、一括破棄では全3組の所有関係と重複を先に検証する。

3D柵も局所的な定型配置なのでSubsystemにはしない。`FFence3DSpawnParams`は始点、XZの正負4方向、
中心間長さ、高さ、最大支柱間隔、横桟数、各部寸法、材質、衝突レイヤーと派生する終点を検証する。
`CFence3DSpawner`は既存の衝突付き直方体だけを均等な支柱と横桟へ合成し、途中失敗時は生成の逆順に
巻き戻す。一括破棄では全要素の所有関係とノード・形状の重複を先に検証する。

3D階段も局所的な定型配置なのでSubsystemにはしない。`FStairs3DSpawnParams`は最下段手前の床上中心、
XZの正負4方向、1から256までの段数、1段の寸法、材質、衝突レイヤーと派生する終端を検証する。
`CStairs3DSpawner`は既存の衝突付き直方体だけを共通底面から合成し、途中失敗時は既生成分を高い側から
逆順に巻き戻す。一括破棄では全段の所有関係とノード・形状の重複を先に検証する。

3D天候と時刻もsceneごとの太陽、空、雲、霧、環境光を変えるためSubsystemにはしない。
`FWeather3DAppearance`は`CWeatherSystem`、`FTimeOfDay3DAppearance`はACSの`CAmbientDirector`から
決まる検証可能な値とする。`AWeather3DScene`は派生場面が設定した晴天時の環境を基準に、
時刻の太陽・空・環境光を先に作り、天候の相対値を後から毎フレーム適用する薄いアダプターとする。
雨雪の素材は固定せず、粒子密度と風向きを公開する。

3D見た目プリセットもsceneごとの設定値なのでSubsystemにはしない。`EVisualPreset3D`は
`Performance`、`Balanced`、`Cinematic`の負荷と見た目を選び、純粋な
`TryApplyVisualPreset3DSettings`がACSの遮蔽、反射、間接光、ポスト処理設定へ原子的に反映する。
`AUi3DScene::TryApplyVisualPreset3D`はその場面窓口だけを担う。GPU資源、描画順、実際の各効果は
ACSへ任せ、適用後の個別調整口も残す。

3D材質の内部散乱もモデル1個ごとの決定論的な値なのでSubsystemにはしない。
`FModel3DSpawnParams`が内部色と強度を検証し、`CModel3DSpawner`が既存のACS PBR材質へ渡す。
画面用の一時資源、材質の有無による経路選択、実際の散乱処理はACSへ任せる。

3D材質の布向け毛羽反射もモデル1個ごとの決定論的な値なのでSubsystemにはしない。
`FModel3DSpawnParams`が毛羽の色、強さ、粗さを検証し、`CModel3DSpawner`が既存のACS PBR材質へ渡す。
光源、環境光、実際の毛羽反射計算はACSへ任せ、Frameworkは布の既定値と接続だけを持つ。

3D材質のブラッシュドメタルもモデル1個ごとの決定論的な値なのでSubsystemにはしない。
`FModel3DSpawnParams`が-1から1の異方性を検証し、`CModel3DSpawner`が既存のACS PBR材質へ渡す。
Frameworkは金属度、粗さ、方向性の既定値だけを揃え、実際の反射計算はACSへ任せる。

3D選択は`AUi3DScene::MakeScreenRay3D`と`Raycast3D`、1回で済ませる`PickScreen3D`へまとめる。
内部では`CScenePicker`からACSの`CSceneNodeGraph`が持つ実形状判定を呼び、再実装しない。
世代付き識別子を使いやすいノードポインタと世界座標の命中情報へ変換し、高速な境界箱判定は
低水準の別用途として互換維持する。

単一モデルを視線操作対象として置く定型処理は`CInteractableModel3DSpawner`へまとめる。
静的または骨付きモデルの既存生成器と`CInteractionFocus3D::RegisterTarget`を順に呼ぶだけの
状態なしアダプターとし、対象登録に失敗した生成ノードは破棄予定へ戻す。複合ノードを登録する
低水準の窓口はそのまま残し、作品固有の操作内容や入力割り当ては所有しない。衝突付き入口は
`CModel3DSpawner`または`CAnimatedModel3DSpawner`が返す形状を引き継ぎ、後段失敗時だけ
`CSceneCollision3D`から形状を外して生成ノードを巻き戻す。場面途中の破棄では、ノード破棄を
受け付けてから対象登録と任意の衝突形状を外し、成功した呼出側の結果だけを空に戻す。
通常の衝突付き結果も`CModel3DSpawner::DestroyCollidable`でノードと形状の対応を検証してから
一括破棄し、`AUi3DScene::DestroyCollidableModel3D`を公開窓口にする。

3Dの重なりと移動判定は、場面側が`CSceneCollision3D`を所有する。ACSの`CCollisionWorld3D`へ
ノードとローカル形状を登録し、問い合わせ時に現在Transformへ同期して、結果をノードポインタへ
戻す。球型キャラクターの次状態も、同期済み形状からACSの決定的な移動処理で計算する。
`CCharacterMover3D`はその結果を親座標へ戻してノードへ反映し、速度と接地状態だけを保持する。
固定更新と入力寿命は所有せず、キャラクターごとの短い寿命なのでsubsystemにはしない。

3D近接トリガーは、場面またはゲーム機能が`CProximityTrigger3D`を所有する。基準ノードへ追従する
球または箱と`CSceneCollision3D`の既存重なり問い合わせを使い、前回からの進入、滞在、退出だけを
世代付きノード識別子で返す。`AUi3DScene::BindProximityTrigger3D`は場面グラフと衝突集合への
接続をまとめる。扉、会話、チェックポイントなど作品固有の反応、入力、描画、時間は所有せず、
短い寿命の局所状態なのでsubsystemにはしない。現在の判定範囲は`TryGetWorldSphere`または
`TryGetWorldBox`で取得でき、`DrawProximityTrigger3D`なら既存の1フレーム線へそのまま表示できる。

追跡対象が1つに決まるゴールや復帰地点は、呼出側が`CCheckpoint3D`を所有する。
内部の`CProximityTrigger3D`と登録済み衝突形状番号から、一度限りまたは再進入可能な発火事象だけを
返す。`CCheckpoint3DSpawner`は識別子付き範囲基準ノードの生成、接続、失敗時巻き戻し、破棄を
状態なしでまとめ、`AUi3DScene`が`BindCheckpoint3D`、`SpawnCheckpoint3D`、
`DestroyCheckpoint3D`、`DrawCheckpoint3D`を公開する。時間、入力、保存、場面遷移、演出は
所有しないため、GPUなしで対象位置と明示更新だけから同じ発火を再現できる。
複数地点を順番に通る場合は、各`CCheckpoint3D`が発火した0始まり番号だけを
`FCheckpointRoute3D::Advance`へ渡す。ルートは場面とチェックポイントを所有せず、順番違い、
周回完了、全体完了を明示結果として返す局所的な値状態に留める。保存時は`CaptureProgress`で
設定識別付きの進行値を得て、同じ件数・周回数のルートだけへ`RestoreProgress`する。
区間計測が必要なら、呼出側の明示経過秒を`FCheckpointRoute3DTimer::Tick`へ渡し、
受理された進行結果だけを`RecordAdvance`する。計測器は時計や場面を読まず、停止・再開と
区間、周回、合計秒だけを保持するため、同じ時間列と発火列を単体で再生できる。
保存時はルートの`CaptureProgress`と計測器の`CaptureState`を対で取得し、同じルート設定へ
`RestoreProgress`と`RestoreState`で戻す。計測状態が不正なら現在値を変えずに拒否する。

単一モデルを第三者視点キャラクターとして使う定型処理は`CThirdPersonCharacter3DSpawner`へまとめる。
静的または骨格モデルの既存生成器、`CSceneCollision3D`、`CThirdPersonCharacter3D`を順に呼ぶだけの
状態なしアダプターとし、自己形状番号を移動設定へ反映する。必須接続の途中で失敗した場合は形状と
ノードを巻き戻し、骨格の4状態だけが不足した場合は初期再生を保って成否を別フラグで返す。
一括破棄ではノードを破棄予定にできた後だけ操作と自己形状を外し、呼出側の生成結果も空にする。

3Dの人物追従カメラは`CNodeOrbitCamera3D`を場面または人物が所有する。ACSの軌道カメラ計算と
場面描画形状の遮蔽物回避を使い、明示した操作量と時刻から注視点、角度、距離を更新する。
入力装置と追従ノードは所有せず、場面ごとの短い寿命なのでsubsystemにはしない。

3Dライト配置は`FLight3DSpawnParams`を検証可能な値、`CLight3DSpawner`を状態なしの接続層とする。
位置または光源方向、色、強さ、到達距離から、識別子付きノードと`ALightComponent3D`を作るだけに
留める。光の収集、上限選択、PBR描画、影はACSへ任せ、Framework側へ照明計算を複製しない。
見える光源は`FLamp3DParams`から自己発光球と同色の点光源を組み立て、`CLamp3DSpawner`が
途中失敗時の巻き戻し、所有確認後の同期更新、一括破棄を受け持つ。局所的な2ノードの生成なので
subsystemにはしない。
衝突付き街灯は`FStreetLamp3DSpawnParams`が床位置から直立ポストとランプの位置を決め、
`CStreetLamp3DSpawner`が既存の`CBlock3DSpawner`と`CLamp3DSpawner`を合成する。光、材質、衝突を
再実装せず、3ノードと1形状の途中失敗巻き戻し、共通親と所有確認後の同期更新、一括破棄だけを
受け持つため、これもsubsystemにはしない。
被写体用の`FStudioLightRig3DParams`も中心、見る方向、半径から3灯の点光源指定を決める値に留め、
`CStudioLightRig3DSpawner`が途中失敗時の巻き戻し、3灯と共通親の所有確認後の同期更新、一括破棄を
受け持つ。平行光を増やさず、既定または時刻連動の太陽を唯一の影付き主光源として保つ。

固定向きの3D画像配置は`FSprite3DSpawnParams`を検証可能な値、`CSprite3DSpawner`を状態なしの
接続層とする。`CImageLibrary`は`Assets`相対名をACSの`CAssetRegistry`へ渡し、配置層は
識別子付きノードと`ASprite3DComponent`だけを作る。画像デコード、GPU画像所有、深度判定、
HDR透過描画はACSへ任せる。カメラへ自動で向くビルボードとは契約を分ける。

3Dアニメーション配置は`FAnimatedModel3DSpawnParams`を検証可能な値、
`CAnimatedModel3DSpawner`を状態なしの接続層とする。骨付きFBXの読み込み、識別子付きノード、
`ASkinnedMeshComponent3D`、初期クリップ再生だけをまとめ、姿勢計算と描画はACSへ任せる。
移動連動は`FCharacterAnimation3DInput`と`FCharacterAnimation3DProfile`で次状態を純粋に選び、
`CCharacterAnimator3D`が待機・歩き・走り・ジャンプの姿勢遷移へ接続する。部品を所有せず、
キャラクターごとの短い寿命だけを持つため、これもsubsystemにはしない。

3Dデバッグ描画もsceneのカメラ、HDR描画先、GPU終了順へ結び付くためSubsystemにはしない。
`FDebugLine3D`と`CDebugDraw3DQueue`はGPUなしで値と1フレーム上限を検証し、
`CDebugDraw3DLayer`はACSの`FDebugDraw3D`を遅延初期化して`AUi3DScene`の透明3Dパスへ接続する。
`DrawLine3D`、`DrawArrow3D`、`DrawAxes3D`、`DrawGrid3D`、`DrawCircle3D`、`DrawCone3D`、`DrawCylinder3D`、`DrawBox3D`、`DrawAabb3D`、`DrawSphere3D`は深度を無視する確認用オーバーレイで、
`DrawCollisionShape3D`は登録済みの1形状、`DrawCollisionShapes3D`はレイヤー一致する全有効形状、
`DrawProximityTrigger3D`は近接判定範囲、`DrawCheckpoint3D`はチェックポイント判定範囲と同じ
球または箱を一括登録する。
表示を続ける側は更新ごとに登録する。
矢印は線キューだけで胴体と立体的な矢尻へ展開し、座標軸は指定回転のX、Y、Zを赤、緑、青の
3本の矢印として原子的に登録する。水平グリッドは中心、高さ、片側距離、分割数だけでXZ面へ展開し、
円は中心、法線、半径、分割数から任意のworld面へ原子的に展開する。
円錐は頂点、方向、長さ、底面半径だけで視野や正面範囲へ展開する。
円柱は中心、軸、高さ、半径だけで回転軸や円柱範囲へ展開する。
向き付き箱は中心、回転、半サイズから回転ノードの局所範囲へ展開する。
球はACSの衝突判定と同じ`FSphere`をそのまま受け取る。

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
| `CPauseScreenSubsystem` | ポーズ表示の状態と時間追従 | time、`FPauseScreenRenderer` / `Update()`・`Draw()` |
| `CSaveSubsystem` | GameInstance単位のセーブ枠公開 | `FSaveSlotStore` / 呼出し時 |
| `CSceneTravelSubsystem` | シーン遷移の共有窓口とゲーム配線 | `FSceneTravelController` / `Update()` |
| `CScreenSubsystem` | 解像度、全画面、窓の操作 | `CApplication` / なし |
| `CGameSettingsSubsystem` | プレイヤー設定の保存先、自動保存、外部窓口 | `FGameSettingsStore` / `Update()` |
| `CAppStateSubsystem` | シーンを跨ぐ型付き状態 | Core所有 / 呼出し時 |
| `CUiFontSubsystem` | GameInstance単位のUIフォント公開 | `FUiFontResource`、`CRenderer` / `Acquire()` |
| `CTimeSubsystem` | 時間の適用とfixed stepの窓口 | `CGame` / `Update()` |
| `CTimerSubsystem` | scaled / unscaled の待機処理 | engine timer / `Update()` |
| `CMusicSubsystem` | 状態に応じたBGM切替と差し込みの一音 | `CMusicDirector` + `CAudioSubsystem` / 実時間 `Update()` |
| `CSpatialAudioSubsystem` | 場所のある効果音と聴取位置 | `CSpatialAudio` + `CAudioSubsystem` / 実時間 `Update()` |
| `CPrefabSubsystem` | 名前からのANode生成 | `CPrefabSystem` / 呼出し時 |
| `CSceneSnapshotSubsystem` | ANodeツリーの保存と復元 | `TrySaveNodeTree` / `CSaveArchive` / 呼出し時 |
| `CLocalizationSubsystem` | 言語ごとの文、`{0}`の差し込み、表の読み込み | `CLocalizationDirector` / 呼出し時 |

開発中だけ使うものは `Source/Debug/` 側に置く。`CPerfBudgetSubsystem`はフレーム予算の計測、
`CDevConsoleSubsystem`は打ち込みコマンド、`CHotReloadSubsystem`はファイル差し替えの監視を担う。
いずれも配線とページの登録は`#if _DEBUG`の内側で行う。各モジュールの取り決めはそれぞれの
`README.md`が正本である。

`FScreenOverlayFadeState`は画面へ重ねる表示の出し入れ時間と現在の濃さだけを保持する通常値型である。
`FPauseScreenRenderer`はポーズ幕のSpriteBatch、遅延初期化、フォント不足の通知、色と配置を保持する。
`CPauseScreenSubsystem`は表示理由、文言、フォントの選択、表示指示を保持し、フェード後の濃さを描画型へ渡す。

`FTimeControlState`はpause理由、通常速度、1フレーム進行、フレームごとの更新可否と実効速度を保持する
非サブシステム型で、`CTimeSubsystem`が1つ所有する。`CTimeSubsystem`は決定した実効速度を`CGame`へ
反映し、fixed timestepの設定と照会を`CGame`へ渡す。

`FSaveSlotInfo`は枠番号、存在、版、サイズ、パスをまとめる値で、`CSaveSubsystem`が一覧情報として返す。
`FSaveSlotStore`は保存先と枠数を値として保持し、枠のパス、範囲、同期入出力を調整する。
`CSaveSubsystem`はGameInstance単位の所有者と公開窓口を担い、保存書式、CRC検証、一時ファイルと
置換処理はEngineの`TSaveSlot`と`CSaveArchive`が所有する。
`ESceneTransition`は幕なしと暗転を使う切替方法を表し、`CSceneTravelSubsystem`の公開要求が受け取る値である。
`FSceneTravelController`は切替方法の選択と暗転待ちの積み下ろし状態を所有し、`CSceneTravelSubsystem`はGameInstanceの共有窓口と`CGame`の配線を所有する。
`CLoadingScreenSubsystem`は読み込み追従、表示世代、表示指示、フォント優先順位を所有し、
`FLoadingScreenRenderer`はスピナー時間、SpriteBatchの遅延初期化、ロード画面のGPU描画を所有する。
`FUiFontResource`はUIフォントのGPU資源、生成設定、読み込み結果を単独所有する。
`FDebugTopOverlayRenderer`はDebugTopを重ねるSpriteBatch、遅延初期化、描画文脈、背景幕からHUDまでの描画順を所有する。
`CDebugTopOverlaySubsystem`はHUDの寿命、表示、切替入力、時間停止、背景幕の設定を所有し、描画時の値だけを通常型へ渡す。
`FEventSubscription`、`FInputRepeat`、`FGameTimer`は共有状態を継続更新する所有者ではないため、
必要な利用側が値として持つ。

`CTimerSubsystem`はGameInstanceの寿命で2つの時計を所有し、アプリの更新から毎フレーム進める。
`CGameTimerScope`はシーンまたは所有者が登録した`FGameTimer`を追跡する通常型で、タイマー窓口を所有しない。
`CGameTimerScope`はコンストラクタで受けた同じ`CTimerSubsystem`へ結び付き、`FGameTimer`は生成元の
GameInstanceまたはタイマー窓口をまたいで使わない。別のGameInstanceまたは別の窓口から得た値は相互に互換ではない。
シーンの`OnExit`で`CancelAll()`を呼び、追跡中の処理と生コールバックをシーン破棄より先に解除する。シーン遷移中も
全体の時計は進むため、自動pauseは行わない。pause中に止める所有者は`OnPause`で`CancelAll()`を呼び、
`OnResume`で必要な処理を再登録する。
デストラクタの取消しは終了順が変わった場合の保護であり、`OnExit`の代わりにはしない。

### 音声の契約

`CAudioSubsystem`はGameInstanceの寿命で音の取りまとめと音の出力先を所有し、`Bind`でアセットレジストリと出力先を接続する。
出力先の初期化に失敗した場合は`false`を返し、呼出し側を止めずに無音で継続する。音声名と音量状態は保持する。
`Update`は実時間で音声を進め、終了時は音を止めて非所有参照を外し、出力先を終了する。
ゲーム時間が停止しても実時間で進むため、止める場合は`Pause()`を明示して呼び出す。
全体、BGM、効果音の3種類の音量を永続化する場合は、GameSettingsへの接続側が担当する。

効果音名は`CAssetRoot`を通し、`Audio/...`または`Assets/Audio/...`を実際の素材パスへ直す。
`CSpatialAudioSubsystem`はEngine発行の音源番号を使い、再生開始時の距離減衰と左右位置を同じ
voiceへ反映する。XAudio2で左右位置を使う3D効果音はモノラル素材にする。

## 起動・更新・描画の契約

`CAcsFrameworkApp`の処理順は意図的に固定する。新しい常駐機能を追加するときは、ここへ
置く理由と、停止中も動くべきかを先に決める。

```text
CAcsFrameworkApp::OnStart(final)
  → CGame::OnStart
     → GameInstance 初期化
     → CAcsFrameworkApp::InitialScene(final: Bind / Configure)
        → CreateInitialScene(virtual hook)
     → push / apply
     → 初期シーンの OnEnter

InitialScene の配線
  Fade / SceneTravel / AssetRegistry / Screen / App / Event / AppState
  Save / GameSettings
  Audio と asset registry / player settings
  Time / AssetLoader

OnUpdate(real dt)
  PerfBudget::BeginFrame（このフレーム全体を含めるため最初に開ける）
  DebugTop の入力と時間の理由
  Time の反映 → シーン更新（停止中は呼ばない）
  AssetLoader → LoadingScreen → SceneTravel
  Timer（ゲーム時間倍率を受ける）→ Audio（実時間）→ Music（実時間）→ SpatialAudio（実時間）
  GameSettings（手が止まったら保存）→ PauseScreen（実時間）
  HotReload（実時間・_DEBUG のみ）
  PerfBudget::EndFrame

OnRender
  UI font → scene → pause → DebugTop → loading screen

OnShutdown
  dirty な GameSettings を保存 → CGame::OnShutdown
```

シーン停止中も進めるものは実時間側へ置く。音声、暗転、ロード表示、ポーズ表示をゲーム
時間へ繋ぐと、止めた瞬間に演出まで止まるためである。

## 設定とデバッグ設定を混ぜない

`FGameSettingsStore`は製品としてプレイヤーが決める値と、設定へ渡すキー・文字列値の
安定した領域を所有する。保存先、存在確認、UTF変換、変更状態、自動保存、警告、終了処理は
`CGameSettingsSubsystem`が所有する。サンプルの既定保存先は
`%APPDATA%/acs_framework/GameSettings.acscfg`であり、実行時の作業フォルダには依存しない。
製品化時は`CAcsFrameworkApp::InitialScene`にある`acs_framework`をゲーム固有の識別名へ変える。
`CDebugTopSettings`は開発中の診断・調整値なので、同じキーを共有しない。

設定ごとの既定値と初期反映はゲーム側が決定する。`Set*`は変更状態を立て、`Update()`と
終了処理が待ち時間を含む保存の実行を所有する。

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

`CAssetLoadScope`は`CAssetLoaderSubsystem`を所有せず、シーンまたは処理単位が自身の要求だけを追跡する通常型である。Loaderはscopeより長く生存し、Begin呼出し中（callbackでscopeが破棄される場合を含む）から追従終了まで参照可能でなければならない。利用側はsceneの`OnExit`で`CancelAll`を呼び、デストラクタの取消しは`OnExit`で明示取消しできなかった場合の保護とする。内部状態の確保に失敗した場合はBeginが無効値を返し、Cancel・IsActiveはfalse、CancelAllは何もしない。
`Begin`の同期callbackや入れ子のBegin・CancelAll、Loader直呼出しの再入では最後に変わった状態を優先し、scopeがcallback内で破棄されても共有状態で外部呼出しを安全に完了する。無効な返却値は旧要求が現在かつ処理中の場合だけ旧追跡を戻す。返却値は完了済みでも返すが、`IsActive`が判定する追跡対象とは分ける。`Cancel`は無効・外部要求なら状態を変えずfalseを返し、所有要求は先に追跡から外してLoaderの実取消結果だけを返すため、完了・置換済みならfalseになり得る。

`CLoadingScreenFollowScope` は `CLoadingScreenSubsystem` の特定要求への追従を、sceneまたは処理単位の寿命へ束ねる通常型である。
LoadingScreenはscope全期間、LoaderはFollow成功からResetまたは自動完了まで参照可能にする。利用側は終了処理で `Reset` を明示し、デストラクタは解除漏れを保護する。外部置換後の古い要求は新しい追従を変更せず、同じRequestを再取得した場合も追従世代が異なるため古いscopeは解除しない。`Owns` と `GetRequest` は現在追従を所有している場合だけ有効値を返す。

`CLoadingScreenDisplayScope` は手動ロード表示をsceneまたは処理単位で所有する通常型である。`Show` は追従中なら失敗し、既存の表示状態を変えない。表示世代が現在の場合だけ `SetMessage`、`SetProgress`、`SetFont`、`Reset` が成功し、別のscopeや公開APIが表示を置き換えた後は古いscopeから状態を戻さない。描画フォントは手動表示用、公開設定用、共有フォントの順で選び、手動表示用は表示世代の交代で解除する。公開設定用は置換またはnullptrまで、手動表示用はReset・失効・再設定まで参照可能にする。LoadingScreenはscopeより長く生存させる。
