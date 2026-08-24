# 3Dチェックポイント

`CCheckpoint3D`は、登録済みの1つの3D衝突形状が球または箱へ進入した瞬間を返す。
ゴール、復帰地点、区間計測、イベント開始位置など、追跡対象が決まっている判定に使う。

```cpp
// CheckpointとSpawnedは場面のメンバーとして保持する。
Spawned = SpawnCheckpoint3D(
    Checkpoint, Player.Shape, FVec3{ 0.0f, 1.0f, 12.0f }, 2.0f, PlayerLayer );

FCheckpoint3DUpdateResult Result;
if ( Checkpoint.Update( Result ) && Result.bActivatedThisUpdate )
{
    SaveRespawnPoint();
}
DrawCheckpoint3D( Checkpoint );
```

複数地点を決めた順番で通るレースや周回コースでは、各チェックポイントの発火番号だけを
`FCheckpointRoute3D`へ渡す。ルートはチェックポイント自体を所有しないため、場面メンバーの
寿命や配置方法を変えずに追加できる。

```cpp
FCheckpointRoute3D Route;
Route.SetParams( FCheckpointRoute3DParams::ForCheckpoints( 3u, 2u ) );
FCheckpointRoute3DTimer Timer;
Timer.Start();

// 毎更新。停止中と完了後は有効な時間を渡しても進まない。
Timer.Tick( DeltaSeconds );

FCheckpoint3DUpdateResult CheckpointState;
if ( Checkpoints[Index].Update( CheckpointState )
    && CheckpointState.bActivatedThisUpdate )
{
    FCheckpointRoute3DAdvanceResult RouteState;
    if ( Route.Advance( Index, RouteState ) )
    {
        FCheckpointRoute3DTimingResult Timing;
        if ( Timer.RecordAdvance( RouteState, Timing )
            && Timing.bRouteCompletedThisAdvance )
        {
            FinishRace( Timing.TotalElapsedSeconds );
        }
    }
}
```

範囲内だが次の番号と異なる発火は`bOutOfOrder`として返し、進行を変えない。周末では
`bLapCompletedThisAdvance`、必要周回数へ到達した瞬間だけ`bRouteCompletedThisAdvance`が
trueになる。複数周では各`CCheckpoint3D`の`bActivateOnce`をfalseにし、退出後の再進入を
発火させる。完了後の発火は正常な無変更結果で、`Reset`すると同じ設定の先頭から再開する。
`CaptureProgress`はチェックポイント数、必要周回数、次番号、完了周回数をまとめ、
`RestoreProgress`は現在の設定と一致する有効値だけを復元する。チェックポイント数・周回数が異なる
保存値や矛盾した完了値では現在の進行を変えない。

`FCheckpointRoute3DTimer`は時計を内部から読まず、呼出側が渡す有限・非負の経過秒だけを加算する。
受理済みの`FCheckpointRoute3DAdvanceResult`を`RecordAdvance`すると、計測開始からの合計、現在周回、
前回受理地点からの区間秒を同じ結果で返す。順番違い、未受理、矛盾した進行結果は区間境界にせず、
一時停止中は時間を進めない。最終通過を記録すると自動停止し、新しい計測は`Reset`後に開始する。
`CaptureState`は3つの時間と実行・完了状態を`FCheckpointRoute3DTimerState`へまとめる。
`RestoreState`は有限かつ`区間 <= 周回 <= 合計`の保存値だけを復元し、不正値では現在値を変えない。
ルート進行と計測を同じ時点から再開する場合は、`FCheckpointRoute3DProgress`と対で保存・復元する。

既定の`bActivateOnce`はtrueで、最初の進入だけ発火する。falseなら、一度範囲外へ出た後の
再進入でも発火する。`ResetActivation`は発火済みと範囲内の記録を両方消すため、対象が現在
範囲内でも次の成功更新を新しい進入として扱う。

`SpawnCheckpoint3D`は識別子付きの範囲基準ノードを生成し、既存の`CProximityTrigger3D`と
対象形状へ接続する。接続に失敗した場合は生成ノードを破棄予定へ戻し、半端な結果を残さない。
既存ノードを基準にする場合は`BindCheckpoint3D`を使う。`DestroyCheckpoint3D`は一括生成した
基準ノードと接続だけを片付け、追跡対象のノードと衝突形状は残す。生成結果は場面、衝突集合、
チェックポイント、ノード番号、接続世代を保持するため、別の所有者から無関係なノードを
破棄しない。対象形状が先に外れた場合や基準ノードが既に破棄予定の場合も、正しい生成結果なら
残った接続と結果を後始末できる。同じチェックポイントを再接続した場合も各生成結果を独立して
片付けられるが、現在接続中の基準が古い基準の子なら、親子を巻き込まないよう子側から破棄する。
生成時または再接続後の追跡対象を基準配下へ付け替えた場合も、対象を場面へ残すため
基準の破棄を拒否する。
設定の`CollisionMask`が対象形状のレイヤーと交わらない場合は、発火不能な接続を作らず失敗する。

チェックポイントは時間、入力装置、コールバック、描画を所有しない。呼出側が`Update`の
明示結果から保存、場面遷移、演出を決める。判定は既存の`CSceneCollision3D`へ登録済みの
世代付き形状番号を使うため、別ノードへの付け替え、対象破棄、場面グラフ差し替えでは古い接続を
解除する。GPUなしの単体テストでは、対象ノード位置を変えるだけで進入、滞在、退出、再発火を
再現できる。
