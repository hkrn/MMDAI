## ！！！注意！！！

これらの情報は旧世代の MMDAI についての記述です。
現行世代のものについては https://github.com/hkrn/MMDAI/wiki/Deploy を参照してください。

## あらかじめ

ビルドがこけてしまう場合は開発者(@hikarincl2)に報告してください。
説明がどうしても甘くなりがちでどこかで間違いがあるかもしれません。

## ビルド手順

まず必要なライブラリが入っているかどうかを確認して下さい。

  - Bullet Physics
  - Qt 4.6 以降
  - OpenGL Easy Extension (MacOSX の場合不要)

プラグインをビルドする際以下が必要です。Phonon は Qt に含まれれます。

  - Phonon (QMAAudioPlugin)
  - AquesTalk2 (QMAAquesTalk2Plugin)
  - Julius (QMAJuliusPlugin)
  - OpenJTalk (QMAOpenJTalkPlugin)
  - PortAudio V19 (QMAAquesTalk2Plugin / QMAJuliusPlugin / QMAOpenJTalkPlugin)
  - hts_engine (QMAOpenJTalkPlugin)
  - Lua (QMAVILuaPlugin)

MMDAI 及び MMDME のビルドに cmake が必要です。MacOSX の場合はさらに Xcode が必要です。
Xcode をインストールしないと gcc 等が入らないので必ずインストールしてください。

インストール先は /usr または /usr/local にインストールする必要があります。
Ubuntu Linux の場合、apt-get でインストールした方が楽です。

Julius/OpenJTalk/hts_engine は独自の CMake ファイルを用いて作成する必要があるため、
パッケージからではなく、後述の手順でソースでビルドする必要があります。

## 環境変数の設定

以下の環境変数を指定するとその値からライブラリ及びヘッダーの検出を行います。MSVC でビルドする場合は必須です。
Windows で環境設定する際はユーザの環境変数を用いるとよいでしょう(注意点として更新の反映は一度ログオフして再度ログインする必要がある)。

  - GLEE_DIR: GLee.lib のソースがあるパス指定
  - MMDAI_LIBRARY_DIR: libMMDAI.(dylib|so) または MMDAI.lib がインストールされているパスを指定
  - MMDME_LIBRARY_DIR: libMMDME.(dylib|so) または MMDME.lib がインストールされているパスを指定
  - BULLET_LIBRARY_DIR: libBullet*.(dylib|so) または Bullet*.lib がインストールされているパスを指定
  - MMDAI_INCLUDE_DIR: MMDAI/PMDObject.h があるパスを指定 (この場合 MMDAI のディレクトリがあるパスを指定)
  - MMDME_INCLUDE_DIR: MMDME/MMDME.h があるパスを指定 (この場合 MMDME のディレクトリがあるパスを指定)
  - BULLET_INCLUDE_DIR: btBulletDynamicsCommon.h があるパスを指定

MSVC でビルドする場合 BULLET_INCLUDE_DIR は Bullet のトップディレクトリ以下の src を指定します。
MMDAI_INCLUDE_DIR 及び MMDME_INCLUDE_DIR ディレクトリはトップディレクトリ以下の MMDAI/include と
MMDME/include を指定します。

  OpenGL Easy Extension を CMake でビルドする場合

    $ cp $MMDAI/libGLEE/CMakeFiles.txt $GLEE_SRC_DIR
    $ mv GLee.c glee.c
    $ cmake .
    $ make
    $ sudo make install

### MSVC 限定で必要な環境変数

