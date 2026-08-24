# 衝突付き3D街灯

`SpawnStreetLamp3D`は床位置を1つ渡すだけで、金属ポスト、同寸法の箱型衝突、bloomする発光球、
周囲を照らす点光源を1基の街灯として置く。

```cpp
FStreetLamp3DSpawnResult StreetLamp = SpawnStreetLamp3D(
    FVec3{ 2.0f, 0.0f, -1.5f } );

DestroyStreetLamp3D( StreetLamp );
```

高さ、ポスト幅、色、照明範囲を変える場合は`FStreetLamp3DSpawnParams::At`で作った値を調整する。
ポストは回転しない直方体なので、見た目と箱型衝突が同じ位置・寸法になる。発光球はポスト上端へ
接する位置へ自動配置される。

街灯1基につき、ACSが同時描画する点光源4灯のうち1灯を使う。途中生成に失敗した場合はポストも
巻き戻す。`DestroyStreetLamp3D`は場面、root、3ノード、衝突形状、重複を先に確認し、別場面の
生成物を巻き込まない。
