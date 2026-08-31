# 変更履歴

ACS Frameworkの利用側から見える追加、変更、修正を記録する。
現在の開発版は`0.5.0-dev`で、公開済みの正式版はまだない。

## [未公開]

### 追加

- 3Dモデル、骨格モデル、画像板、ビルボード、ライト、水面を場面から一呼出しで配置する窓口
- 表示面と歩ける厚み付き衝突を同じ尺度で置き、形状番号を保ったまま同期更新する`SpawnGround3D` / `TryUpdateGround3D`
- 表示用立方体と箱型衝突を同じローカル寸法で置き、形状番号を保ったまま同期更新する`SpawnBlock3D` / `TryUpdateBlock3D`
- 表示用球と球型衝突を同じ半径で置き、形状番号を保ったまま同期更新する`SpawnSphere3D` / `TryUpdateSphere3D`
- 歩ける床と四方の壁を途中失敗時の巻き戻し込みで置き、5形状の番号を保ったまま同期更新する`SpawnRoom3D` / `TryUpdateRoom3D`
- 段数と1段の寸法から、隙間のない衝突付き階段を4方向へ置く`SpawnStairs3D`
- 入口、内幅、長さから、床と側壁2枚を持つ両端が開いた通路を4方向へ配置・同期更新する`SpawnCorridor3D` / `TryUpdateCorridor3D`
- 幅、長さ、柵高から、床板と両側柵を持つ衝突付き橋を4方向へ置く`SpawnBridge3D`
- 壁と開口の寸法から、見えない衝突を残さない左右柱と上枠を配置・同期更新する`SpawnDoorway3D` / `TryUpdateDoorway3D`
- 長さ、高さ、最大支柱間隔から、両端支柱と水平な横桟を4方向へ置く`SpawnFence3D`
- 衝突付き3D生成結果のノードと形状を対で片付ける`DestroyCollidableModel3D`
- ヘアライン金属へACSの異方性反射を短い指定で適用する`FromBrushedMetalPrimitive`
- 布、フェルト、ベルベットへACSの毛羽反射を短い指定で適用する`FromFabricPrimitive`
- 肌、蝋、乳白素材へACSの内部散乱を短い指定で適用する`FromSubsurfacePrimitive`
- ACSの24時間補間から太陽、空、IBL環境光を同時に動かす`EnableTimeOfDay3D`
- 自己発光球と同色の点光源を位置だけで同期配置・一括破棄する`SpawnLamp3D`
- 配置済みランプの位置、半径、色、発光と照明を片側だけずらさず変える`TryUpdateLamp3D`
- 床位置だけで衝突付き金属ポスト、発光球、点光源を一括配置・同期更新・破棄する`SpawnStreetLamp3D` / `TryUpdateStreetLamp3D`
- 既存の太陽を保ち、中心・見る方向・半径からキー、フィル、リムを配置・同期更新する`SpawnStudioLightRig3D` / `TryUpdateStudioLightRig3D`
- 3D衝突、近接トリガー、視線操作、第三者視点キャラクター、追従カメラの接続層
- 指定した1形状の進入を一度限りまたは再進入ごとに発火する`SpawnCheckpoint3D`
- 発火番号を順番どおりに受理し、複数周の完了を決定論的に返す`FCheckpointRoute3D`
- チェックポイント数と周回数の不一致を拒否して順序ルートを保存・復元する`FCheckpointRoute3DProgress`
- 明示経過秒と受理済み通過結果から区間・周回・合計タイムを再現可能に計測する`FCheckpointRoute3DTimer`
- 有限性と時間の大小関係を検証して計測を原子的に保存・復元する`FCheckpointRoute3DTimerState`
- PBR、自己発光、光沢コート、トゥーン陰影、見た目プリセットを短い指定で使う3D材質窓口
- 天候、空気遠近、Effekseer、3D音響、3D位置文字、照準、選択輪郭、3Dデバッグ描画
- 通常の場面更新で、実機、AI、再生入力の現在・前回・押下・解放・軸を共通利用する`CActionInputTracker`
- メニューや演出中に許可したアクションと軸だけを履歴ごと通し、複数制限も合成する`FActionInputMask`
- 入れ子になった入力制限を最大8層まで重ね、LIFO解除と途中復元を行う`FActionInputMaskStack`
- 必要操作と禁止操作を組み合わせ、最後の必要操作を押した瞬間だけ同時押しを有効化する`FActionChord`
- 最大8操作の順番と間隔を追跡し、誤入力、同時押し、途中保存を決定論的に扱う`FActionCommandSequenceTracker`
- アナログ1軸と2軸の遊びを除き、最大入力と方向を保って応答曲線を適用する`FActionAxisResponse`
- 2軸入力を開始・解除閾値付きの4方向または8方向へ変換する`FActionDirectionQuantizer`
- 2軸の離散方向を保持し、今回だけの開始・方向変更・解除と途中復元を扱う`FActionDirectionTracker`
- 着地や硬直終了の直前に押した操作を短時間保持し、受理時に1回だけ消費する`FActionInputBuffer`
- 押下猶予の設定と残り時間を検証付きで保存・原子的復元する`FActionInputBufferState`
- 明示時間から短押し、長押し閾値到達、長押し解放を1回ずつ判定し、途中状態も復元できる`FActionHoldTracker`
- 押下直後と長押し後の一定間隔を発火回数へ変え、追い付き上限超過と途中状態を保持する`FActionRepeatTracker`
- 押し続けを重複させず、明示した押下間隔と回数でダブルタップや複数回タップを判定する`FActionTapSequenceTracker`
- 押下開始だけで有効・無効を反転し、通常入力と固定ステップ入力へ接続する`FActionToggle`
- 使用成功から再使用可能までを明示時間で進め、進行率と途中状態を保存できる`FGameplayCooldown`
- 上限付き整数チャージを消費し、複数回の追い付き、一時停止、手動補充、途中保存に対応する`FGameplayChargePool`
- 一定間隔の到達回数を上限付きで返し、超過ぶんを失わず保存復元できる`FGameplayInterval`
- 上限内の現在値を全量消費・回復し、途中状態を原子的に保存復元できる局所値型`FGameplayResource`
- 制限時間や状態持続時間を開始・一時停止・再開し、完了通知と途中状態を保存できる`FGameplayTimer`
- 0〜1の成功確率を32bit乱数1個で判定し、確定値と不正入力では乱数位置を変えない`CDeterministicRandom::TryChance`
- 有限な非負重みから53bit相当で添字を1つ選び、不正入力では乱数位置と出力を変えない`CDeterministicRandom::TryChooseWeightedIndex`
- 候補数から均等な添字を選び、棄却した出目も消費数へ含める`CDeterministicRandom::TryChooseIndex`
- 任意の配列を棄却法で偏りなく並べ、消費乱数と途中再生を一致させる`CDeterministicRandom::TryShuffle`
- 原点中心のローカル3D箱内部へ各軸の長さに対して均等なoffsetを作る決定論的抽選
- 有限かつ非退化な3D三角形へ、面積に対して均等な位置を作る決定論的抽選
- 任意長の法線を持つ原点中心の3D円盤内部へ、面積に対して均等なoffsetを作る決定論的抽選
- 任意長の軸を持つ原点中心の3D円柱内部へ、体積に対して均等なoffsetを作る決定論的抽選
- 中央円柱と両端半球を体積比で選び、原点中心の3Dカプセル内部へ均等なoffsetを作る決定論的抽選
- Y軸を上下とする3D球面と球内部から、面積・体積に対して均等な原点相対位置を選ぶ決定論的抽選
- 任意長の中心軸と度単位の半角から、3D円錐の立体角に対して均等な単位方向を選ぶ決定論的抽選
- 固定ステップの途中復元で長押しを再発火させない、入力履歴付き`CSimulationSnapshot` v2
- 入力再割り当て、保存、シーンスナップショット、多言語、固定更新、記録再生、開発支援
- ローカルとGitHub Actionsで共有するDebug/Release完全検証コマンド
- 機械可読な現在版と、v1.0.0以降の公開API互換性規則

### 変更

- 公開実行時APIの失敗、ログ、スレッド、セーブ互換性の既定契約を正本文書へ統一
- 対応環境をWindows x64、MSVC v145、DirectX 12として明文化

### 修正

- 同じPowerShellプロセスでDebug/Releaseを連続検証するとMSVC環境が重複し、
  Windowsのコマンド行上限を超える問題を修正
- 単体テストのソース追加で`cl.exe`のコマンド行上限を超える問題を、UTF-16応答ファイルで修正
