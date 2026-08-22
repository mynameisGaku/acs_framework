# ロードマップ — v1.0.0 まで

## この枠組みは何を目指すか

**3D のゲームを、DXLib と同等以下の手数で作り始められて、DXLib より綺麗に映ること。**

- 2D は「使えて当たり前」。売りにしない。
- 設計思想は ACS 独自のものに従う。真似るのは**使いやすさ**だけ。
- 描く力は ACS が持っている。この枠組みが足すのは**手数の少なさ**。

### v1.0.0 の判定基準

1. 別のマシンで `clone` → 1 コマンド → ビルドが通り、サンプルが動く
2. 何も設定せずに数十行で、モデルが綺麗に映って動く
3. 破壊的変更は版を上げる、と約束できる（API が固まっている）

---

## ACS はどこまで持っているか

ACS のモジュールは **438 個**ある。この枠組みが窓口を用意できているのは、そのごく一部。
**全部は包まない。** v1.0.0 では「3D のゲームを 1 本作るのに要る筋」に絞る。

### 覆えているもの

| 分野 | 枠組み側 |
|---|---|
| 起動・シーン遷移・状態 | `CAppSubsystem`、`CSceneTravelSubsystem`、`CAppStateSubsystem` |
| アセット読み込み | `CAssetLoaderSubsystem` |
| 音 (BGM・効果音・位置) | `CAudioSubsystem`、`CMusicSubsystem`、`CSpatialAudioSubsystem` |
| セーブ・設定 | `CSaveSubsystem`、`CGameSettingsSubsystem` |
| 時間・待機 | `CTimeSubsystem`、`CTimerSubsystem` |
| 入力の割り当て | `CActionBindingTable`、`FActionKeyRebindState`、`CBoundActionSource` |
| 画面・フェード・ロード中・ポーズ | `CScreenSubsystem`、`CFadeSubsystem`、`CLoadingScreenSubsystem`、`CPauseScreenSubsystem` |
| ノード生成・シーン保存 | `CPrefabSubsystem`、`CSceneSnapshotSubsystem` |
| 3D 水面 | `CWater3DSpawner`、`FWater3DSpawnParams`、ACSの動的波紋 |
| 3D 天候 | `AWeather3DScene`、`FWeather3DAppearance`、ACSの`CWeatherSystem` |
| 3D エフェクト | `AEffect3DScene`、`CEffect3DPlayer`、同梱の Effekseer |
| 3D の形状重なり | `CSceneCollision3D`、ACSの`CCollisionWorld3D` |
| 遊ぶ人向け UI | `AUi3DScene`、`CUiLayer`、ACSの `AWidget` 群 |
| 多言語 | `CLocalizationSubsystem` |
| 決定性 (記録と再生) | `CSimulationSubsystem` |
| 開発支援 | `CPerfBudgetSubsystem`、`CDevConsoleSubsystem`、`CHotReloadSubsystem`、`DebugTop` |

### 覆えていないもの (v1.0.0 で埋める)

| 分野 | ACS 側に在るもの |
|---|---|
| **3D 描画** | `MeshComponent3D`、`CameraComponent3D`、`Transform3D`、`PbrShader`、`StandardShader`、`ShadowMap`、`Ibl`、`Sky`、`Atmosphere`、`SceneRenderResources` |
| **ポストプロセス** | `PostProcess`、`Ssao`、`Ssgi`、`Ssr`、`Fxaa`、`SubsurfaceScattering`、`MotionVector`、`TemporalHistory`、`HiZ` |
| **アニメーション** | `AnimationGraph`、`AnimationCurve`、`SkinnedShader`、`asset/SkinnedMesh` |

### 覆えていないもの (v1.0.0 の範囲外と宣言する)

ACS には遊びの部品が大量に在るが、**どれもゲームごとに形が変わる**ので、枠組みが窓口を
固定すると邪魔になる。使いたい人が ACS を直接呼べばよい。

`HealthSystem`、`InventorySystem`、`CraftingSystem`、`DeckSystem`、`TurnManager`、
`ProjectileSystem`、`WeaponSystem`、`BuffSystem`、`CombatStateMachine`、`BehaviorTree`、
`Pathfinding`、`Perception`、`WaveSpawner`、`DungeonGenerator`、`PartySystem`、`Progression`、
`ScoreSystem`、`AchievementManager`、`SeasonPass`、`EconomyDirector`、`DialogueSystem`、
`CinematicsDirector`、`PhotoMode`、`HungerSystem` ほか

