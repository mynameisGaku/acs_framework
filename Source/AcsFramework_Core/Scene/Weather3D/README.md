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
		EnableTimeOfDay3D( 8.0f );
		Clouds().Coverage = 0.30f;
		SetWeather( EWeatherKind::Storm, 4.0f );
	}
};
```

`EnableTimeOfDay3D`は`AWeather3DScene::OnEnter`の後に呼び、ACSの`CAmbientDirector`を使って
専用の太陽1灯、空色、IBL環境光を
同じ時刻から更新します。初期時刻だけなら上の1行で開始でき、2番目の値へゲーム内時/実秒、
3番目へ太陽軌道の方位角を渡せます。例えば`EnableTimeOfDay3D( 18.0f, 0.0f, 35.0f )`なら、
進行を止めた夕方を場面の向きへ合わせます。手動で置いた太陽とは併用せず、どちらか一方を
場面の主光源にしてください。

時刻制御は空と環境光を先に作り、天候はその結果へ雲量、霧、空の色調、環境光倍率を掛けます。
この順序により、夜の嵐でも昼の晴天値へ戻らず、晴天へ戻しても現在時刻の明るさを保ちます。
`SetTimeOfDay3D`、`SetTimeOfDayRate3D`、`SetTimeOfDaySunAzimuth3D`は成功時だけ現在値を更新し、
`DisableTimeOfDay3D`は専用の太陽を破棄して、派生場面が設定した環境へ戻します。

派生場面が`OnEnter`で調整した雲・霧・空を、最初の`OnUpdate`で晴天時の基準として
記録します。晴天へ戻すとその値へ戻るため、天候を切り替えるたびに設定値を掛け合わせて
暗くし続けることはありません。

雲量が正で`Clouds().bAffectEnvironmentLighting`がtrueなら、表示と同じ雲の形と照明も
IBLへ自動で反映されます。天候遷移中の高価な再生成はACSが固定間隔へまとめるため、
`SetWeather`を呼ぶ側で更新頻度を管理する必要はありません。

雨滴や雪片の素材は作品ごとに異なるので、Frameworkは勝手に固定しません。
`WeatherAppearance().ParticleDensity`と`Weather().WindDirection()`を使い、
`Effects3D()`などの発生量と向きを決めてください。

天候と時刻は場面だけが所有する状態であり、別の場面と寿命を共有しないためsubsystemではありません。
