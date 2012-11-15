VPVM のフォーマット
===================

VPVM のプロジェクトファイルのフォーマットを暫定的に策定中。

新仕様
======
  - XML を用いたテキスト形式である
  - 実装は libxml2 SAX を用いる(予定)

以下は現在暫定的に使われることが決まっているタグのまとめである。

<vpvm:project version="1.0">
  <vpvm:preferences>
    <width>640</width>
    <height>480</height>
    <showInformation>true</showInformation>
    <showGrid>true</showGrid>
    <preferredFPS>30</preferredFPS>
  </vpvm:preferences>
  <vpvm:physics enable="true" />
  <vpvm:models>
    <vpvm:model name="foo">
      <path>/foo/bar/baz</path>
      <edge>1.0</edge>
      <position>1,2,3</position>
    </vpvm:model>
    ...
  </vpvm:models>
  <vpvm:assets>
    <vpvm:asset name="bar">
      <path>/foo/bar/baz</path>
    </vpvm:asset>
    ...
  </vpvm:assets>
  <vpvm:motions>
    <vpvm:motion type="bone" model="foo">
      <keyframe name="foo" index="0" position="1,2,3" rotation="1,2,3"
        interpolation="1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16" />
      ...
    </vpvm:motion>
    <vpvm:motion type="vertices" model="foo">
      <keyframe name="foo" index="0" weight="1.0" />
      ...
    </vpvm:motion>
    <vpvm:motion type="camera">
      <keyframe index="0" position="1,2,3" rotation="1,2,3" distance="0" fovy="0"
        interpolation="1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24" />
      ...
    </vpvm:motion>
    <vpvm:motion type="light">
      <keyframe index="0" color="1,2,3" direction="1,2,3" />
      ...
    </vpvm:motion>
    <vpvm:motion type="ik" model="foo">
      <keyframe name="foo" index="0" enable="true" />
      ...
    </vpvm:motion>
    <vpvm:motion type="asset" asset="foo">
      <keyframe index="0" visible="true" model="null" bone="null" position="1,2,3"
        rotation="1,2,3" scale="10.0" opacity="1.0" />
      ...
    </vpvm:motion>
    ...
  </vpvm:motions>
</vpvm:project>

旧仕様
======
  - バイナリ形式である
  - グローバルな一意なキーに基づいてデータをシリアライズする
  - キーは enum = int として取り扱う
  - データの長さは int (実際には uint32_t として取り扱う) で示す
    - ただしファイル自体は size_t で扱う

# ファイルの中身

  - Project::kFileHeader
    - 4bytes:int
    - マジックヘッダー。中身は 'VPVM'
  - Project::kFileVersion
    - 4bytes:float
    - プロジェクトファイルのバージョン。中身は 1.0
  - Project::kFileData
    - ここからは VPVM で使うグローバルキーとその中身で構成される

# VPVM で使うグローバルキー

  - Project::kMetadata
    - 4bytes:int + nbytes:blob
  - Project::kModelName
    - 4bytes:int + nbytes:string
  - Project::kModelPath
    - 4bytes:int + nbytes:string
  - Project::kModelBlob
    - 4bytes:int + nbytes:blob
  - Project::kMotionPath
    - 4bytes:int + nbytes:string
  - Project::kMotionBlob
    - 4bytes:int + nbytes:blob
  - Project::kSceneWidth
    - 4bytes:int
  - Project::kSceneHeight
    - 4bytes:int
  - Project::RenderInformation
    - 1byte:boolean
  - Project::RenderAxis
    - 1byte:boolean
  - Project::RenderShadow
    - 1byte:boolean
  - Project::kSelfShadowMode
    - 1bytes:byte
  - Project::kSelfShadowDepth
    - 4bytes:float
  - Project::kPhysicsMode
    - 1bytes:byte
  - Project::kPhysicsGravityVelocity
    - 4bytes:float
  - Project::kPhysicsNoise
    - 4bytes:int
  - Project::kPhysicsGravityDirection
    - 12bytes:float3

