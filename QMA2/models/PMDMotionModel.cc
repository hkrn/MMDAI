/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2011  hkrn                                    */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the MMDAI project team nor the names of     */
/*   its contributors may be used to endorse or promote products     */
/*   derived from this software without specific prior written       */
/*   permission.                                                     */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

#include "PMDMotionModel.h"
#include "common/util.h"

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>

PMDMotionModel::PMDMotionModel(QUndoGroup *undo, QObject *parent) :
    MotionBaseModel(undo, parent),
    m_model(0),
    m_state(0)
{
    /* 空のモデルのデータを予め入れておく */
    m_roots.insert(0, RootPtr(0));
    m_keys.insert(0, Keys());
    m_values.insert(0, Values());
}

PMDMotionModel::~PMDMotionModel()
{
    if (m_model)
        m_model->discardState(m_state);
    if (m_state)
        qWarning("It seems memory leak occured: m_state");
}

QVariant PMDMotionModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole && index.column() == 0) {
        /* アイテムの名前(カテゴリ名とかボーン名とか頂点モーフ名とか)を返す */
        ITreeItem *item = static_cast<ITreeItem *>(index.internalPointer());
        return item->name();
    }
    else if (role == kBinaryDataRole && m_model) {
        /* BaseKeyFrame#write によって書き出されたキーフレームのバイナリのデータを返す */
        QVariant value = m_values[m_model].value(index);
        return value;
    }
    else {
        return QVariant();
    }
}

bool PMDMotionModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (m_model && index.isValid() && role == Qt::EditRole) {
        m_values[m_model].insert(index, value);
        setModified(true);
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

const QModelIndex PMDMotionModel::frameIndexToModelIndex(ITreeItem *item, int frameIndex) const
{
    int rowIndex = item->rowIndex();
    /* カテゴリ名を含むアイテム */
    const QModelIndex &parentIndex = index(item->parent()->rowIndex(), 0);
    const ITreeItem *parentItem = static_cast<ITreeItem *>(parentIndex.internalPointer());
    QModelIndex modelIndex;
    if (parentItem->isCategory()) {
        /* ボーン名または頂点モーフ名を含むアイテム */
        modelIndex = index(rowIndex, toModelIndex(frameIndex), parentIndex);
    }
    /* モデルのインデックスが存在しなければ作成しておき、自動的にそのインデックスが存在するように処理する */
    if (!modelIndex.isValid())
        createIndex(rowIndex, frameIndex, item);
    return modelIndex;
}

void PMDMotionModel::saveState()
{
    /* 前の状態を破棄する機能がついた saveState のラッパー */
    if (m_model) {
        m_model->discardState(m_state);
        m_state = m_model->saveState();
    }
}

void PMDMotionModel::restoreState()
{
    /* 復元とモデル状態の更新、現在の状態を破棄する機能がついた restoreState のラッパー */
    if (m_model) {
        m_model->restoreState(m_state);
        m_model->updateImmediate();
        m_model->discardState(m_state);
    }
}

void PMDMotionModel::discardState()
{
    if (m_model)
        m_model->discardState(m_state);
}

int PMDMotionModel::columnCount(const QModelIndex & /* parent */) const
{
    /* カラムは常に1つ以上存在するようにしないと assertion error が発生する */
    return m_model ? maxFrameCount() + 2 : 1;
}

void PMDMotionModel::markAsNew(vpvl::PMDModel *model)
{
    if (model == m_model)
        setModified(false);
}

void PMDMotionModel::refreshModel()
{
    /* モデルのフレーム移動なしの更新とテーブルモデルの更新両方を含む */
    updateModel();
    reset();
    emit motionDidUpdate(m_model);
}

int PMDMotionModel::maxFrameCount() const
{
    return 54000;
}

void PMDMotionModel::updateModel()
{
    if (m_model) {
        m_model->seekMotion(m_frameIndex);
        m_model->updateImmediate();
    }
}

void PMDMotionModel::addPMDModel(vpvl::PMDModel *model, const RootPtr &root, const Keys &keys)
{
    /* モデルが新規の場合はそのモデルの巻き戻しスタックを作成し、そうでない場合は該当のモデルの巻戻しスタックを有効にする */
    if (!m_stacks.contains(model)) {
        QUndoStack *stack = new QUndoStack();
        m_stacks.insert(model, UndoStackPtr(stack));
        m_undo->addStack(stack);
        m_undo->setActiveStack(stack);
    }
    else {
        m_undo->setActiveStack(m_stacks[model].data());
    }
    /* 各モデル毎のルートアイテム、ボーンまたは頂点モーフのキー名、テーブルのデータを作成する。作成済みの場合は何も処理しない */
    if (!m_roots.contains(model))
        m_roots.insert(model, root);
    if (!m_keys.contains(model))
        m_keys.insert(model, keys);
    if (!m_values.contains(model))
        m_values.insert(model, Values());
}

void PMDMotionModel::removePMDModel(vpvl::PMDModel *model)
{
    /* PMD 追加で作成されたテーブルのモデルのデータと巻き戻しスタックの破棄を行う。モデルは削除されない */
    m_model = 0;
    m_undo->setActiveStack(0);
    m_values.remove(model);
    m_keys.remove(model);
    m_stacks.remove(model);
    m_roots.remove(model);
}

void PMDMotionModel::removePMDMotion(vpvl::PMDModel *model)
{
    /* テーブルのモデルのデータの破棄と巻き戻しスタックの破棄を行う。モーションは削除されない */
    if (m_values.contains(model))
        m_values[model].clear();
    QUndoStack *stack = m_undo->activeStack();
    if (stack)
        stack->clear();
}