同じく範囲外: ネットワーク (`Lockstep`、`RollbackSession`、`NetSnapshot`)、VR (`OpenXrBridge`)、
Steam (`SteamworksBridge`)、スクリプト (`ScriptHost`)、機械学習 (`MlRuntime`)、
複数形・性別・右から書く言語、エディタ。

**3D の剛体物理は ACS にも無い。** 在るのは当たり判定の材料 (`MeshCollider` / `Collision3D`) と
2D 専用の `RigidWorld2D` だけ。**これは ACS 側へ入れる** (2026-08-17 判断)。2D の剛体物理が
既に ACS に在るので、同じ役目のものを枠組み側に置くと、どちらを使うか分からなくなるため。
提案は `acs_temp_doc/0003-rigid-body-3d.md`。

---

## マイルストーン

### v0.2 — 3D の入口 (完了)

**判定基準: 何も設定せずに数十行で、モデルが綺麗に映って動く。**

2026-08-17 時点でここまで映る (`Source/AcsFramework_Sample/Scene/Demo3DScene.cpp`、約 70 行)。

![3D デモ](demo3d.png)

空・太陽・影・環境光が 1 つの太陽から繋がっている。**空は物理ベースの大気を焼いたもので、
それがそのまま環境光にもなる**ので、空と光が食い違わない。
霧・仕上げ・大気・空は場面から触れる (`Fog()` / `PostParams()` / `Atmosphere()` / `Sky()`)。

雲も本物 (`CVolumetricClouds`) が出る。ライティングは名前付きの係数へ組み直した
(`acs_temp_doc/0012`)。**参照描画** (`Clouds().bReferenceMode`) で «汚さの原因が
ライティングか再構成か» を切り分けられる。

雲影はACSのワールド影へ接続済みで、天候による環境光の暗化も`AWeather3DScene`から
IBLへ反映する。残りは空を焼くIBLへ雲の形そのものを含めることと、空気遠近。
~~影の CSM 化~~ **済** (2026-08-18、既定 4 枚)。

- 3D シーンの窓口 — モデルを置く / 動かす / 消す
- ~~カメラ~~ → **ACS 側へ実装済み** (`CCamera3D`、`acs_temp_doc/0004`)。
  `CCameraStack` は 2D 専用だったので使えず、`FCamera2D` の 3D 版を ACS に足した。
  Frameworkでは`CNodeOrbitCamera3D`が人物などのノードを注視点として追い、明示した回転・距離
  操作をACSの軌道計算へ渡す。場面の描画形状による遮蔽物回避も既定で有効にする。
- ライティング — **ACS側とFramework側へ実装済み**: 光のコンポーネント
  (`ALightComponent3D`) と木から集める層 (`CLightCollector3D`) を描画へ接続し、Frameworkでは
  `CLight3DSpawner`へ方向または位置を渡すだけで置ける。光が無い場面も既定の太陽 + IBL + 影 +
  トーンマップを維持する。`acs_temp_doc/0005` `0006`。
- アニメーション再生 — **ACS側の穴とFramework側の配置手数を埋めた** (2026-08-18、
  簡単配置は2026-08-22)。
  `ASkinnedMeshAsset` / `CAnimationPlayer` / `CSkinnedShader` は在ったのに、
  **FBX から骨付きメッシュを作るローダだけが無かった**。`LoadSkinnedMeshFromFbxMemory`
  として ufbx で実装 (骨・祖先・逆バインド・重み 4 本・クリップを 30Hz で焼く)。
  枠組み側は`CAnimatedModel3DSpawner`へパス、位置、初期クリップを渡すだけで、読み込み、
  識別子付きノード、部品追加、再生まで行う。`CCharacterAnimator3D`へ速度と接地状態を渡せば、
  待機・歩き・走り・ジャンプを速度境界の揺れに強い規則で選び、ローカル骨姿勢を滑らかに繋ぐ。
  描画 (`ASkinnedMeshComponent3D` +
  `DrawSkinnedScene`) も繋がり、**デモで実際に骨が動いている。**
  **質感も揃えた**: CPU で変形して普通のメッシュとして PBR の経路へ流すので、
  IBL・影・遮蔽・反射・霧が静的メッシュとまったく同じに効く (`acs_temp_doc/0022`)。
  何十体も出すなら `CPbrShader` へ GPU スキニングを足すのが次
