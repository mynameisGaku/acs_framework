# 3Dアニメーションモデル

骨付きFBXの読み込み、ノード作成、部品追加、初期アニメーション再生を1回の呼び出しへまとめる。

```cpp
FAnimatedModel3DSpawnParams Hero = FAnimatedModel3DSpawnParams::FromModel(
    FStringView( "Models/Hero.fbx" ), FVec3{ 0.0f, 0.0f, 3.0f } );
Hero.InitialAnimation = FStringView( "Idle" );
Hero.Scale = FVec3{ 0.01f, 0.01f, 0.01f };

ANode* const HeroNode = CAnimatedModel3DSpawner::SpawnInto(
    Graph(), Hero, Assets->Models() );
```

`MeshAsset`へ読み込み済みの`ASkinnedMeshAsset`を渡せば、`CModelLibrary`なしでも置ける。
指定名のクリップが無い、モデルに頂点・index・骨が無い、数値が有限でない場合は、
半端なノードを残さず`nullptr`を返す。

姿勢計算と描画はACSの`ASkinnedMeshComponent3D`が所有する。Framework側は場面を跨ぐ状態を
持たないため、subsystemにはしない。
