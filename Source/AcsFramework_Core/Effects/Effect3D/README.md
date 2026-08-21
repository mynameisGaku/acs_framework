# 3D Effect

`AEffect3DScene` は、Effekseer の 3D エフェクトを ACS の HDR 透明描画へ接続します。
エフェクトは scene depth で隠れ、同じ露出・bloom・tonemap を通るため、後から画面へ貼った
2D 演出より場面の光になじみます。

```cpp
class AGameScene : public AEffect3DScene
{
public:
	void OnEnter() noexcept override
	{
		AEffect3DScene::OnEnter();
		PlayEffect3D( FStringView( "Effects/hit.efkefc" ), FVec3{ 0.0f, 1.0f, 0.0f } );
	}
};
```

`PlayEffect3D` は最初の描画より前でも呼べます。D3D12 backend の準備が整うまで再生指定を
保持し、準備後に自動で素材を読み込みます。同じ素材は scene 内で再利用されます。

位置を追従させる場合は、戻り値を保持して `Effects3D().SetPosition()` を呼びます。時間は
`OnUpdate` の `DeltaSeconds` だけで進み、隠れた時計や別threadには依存しません。

`AEffect3DScene` は `AUi3DScene` も含みます。同じ場面で `Ui().AddButton` / `Ui().AddText` を
呼べば、3DエフェクトはHDR内、プレイヤーUIはポスト処理後という適切な順で自動合成されます。

現時点の描画backendは D3D12 です。D3D12を借りられないbackendでは安全に描画を省略し、
理由をlogへ1回残します。
