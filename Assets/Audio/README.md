# Audio — 3D音響デモ素材

`SpatialPulse.wav`は、左右定位と距離減衰を実機で確かめるための短いモノラルチャイムです。
外部素材は使わず、[`Tools/GenerateSpatialAudioDemo.ps1`](../../Tools/GenerateSpatialAudioDemo.ps1)で
決定論的に生成しています。ソースと同じApache-2.0で扱えます。

```powershell
.\Tools\GenerateSpatialAudioDemo.ps1
```

XAudio2の簡易左右定位はモノラル音源へ適用されるため、ステレオへ変換せず1チャンネルのまま保ちます。
