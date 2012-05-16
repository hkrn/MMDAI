libvpvl2
========

libvpvl2 は PMX の読み込みに対応するライブラリです。
PMX の仕様は PMDEditor に付属する PMX 仕様書に準拠しています。

libvpvl2 単体では PMD の読み込みが出来ないため、libvpvl を利用しますが、
レンダリングは libvpvl2 が持つレンダリングエンジンを使用します。

# libvpvl の違い

  * PMX の読み込みが可能
    * ただし PMD の読み込みが出来ないため libvpvl を利用
  * ソフトシャドウ含むセルフシャドウの描写が可能
  * キーフレーム探索が高速
    * libvpvl では単純な線形探索、libvpvl2 はハッシュと二分探索の組み合わせで行います。
  * Dependency Injection ベースのクラス設計
    * ShiftJIS/UTF-8/UTF-16 の取り扱いを外部のライブラリに行わせるため
    * 利用側は IEncoding/IString を実装する必要がある
      * 依存するクラスを外部に任せることにより、libvpvl 同様必須なライブラリを BulletPhysics のみにとどめている。

# 使い方

libvpvl2 は libvpvl 同様 OpenGL に特化して作られています。
実装必須なクラスが必要な関係で、libvpvl に比べるとより面倒です。
実装例は render/qt/basic.cc にあります。

1. IEncoding/IString を実装し、vpvl2::Factory に IEncoding を実装したインスタンスを渡す

  IEncoding *encoding = ... // IEncoding を実装したインスタンスを作成
  Factory *factory = new Factory(encoding);

2. IRenderDelegate を実装し、vpvl2::Scene を作成する (1 と 2 は順序逆でも良い)

  IRenderDelegate *delegate = ... // IRenderDelegate を実装したインスタンスを作成
  Scene *scene = new Scene();

3. OpenGL を以下のコードで状態変数を初期化

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

4. モデルの読み込み

モデルは拡張子が PMD/PMX/X ファイルの読み込みに対応しています。
オンメモリで処理するため、モデルデータは全てメモリ上に置く必要があります。

  const uint8_t *data = ... // 外部から読み込まれたバイト配列。PMD/PMX のデータは自動的に識別する
  size_t size = ... // data の長さ
  bool ok = true;
  // 読み込みに成功した場合 ok に true が設定され、IModel のインスタンスを返す
  // 失敗した場合は ok に false が設定され、0 の値を返す
  // 失敗時 IModel#error には IModel::kNoError 以外の値が設定される
  IModel *model = factory->createModel(bytes, size, ok); 

5. モデルのアップロード

  IRenderEngine *engine = scene->createRenderEngine(delegate, model);
  IString *s = ... // モデルの絶対パスを示すディレクトリの IString のインスタンスを作成
  engine->upload(s);
  scene->addModel(model, engine);

6. モーションの作成

  const uint8_t *data = ... // 外部からの読み込まれたバイト配列
  size_t size = ... // data の長さ
  bool ok = true;
  // 読み込みに成功した場合 ok に true が設定され、IMotion のインスタンスを返す
  // 失敗した場合は ok に false が設定され、0 の値を返す
  // 失敗時 IMotion#error には IMotion::kNoError 以外の値が設定される
  IMotion *motion = factory->createMotion(bytes, size, model, ok); 
  scene->addMotion(motion);

7. モーションを動かす

7. と 8. はタイマー処理またはイベント処理を用いて定期的に実行する必要があります。

  motion->seek(0.0); // モーションをフレーム値で絶対基準で移動する
  motion->advance(1.0); // モーションをフレーム値で相対基準で前に進む

8. 射影行列を設定し、モデルビュー行列と掛け合わせる

7. と 8. はタイマー処理またはイベント処理を用いて定期的に実行する必要があります。

  float modelViewMatrix[16], modelViewProjectionMatrix[16];
  scene->matrices()->getModelView(modelViewMatrix);
  // 射影行列を設定し、modelViewMatrix と掛け合わせ、modelViewProjectionMatrix に設定する
  scene->matrices()->setModelViewProjection(modelViewProjectionMatrix);

9. レンダリングを実行

レンダリング可能なタイミングでレンダリングを実行します。

  glViewport(0, 0, width, height);
  glEnable(GL_DEPTH_TEST);
  glClearColor(1, 1, 1, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  const Array<IRenderEngine *> &engines = scene->renderEngines();
  const int nengines = engines.count();
  for (int i = 0; i < nengines; i++) {
    IRenderEngine *engine = engines[i];
    engine->renderModel();
    engine->renderEdge();
    engine->renderShadow();
  }

# ライセンス

libvpvl 同様、修正 BSD ライセンスで配布されます。

