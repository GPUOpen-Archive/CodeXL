//#pragma once

// Qt
#ifndef GPOBJECTMODELS_H
#define GPOBJECTMODELS_H

#include <QStandardItemModel>
#include <QDomDocument>

class ObjTreeModel : public QStandardItemModel
{
    //    Q_OBJECT

public:
    explicit ObjTreeModel(QDomDocument document, QObject* parent = 0);
    ~ObjTreeModel();

    void setIncludeDestroyed(int iIncludeDestroyed);
    void setGroupDestroyed(int iGroupDestroyed);

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

private:
    QDomDocument m_DomDoc;
    QStandardItem* m_rootItem;  // root item of the entire tree
    int m_iIncludeDestroyed = Qt::Checked;
    int m_iGroupDestroyed = Qt::Unchecked;

    void setDomDoc(QDomDocument document);
    void refreshTree();
};

class ObjDatabaseModel : public QStandardItemModel
{
    Q_OBJECT

public:
    explicit ObjDatabaseModel(QDomDocument document, QObject* parent = 0);
    ~ObjDatabaseModel();

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

signals:
    void ObjDisplayed(QString  strObjDisplayed);

protected slots:

    /// Is handling the click of a table item
    /// \param clickedItem the item clicked
    void dBaseObjSelected(const QString strObjectSelected);

private:
    QDomDocument m_DomDoc;
    QStandardItem* m_rootItem;

    void DisplayObjectNode(QDomNode domObjectNode);
    void DisplayObject(QString strHandle);
    void FillTree(QStandardItem* pNextTreeItem, QDomNode elementNode);
};

#endif // GPOBJECTMODELS_H 
