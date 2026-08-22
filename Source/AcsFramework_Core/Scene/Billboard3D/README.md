# Scene/Billboard3D — カメラへ向く3D画像板

`AUi3DScene::SpawnBillboard3D`は、画像読込、3D画像板の生成、カメラ追従登録を1回にまとめる。
位置と大きさは`FSprite3DSpawnParams`をそのまま使い、描画直前に画像板の正面だけを現在カメラへ
向ける。

```cpp
FSprite3DSpawnParams Marker = FSprite3DSpawnParams::FromImage(
    FStringView( "Textures/Marker.png" ), FVec3{ 0.0f, 2.0f, 3.0f }, FVec2{ 0.8f, 0.8f } );
SpawnBillboard3D( Marker );
```

`EBillboard3DMode::FaceCamera`は上下を含めて正面を向ける。木や人物名札のように常に立てたい板は
`FaceCameraYAxis`を選ぶ。`RollDegrees`は板の正面軸まわりの回転で、親ノードが回転していても
worldでカメラへ向くように親の逆回転をローカル値へ戻している。

既存ノードを追従させる場合は`Billboards().Track( Node )`、追従だけを外す場合は
`Billboards().Remove( Node )`を使う。ノードの所有権は場面グラフに残り、ノード破棄と場面読込による
root差し替えは世代付き識別子から自動で検出する。

画像は固定向き3D板と同じACS経路で、scene depthを読み、HDR透明3Dへ合成される。そのため
露出とbloomの対象になり、HUDへ貼るだけの画像より3D場面に馴染む。
