# 公開API一覧

この文書は、ゲーム側が直接利用する3D入口の監査対象をまとめた正本である。
`AcsFramework_Core/AcsFramework.h`はここに記載したヘッダーを再公開し、
`Tools/RunPublicApiAudit.ps1`がヘッダーの存在、再公開、宣言の実在を確認する。

一覧へ追加するAPIは、利用側が呼ぶ入口または設定・結果として必要な型に限る。
`private`、`protected`の内部処理、`Foo_Internal`、実装用のSpawner内部型は含めない。

## 3D場面の統合入口

| 種別 | ヘッダー | 宣言 |
|---|---|---|
| 基底場面 | `AcsFramework_Core/UI/Ui3DScene.h` | `AUi3DScene` |
| 見た目 | `AcsFramework_Core/UI/Ui3DScene.h` | `TryApplyVisualPreset3D` |
| ノード | `AcsFramework_Core/UI/Ui3DScene.h` | `SpawnNode3D` |
| 地面 | `AcsFramework_Core/UI/Ui3DScene.h` | `SpawnGround3D` |
| 箱 | `AcsFramework_Core/UI/Ui3DScene.h` | `SpawnBlock3D` |
| 球 | `AcsFramework_Core/UI/Ui3DScene.h` | `SpawnSphere3D` |
| 部屋 | `AcsFramework_Core/UI/Ui3DScene.h` | `SpawnRoom3D` |
| 通路 | `AcsFramework_Core/UI/Ui3DScene.h` | `SpawnCorridor3D` |
| 開口壁枠 | `AcsFramework_Core/UI/Ui3DScene.h` | `SpawnDoorway3D` |
| 柵 | `AcsFramework_Core/UI/Ui3DScene.h` | `SpawnFence3D` |
| 階段 | `AcsFramework_Core/UI/Ui3DScene.h` | `SpawnStairs3D` |
| 静的モデル | `AcsFramework_Core/UI/Ui3DScene.h` | `SpawnModel3D` |
| 衝突付きモデル | `AcsFramework_Core/UI/Ui3DScene.h` | `SpawnCollidableModel3D` |
| 骨格モデル | `AcsFramework_Core/UI/Ui3DScene.h` | `SpawnAnimatedModel3D` |
| 画像板 | `AcsFramework_Core/UI/Ui3DScene.h` | `SpawnImage3D` |
| ビルボード | `AcsFramework_Core/UI/Ui3DScene.h` | `SpawnBillboard3D` |
| 光源 | `AcsFramework_Core/UI/Ui3DScene.h` | `SpawnLight3D` |
| 被写体用3点照明 | `AcsFramework_Core/UI/Ui3DScene.h` | `SpawnStudioLightRig3D`、`DestroyStudioLightRig3D` |
| 水面 | `AcsFramework_Core/UI/Ui3DScene.h` | `SpawnWater3D` |
| 衝突破棄 | `AcsFramework_Core/UI/Ui3DScene.h` | `DestroyCollidableModel3D` |
| 画面選択 | `AcsFramework_Core/UI/Ui3DScene.h` | `PickScreen3D` |
| 近接判定 | `AcsFramework_Core/UI/Ui3DScene.h` | `BindProximityTrigger3D` |
| チェックポイント接続 | `AcsFramework_Core/UI/Ui3DScene.h` | `BindCheckpoint3D` |
| チェックポイント生成 | `AcsFramework_Core/UI/Ui3DScene.h` | `SpawnCheckpoint3D` |
| チェックポイント破棄 | `AcsFramework_Core/UI/Ui3DScene.h` | `DestroyCheckpoint3D` |
| チェックポイント描画 | `AcsFramework_Core/UI/Ui3DScene.h` | `DrawCheckpoint3D` |
| 衝突描画 | `AcsFramework_Core/UI/Ui3DScene.h` | `DrawCollisionShape3D` |

## 3D配置の設定と結果

