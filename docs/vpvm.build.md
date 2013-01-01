libvpvl2/VPVM のビルド方法
==========================

0.26.0 から libvpvl/libvpvl2 と必要なライブラリを [Thor](https://github.com/wycats/thor "Thor") を用いてビルドしています。VPVM a.k.a MMDAI2 は QtCreator から VPVM.pro を読み込んでビルドしてください。ビルドを実行するには以下が必要です。

  - Ruby
    - 事前に <pre>gem install thor</pre> を実行し、thor をインストールする
  - Subversion
  - git
  - wget
  - yasm
    - libav のビルドに必要
  - CMake
    - 2.8 以降。以下のライブラリのビルドに必要
	  - bullet
	  - assimp
	  - libvpvl
	  - libvpvl2
  - Qt
    - 4.8 以降
    - QtSDK 入れた方が楽
      - ただし MacOSX ユニバーサルバイナリ作成時だけソースビルドが必要

※ Linux 上では nvtt が正しくビルド出来ないため、scripts/nvtt-r1357.diff を適用した上でコンパイラを clang にしてビルドする必要があります。また、ライブラリが生成できればよいので、ライブラリではなく実行ファイル関連のビルドが失敗した場合は無視してください。

リリースビルドを作成する場合は以下を実行します。

<pre>thor mmdai:all:release
</pre>

デバッグつきでビルドする場合は以下を実行します。

<pre>thor mmdai:all:debug
</pre>

いずれも libvpvl2 とそれに依存するライブラリのソース取得、ビルドが自動的に行われます。ビルド時のコマンドラインを取得する場合は以下を実行します。

<pre>thor mmdai:all:flags_debug # リリース版の場合は mmdai:all:flags_release
</pre>

VPVM a.k.a MMDAI2
=================

[Qt](http://qt.digia.com "Qt") を全面的に使用しているため、Qt の 4.8 以降がインストールされている必要があります。

## 翻訳ファイルをの作成
※ 現在この作業は VPVM を開発する時のみ必要です。ビルドするだけならこの作業は必要ありません。

lrelease を使った方法です。VPVM/resources/translations/MMDAI2.ts を読み込み、VPVM/resources/translations/MMDAI2_ja.qm として生成（Linguist 上ではリリース）してください。

<pre><code>cd VPVM/resources/translations
lrelease MMDAI2.ts -qm MMDAI2_ja.qm
</code></pre>

## ビルド

### QtCreator でビルドする方法

QtCreator を使う場合は VPVM.pro を読み込ませてください。ここでは VPVM があるディレクトリに事前にビルドするディレクトリを作成します。MacOSX の場合はパッケージングを行うスクリプトの関係で VPVM-release-desktop かVPVM-debug-desktop という名前でディレクトリを作成する必要があります。

### qmake でビルドする方法

MacOSX/Unix では qmake で作成する場合 Makefile が作成されるので、それを用いて make を行います。

Windows では qmake 使って Visual Studio 2010 用のプロジェクトファイルを生成する必要があります。qmake ではプロジェクト
ファイルのみ生成されるので、保存する際はソリューションファイルも保存してください。

<pre><code># 事前にビルドするディレクトリを作成する
mkdir VPVM-release-desktop
cd VPVM-release-desktop
# Visual Studio 2010 のプロジェクトとして作成する場合は "qmake -tp vc ../VPVM/VPVM.pro" とする。その場合 make コマンドは実行しないこと
qmake ../VPVM/VPVM.pro
# Visual Studio の場合は nmake を実行する
make
</code></pre>

## パッケージング

MacOSX と Linux 版ではパッケージングを行うためのスクリプトが用意されています。これらのスクリプトは実際にバイナリを作成する時に用いられるものです。

### MacOSX 

MacOSX は scripts/osx_deploy.sh でデプロイ可能です。実行すると MMDAI2.dmg が作成されます。これは実行するために必要なライブラリ及びフレームワークがすべて入った状態で作成され、展開してすぐに実行可能になります。

<pre><code>cd VPVM-release-desktop
../scripts/osx_deploy.sh
</code></pre>

### Linux

Linux は scripts/linux_deploy.sh でデプロイ可能です。実行すると MMDAI2.zip が作成されます。MacOSX のデプロイスクリプトと同じく、実行に必要なライブラリが入った状態で作成され、展開して実行可能になりますが、全てのディストリビューションで実行可能であることは保証しません。

<pre><code>cd VPVM-release-desktop
../scripts/linux_deploy.sh
</code></pre>

