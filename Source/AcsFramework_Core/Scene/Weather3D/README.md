# 3D天候

`AWeather3DScene`を継承すると、ACSの`CWeatherSystem`が返す天候遷移を、場面の
ボリューム雲、雲影、霧、空色、IBL環境光へ同時に反映できます。

```cpp
class AGameScene : public AWeather3DScene
{
public:
	void OnEnter() noexcept override
	{
		AWeather3DScene::OnEnter();
		Clouds().Coverage = 0.30f;
		SetWeather( EWeatherKind::Storm, 4.0f );
	}
};
```

派生場面が`OnEnter`で調整した雲・霧・空を、最初の`OnUpdate`で晴天時の基準として
記録します。晴天へ戻すとその値へ戻るため、天候を切り替えるたびに設定値を掛け合わせて
暗くし続けることはありません。

雲量が正で`Clouds().bAffectEnvironmentLighting`がtrueなら、表示と同じ雲の形と照明も
IBLへ自動で反映されます。天候遷移中の高価な再生成はACSが固定間隔へまとめるため、
`SetWeather`を呼ぶ側で更新頻度を管理する必要はありません。

雨滴や雪片の素材は作品ごとに異なるので、Frameworkは勝手に固定しません。
`WeatherAppearance().ParticleDensity`と`Weather().WindDirection()`を使い、
`Effects3D()`などの発生量と向きを決めてください。

天候は場面だけが所有する状態であり、別の場面と寿命を共有しないためsubsystemではありません。