MSVC 上でビルドする場合はさらに以下の環境変数を設定する必要があります。

  - MMDAI_DIRECTX_SDK_LIBRARY_DIR: DirectX SDK に含まれるライブラリのパスを指定 (dsound.lib で必要)
  - MMDAI_HTS_ENGINE_LIBRARY_DIR: hts_engine_API.lib があるパスを指定
  - MMDAI_JULIUS_LIBRARY_DIR: hts_engine_API.lib があるパスを指定
  - MMDAI_OPENJTALK_LIBRARY_DIR: Open_JTalk.lib があるパスを指定
  - MMDAI_PORTAUDIO_LIBRARY_DIR: PortAudio.lib があるパスを指定
  - MMDAI_AQUESTALK2_LIBRARY_DIR: AquestTalk2Da.lib があるパスを指定
  - MMDAI_AQKANJI2KOE_LIBRARY_DIR: AqKanji2Koe.lib があるパスを指定
  - MMDAI_HTS_ENGINE_INCLUDE_DIR: HTS_engine.h があるパスを指定
  - MMDAI_JULIUS_INCLUDE_DIR: julius/julius.h があるパスを指定
  - MMDAI_OPENJTALK_INCLUDE_DIR: njd.h / jpcommon.h などがあるパスを指定
  - MMDAI_PORTAUDIO_INCLUDE_DIR: portaudio.h があるを指定
  - MMDAI_AQUESTALK2_INCLUDE_DIR: AquestTalk2Da.h があるパスを指定
  - MMDAI_AQKANJI2KOE_INCLUDE_DIR: AqKanji2Koe.h があるパスを指定

## Linux か MacOSX でビルドする場合

$MMDAI は MMDAI のレポジトリがあるパスを指します。
cmake でビルドする際に共有ライブラリを作成する場合は

    $ cmake -DBUILD_SHARED_LIBS=ON .

とすることにより共有ライブラリが作成されます。また、静的ライブラリを作成する場合は

    # cmake . と同じ
    $ cmake -DBUILD_SHARED_LIBS=OFF

とすることによって静的ライブラリを作成することが出来ます。

    # 最初に MMDAgent のレポジトリをチェックアウトする
    # 回線次第ではあるものの、時間がかかる
    $ svn co https://mmdagent.svn.sourceforge.net/svnroot/mmdagent/MMDAgent

    # MMDAgent のレポジトリをチェックアウトしたらまず HTSEngine API をビルド
    $ cd Library_hts_engine_API/src
    $ cp $MMDAI/scripts/CMake/HTSEngine.cmake CMakeLists.txt
    $ cmake .
    $ make
    $ sudo make install

    # OpenJTalk をビルド
    $ cd ../../Library_OpenJTalk/src
    $ cp $MMDAI/scripts/CMake/OpenJTalk.cmake CMakeLists.txt
    $ cmake .
    $ make
    $ sudo make install

    # Julius をビルド
    # ※事前に PortAudio を入れる必要があります
    $ cd ../../Library_Julius/src
    $ cp $MMDAI/scripts/CMake/Julius.cmake CMakeLists.txt
    $ cmake .
    $ make
    $ sudo make install

    # MMDME をビルド
    $ cd $MMDAI/MMDME
    $ cmake .
    $ make 
    $ sudo make install

    # MMDAI をビルド
    $ cd $MMDAI/MMDAI
    $ cmake .
    $ make
    $ sudo make install

    # QMA をビルド
    $ cd ..
    $ qmake
    $ make # MacOSX の場合は xcodebuild

### ディレクトリ構成を変更したい場合 (Linux向け)

QMA/QMACustom.pri.sample を QMA/QMACustom.pri にコピーし、以下の定数を任意のパスに変更します。
(コンパイルの関係で、定数の値が複雑になっています。値を書き間違えるとコンパイルに失敗します)

  - QMA_CONFIG_PATH
    - MMDAI.fst や MMDAI.ojt が置かれるディレクトリ
  - QMA_PLUGIN_PATH
    - プラグインが置かれるディレクトリ
  - QMA_RESOURCE_PATH
    - モデルやモーション、JuliusやOpenJTalkに必要なデータが置かれるディレクトリ

一度 QMA の Makefile を削除し、qmake を再実行すれば値が反映されます。
それから再コンパイルを実行してください。

## MSVC でビルドする場合 (暫定。記述が間違っている箇所があると思われる)

まずは Visual Studio 2008 をインストールする必要があります。Express C++ でも可能です。
(Visual Studio 2010 は Qt ライブラリの関係でビルドが正しく行えない)

