これは何？
=========

MMDAI/MMDAI2 で使うスクリプトまたは CMake 用のファイルが格納されています。

## デプロイ用スクリプト

MMDAI/MMDAI2 をリリース用のバイナリとしてデプロイするシェルスクリプトです。
ビルドしたディレクトリの名前は QMA1-release-desktop または QMA2-release-desktop である
必要があります。それ以外のディレクトリを指定すると終了します。

行うことは Qt のライブラリと必要なリソースを含める処理を行い、その中から必要ないライブラリ
またはプラグインを削除して ZIP として作成する処理を行います。

  - linux_deploy.sh
  - osx_deploy.sh

## CMake

CMake を使ったビルドは非公式のため、バージョンの更新に対して脆弱です。
また、これを用いたビルドによるサポートはありません。その点ご注意ください。

HTSEngine/OpenJTalk/Julius を configure ではなく CMake でビルドするための CMakeLists.txt が
格納されています。これは OSX で 32/64bit のユニバーサルバイナリを作成するために作られている為です。
以下使用例です。

  # ビルドするディレクトリは分けたほうがよい
  # CMake 実行時依存関係を調べるため、HTSEngine => OpenJTalk => Julius の順番でビルドしたほうがよい
  ln -s /path/to/scripts/CMake/HTSEngine.cmake /path/to/hts_engine/CMakeLists.txt
  mkdir /path/to/hts_engine/release; cd /path/to/hts_engine/release
  cmake ..
  make
  sudo make install
  ln -s /path/to/scripts/CMake/OpenJTalk.cmake /path/to/open_jtalk/OpenJTalk.txt
  mkdir /path/to/open_jtalk/release; cd /path/to/open_jtalk/release
  cmake ..
  make
  sudo make install
  ln -s /path/to/scripts/CMake/Julius.cmake /path/to/julius/CMakeLists.txt
  mkdir /path/to/julius/release; cd /path/to/julius/release
  cmake ..
  make
  sudo make install

configure で実行するものとは異なったディレクトリ構成でインストールされます。

## PVRTC

PMD のモデルに含まれるテクスチャを PVRTC 形式に変換する CLI のアプリケーションです。
Xcode をインストールすると iOS SDK に含まれる texturetool を使うため、Xcode が必要です。
その上で QtCreator で PVRTC.pro を読み込んでビルドしてください。

pvrtc の扱い方は以下の通りです。PMD モデルは PVRTC のファイルを使うように上書きするため、
実行前にバックアップを取ることを推奨します。

  pvrtc /path/to/model_dir /path/to/model_dest /path/to/model.pmd

