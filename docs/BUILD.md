libvpvl
=======
libvpvl は CMake (http://cmake.org) をビルドシステムとして採用しています。そのため、CMake を事前にインストールする必要があります。

## デバッグ版のビルド

libvpvl は BulletPhysics に依存しているため、まず BulletPhysics をビルドしておく必要があります。
<pre><code>// MacOSX でビルドする場合は 2.77 にしないとビルドに失敗するっぽい?
$ svn co http://bullet.googlecode.com/svn/tags/bullet-2.78/
$ mkdir debug
$ cd debug
// BUILD_DEMOS と BUILD_EXTRAS を無効にしておくとビルドが高速化する
$ cmake -DBUILD_SHARED_LIBS=ON -DBUILD_DEMOS=OFF -DBUILD_EXTRAS=OFF -DCMAKE_BUILD_TYPE="Debug" -DLIBRARY_OUTPUT_PATH=`pwd`/lib ..
$ make
</code></pre>

以下は libvpvl を共有ライブラリとしてデバッグ版をビルドする手順です
<pre><code>$ mkdir debug
$ cd debug
$ cmake -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE="Debug" ..
$ make
</code></pre>

### リリース版のビルド

#### Bullet
<pre><code>// MacOSX でビルドする場合は 2.77 にしないとビルドに失敗するっぽい?
$ svn co http://bullet.googlecode.com/svn/tags/bullet-2.78/
$ mkdir release
$ cd release
// BUILD_DEMOS と BUILD_EXTRAS を無効にしておくとビルドが高速化する
$ cmake -DBUILD_SHARED_LIBS=ON -DBUILD_DEMOS=OFF -DBUILD_EXTRAS=OFF -DCMAKE_BUILD_TYPE="Release" -DLIBRARY_OUTPUT_PATH=`pwd`/lib ..
$ make
</code></pre>

#### libvpvl (共有ライブラリとしてビルドする場合)
<pre><code>$ mkdir release
$ cd release
$ cmake -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE="Release" ..
$ make
</code></pre>

### ビルドオプション

#### VPVL_COORDINATE_OPENGL
libvpvl を OpenGL と同じく右手座標系にあわせた処理を行います。VPVL_OPENGL_RENDERER を有効にすると自動的にこのオプションも
有効化されます。

#### VPVL_NO_BULLET
libvpvl を LinearMath を除く BulletPhysics のライブラリを使用せずにビルドします。このオプションを有効にした場合、
物理シミュレーションが自動的に無効になります。

#### VPVL_OPENGL_RENDERER
libvpvl に OpenGL レンダリングエンジンを追加してビルドします。このオプションを有効にする際 OpenGL のライブラリが必要です。
また、MacOSX 以外では追加で GLEW (http://glew.sf.net/) が必要になります。

#### VPVL_USE_GLSL
固定シェーダではなくプログラマブルシェーダを使ったレンダリングエンジンでビルドします。このオプションを使用するときは
必ず VPVL_OPENGL_RENDERER を有効にしてください。

#### VPVL_USE_NVIDIA_CG
HLSL と互換な Cg が利用可能なレンダリングエンジンをビルドします。現在まだこのレンダリングエンジンは未完成です。

#### VPVL_LINK_ASSIMP
Open Asset Import Library (http://assimp.sf.net) を有効にしてビルドします。これはアクセサリで用いられる X 形式のファイルを
取り扱うのに必要です。無効にした場合は X 形式のファイルを読むことが出来ません。

#### VPVL_BUILD_SDL
SDL を使ったレンダリングプログラムを作成します。必ず VPVL_OPENGL_RENDERER を有効にする必要があります。
また、事前に SDL (http://www.libsdl.org) がインストールされている必要があります。

#### VPVL_BUILD_QT
Qt を使ったレンダリングプログラムを作成します。必ず VPVL_OPENGL_RENDERER を有効にする必要があります。
また、事前に Qt (http://qt.nokia.com) がインストールされている必要があります。

MacOSX で 32/64bit のバイナリを生成したい場合は -DCMAKE_OSX_ARCHITECTURES="i386;x86_64" を指定してください

## ビルドに便利な CMake 用のエイリアス

以下は CMake をターミナルまたはコマンドライン経由で実行する際に便利なエイリアス集です。コピペして使うことが出来ます。これは実際に用いているものです。CMake GUI を使っている場合はそちらを用いたほうが良いかもしれません。

<pre><code>alias hts_engine_dev='cmake -DBUILD_SHARED_LIBS:BOOL=OFF -DCMAKE_BUILD_TYPE:STRING="Debug"'
alias open_jtalk_dev='cmake -DBUILD_SHARED_LIBS:BOOL=OFF -DCMAKE_BUILD_TYPE:STRING="Debug"'
alias julius_dev='cmake -DBUILD_SHARED_LIBS:BOOL=OFF -DCMAKE_BUILD_TYPE:STRING="Debug"'
alias hts_engine_prod='cmake -DBUILD_SHARED_LIBS:BOOL=OFF -DCMAKE_BUILD_TYPE:STRING="Release" -DCMAKE_OSX_ARCHITECTURES="i386;x86_64"'
alias open_jtalk_prod='cmake -DBUILD_SHARED_LIBS:BOOL=OFF -DCMAKE_BUILD_TYPE:STRING="Release" -DCMAKE_OSX_ARCHITECTURES="i386;x86_64"'
alias julius_prod='cmake -DBUILD_SHARED_LIBS:BOOL=OFF -DCMAKE_BUILD_TYPE:STRING="Release" -DCMAKE_OSX_ARCHITECTURES="i386;x86_64"'
alias bullet_dev='cmake -DCMAKE_BUILD_TYPE:STRING="Debug" -DBUILD_DEMOS:BOOL=ON -DBUILD_EXTRAS:BOOL=ON -DBUILD_MINICL_OPENCL_DEMOS:BOOL=ON -DBUILD_CPU_DEMOS:BOOL=ON -DBUILD_SHARED_LIBS:BOOL=OFF -DLIBRARY_OUTPUT_PATH=`pwd`/lib'
alias bullet_prod='cmake -DCMAKE_BUILD_TYPE:STRING="Release" -DBUILD_DEMOS:BOOL=OFF -DBUILD_EXTRAS:BOOL=OFF -DBUILD_MINICL_OPENCL_DEMOS:BOOL=OFF -DBUILD_CPU_DEMOS:BOOL=OFF -DBUILD_SHARED_LIBS:BOOL=OFF -DLIBRARY_OUTPUT_PATH=`pwd`/lib -DCMAKE_OSX_ARCHITECTURES="i386;x86_64" -DCMAKE_CXX_FLAGS="-fvisibility=hidden -fvisibility-inlines-hidden"'
alias vpvl_dev='cmake -DBUILD_SHARED_LIBS=OFF -DVPVL_OPENGL_RENDERER=ON -DVPVL_LINK_ASSIMP=ON -DVPVL_BUILD_SDL=ON -DVPVL_BUILD_QT=ON -DVPVL_BUILD_QT_WITH_OPENCV=OFF -DCMAKE_BUILD_TYPE="Debug"'
alias vpvl_prod='cmake -DBUILD_SHARED_LIBS=OFF -DVPVL_OPENGL_RENDERER=ON -DVPVL_LINK_ASSIMP=ON -DCMAKE_BUILD_TYPE="Release" -DCMAKE_OSX_ARCHITECTURES="i386;x86_64" -DCMAKE_CXX_FLAGS="-fvisibility=hidden -fvisibility-inlines-hidden"'
alias assimp_dev='cmake -DBUILD_ASSIMP_TOOLS:BOOL=ON -DENABLE_BOOST_WORKAROUND=ON -DCMAKE_BUILD_TYPE="Debug"'
alias assimp_prod='cmake -DBUILD_ASSIMP_TOOLS:BOOL=ON -DENABLE_BOOST_WORKAROUND=ON -DCMAKE_BUILD_TYPE="Release" -DCMAKE_OSX_ARCHITECTURES="i386;x86_64"'
alias opencv_dev='cmake -DBUILD_SHARED_LIBS=ON -DBUILD_TESTS=OFF -DCMAKE_BUILD_TYPE="Debug"'
alias opencv_prod='cmake -DBUILD_SHARED_LIBS=ON-DBUILD_TESTS=OFF -DOPENCV_BUILD_3RDPARTY_LIBS=TRUE -DCMAKE_BUILD_TYPE="Release" -DCMAKE_OSX_ARCHITECTURES="i386;x86_64"'
</code></pre>

QMA2 (a.k.a VPVM or MMDAI2)
===========================
Qt を全面的に使用しているため、Qt の 4.7 以降がインストールされている必要があります。

## 翻訳ファイルをの作成
lrelease を使った方法です。Linguist を使う場合は QMA2/resources/translations/MMDAI2.ts を読み込み、
QMA2/resources/translations/MMDAI2_ja.qm としてリリースしてください。

<pre><code>cd QMA2/resources/translations
lrelease MMDAI2.ts -qm MMDAI2_ja.qm
</code></pre>

## ビルド
qmake を使った方法です。QtCreator を使う場合は QMA2.pro を読み込ませてください。ここでは QMA2 があるディレクトリに
事前にビルドするディレクトリを作成します。MacOSX の場合はパッケージングを行うスクリプトの関係で QMA2-release-build か
QMA2-debug-build という名前でディレクトリを作成する必要があります。

<pre><code># 事前にビルドするディレクトリを作成する
mkdir QMA2-release-build
cd QMA2-release-build
# Visual Studio のプロジェクトとして作成する場合は "qmake -tp vc"
qmake ../QMA2/QMA2.pro
make
</code></pre>

## パッケージング
Windows と Linux は手動でパッケージングする必要があります。

<pre><code>cd QMA2-qmake-build-desktop
mkdir Locales
cp ../QMA2/resources/translations/MMDAI2_ja.qm Locales
</code></pre>

MacOSX は osx_deploy.sh でデプロイ可能です。実行するとフレームワーク及びライブラリが全て入った MMDAI.app と
そのディスクイメージファイルである MMDAI2.dmg が作成されます。

<pre><code>cd QMA2-release-build
../scripts/osx_deploy.sh
</code></pre>