- ~~3D の当たり判定の窓口~~ → **`Scene/Pick3D` として実装済み** (2026-08-18、
  実形状は2026-08-22)。`FSceneRay::FromScreen`で画面から線を作り、`RaycastGeometry`へ
  場面ごと渡すだけで、球、有限平面、立方体、読み込みメッシュの三角形から最近点と実法線を返す。
  従来の境界箱`Raycast`と、全候補を返す`RaycastAll`も用途別に残す。
  ※ 剛体物理は引き続きACS側の責務とする。球型キャラクター移動はACSへ実装し、Frameworkには
  ノードへ反映する薄い窓口だけを置いた。
- ~~3Dの形状重なりと移動判定~~ → **`Scene/Collision3D`として実装済み** (2026-08-22)。
  ノードへ球、箱、描画境界を1回登録すれば、現在Transformへの同期、レイヤー、自身除外、
  球・箱の重なり列挙、球スイープを`CSceneCollision3D`がまとめる。判定計算と世代付き形状番号は
  ACSの`CCollisionWorld3D`を使い、Frameworkはノード寿命との接続だけを受け持つ。
- ~~3Dキャラクター移動~~ → **ACSと`Scene/Character3D`へ実装済み** (2026-08-22)。
  `TryMoveKinematicCharacter3D`が希望水平速度、状態、経過秒、調整値から、重力、接地、ジャンプ、
  球スイープ、壁沿い移動、貫通解消を固定回数で計算する。`CCharacterMover3D`は球中心をノードから
  読み、成功結果の世界移動量だけを親座標へ戻して反映する。画面の左右・前後操作量は
  `MoveFromCamera`が水平なカメラ基準速度へ変換し、`TurnTowardMovement`が実速度へ滑らかに向ける。
  `CThirdPersonCharacter3D`はこれらと追従カメラ、任意アニメーションを1回の更新へまとめる。
  `FThirdPersonCharacter3DActionSet`により、既存の`FActionInput`を現在・前回の2値だけで移動、視点、
  ズーム、単発ジャンプへ変換できる。失敗時はノードと保持状態を変更しない。

### v0.3 — 見た目と手触り (完了)

**判定基準: DXLib と並べて «明らかに綺麗» と言える。**

- ~~ポストプロセスの窓口 (AO / SSR / SSGI / FXAA)~~ → **配線済み** (2026-08-22)。
  `AmbientOcclusion()` (既定 ON)、`Reflections()` (既定 OFF、映すものが要る)、
  `GlobalIllumination()` (既定 OFF、近くの色の回り込み) を場面から触れる。
  bloom と FXAA (`PostParams().fxaa_enabled`、既定 OFF) は `PostParams()` から調整でき、
  材質も `FModel3DSpawnParams::Metallic` / `Roughness` で触れる
- ~~3D エフェクトの配線 (Effekseer)~~ → **実装済み** (2026-08-21)。
  `AEffect3DScene` を継承し、`PlayEffect3D( 素材名, 位置 )` で再生する。最初の描画前でも
  指定でき、D3D12 の準備後に自動開始する。ACS の HDR 透明3Dパス内で描くため、scene depthで
  隠れ、露出・bloom・tonemapも同じ経路を通る。backend固有型は`CEffect3DPlayer`の実装へ隠した
- ~~遊ぶ人向け UI の土台 (`UiLayer`、`Widget`)~~ → **配線済み** (2026-08-22)。
  `AUi3DScene` を継承し、`Ui().AddText` / `Ui().AddButton` で置く。UIの初期化、入力、更新、
  終了は場面寿命へまとまり、HDR、tonemap、TAAまたはFXAAが終わった後のLDR画面へ描く。
  そのため文字はbloomで光らずFXAAでぼけない。`AEffect3DScene`もこの基底を含むので、3D演出と
  UIを同じ場面で使える。複雑な画面はACSの`AWidget` / `AAnchorPanel` / 標準widget群を直接使う
- ~~3D 素材の置き場と取り込み手順~~ → **決定・実装済み** (2026-08-18)。
  置き場は `Assets`、形式は **FBX** (`.gltf` `.glb` `.obj` も通る)。
  `CModelLibrary` が置き場を探して読み、`CModel3DSpawner::SpawnInto(..., Library)` が置く。
  実行ファイルから上へ辿って `Assets` を探すので、**素材をコピーせずに IDE からも
  出来上がりからも動く**。`Assets/README.md` と `Assets/Models/README.md` に規約

### v0.4 — 残りと品質

