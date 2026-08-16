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
| 入力の割り当て | `CActionBindingTable`、`CBoundActionSource` |
| 画面・フェード・ロード中・ポーズ | `CScreenSubsystem`、`CFadeSubsystem`、`CLoadingScreenSubsystem`、`CPauseScreenSubsystem` |
| ノード生成・シーン保存 | `CPrefabSubsystem`、`CSceneSnapshotSubsystem` |
| 多言語 | `CLocalizationSubsystem` |
| 決定性 (記録と再生) | `CSimulationSubsystem` |
| 開発支援 | `CPerfBudgetSubsystem`、`CDevConsoleSubsystem`、`CHotReloadSubsystem`、`DebugTop` |

### 覆えていないもの (v1.0.0 で埋める)

| 分野 | ACS 側に在るもの |
|---|---|
| **3D 描画** | `MeshComponent3D`、`CameraComponent3D`、`Transform3D`、`PbrShader`、`StandardShader`、`ShadowMap`、`Ibl`、`Sky`、`Atmosphere`、`SceneRenderResources` |
| **ポストプロセス** | `PostProcess`、`Ssao`、`Ssgi`、`Ssr`、`Fxaa`、`SubsurfaceScattering`、`MotionVector`、`TemporalHistory`、`HiZ` |
| **UI (遊ぶ人向け)** | `ui/UiRenderer`、`ui/Widget`、`ui/Widgets`、`gameframework/UiLayer` |
| **エフェクト** | `EffectSystem`、`ParticleEffectSystem`、`render/Particles`、同梱の Effekseer |
| **アニメーション** | `AnimationGraph`、`AnimationCurve`、`SkinnedShader`、`asset/SkinnedMesh` |
| **3D の当たり判定** | `collision/MeshCollider`、`collision/ConvexHull3`、`math/Collision3D` |
| 水面・天候 | `WaterSurface3D`、`WaterSurface3DComponent`、`WeatherSystem` |

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

### v0.2 — 3D の入口 (いまここ)

**判定基準: 何も設定せずに数十行で、モデルが綺麗に映って動く。**

- 3D シーンの窓口 — モデルを置く / 動かす / 消す
- ~~カメラ~~ → **ACS 側へ実装済み** (`FCamera3D`、`acs_temp_doc/0004`)。
  `CCameraStack` は 2D 専用だったので使えず、`FCamera2D` の 3D 版を ACS に足した。
  枠組みからは配布物の再生成後に使える
- ライティング — **光のコンポーネントは ACS 側へ実装済み** (`ALightComponent3D`、
  `acs_temp_doc/0005`)。次は木から光を集めて `CPbrShader` へ渡す層。
  そのうえで «何もしなくても綺麗» な既定 (太陽 + IBL + 影 + トーンマップ) を枠組みが持つ
- アニメーション再生の窓口 (`AnimationGraph`)
- 3D の当たり判定の窓口 (`MeshCollider`、`Collision3D` のレイ判定・重なり判定)
  ※ **剛体物理とキャラ移動は ACS 側に入れる** (2026-08-17 判断、`acs_temp_doc/0003`)。
  枠組みには書かない。ACS に入った後で窓口だけを足す。

### v0.3 — 見た目と手触り

**判定基準: DXLib と並べて «明らかに綺麗» と言える。**

- ポストプロセスの窓口 (bloom / AO / SSR / FXAA)。**既定で入っていること**が要
- エフェクトの配線 (`ParticleEffectSystem` と Effekseer)
- 遊ぶ人向け UI の土台 (`UiLayer`、`Widget`)
- 3D 素材の置き場と取り込み手順

### v0.4 — 残りと品質

- 訳文のファイル読み込み
- 入力の再割り当て (UI + 保存)
- 位置のある音の pan ※**ACS 本体への変更が要る** (`acs_temp_doc/0001`)
- シーン保存でノード名を残す ※ACS の形式 v4 に名前欄が無い
- エラーとログの方針統一、スレッド安全性の明示、セーブ互換性の方針

### v0.5 — 配れる形

**判定基準: 別マシンで clone → 1 コマンド → ビルド → サンプルが動く。**

- `LICENSE` (Apache-2.0) と、トップの `README.md`
- ACS 配布物を GitHub Releases へ置き、取得スクリプトと sha256 の照合を用意する
- CI (Windows x64: 取得 → ビルド → テスト)
- 入門ドキュメントと、3D の最小サンプル 1 本
- 対応プラットフォームの宣言 (Windows x64 のみ)

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
| 3D カメラ (追従・揺れ) | ACS 側へ実装済み (`FCamera3D`) | 2026-08-17 |
| 3D の光 | ACS 側へ実装済み (`ALightComponent3D`) | 2026-08-17 |
| 追う ACS のブランチ | **`dev`** (main は ABI ガードが逆で使えない) | 2026-08-17 |
| 配布物の作り方 | `.\Tools\UpdateAcsDist.ps1` で dev の worktree からビルドして配置 | 2026-08-17 |
| 対応する配布物 | dev から生成したもの (`C:\acs_dev`)。世代差は `Source/Common/Compat/` が吸収 | 2026-08-17 |
