# ThirdParty/acs — 対応する ACS 配布物について

この framework は ACS を**単一ヘッダ + lib の配布物**として使う。ビルドするには配布物の場所を
指定する必要がある。

| やり方 | 指定 |
|---|---|
| 環境変数 | `ACS_DIST_ROOT=<配布物のルート>` |
| ビルド時に指定 | `msbuild ... /p:AcsDistRoot=<配布物のルート>` |

`<配布物のルート>` の下に `acs.h` と `lib/x64/{Debug,Release}/` がある構成を想定する。

---

## いまの状態: 対応版が確定していない

**2026-08-16 時点で、この repo をビルドできる配布物が手元で特定できていない。**

- `Source/Debug/DebugTop/Element/DebugTopElementEnum.h` が `acs::FEnumName` を使う。
  これは 2026-08-03 生成の配布物には**無い**（`acs/src/foundation/EnumTraits.h` にはある）。
- 一方、現行 main の source から生成したヘッダには **ABI ガード**が入っており、
  例外と RTTI を無効にしていない TU を `#error` で弾く。この repo は
  `ExceptionHandling=Sync` / `_HAS_EXCEPTIONS=1` なので、そのままでは 1 行も通らない。

つまり必要なのは「`FEnumName` があり、まだ ABI ガードが入っていない世代」の配布物である。

調査の詳細は `C:\dev\acs_temp_doc\0002-dist-abi-guard-blocks-framework.md`。

### 決める必要があること

1. 該当世代の配布物を持ってきて、ここへ固定する（一番安い）
2. この repo を `/EHs-c- /D_HAS_EXCEPTIONS=0 /GR-` へ移行する
   （同じ exe で Effekseer と DiligentCore を使っているので、そちらの検証が先）
3. `FEnumName` を使わない形へ戻し、8/3 世代の配布物で通るようにする

---

## lib はここへ入れない

配布物の `.lib` は合計 1.1 GB あり、Release の `acs.lib` が 291 MB、
`Diligent-GraphicsEngineD3D12-static.lib` が 308 MB ある。**GitHub は 100 MB を超えるファイルを
受け付けない**（Git LFS も無料枠 1 GB を即超える）。

手元に置く場合は `ThirdParty/acs/lib/x64/{Debug,Release}/` へ。git は無視する。

---

## 対応版が決まったら

1. その配布物の `acs.h` と `acs-distribution.sha256` をこのフォルダへ入れて固定する
2. Debug / Release の両方でビルドし直す
3. コミットメッセージに配布物の生成日と元 commit を書く

ヘッダと lib の版がずれるとリンクエラーや実行時の不整合になる。`acs-distribution.sha256` は
そのための照合表なので、ヘッダだけ差し替えない。