次に Qt の Visual Studio ビルド版と CMake をインストールし、Bullet のソースコードを入手します。
両者をインストールしたら CMake を立ち上げ、先程の Bullet のトップディレクトリ上を指定して
"Visual Studio 9 2008" 形式でプロジェクトファイルを生成します。
デモアプリの生成を行わないように設定しておくとビルド時間を短縮することが可能です。

ビルドした Bullet のパスを上記の環境変数を設定し、同じように MMDME を CMake で
プロジェクトファイルを生成します。生成したら MMDME のパスを環境変数に設定し、
MMDAI のプロジェクトファイルを生成、ビルドを行います。

QMA のファイル生成は qmake で行います。コマンドラインから "qmake -tp vc" を実行し、プロジェクトファイルを生成します。
ソリューションファイル (sln) は作成されないので、ダイアログが出てきたら保存してください。ビルドする前に QMA のプロジェクトの
プロパティを開き、「構成プロパティ」->「リンカー」->「入力」にある「特定の既定のライブラリの無視」の項目に"libc.lib" を追加してください。

上記過程を終わったら QMA をビルドしてください。ビルドが正常に完了すると debug または release ディレクトリの下に
MMDAI.exe が出来上がります。

## iPhone 向けのビルド (まだシミュレータ上のみのため、途中の段階)

前提条件として Xcode4 を用いるため、それ向けの説明を行います。
まず Bullet を cmake で Xcode プロジェクトファイルを生成するようにします。

    $ cmake -G Xcode

複数のターゲットが表示されますが、使用するターゲットは以下の4つのみです。

  - BulletSoftBody
  - BulletCollision
  - BulletDynamics
  - LinearMath

これらを以下の手順でビルドします。

  - "Base SDK" を "Latest iOS" に変更する
  - "Architectures" を "Optimized (arm7)" にする
  - "Other C Flags" と "Other C++ Flags" から "-arch i386" を削除
    - これを忘れてビルドするとエラー多発で Xcode が固まる危険性大です
  - CTRL + B でビルドを実行

MMDAI 及び MMDME も上記のような流れで同じです。
使用するアプリケーションで生成した静的ライブラリを組み込んでください。

## MinGW (現在この方法でビルドは行っていないため、記述が古い)

Linux 上でクロスコンパイルを行います。yum で MinGW の開発環境が揃えられる
Fedora Linux でビルドしたほうがよいかもしれません。
以下の解説は Fedora Linux 14 を使った場合でのビルド方法です。

### OpenGL Easy Extension

cmake を使います。mingw32-cmake を使うこと以外方法は同じです。

### Bullet Physics

一旦トップディレクトリで mingw32-cmake を実行します。その後 src ディレクトリに移動し、
make を実行します。ビルド出来たら make install を実行します。

### MMDME / MMDAI

mingw32-cmake を使うこと以外方法は同じです。OpenGL Easy Extension と
Bullet Physics の二つがインストールされていることが前提条件です。

### QMA

qmake-qt4 の代わりに mingw32-qmake-qt4 を使う必要があります。
mingw32-qmake-qt4 を実行し、make すればバイナリが作成されます。
生成されたバイナリは debug または release の MMDAI.exe になります。

実行する際 dll が必要なため、配置スクリプトを実行します。
配置スクリプトの第一引数に "-release" つけることで Qt のリリースビルドをリンクします。
さらに第二引数に "-deploy" つけることで dll をコピーしてパッケージを作成します。

    $ cd debug # or release
    $ perl ../scripts/mingw_deloy.pl
    $ wine MMDAI.exe

## ビルドの注意点

OpenJTalk について取り扱いに注意が必要です。まず、OpenJTalk のビルドが終わっても
そのままビルドすることは出来ません。QMAOpenJTalkPlugin.pro にある手順を行う必要があります。
また、以下を行わないと実行時に SEGFAULT を起こします。

  - CXXFLAGS に -DMECAB_WITHOUT_SHARE_DIC をつける
  - "./configure --with-charset=SHIFT_JIS" で実行する

    $ export CXXFLAGS="-DMECAB_WITHOUT_SHARE_DIC"; ./configure --with-charset=SHIFT_JIS

