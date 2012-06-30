libvpvl/libvpvl2/QMA のビルド方法
=================================

0.24.0 から libvpvl/libvpvl2 と必要なライブラリを自動的にビルドするスクリプトである build.pl (scripts/build.pl) を用いてビルドするように変更しています。可能であればそちらでビルドしたほうがはるかに楽です。QMA は QtCreator から QMA.pro または QMA2.pro を読み込んでビルドしてください。build.pl を実行するには以下が必要です。

  - perl (5.10)
  - subversion
  - git
  - cmake (2.8)

一度目は一回 build.pl をダウンロードします。build.pl は実行したディレクトリに MMDAI と関連のライブラリがチェックアウトされるため、二回目以降はは以下のように実行します。 

  ./MMDAI/scripts/build.pl 

MacOSX 版のリリースビルドは以下を実行して作成しています。

  ./MMDAI/scripts/build.pl -opencl -march -static -production

Linux 版のリリースビルドは以下を実行して作成しています。

  ./MMDAI/scripts/build.pl -production

デバッグ版は以下を実行して作成しています。

  ./MMDAI/scripts/build.pl

build.pl で使用可能なオプションは以下を実行することによって列挙されます。

  build.pl --help 

libvpvl/libvpvl2
================

以下の項目は build.pl を用いない方法でのビルドです。

