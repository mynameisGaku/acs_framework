# 3Dライト

`CLight3DSpawner`は、ACSの`ALightComponent3D`をノードへ付けてシーンに置くまでを1回にまとめる。
描画、上限管理、視点に近い点光源の選択はACS側の`CLightCollector3D`へ任せ、この層では
ライティング計算を持たない。

```cpp
CLight3DSpawner::SpawnInto( Graph(), FLight3DSpawnParams::Sun( FVec3{ -0.47f, 0.58f, 0.66f } ) );

CLight3DSpawner::SpawnInto( Graph(), FLight3DSpawnParams::Point( FVec3{ 0.0f, 2.0f, 0.0f }, 8.0f ) );
```

平行光の`DirectionToLight`は、光が進む向きではなく、面から光源へ向かう向きである。
長さそのものは明るさに影響せず、配置時に正規化する。点光源は`Position`と`Range`を使う。

いずれも親を渡した場合は親ノード内の位置・方向として扱う。シーン全体の太陽は、通常どおり
ルート直下へ置けばワールド空間と一致する。

色は線形空間のRGBで、1より大きい値も使える。`Intensity`を0にすると、ノードを残したまま
ACSの収集対象から外せる。負値、NaN、無限大、演算時に溢れる色と強さ、正規化できない太陽方向、
0以下の点光源距離は、半端なノードを残さず拒否する。