- 訳文のファイル読み込み
- 入力の再割り当て (UI + 保存)
  - ~~キーボードのボタン操作~~ → **実装済み** (2026-08-22)。`FActionKeyRebindState`で
    入力待ち・確定・取消を決定論的に処理し、`ReplaceKeyBinding`でゲームパッド割り当てを
    維持したまま差し替える。Demo3DでUI操作、設定保存、次回起動時の復元まで確認できる
  - 残り: ゲームパッドのボタンと軸を選ぶUI
- ~~位置のある効果音の左右定位~~ → **実装済み** (2026-08-22)。Engine発行の音源番号、
  距離減衰、要求音量、再生速度を同じvoiceへ合成し、モノラル素材の左右位置をXAudio2へ渡す。
  Demo3Dのボタンで左・右を交互に実行できる
- ~~対話できる3D水面の配置~~ → **実装済み** (2026-08-22)。`CWater3DSpawner`へ位置と広さを
  渡すだけで、ACSの屈折、反射、泡、波紋を使う識別子付き水面を置ける。高度な描画実装を
  複製せず、値の検証とシーンへの接続だけをFrameworkが受け持つ
- ~~3D天候の遷移と描画接続~~ → **実装済み** (2026-08-22)。`AWeather3DScene`で
  `SetWeather( EWeatherKind::Storm, 2.5f )`と書くだけで、ACSの天候状態を雲量、雲影、霧、
  空色、IBL環境光へ同じ遷移率で反映する。雨雪の素材は作品ごとに選べるよう、降水密度と
  風向きだけを公開する
- シーン保存でノード名を残す ※ACS の形式 v4 に名前欄が無い
- エラーとログの方針統一、スレッド安全性の明示、セーブ互換性の方針

### v0.5 — 配れる形

**判定基準: 別マシンで clone → 1 コマンド → ビルド → サンプルが動く。**

- ~~`LICENSE` (Apache-2.0) と、トップの `README.md`~~ **済** (2026-08-18)
- ~~取得スクリプトと sha256 の照合~~ **済**: `Tools\FetchAcs.ps1` + `Toolscs-version.json`。
  配布物は `ThirdPartycs` へ入り、**環境変数も /p: も無しでビルドが通る**ことを確認済み
  (Debug/Release、単体テスト 538 件、サンプル起動)
- ~~入門ドキュメントと 3D の最小サンプル~~ **済**: トップ README とデモ 1 本
- **残り: ACS 側の GitHub Release を実際に publish する。** いまは `-FromLocal` で
  ローカルビルドから持ってくる道しか通っていない。`acs-version.json` の `sha256` が空なのは
  そのため (資産が無いので照合対象が無い)
- 残り: CI (Windows x64: 取得 → ビルド → テスト)
- 残り: 対応プラットフォームの宣言 (Windows x64 のみ)

### v1.0.0 — 凍結

- API を固定し、破壊的変更は版を上げると宣言する
- `CHANGELOG.md` を置き、以後の版を追えるようにする

---

## 決まっていること

| 項目 | 決定 | 決めた日 |
|---|---|---|
| 目指すもの | 3D。2D は当たり前として売りにしない | 2026-08-17 |
| 使いやすさの目標 | DXLib と同等以上 | 2026-08-17 |
| 設計思想 | ACS 独自のものに従う (DXLib を真似るのは手数だけ) | 2026-08-17 |
| v1.0.0 の意味 | 他人が使える形で配れる | 2026-08-17 |
| ACS 配布物 | GitHub Releases + 取得スクリプト | 2026-08-17 |
| 3D の剛体物理 | ACS 側へ入れる (枠組みには書かない) | 2026-08-17 |
| 3D カメラ (追従・揺れ) | ACSの`CCamera3D`とFrameworkの`CNodeOrbitCamera3D`を実装済み | 2026-08-22 |
| 3D の光 | ACS 側へ実装済み (`ALightComponent3D`) | 2026-08-17 |
| 光を集める層 | ACS 側へ実装済み (`CLightCollector3D`) | 2026-08-17 |
| 3D の光を置く窓口 | Framework側へ実装済み (`CLight3DSpawner`) | 2026-08-22 |
| 追う ACS のブランチ | **`dev`** (main は ABI ガードが逆で使えない) | 2026-08-17 |
| 配布物の作り方 | `.\Tools\UpdateAcsDist.ps1` で dev の worktree からビルドして配置 | 2026-08-17 |
| 対応する配布物 | dev から生成したもの (`C:\acs_dev`)。世代差は `Source/Common/Compat/` が吸収 | 2026-08-17 |
