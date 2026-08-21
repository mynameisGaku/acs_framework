# Water3D

ACSの屈折、反射、泡、波紋を使う3D水面を少ない指定で置く。

```cpp
FWater3DSpawnParams Water;
Water.Position = FVec3{ 2.5f, 0.1f, -1.0f };
Water.Size = FVec2{ 4.0f, 3.0f };

ANode* const Surface = CWater3DSpawner::SpawnInto( Graph(), Water );
if ( Surface != nullptr )
{
	AddWaterDisturbance( Surface->Id(), Water.Position, 0.24f, 0.30f );
}
```

`FWater3DSpawnParams`は位置、広さ、主要な見た目だけを持つ検証可能な値で、描画機能や
時刻には依存しない。`CWater3DSpawner`は平面メッシュと`AWaterSurface3DComponent`を
シーンの識別子管理へ接続するだけで、状態を持たない。

水面はローカルXZ平面として置かれる。戻り値の`FNodeId`は同じ場面の
`AddWaterDisturbance`、`AddWaterWake`、`ActiveWaterRippleCount`へ渡せる。
高度な吸収、散乱、泡の色を調整するときは、戻り値から
`GetComponent<AWaterSurface3DComponent>()`を取得してACSの設定を直接変更する。

水面の描画、背景の屈折、SSR、深度との前後関係、波紋の寿命はACSが所有する。
このフォルダには描画器や常駐サブシステムを置かない。
