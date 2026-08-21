# Assets — 素材の置き場

**素材はここに置く。** 探し回らずに済むことの方が、自由に置けることより価値がある。

```
Assets/
  Models/     モデル (.fbx を薦める)
  Effects/    3D エフェクト (.efkefc)
  Textures/   画像
  Audio/      音
```

## モデルは FBX

```cpp
FModel3DSpawnParams Params =
    FModel3DSpawnParams::FromMesh( FStringView( "Models/Robot.fbx" ), FVec3{ 0, 0, 0 } );
CModel3DSpawner::SpawnInto( Root(), Params, Models );
```

パスは **`Assets` からの相対**。`..` と絶対パスは受け付けない (配る段になって
「自分の機械にしか無いファイル」を掴んでいたことに気付くのを防ぐため)。

`.gltf` `.glb` `.obj` も読める。ACS 側にローダが在るので通しているだけで、
**この枠組みが薦めるのは FBX**。

## 3D エフェクトは efkefc

Effekseerで書き出した`.efkefc`を`Effects/`へ置く。`AEffect3DScene`を継承した場面なら、
素材名と位置だけでHDRの3D演出として再生できる。

```cpp
PlayEffect3D( FStringView( "Effects/hit.efkefc" ), FVec3{ 0, 1, 0 } );
```

同梱sampleと参照fileの配置は[`Effects/README.md`](Effects/README.md)を参照。

## 置き場の見つけ方

上から順に見て、最初に見つかったものを使う。

1. 環境変数 `ACSFW_ASSETS`
2. いま居るフォルダの `Assets`
3. 実行ファイルの隣の `Assets`
4. 実行ファイルから**上へ辿って**最初に見つかった `Assets`

4 が要るのは、実行ファイルが `x64\Release` に出るため。**IDE から実行しても、
出来上がりを直接叩いても、素材をコピーせずに同じように動く。**

## 入れないもの

- 中間ファイル (`.blend`、`.max`、`.psd` の作業用)。**出力だけ置く**
- 巨大なもの。git に入るので、100 MB を超えるなら置き方から考え直す
- 生成できるもの (焼いたライトマップなど)

詳しくは `Source/AcsFramework_Core/Assets/Model3D/README.md`。
