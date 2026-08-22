# 3Dキャラクターアニメーション

`AUi3DScene::SpawnAnimatedModel3D`で、骨付きFBXの読み込み、ノード作成、部品追加、初期アニメーション再生を
1回の呼び出しへまとめる。衝突も必要なら`SpawnCollidableAnimatedModel3D`が登録と失敗時の巻き戻しまで
一括で行う。asset窓口を直接扱う場面では`CAnimatedModel3DSpawner`も使える。
さらに、移動速度と接地状態だけで待機・歩き・走り・ジャンプを選び、姿勢を滑らかに切り替えられる。

```cpp
FAnimatedModel3DSpawnParams Hero = FAnimatedModel3DSpawnParams::FromModel(
    FStringView( "Models/Hero.fbx" ), FVec3{ 0.0f, 0.0f, 3.0f } );
Hero.InitialAnimation = FStringView( "Idle" );
Hero.Scale = FVec3{ 0.01f, 0.01f, 0.01f };

const FCollidableModel3DSpawnResult HeroSpawn = SpawnCollidableAnimatedModel3D(
    Hero, FCollisionShape3DParams::FromSphere( FVec3{ 0.0f, 0.9f, 0.0f }, 0.45f, 0x1u ) );
ANode* const HeroNode = HeroSpawn.Node;

// HeroAnimatorはHeroNodeより長生きしない場所で所有する。
CCharacterAnimator3D HeroAnimator;
if ( HeroNode != nullptr ) HeroAnimator.Bind( *HeroNode );

// 毎フレーム、移動処理で求めた速度と接地状態だけを渡す。
HeroAnimator.Update( FCharacterAnimation3DInput{ HorizontalSpeed, bGrounded } );
```

`MeshAsset`へ読み込み済みの`ASkinnedMeshAsset`を渡せば、`CModelLibrary`なしでも置ける。
指定名のクリップが無い、モデルに頂点・index・骨が無い、数値が有限でない場合は、
半端なノードを残さず`nullptr`を返す。

`SpawnCollidableAnimatedModel3D`は成功時に`Node`と`Shape`を返す。登録できなければ生成ノードを
破棄予定へ戻し、「動いて見えるが当たらない」状態を成功として残さない。読込時の描画境界も
選べるが、手足が大きく動く人物では`FCollisionShape3DParams::FromBox`または`FromSphere`で
ゲーム用の安定した形状を明示する。

`FCharacterAnimation3DProfile`で4つのクリップ名、歩きと走りの開始・終了速度、姿勢を混ぜる秒数を
変更できる。開始と終了に別の速度を使うため、速度が境界付近で揺れても状態が細かく往復しない。
進行中の切替へ次の切替を重ねず、完了後の更新で自動的に再試行する。

`Input + CurrentState + Profile -> NextState`は描画から独立した値計算として検証する。実際の姿勢計算と
描画はACSの`ASkinnedMeshComponent3D`が所有し、`CCharacterAnimator3D`は利用側が所有する薄い接続層に
留めるため、subsystemにはしない。
