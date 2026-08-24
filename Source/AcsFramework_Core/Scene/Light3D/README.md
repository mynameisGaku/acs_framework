# 3Dライト

`AUi3DScene::SpawnLight3D`は、ACSの`ALightComponent3D`をノードへ付けてシーンに置くまでを
1回にまとめる。描画、上限管理、視点に近い点光源の選択はACS側の`CLightCollector3D`へ任せ、
この層ではライティング計算を持たない。

```cpp
SpawnLight3D( FLight3DSpawnParams::Sun( FVec3{ -0.47f, 0.58f, 0.66f } ) );

SpawnLight3D( FLight3DSpawnParams::Point( FVec3{ 0.0f, 2.0f, 0.0f }, 8.0f ) );

// 自己発光球と周囲を照らす同色の点光源を、位置だけで置く
FLamp3DSpawnResult Lamp = SpawnLamp3D( FVec3{ 1.6f, 2.1f, -0.8f } );

// 動く光も発光球と点光源を同じ位置・色へ更新する
FLamp3DParams MovedLamp = FLamp3DParams::At( FVec3{ 2.0f, 2.4f, 0.0f } );
MovedLamp.Color = FVec3{ 0.18f, 0.52f, 1.0f };
TryUpdateLamp3D( Lamp, MovedLamp );

// 被写体中心、被写体からカメラへの方向、半径だけでキー・フィル・リムを置く
FStudioLightRig3DSpawnResult Rig = SpawnStudioLightRig3D(
    FVec3{ 0.0f, 1.0f, 0.0f }, FVec3{ 0.0f, 0.0f, -1.0f }, 1.2f );

// 動く被写体へ3灯の位置、色、強さ、到達距離をまとめて追従させる
FStudioLightRig3DParams MovedRig = FStudioLightRig3DParams::AroundSubject(
    FVec3{ 2.0f, 1.2f, -1.0f }, FVec3{ 1.0f, 0.0f, 0.0f }, 1.2f );
TryUpdateStudioLightRig3D( Rig, MovedRig );

// 場面から外すときも3灯を1つの結果で片付ける
DestroyStudioLightRig3D( Rig );

// ランプも発光球と点光源を一緒に片付ける
DestroyLamp3D( Lamp );
```

`AUi3DScene`を使わない独自の場面グラフでは、低水準の`CLight3DSpawner::SpawnInto`を直接使える。
配置後に時刻や演出で値を変える場合は`CLight3DSpawner::TryApplyTo`へ同じ
`FLight3DSpawnParams`を渡す。光の部品が無いノードには部品を追加せず、入力不正時も変形と
既存の光を変更しないため、内部の回転変換を外部から直接呼ぶ必要はない。

`SpawnLamp3D`は、HDR自己発光する球と実際に周囲を照らす点光源を同じ位置・色で置く。
自己発光だけでは周囲が明るくならず、点光源だけでは光源本体が見えないため、その両方を
1回で揃える。既定値は暖色、半径0.16、自己発光強度4、照明強度2、到達距離5である。
色、半径、2種類の強度を変える場合は`FLamp3DParams::At`で作った値を調整する。1個につき
ACSが同時描画する点光源4灯のうち1灯を使い、途中失敗では発光球を残さない。
生成後の移動、色替え、点滅は同じ値を`TryUpdateLamp3D`へ渡す。生成結果が同じ場面とrootを
指し、発光球と点光源が両方生存する場合だけ更新するため、失敗時に片方だけ変わらない。

`SpawnStudioLightRig3D`は、被写体を正面左上から照らす暖色のキー、陰側を持ち上げる寒色の
フィル、背面から輪郭を分けるリムを全て点光源で置く。平行光を増やさないため、既定または
`EnableTimeOfDay3D`の太陽が影と空を一貫して担当し続ける。ACSが同時描画する点光源4灯のうち
3灯を使うので、同じ場面で追加する点光源は残り1灯を目安にする。

より細かく調整する場合は`FStudioLightRig3DParams::AroundSubject`で作った値の色、強さ、
到達距離倍率を変更する。配置後は同じ生成結果と新指定を`TryUpdateStudioLightRig3D`へ渡すと、
3灯の生成番号と共通親を保ったまま同期更新できる。別場面、破棄予定、共通親を失った構成、
不正値はどの灯も変更せず拒否する。無効な親や値、途中の生成失敗では3灯を半端に残さない。
`CStudioLightRig3DSpawner`を直接使う場合も、生成結果を`TryApplyTo`または`Destroy`へ渡せば、
別場面の光を巻き込まずに更新・破棄できる。

平行光の`DirectionToLight`は、光が進む向きではなく、面から光源へ向かう向きである。
長さそのものは明るさに影響せず、配置時に正規化する。点光源は`Position`と`Range`を使う。

いずれも親を渡した場合は親ノード内の位置・方向として扱う。シーン全体の太陽は、通常どおり
ルート直下へ置けばワールド空間と一致する。

色は線形空間のRGBで、1より大きい値も使える。`Intensity`を0にすると、ノードを残したまま
ACSの収集対象から外せる。負値、NaN、無限大、演算時に溢れる色と強さ、正規化できない太陽方向、
0以下の点光源距離は、半端なノードを残さず拒否する。
