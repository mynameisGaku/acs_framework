# 対応プラットフォーム

この文書を、ACS Frameworkが現在保証する実行環境と検証範囲の正本とする。

## 対応対象

- OSとCPU: Windows x64
- コンパイラ: Visual Studio 2026のMSVC v145、C++20
- 描画: DirectX 12を実行できるGPUとドライバー
- CI: GitHub Actionsの`windows-2025-vs2026` x64走者

Framework本体とACS、Effekseer、Diligentの配布ライブラリは同じv145で揃える。
Visual Studio 2022のv143では、v145で作った配布ライブラリと標準ライブラリのABIが合わないため、
現在の配布単位では対応対象に含めない。

## 対象外

x86、ARM64、Linux、macOS、DirectX 11のみの環境は、現時点ではビルドと実行を保証しない。
これらを拒否する設計上の制約ではなく、配布物と継続検証がまだ無いという意味である。

## 検証方法

リポジトリの最上位で次を実行する。

```powershell
.\Tools\RunCiChecks.ps1
```

この1コマンドがリポジトリ検査、ACS配布物の解決、Debug/Releaseアプリのビルド、
両構成の単体テストと決定性テストを順番に実行する。`ACS_DIST_ROOT`または
`-AcsDistRoot`を指定した場合は、その配布物を使う。指定が無ければ`ThirdParty\acs`、
開発機の`C:\acs_dev`、SHA-256で固定したGitHub Releaseの順に解決する。

単体テストと決定性テストは窓、描画、音声を初期化しないためGPU不要である。
一方、サンプル起動と見た目の確認にはDirectX 12 GPUが必要なので、対応GPUを持つ実機で
`.\Tools\CaptureApp.ps1`も実行する。

ACSのGitHub ReleaseとSHA-256がまだ用意されていない期間は、Engine側で作った配布物を
`-AcsDistRoot C:\acs_dev`として明示すれば同じ完全検証を実行できる。
