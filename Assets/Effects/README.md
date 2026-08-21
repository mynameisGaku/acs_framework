# 3D エフェクト素材

`hit.efkefc` は、Effekseer の `ResourceData/samples/02_Tktk03/ToonHit` を
ACS のメートル尺度で確認するために同梱したサンプルです。Effekseer で再生する用途では
サンプル素材を自由に利用でき、runtime 本体の条件は
[`ThirdParty/Effekseer/LICENSE`](../../ThirdParty/Effekseer/LICENSE) にあります。

`hit.efkefc` は `Parts/` の画像・model・materialを相対pathで参照します。配布するときは
`Effects/Parts/` を含め、名前や階層を変えないでください。独自素材へ差し替える場合も、
`.efkefc` が参照するfileを同じ相対pathで `Assets/Effects/` 以下へ置きます。
