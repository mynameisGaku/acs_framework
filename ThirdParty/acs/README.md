# ThirdParty/acs — 対応する ACS 配布物について

この framework は ACS を**単一ヘッダ + lib の配布物**として使う。ビルドするには配布物の場所を
指定する必要がある。

**いちばん簡単なのは、このフォルダへ置くこと。** `Tools\FetchAcs.ps1` がそれをする。

```powershell
.\Tools\FetchAcs.ps1                        # 固定した版を取得して、ここへ展開する
.\Tools\FetchAcs.ps1 -FromLocal C:\acs_dev   # 自分でビルドしたものを持ってくる
```

探す順番は次のとおりで、**ここに `acs.h` が在れば何も指定しなくても通る**。

| 順 | やり方 |
|---|---|
| 1 | ビルド時に指定 `msbuild ... /p:AcsDistRoot=<配布物のルート>` |
| 2 | 環境変数 `ACS_DIST_ROOT=<配布物のルート>` |
| 3 | **このフォルダ (`ThirdParty\acs`)** |
| 4 | `C:\acs` (古い版の名残。落ちるとビルド時に警告が出る) |

固定している版は `Tools\acs-version.json`。**tag と sha256 は必ず一緒に変える。**
片方だけ変えると「取得できたのに中身が違う」という一番分かりにくい壊れ方をする。

**中身は commit しない。** 1 GB 近くあり、取り直せるものを repo に持つ意味が無いので、
`.gitignore` がこの README 以外を無視している。

`<配布物のルート>` の下に `acs.h` と `lib/x64/{Debug,Release}/` がある構成を想定する。

どちらも指定しない場合は **`C:\acs` を使う**。その場合は build 時に警告が出るので、
「意図せず既定へ落ちている」ことに気付ける。既定が無いと `acs.h` が 1 つも見つからず、
IDE では全ファイルが «識別子 f32 が定義されていません» で埋まってしまう。

---

## どの ACS を使うか — **dev** を追う

**この枠組みは ACS の `dev` を追う。`main` は使わない。**

`main` と `dev` で **ABI ガードの向きが逆**になっている。

| ブランチ | 生成されるヘッダが要求するもの |
|---|---|
| `main` | 例外**無効** + RTTI 無効 (`/EHs-c- /D_HAS_EXCEPTIONS=0`) |
| **`dev`** | **例外有効 + RTTI 無効** (`/EHsc /D_HAS_EXCEPTIONS=1`、Diligent に合わせるため) |

この枠組みは例外有効でビルドするので、**`dev` から作った配布物でなければ 1 行も通らない**。
以前ここに「対応版が特定できない」と書いてあったのは、`main` 側を見ていたため。

### 作り直す

```powershell
.\Tools\UpdateAcsDist.ps1
```

`origin/dev` を取得し、**別の git worktree** (既定 `C:\dev\acs_dev`) でビルドして
`C:\acs_dev` へ配置する。**元の repo の作業ツリーには触れない**ので、そちらで作業中でも安全。

ヘッダと lib は必ず同じ commit から作る。食い違うとリンクは通るのに実行時に壊れ、原因が
追いにくいので、スクリプトが**組み合わせが違えば止まる**ようにしてある。

### 使う場所を指す

```powershell
[Environment]::SetEnvironmentVariable('ACS_DIST_ROOT', 'C:\acs_dev', 'User')
```

設定後は **Visual Studio を再起動**すること (起動時にしか読まない)。
指定が無いときは `C:\acs` へ落ちるが、そちらは 2026-08-03 生成の古いもの。

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