| 種別 | ヘッダー | 宣言 |
|---|---|---|
| 地面設定 | `AcsFramework_Core/Scene/Ground3D/Ground3DSpawnParams.h` | `FGround3DSpawnParams` |
| 箱設定 | `AcsFramework_Core/Scene/Block3D/Block3DSpawnParams.h` | `FBlock3DSpawnParams` |
| 球設定 | `AcsFramework_Core/Scene/Sphere3D/Sphere3DSpawnParams.h` | `FSphere3DSpawnParams` |
| 部屋設定 | `AcsFramework_Core/Scene/Room3D/Room3DSpawnParams.h` | `FRoom3DSpawnParams` |
| 通路設定 | `AcsFramework_Core/Scene/Corridor3D/Corridor3DSpawnParams.h` | `FCorridor3DSpawnParams` |
| 開口壁枠設定 | `AcsFramework_Core/Scene/Doorway3D/Doorway3DSpawnParams.h` | `FDoorway3DSpawnParams` |
| 柵設定 | `AcsFramework_Core/Scene/Fence3D/Fence3DSpawnParams.h` | `FFence3DSpawnParams` |
| 階段設定 | `AcsFramework_Core/Scene/Stairs3D/Stairs3DSpawnParams.h` | `FStairs3DSpawnParams` |
| 衝突形状設定 | `AcsFramework_Core/Scene/Collision3D/CollisionShape3DParams.h` | `FCollisionShape3DParams` |
| 衝突結果 | `AcsFramework_Core/Scene/Model3D/CollidableModel3DSpawnResult.h` | `FCollidableModel3DSpawnResult` |
| 部屋結果 | `AcsFramework_Core/Scene/Room3D/Room3DSpawnResult.h` | `FRoom3DSpawnResult` |
| 通路結果 | `AcsFramework_Core/Scene/Corridor3D/Corridor3DSpawnResult.h` | `FCorridor3DSpawnResult` |
| 開口壁枠結果 | `AcsFramework_Core/Scene/Doorway3D/Doorway3DSpawnResult.h` | `FDoorway3DSpawnResult` |
| 柵結果 | `AcsFramework_Core/Scene/Fence3D/Fence3DSpawnResult.h` | `FFence3DSpawnResult` |
| 階段結果 | `AcsFramework_Core/Scene/Stairs3D/Stairs3DSpawnResult.h` | `FStairs3DSpawnResult` |
| 衝突集合 | `AcsFramework_Core/Scene/Collision3D/SceneCollision3D.h` | `CSceneCollision3D` |
| チェックポイント | `AcsFramework_Core/Scene/Checkpoint3D/Checkpoint3D.h` | `CCheckpoint3D` |
| チェックポイント生成器 | `AcsFramework_Core/Scene/Checkpoint3D/Checkpoint3DSpawner.h` | `CCheckpoint3DSpawner` |
| チェックポイント設定 | `AcsFramework_Core/Scene/Checkpoint3D/Checkpoint3DParams.h` | `FCheckpoint3DParams` |
| チェックポイント生成結果 | `AcsFramework_Core/Scene/Checkpoint3D/Checkpoint3DSpawnResult.h` | `FCheckpoint3DSpawnResult` |
| チェックポイント更新結果 | `AcsFramework_Core/Scene/Checkpoint3D/Checkpoint3DUpdateResult.h` | `FCheckpoint3DUpdateResult` |
| チェックポイント順序 | `AcsFramework_Core/Scene/Checkpoint3D/CheckpointRoute3D.h` | `FCheckpointRoute3D` |
| チェックポイント順序設定 | `AcsFramework_Core/Scene/Checkpoint3D/CheckpointRoute3DParams.h` | `FCheckpointRoute3DParams` |
| チェックポイント順序結果 | `AcsFramework_Core/Scene/Checkpoint3D/CheckpointRoute3DAdvanceResult.h` | `FCheckpointRoute3DAdvanceResult` |
| チェックポイント順序進行 | `AcsFramework_Core/Scene/Checkpoint3D/CheckpointRoute3DProgress.h` | `FCheckpointRoute3DProgress` |
| 見た目設定 | `AcsFramework_Core/Scene/Visual3D/VisualPreset3D.h` | `EVisualPreset3D` |
