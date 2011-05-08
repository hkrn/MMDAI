#
# MacOSX 上で zip を解凍する際場合によっては文字化けが発生することがあります。
# その時は MacWinZipper を使用して解凍してください。
# (10.6.6 のアーカイブユーティリティで解凍した場合は問題ないようです)
#
# レアさんのステージの入手先(下の動画説明文の中にある)
# http://www.nicovideo.jp/watch/sm8695572
# -> 解凍したディレクトリを Stage に入れる
#
# Lat式ミク(v2.3)の入手先
# http://loda.jp/lat/
# -> 解凍したディレクトリを Model に入れる
#
# Yellow モーションの入手先(下の動画説明文の中にある)
# http://www.nicovideo.jp/watch/sm12750876
# -> 解凍したディレクトリを Motion に入れる
#
# ステージの読み込み
0     1   <eps>                       STAGE|Stage/kkh01_set101112/UserFile/Model/kkhbg01_v1_1.pmd
# モデルの読み込み
1     2   <eps>                       MODEL_ADD|miku|Model/Lat式ミクVer2.3/Lat式ミクVer2.3_White.pmd
# 音源があるなら Motion にいれておく。mp3 前提で配置
# そして以下 2行の "#" を削る
#2     3   <eps>                       SOUND_START|yellow|Motion/Yellow.mp3
#3     4   SOUND_EVENT_START|yellow    MOTION_ADD|miku|morph|Motion/Yellowモーションせっと2/Yellow表情データ.vmd
# 表情データの読み込み
2     4   <eps>                       MOTION_ADD|miku|morph|Motion/Yellowモーションせっと2/Yellow表情データ.vmd
# モデルのモーションデータの読み込み
4     5   <eps>                       MOTION_ADD|miku|motion|Motion/Yellowモーションせっと2/Yellowモーションデータ.vmd
# カメラワークのデータの読み込み
5     6   <eps>                       CAMERA|Motion/Yellowモーションせっと2/Yellowカメラ.vmd

