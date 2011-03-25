MMDAI - MMDAI is a fork project of MMDAgent (http://www.mmdagent.jp)
=====

## What's this?
MMDAI とは MMDAgent からフォークしたプロジェクトです。
主な目的はクロスプラットフォームで動かせるようにすることです。
(MMDAgent は Win32 API に強く依存する設計になっているため)

## Supported OS
以下は動作の確認が出来ている環境です。

 - Windows7 (MSVC 9.0) 32bit
 - MacOSX 10.6 32/64bit
 - Fedora 14 32bit
 - Ubuntu 10.10 64bit

## How to use?
オリジナルの MMDAgent とあわせていますが、現時点で一部の動作のみ実装しています。

### メニューからの操作
キーボード、マウス、及びドラッグアンドドロップの操作は基本的にここでも可能です。
  
### キーボードでの操作
 - ↑
  - カメラビューを上に回転
 - ↓
  - カメラビューを下に回転
 - ←
  - カメラビューを左に回転
 - →
  - カメラビューを右に回転
 - CTRL + ↑
  - カメラビューを上に移動
 - CTRL + ↓
  - カメラビューを下に移動
 - CTRL + ←
  - カメラビューを左に移動
 - CTRL + →
  - カメラビュー右に移動
 - B
  - 全てのモデルのボーンの表示
 - P
  - 物理エンジンの有効/無効
 - SHIFT + X
  - シャドウマップの有効/無効
 - X
  - シャドウマップの影優先または光優先の切り替え
 - ALT + ? or ALT + /
  - アプリケーションについてのダイアログの表示
 - +
  - カメラビューの拡大
 - -
  - カメラビューの縮小
 - DELETE
  - 選択されたモデルの削除。選択されていなければなにもしない
 - ESCAPE
  - アプリケーションの終了

### マウスを使った操作

 - ドラッグ
  - カメラビューの回転
 - SHIFT を押しながらのドラッグ
  - カメラビューの平行移動
 - ホイール操作
  - カメラビューの拡大及び縮小
 - CTRL を押しながらホイール操作
  - 高速なカメラビューの拡大縮小
 - SHIFT を押しながらホイール操作
  - 低速なカメラビューの拡大縮小
 - SHIFT + CTRL を押しながらのドラッグ
  - 光源の移動
 - CTRL を押しながらモデルをドラッグ
  - モデルの平行移動
 - モデルのダブルクリック
  - モデルの選択

### ドラッグ and ドロップ

 - PMD ファイル
  - モーションを維持しながら選択されたモデルを変更
 - CTRL を押しながら PMD ファイル
  - モデルを追加(最大20まで)
 - VMD ファイル
  - 選択されたモデルに対してモーションを追加
 - CTRL を押しながら VMD ファイル
  - 全てのモデルに対してモーションを追加
 - XPMD ファイル
  - ステージを変更
 - BMP/TGA/PNG ファイル
  - フロアのテクスチャを変更
 - CTRL を押しながら BMP/TGA/PNG ファイル
  - 背景のテクスチャを変更

## プラグインについて

現在以下のプラグインが入っています。

### QMAAquesTalk2Plugin

音声合成ライブラリである AquesTalk2 を使って喋らせるプラグインです。
以下が利用可能です。喋らせる文字列は AquesTalk2 に渡す文字列と同様です。
引数は MMDAgent の OpenJTalk プラグインにあわせています。

 - Command
  - MMDAI_AQTK2_START(エイリアス名,phontファイルのパス,会話内容)
  - MMDAI_AQTK2_STOP(エイリアス名)

### QMAAudioPlugin

BGM または SE を流すためのプラグインです。以下が利用可能です。
MMDAgent のコマンド及びイベントと互換性があります。

 - Command
  - SOUND_START(エイリアス名, ファイル名)
  - SOUND_STOP(エイリアス名)
 - Event
  - SOUND_EVENT_START(エイリアス名, ファイル名)
  - SOUND_EVENT_STOP(エイリアス名)

### QMAJuliusPlugin

音声認識エンジンである Julius を用いて音声認識を行うプラグインです。
以下が利用可能です。MMDAgent のコマンド及びイベントと互換性があります。

 - EVENT
  - RECOG_EVENT_START()
  - RECOG_EVENT_STOP(認識結果の内容)

### QMALookAtPlugin

マウスのカーソルに合わせてモデルの顔が動くプラグインです。
キーボードの "L" を押すことによって有効無効を切り替えることが出来ます。

### QMAOpenJTalkPlugin

音声合成ライブラリである OpenJTalk を用いて喋らせるプラグインです。
以下が利用可能です。MMDAgent のコマンド及びイベントと互換性があります。

 - COMMAND 
  - SYNTH_START(エイリアス名,表情名,会話内容)
  - SYNTH_STOP(エイリアス名)
 - EVENT
  - SYNTH_EVENT_START(エイリアス名)
  - SYNTH_EVENT_STOP(エイリアス名)

※ SYNTH_STOP コマンドは動作しません

### QMAVILuaPlugin

Lua を用いてコマンド及びイベントを制御するプラグインです。
アプリケーションディレクトリの MMDAI.lua を読み込んで実行します。
スクリプトの文字コードは必ず UTF-8 にしてください。

Lua に以下の拡張が行われます。

 - mmdai.command(command, arg1, ...)
コマンドを実行します。第一引数にコマンド名を渡します。必須です。
第二引数以降の引数は任意であり、可変です。

 - mmdai.event(type, arg1, ...)
イベントを実行します。第一引数にイベント名を渡します。必須です。
第二引数以降の引数は任意であり、可変です。

 - mmdai_handle_command(command, ...)
プラグインが呼び出すコマンドを取り扱うコールバック関数です。
第一引数にコマンド名が渡されます。第二引数に可変引数が入ります。

 - mmdai_handle_event(event, ...)
プラグインが呼び出すイベントを取り扱うコールバック関数です。
第一引数にイベント名が渡されます。第二引数に可変引数が入ります。

mmdai.command 及び mmdai.event はすぐに実行されません。
関数の実行が終了してからはじめて実行されます。

### QMAVariablePlugin

浮動小数点値の変数の設定及びタイマーの設定が可能なプラグインです。
MMDAgent のコマンド及びイベントと互換性があります。

 - Command
  - VALUE_SET(変数名,値)
  - VALUE_SET(変数名,ランダムの最小値,ランダムの最大値)
  - VALUE_UNSET(変数名)
  - VALUE_EVAL(変数名,演算子[EQ/NE/LT/LE/GT/GE],値)
  - TIMER_START(タイマー名,秒数)
  - TIMER_STOP(タイマー名)
 - Event
  - VALUE_EVENT_SET(変数名)
  - VALUE_EVENT_UNSET(変数名)
  - VALUE_EVENT_EVAL(変数名,演算子[EQ/NE/LT/LE/GT/GE],値,結果[TRUE/FALSE])
  - TIMER_EVENT_START(タイマー名)
  - TIMER_EVENT_STOP(タイマー名)

### QMAVIManagerPlugin

MMDAgent の fst を取り扱ってイベント及びコマンドを制御するプラグインです。
アプリケーションディレクトリにある MMDAI.fst を読み込んで実行します。
fst の文字コードは Shift-JIS にしてください。

## License
Distributed under new BSD License. See also LICENSE

## Authors
See AUTHORS

