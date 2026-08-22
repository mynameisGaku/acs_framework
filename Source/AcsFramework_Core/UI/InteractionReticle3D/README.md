# 3Dインタラクション照準

`AUi3DScene`の`InteractionFocus()`へ対象を1件以上登録すると、視線判定と同じ正規化画面位置へ
小さな照準を自動表示する。対象を捉えていない間は白、捉えると黄色になり、少し外へ広がる。

```cpp
InteractionFocus().RegisterTarget( *Door, FStringView( "E: OPEN" ) );

InteractionReticleParams().FocusedColor = FVec4{ 0.3f, 1.0f, 0.7f, 1.0f };
InteractionReticleParams().bVisible = true;
```

照準位置を別に指定する必要はない。`InteractionFocus().SetParams(...)`の`ScreenPosition`を共有するため、
見た場所と判定場所がずれない。対象登録が0件、設定が不正、描画先が0pixelなら何も描かない。

描画はポスト処理後、ワールドラベルの後、通常UIの前に行う。HDRのbloomやFXAAでぼけず、画面UIが
同じ場所へ重なった場合はUIを優先する。影と4本の短い矩形、中央点だけを既存`CSpriteBatch`へ積み、
専用textureやGPU資源は持たない。矩形計算は`MakeInteractionReticle3DLayout`へ分離しているため、
GPUを起動せずに位置、対象時の倍率、非表示条件を単体検証できる。