libvpvl は [CMake](http://cmake.org "CMake") をビルドシステムとして採用しています。そのため、まず CMake を事前にインストールする必要があります。

## libvpvl のビルド

libvpvl は [Bullet Physics](http://bulletphysics.org/ "Bullet Physics") に依存しているため、まず Bullet Physics をビルドしておく必要があります。
また、[Open Asset Import Library](http://assimp.sf.net "Open Asset Import Library") もビルドしておきます。Qt とリンクせずに libvpvl を使う場合でかつ
Windows または Linux でビルドする場合は [GLEW](http://glew.sf.net "GLEW") が必要です。GLEW はバイナリとして入手しやすいので、
GLEW についてはバイナリ版を使ったほうがビルド作業が楽になります。

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
$ cmake -DBUILD_ASSIMP_TOOLS:BOOL=OFF  -DENABLE_BOOST_WORKAROUND=ON -DCMAKE_BUILD_TYPE="Debug"
$ make
</code></pre>

#### libvpvl

<pre><code>$ cd libvpvl
$ mkdir debug
$ cd debug
# QMA2 がビルド可能な設定にする
# MacOSX 版では -DVPVL_ENABLE_OPENCL=ON を追加する
$ cmake -DBUILD_SHARED_LIBS=ON -DVPVL_LINK_ASSIMP=ON -DVPVL_OPENGL_RENDERER=ON -DVPVL_ENABLE_GLSL=ON -DVPVL_LINK_QT=ON -DVPVL_ENABLE_PROJECT=ON -DCMAKE_BUILD_TYPE="Debug" ..
$ make
</code></pre>

### リリース版のビルド
リリース版は Bullet と libvpvl を静的ライブラリとしてビルドします。assimp は CMakeFile が共有ライブラリとして
ビルドするように強制されているため、共有ライブラリとしてビルドします。
ただし、Linux 版では静的ビルドではうまく生成出来ないため、共有ライブラリとしてビルドする必要があります。

#### Bullet

<pre><code># MacOSX でビルドする場合は 2.77 にしないとビルドに失敗するっぽい?
$ svn co http://bullet.googlecode.com/svn/tags/bullet-2.78/ bullet
$ cd bullet
$ mkdir release
$ cd release
# BUILD_DEMOS と BUILD_EXTRAS を無効にしておくとビルドが高速化する
# Linux では BUILD_SHARED_LIBS=ON にしておく
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
$ cmake -DBUILD_ASSIMP_TOOLS:BOOL=OFF -DENABLE_BOOST_WORKAROUND=ON -DCMAKE_BUILD_TYPE="Release" -DCMAKE_OSX_ARCHITECTURES="i386;x86_64"
$ make
</code></pre>

#### libvpvl

<pre><code>$ cd libvpvl
$ mkdir release
$ cd release
# QMA2 がビルド可能な設定にする
# Linux では BUILD_SHARED_LIBS=ON にしておく
# MacOSX 版では -DVPVL_ENABLE_OPENCL=ON を追加する
$ cmake -DBUILD_SHARED_LIBS=OFF -DVPVL_LINK_ASSIMP=ON -DVPVL_OPENGL_RENDERER=ON -DVPVL_ENABLE_GLSL=ON -DVPVL_LINK_QT=ON -DVPVL_ENABLE_PROJECT=ON -DCMAKE_BUILD_TYPE="Release" -DCMAKE_OSX_ARCHITECTURES="i386;x86_64" ..
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
libvpvl に [OpenGL](http://www.khronos.org/opengl/ "OpenGL") レンダリングエンジンを追加してビルドします。このオプションを有効にする際 OpenGL のライブラリが必要です。
また、MacOSX 以外でかつ VPVL_LINK_QT を無効にしている場合は追加で [GLEW](http://glew.sf.net/ "GLEW") が必要になります。
QMA2 をビルドするのに必須なオプションです。

#### VPVL_ENABLE_GLSL
固定シェーダではなくプログラマブルシェーダを使ったレンダリングエンジンでビルドします。このオプションを使用するときは
必ず VPVL_OPENGL_RENDERER を有効にしてください。
QMA2 をビルドするのに必須なオプションです。

#### VPVL_ENABLE_PROJECT
プロジェクトファイルの読み込み及び保存機能を libvpvl に追加します。
libxml2 を使っているため、libxml2 をインストールする必要があります。
QMA2 をビルドするのに必須なオプションです。

#### VPVL_ENABLE_OPENCL
[OpenCL](http://www.khronos.org/opencl/ "OpenCL") を使った処理の高速化(主に頂点スキニング)を有効にします。
MacOSX 版の QMA2 をビルドするのに必須なオプションです。

#### VPVL_ENABLE_NVIDIA_CG
HLSL と互換な Cg が利用可能なレンダリングエンジンをビルドします。現在まだこのレンダリングエンジンは未完成です。

#### VPVL_LINK_ASSIMP
[Open Asset Import Library](http://assimp.sf.net "Open Asset Import Library") を有効にしてビルドします。これはアクセサリで用いられる X 形式のファイルを
取り扱うのに必要です。無効にした場合は X 形式のファイルを読むことが出来ません。
QMA2 をビルドするのに必須なオプションです。

#### VPVL_BUILD_SDL
[SDL](http://www.libsdl.org "SDL") を使ったレンダリングプログラムを作成します。必ず VPVL_OPENGL_RENDERER を有効にする必要があります。
また、事前に SDL がインストールされている必要があります。

#### VPVL_LINK_QT
libvpvl と [Qt](http://qt.nokia.com "Qt") をリンクします。必ず VPVL_OPENGL_RENDERER を有効にする必要があります。
QMA2 をビルドするのに必須なオプションです。
事前に Qt 4.8 がインストールされている必要があります。

#### VPVL_BUILD_QT_RENDERER
[Qt](http://qt.nokia.com "Qt") を使ったレンダリングプログラムを作成します。必ず VPVL_OPENGL_RENDERER と VPVL_LINK_QT を有効にする必要があります。
また、事前に Qt 4.8 がインストールされている必要があります。

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
alias bullet_prod='cmake -DCMAKE_BUILD_TYPE:STRING="Release" -DBUILD_DEMOS:BOOL=OFF -DBUILD_EXTRAS:BOOL=OFF -DBUILD_MINICL_OPENCL_DEMOS:BOOL=OFF -DBUILD_CPU_DEMOS:BOOL=OFF -DBUILD_SHARED_LIBS:BOOL=OFF -DLIBRARY_OUTPUT_PATH=`pwd`/lib -DCMAKE_OSX_ARCHITECTURES="i386;x86_64"'
alias vpvl_dev='cmake -DBUILD_SHARED_LIBS=OFF -DVPVL_OPENGL_RENDERER=ON -DVPVL_LINK_ASSIMP=ON -DVPVL_BUILD_SDL=ON -DVPVL_BUILD_QT=ON -DVPVL_ENABLE_PROJECT=ON -DVPVL_LINK_QT=ON -DVPVL_BUILD_QT_RENDERER=ON -DCMAKE_BUILD_TYPE="Debug"'
alias vpvl_prod='cmake -DBUILD_SHARED_LIBS=OFF -DVPVL_OPENGL_RENDERER=ON -DVPVL_LINK_ASSIMP=ON -DVPVL_ENABLE_PROJECT=ON -DVPVL_LINK_QT=ON -DCMAKE_BUILD_TYPE="Release" -DCMAKE_OSX_ARCHITECTURES="i386;x86_64"'
alias assimp_dev='cmake -DBUILD_ASSIMP_TOOLS:BOOL=OFF -DENABLE_BOOST_WORKAROUND=ON -DCMAKE_BUILD_TYPE="Debug"'
alias assimp_prod='cmake -DBUILD_ASSIMP_TOOLS:BOOL=OFF -DENABLE_BOOST_WORKAROUND=ON -DCMAKE_BUILD_TYPE="Release" -DCMAKE_OSX_ARCHITECTURES="i386;x86_64"'
alias opencv_dev='cmake -DBUILD_SHARED_LIBS=ON -DBUILD_TESTS=OFF -DCMAKE_BUILD_TYPE="Debug"'
alias opencv_prod='cmake -DBUILD_SHARED_LIBS=ON-DBUILD_TESTS=OFF -DOPENCV_BUILD_3RDPARTY_LIBS=TRUE -DCMAKE_BUILD_TYPE="Release" -DCMAKE_OSX_ARCHITECTURES="i386;x86_64"'
</code></pre>

QMA2 (a.k.a VPVM or MMDAI2)
===========================
[Qt](http://qt.nokia.com "Qt") を全面的に使用しているため、Qt の 4.8 以降がインストールされている必要があります。

## 翻訳ファイルをの作成
※ 現在この作業は QMA2 を開発する時のみ必要です。ビルドするだけならこの作業は必要ありません。

lrelease を使った方法です。Linguist を使う場合は QMA2/resources/translations/MMDAI2.ts を読み込み、
QMA2/resources/translations/MMDAI2_ja.qm としてリリースしてください。

<pre><code>cd QMA2/resources/translations
lrelease MMDAI2.ts -qm MMDAI2_ja.qm
</code></pre>

## ビルド
まず QMA2 で依存しているライブラリである libav と PortAudio をビルドする必要があります。

### libav のビルド
libav は http://libav.org/download.html からダウンロード可能。0.7.4 を用いること。
MacOSX の場合 configure の引数に x86 向けに '--arch=i386 --cc="clang -m32"' を、x64 向けに '--arch=x86_64' を入れる。一発でユニバーサルバイナリを作ることができないため、個々にビルドし、lipo でバイナリを結合すること。

<pre><code>cd $MMDAI_SRC_DIR
cd libav
./configure \
  --disable-static \
  --enable-shared \
  --disable-bzlib \
  --disable-libfreetype \
  --disable-libopenjpeg \
  --disable-decoders \
  --enable-decoder="pcm_s16le" \
  --disable-encoders \
  --enable-encoder="png" \
  --enable-encoder="pcm_s16le" \
  --disable-parsers \
  --disable-demuxers \
  --enable-demuxer="pcm_s16le" \
  --enable-demuxer="wav" \
  --disable-muxers \
  --enable-muxer="mov" \
  --disable-protocols \
  --enable-protocol="file" \
  --disable-filters \
  --disable-bsfs \
  --disable-indevs \
  --disable-outdevs \
  --enable-zlib
make
make install
</code></pre>

### PortAudio のビルド
PortAudio は http://portaudio.com/download.html からダウンロード可能。V19 の 20111121 (記述時点) を用いること。
また、PortAudio のビルドは scons を用いるため、予め scons をインストールしておく必要がある。
MacOSX の場合は scons の引数に 'customCFlags="-arch i386 -arch x86_64" customCxxFlags="-arch i386 -arch x86_64" customLinkFlags="-arch i386 -arch x86_64"' を追加する

<pre><code># scons はビルドも自動的に行なってくれるので、make を行う必要はない
cd $MMDAI_SRC_DIR
cd portaudio
scons enableOptimize=1 enableDebug=0 enableTests=0 enableAsserts=0
</code></pre>

上記のライブラリを構築したら QMA2 をビルドします。

qmake を使った方法です。QtCreator を使う場合は QMA2.pro を読み込ませてください。ここでは QMA2 があるディレクトリに
事前にビルドするディレクトリを作成します。MacOSX の場合はパッケージングを行うスクリプトの関係で QMA2-release-desktop か
QMA2-debug-desktop という名前でディレクトリを作成する必要があります。

Windows では qmake 使って Visual Studio 2008 用のプロジェクトファイルを生成する必要があります。qmake ではプロジェクト
ファイルのみ生成されるので、保存する際はソリューションファイルも保存してください。

<pre><code># 事前にビルドするディレクトリを作成する
mkdir QMA2-release-desktop
cd QMA2-release-desktop
# Visual Studio 2008 のプロジェクトとして作成する場合は "qmake -tp vc ../QMA2/QMA2.pro" とする。その場合 make コマンドは実行しないこと
qmake ../QMA2/QMA2.pro
make
</code></pre>

## パッケージング
MacOSX と Linux 版ではパッケージングを行うためのスクリプトが用意されています。
これらのスクリプトは実際にバイナリを作成する時に用いられるものです。

### MacOSX 版
MacOSX は scripts/osx_deploy.sh でデプロイ可能です。実行すると MMDAI2.dmg が作成されます。
これは実行するために必要なライブラリ及びフレームワークがすべて入った状態で作成され、
展開してすぐに実行可能になります。

<pre><code>cd QMA2-release-desktop
../scripts/osx_deploy.sh
</code></pre>

### Linux 版
Linux は scripts/linux_deploy.sh でデプロイ可能です。実行すると MMDAI2.zip が作成されます。
MacOSX のデプロイスクリプトと同じく、実行に必要なライブラリが入った状態で作成され、
展開して実行可能になりますが、全てのディストリビューションで実行可能であることは保証しません。

<pre><code>cd QMA2-release-desktop
../scripts/linux_deploy.sh
</code></pre>

