VPVM のフォーマット
===================

VPVM のプロジェクトファイルのフォーマットを暫定的に策定中。

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

