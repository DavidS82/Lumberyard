/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/
// Original file Copyright Crytek GMBH or its affiliates, used under license.

#include "StdAfx.h"
#include "QAudioControlTreeWidget.h"
#include "QAudioControlEditorIcons.h"
#include "QtUtil.h"
#include "common/ACETypes.h"

using namespace AudioControls;

//-----------------------------------------------------------------------------------------------//
QFolderItem::QFolderItem(const QString& sName)
    : QStandardItem(sName)
{
    setIcon(GetFolderIcon());
    setData(eIT_FOLDER, eDR_TYPE);
    setData(ACE_INVALID_CID, eDR_ID);

    setFlags(flags() | Qt::ItemIsDropEnabled);
    setFlags(flags() | Qt::ItemIsDragEnabled);

    setData(false, eDR_MODIFIED);
}

//-----------------------------------------------------------------------------------------------//
QAudioControlItem::QAudioControlItem(const QString& sName, CATLControl* pControl)
    : QStandardItem(sName)
{
    setIcon(GetControlTypeIcon(pControl->GetType()));
    setData(eIT_AUDIO_CONTROL, eDR_TYPE);
    setData(pControl->GetId(), eDR_ID);

    EACEControlType eType = pControl->GetType();
    if (eType == eACET_SWITCH)
    {
        setFlags(flags() | Qt::ItemIsDropEnabled);
        setFlags(flags() | Qt::ItemIsDragEnabled);
    }
    else if (eType == eACET_SWITCH_STATE)
    {
        setFlags(flags() & ~Qt::ItemIsDropEnabled);
        setFlags(flags() & ~Qt::ItemIsDragEnabled);
    }
    else
    {
        setFlags(flags() & ~Qt::ItemIsDropEnabled);
        setFlags(flags() | Qt::ItemIsDragEnabled);
    }
    setData(false, eDR_MODIFIED);
}

//-----------------------------------------------------------------------------------------------//
QAudioControlSortProxy::QAudioControlSortProxy(QObject* pParent /*= 0*/)
    : QSortFilterProxyModel(pParent)
{
}

//-----------------------------------------------------------------------------------------------//
bool QAudioControlSortProxy::setData(const QModelIndex& index, const QVariant& value, int role /* = Qt::EditRole */)
{
    if ((role == Qt::EditRole))
    {
        QString sInitialName = value.toString();
        if (sInitialName.isEmpty() || sInitialName.contains(" "))
        {
            // TODO: Prevent user from inputing spaces to name
            return false;
        }

        if (index.data(eDR_TYPE) == eIT_FOLDER)
        {
            // Validate that the new folder name is valid
            bool bFoundValidName = false;
            QString sCandidateName = sInitialName;
            int nNumber = 1;
            while (!bFoundValidName)
            {
                bFoundValidName = true;
                int i = 0;
                QModelIndex sibiling = index.sibling(i, 0);
                while (sibiling.isValid())
                {
                    QString sSibilingName = sibiling.data(Qt::DisplayRole).toString();
                    if ((sibiling != index) && (sibiling.data(eDR_TYPE) == eIT_FOLDER) && (QString::compare(sCandidateName, sSibilingName, Qt::CaseInsensitive) == 0))
                    {
                        sCandidateName = sInitialName + "_" + QString::number(nNumber);
                        ++nNumber;
                        bFoundValidName = false;
                        break;
                    }
                    ++i;
                    sibiling = index.sibling(i, 0);
                }
            }
            return QSortFilterProxyModel::setData(index, sCandidateName, role);
        }
    }
    return QSortFilterProxyModel::setData(index, value, role);
}

//-----------------------------------------------------------------------------------------------//
bool QAudioControlSortProxy::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    uint eLeftType = sourceModel()->data(left, eDR_TYPE).toUInt();
    uint eRightType = sourceModel()->data(right, eDR_TYPE).toUInt();
    if (eLeftType != eRightType)
    {
        return eLeftType > eRightType;
    }
    return left.data(Qt::DisplayRole) > right.data(Qt::DisplayRole);
}

//-----------------------------------------------------------------------------------------------//
QAudioControlsTreeView::QAudioControlsTreeView(QWidget* pParent /*= 0*/)
    : QTreeView(pParent)
{
}

//-----------------------------------------------------------------------------------------------//
void QAudioControlsTreeView::scrollTo(const QModelIndex& index, ScrollHint hint /*= EnsureVisible*/)
{
    // QTreeView::scrollTo() expands all the parent items but
    // it is disabled when handling a Drag&Drop event so have to do it manually
    if (state() != QAbstractItemView::NoState)
    {
        QModelIndex parent = index.parent();
        while (parent.isValid())
        {
            if (!isExpanded(parent))
            {
                expand(parent);
            }
            parent = parent.parent();
        }
    }
    QTreeView::scrollTo(index, hint);
}

//-----------------------------------------------------------------------------------------------//
bool QAudioControlsTreeView::IsEditing()
{
    return state() == QAbstractItemView::EditingState;
}
