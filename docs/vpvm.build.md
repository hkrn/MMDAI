libvpvl
=======
libvpvl は [CMake](http://cmake.org "CMake") をビルドシステムとして採用しています。そのため、CMake を事前にインストールする必要があります。

## libvpvl のビルド

libvpvl は [Bullet Physics](http://bulletphysics.org/ "Bullet Physics") に依存しているため、まず Bullet Physics をビルドしておく必要があります。
また、[Open Asset Import Library](http://assimp.sf.net "Open Asset Import Library") もビルドしておきます。Windows と Linux では [GLEW](http://glew.sf.net "GLEW") が必要です。
GLEW はバイナリとして入手しやすいので、GLEW についてはバイナリ版を使ったほうがビルド作業が楽になります。

以下の項目は MacOSX と Linux を対象にしています。Windows では以下の条件を除けば同じ方法でビルド可能です。

  - debug/release の代わりに msvc-build というディレクトリを作成する
  - cmake または cmake-gui を使って MSVC9 (Visual Studio 2008) のプロジェクトとして作成する
    - Visual Studio 2008 Express でもビルド可能。実際バイナリ作成は Express で行なっている。

### デバッグ版のビルド
デバッグ版は依存関係による再ビルドを減らすため、共有ライブラリとしてビルドしておきます。

#### bullet

<pre><code># MacOSX でビルドする場合は 2.77 にしないとビルドに失敗する様子
$ svn co http://bullet.googlecode.com/svn/tags/bullet-2.78/ bullet
$ cd bullet
$ mkdir debug
$ cd debug
# BUILD_DEMOS と BUILD_EXTRAS を無効にしておくとビルドが高速化する
$ cmake -DBUILD_SHARED_LIBS=ON -DBUILD_DEMOS=OFF -DBUILD_EXTRAS=OFF -DCMAKE_BUILD_TYPE="Debug" -DLIBRARY_OUTPUT_PATH=`pwd`/lib ..
$ make
</code></pre>

#### assimp

<pre><code># 予め assimp--2.0.863-sdk.zip をダウンロードしておく
$ unzip assimp--2.0.863-sdk.zip
$ mv assimp--2.0.863-sdk assimp
$ cd assimp
# ENABLE_BOOST_WORKAROUND をつけておくのはファイルサイズを小さくするため
# また assimp に限り debug/release のディレクトリを作成せず直接ビルドする
$ cmake -DBUILD_ASSIMP_TOOLS:BOOL=ON  -DENABLE_BOOST_WORKAROUND=ON -DCMAKE_BUILD_TYPE="Debug"
$ make
</code></pre>

#### libvpvl

<pre><code>$ cd libvpvl
$ mkdir debug
$ cd debug
# QMA2 がビルド可能な設定にする
$ cmake -DBUILD_SHARED_LIBS=ON -DVPVL_LINK_ASSIMP=ON -DVPVL_OPENGL_RENDERER=ON -DVPVL_USE_GLSL=ON -DCMAKE_BUILD_TYPE="Debug" ..
$ make
</code></pre>

### リリース版のビルド
リリース版は Bullet と libvpvl を静的ライブラリとしてビルドします。assimp は CMakeFile が共有ライブラリとして
ビルドするように強制されているため、共有ライブラリとしてビルドします。

#### Bullet

<pre><code># MacOSX でビルドする場合は 2.77 にしないとビルドに失敗するっぽい?
$ svn co http://bullet.googlecode.com/svn/tags/bullet-2.78/ bullet
$ cd bullet
$ mkdir release
$ cd release
# BUILD_DEMOS と BUILD_EXTRAS を無効にしておくとビルドが高速化する
$ cmake -DBUILD_SHARED_LIBS=OFF -DBUILD_DEMOS=OFF -DBUILD_EXTRAS=OFF -DCMAKE_BUILD_TYPE="Release" -DLIBRARY_OUTPUT_PATH=`pwd`/lib -DCMAKE_OSX_ARCHITECTURES="i386;x86_64" ..
$ make
</code></pre>

#### assimp

<pre><code># 予め assimp--2.0.863-sdk.zip をダウンロードしておく
$ unzip assimp--2.0.863-sdk.zip
$ mv assimp--2.0.863-sdk assimp
$ cd assimp
# ENABLE_BOOST_WORKAROUND をつけておくのはファイルサイズを小さくするため
# また assimp に限り debug/release のディレクトリを作成せず直接ビルドする
$ cmake -DBUILD_ASSIMP_TOOLS:BOOL=ON -DENABLE_BOOST_WORKAROUND=ON -DCMAKE_BUILD_TYPE="Release" -DCMAKE_OSX_ARCHITECTURES="i386;x86_64"
$ make
</code></pre>

#### libvpvl

<pre><code>$ cd libvpvl
$ mkdir release
$ cd release
# QMA2 がビルド可能な設定にする
$ cmake -DBUILD_SHARED_LIBS=OFF -DVPVL_LINK_ASSIMP=ON -DVPVL_OPENGL_RENDERER=ON -DVPVL_USE_GLSL=ON -DCMAKE_BUILD_TYPE="Release" -DCMAKE_OSX_ARCHITECTURES="i386;x86_64" ..
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
また、MacOSX 以外では追加で [GLEW](http://glew.sf.net/ "GLEW") が必要になります。

#### VPVL_USE_GLSL
固定シェーダではなくプログラマブルシェーダを使ったレンダリングエンジンでビルドします。このオプションを使用するときは
必ず VPVL_OPENGL_RENDERER を有効にしてください。

#### VPVL_USE_NVIDIA_CG
HLSL と互換な Cg が利用可能なレンダリングエンジンをビルドします。現在まだこのレンダリングエンジンは未完成です。

#### VPVL_LINK_ASSIMP
[Open Asset Import Library](http://assimp.sf.net "Open Asset Import Library") を有効にしてビルドします。これはアクセサリで用いられる X 形式のファイルを
取り扱うのに必要です。無効にした場合は X 形式のファイルを読むことが出来ません。

#### VPVL_BUILD_SDL
[SDL](http://www.libsdl.org "SDL") を使ったレンダリングプログラムを作成します。必ず VPVL_OPENGL_RENDERER を有効にする必要があります。
また、事前に SDL がインストールされている必要があります。

#### VPVL_BUILD_QT
[Qt](http://qt.nokia.com "Qt") を使ったレンダリングプログラムを作成します。必ず VPVL_OPENGL_RENDERER を有効にする必要があります。
また、事前に Qt がインストールされている必要があります。

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
[Qt](http://qt.nokia.com "Qt") を全面的に使用しているため、Qt の 4.7 以降がインストールされている必要があります。

## 翻訳ファイルをの作成
※ 現在この作業は QMA2 を開発する時のみ必要です。ビルドするだけならこの作業は必要ありません。

lrelease を使った方法です。Linguist を使う場合は QMA2/resources/translations/MMDAI2.ts を読み込み、
QMA2/resources/translations/MMDAI2_ja.qm としてリリースしてください。

<pre><code>cd QMA2/resources/translations
lrelease MMDAI2.ts -qm MMDAI2_ja.qm
</code></pre>

## ビルド
qmake を使った方法です。QtCreator を使う場合は QMA2.pro を読み込ませてください。ここでは QMA2 があるディレクトリに
事前にビルドするディレクトリを作成します。MacOSX の場合はパッケージングを行うスクリプトの関係で QMA2-release-build か
QMA2-debug-build という名前でディレクトリを作成する必要があります。

Windows では qmake 使って Visual Studio 2008 用のプロジェクトファイルを生成する必要があります。qmake ではプロジェクト
ファイルのみ生成されるので、保存する際はソリューションファイルも保存してください。

<pre><code># 事前にビルドするディレクトリを作成する
mkdir QMA2-release-build
cd QMA2-release-build
# Visual Studio 2008 のプロジェクトとして作成する場合は "qmake -tp vc ../QMA2/QMA2.pro" とする。その場合 make コマンドは実行しないこと
qmake ../QMA2/QMA2.pro
make
</code></pre>

## パッケージング
Windows と Linux は手動でパッケージングする必要があります。

<pre><code>cd QMA2-qmake-build-desktop
mkdir Locales
cp -r $QT_PLUGINS Plugins
rm -rf Plugins/bearer Plugins/graphicssystems Plugins/qmltooling
cp ../QMA2/resources/translations/MMDAI2_ja.qm Locales
zip -r MMDAI2.zip MMDAI* Plugins Locales
</code></pre>

MacOSX は osx_deploy.sh でデプロイ可能です。実行するとフレームワーク及びライブラリが全て入った MMDAI.app と
そのディスクイメージファイルである MMDAI2.dmg が作成されます。

<pre><code>cd QMA2-release-build
../scripts/osx_deploy.sh
</code></pre>
