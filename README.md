MMDAI
=====

MMDAI is a project forked from [MMDAgent](http://www.mmdagent.jp "MMDAgent") to support cross platform.

**MMDAI Project Portal2: https://hkrn.github.com/MMDAI/**

# これは何?

MMDAI とは MMDAgent からフォークしたプロジェクトです。現在は MMDAI2 の開発を就寝に行なっており、MMDAI は開発停止状態にあります。

名前にあるとおり MMD ([MikuMikuDance](http://www.geocities.jp/higuchuu4/index.htm "MikuMikuDance")) で使われているモデルやモーションの読み込みが可能です。あくまで MMD 互換であり、完全に同じ結果になることは全く保証されていません。

VPVM (a.k.a. MMDAI2) は MMDAI から音声認識機能を取り除き、代わりにモーション編集及び動画の書き出しを行えるようにしたアプリケーションです。また、MMDAI/VPVM 両方で用いられる OS やレンダリングエンジンに依存しない小型ライブラリである libvpvl2 があります。モデル形式として pmd/pmx/x 形式を、モーション形式として vmd/mvd 形式の読み込みをサポートしています。

## サポートされている OS

以下は開発で動作の確認が出来ている環境です。

  - MacOSX 10.7 64bit (10.6 以上必須)
  - Ubuntu Linux 12.04 32/64bit

公式で配布されているバイナリは MacOSX 版の x86/x64 両対応のユニバーサルバイナリと Linux 版の x86/64 バイナリです。Linux 版については Ubuntu Linux 12.04 でビルドしたバイナリとなります。

# 使い方

VPVM については [MMDAI Project Portal2](http://hkrn.github.com/MMDAI/) を参照してください。MMDAI は [docs/MMDAI.md](docs/MMDAI.md "MMDAI.md") を参照してください。

# ビルド方法

VPVM については [docs/vpvm.build.md](docs/vpvm.build.md "vpvm.build.md") にあります。

# 名前の由来

MMDAgent をぼかした名前の結果 MMDAI になっています。"AI" には特にこれといった略称はありません。
内部的に使用している名前である QMA は元々 "Qt MMD Agent" から名前をとっています。

VPVM は VOCALOID Promotion Video Maker の頭文字をとっており、VPVL も VOCALOID Promotion Video Library の
頭文字をとっています。これらは MMD を配布しているサイト名である VPVP (VOCALOID Promotion Video Project)
からとっています。

# ライセンス

MMDAgent と同じく修正 BSD ライセンスの下で配布されています。LICENSE を参照してください。

ただし MMDAI/MMDAI2 は [Qt](http://qt.digia.com "Qt") を必要とするため、MMDAI/MMDAI2 を元に改造して派生品として作成しかつ利用規約に
リバースエンジニアリングをも禁止して配布したい場合は Qt の商用ライセンスを購入する必要があります。開発ではなく単純に利用するだけならもちろん無料で利用可能です。

# 開発者

[AUTHORS.md](AUTHORS.md "AUTHORS.md") を参照してください

