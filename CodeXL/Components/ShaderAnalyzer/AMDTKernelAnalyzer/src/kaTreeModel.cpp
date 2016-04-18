//------------------------------ kaTreeModel.cpp ------------------------------

#include <stdio.h>

#include <set>
#include <string>
#include <vector>

#ifdef _WIN32
    #pragma warning(push)
    #pragma warning(disable : 4127)
    #pragma warning(disable : 4251)
    #pragma warning(disable : 4800)
#endif

#include <QString>
#include <QGridLayout>
#include <QCloseEvent>
#include <QTreeView>

#ifdef _WIN32
    #pragma warning(pop)
#endif

#include <AMDTKernelAnalyzer/src/kaTreeModel.h>

using std::list;
using std::string;
using std::vector;

static const QChar DEPTH_INDENT = QChar('\t');
static const QChar CHAR_TRUE = QChar('1');
static const QChar CHAR_FALSE = QChar('0');
static const QChar CHAR_DELIMITER = QChar('-');

//-----------------------------------------------------------------------------
CheckableTreeItem::CheckableTreeItem(const QString& itemName,
                                     bool checked,
                                     CheckableTreeItem* pNodeParent)
    : m_pParent(pNodeParent)
{
    Qt::CheckState state;

    if (true == checked)
    {
        state = Qt::Checked;
    }
    else
    {
        state = Qt::Unchecked;
    }

    m_Data.reset(new std::pair<QString, Qt::CheckState>(itemName, state));

    // Update parent state for partially checked state
    updateParentState();
}

//-----------------------------------------------------------------------------
void CheckableTreeItem::appendChild(boost::shared_ptr<CheckableTreeItem>& child)
{
    m_ChildList.push_back(child);
}

//-----------------------------------------------------------------------------
CheckableTreeItem* CheckableTreeItem::getParent() const
{
    return m_pParent;
}

//-----------------------------------------------------------------------------
CheckableTreeItem* CheckableTreeItem::getChild(int row) const
{
    assert(row >= 0);
    assert((unsigned int)row < m_ChildList.size());

    return m_ChildList[row].get();
}

//-----------------------------------------------------------------------------
int CheckableTreeItem::childCount() const
{
    return m_ChildList.size();
}

//-----------------------------------------------------------------------------
int CheckableTreeItem::row() const
{
    int ret = 0;

    if (NULL != m_pParent)
    {
        unsigned int i;

        // Find index of this object on the parent's child list.
        for (i = 0; i < m_pParent->m_ChildList.size(); ++i)
        {
            if (m_pParent->m_ChildList[i].get() == this)
            {
                break;
            }
        }

        ret = (int)i;
    }

    return ret;

}

//-----------------------------------------------------------------------------
int CheckableTreeItem::columnCount() const
{
    // Only one column
    return 1;
}

//-----------------------------------------------------------------------------
std::pair<QString, Qt::CheckState> CheckableTreeItem::data(int column) const
{
    // column is ignored.  Passing column is for fitting TreeModel
    // framework
    (void)column;
    std::pair<QString, Qt::CheckState> myData(m_Data->first, m_Data->second);
    return myData;
}

//-----------------------------------------------------------------------------
void CheckableTreeItem::setChecked(bool checked)
{
    if (true == checked)
    {
        m_Data->second = Qt::Checked;
    }
    else
    {
        m_Data->second = Qt::Unchecked;
    }

    // All child nodes should have same check status
    std::vector<boost::shared_ptr<CheckableTreeItem> >::iterator it;

    for (it = m_ChildList.begin(); it != m_ChildList.end(); ++it)
    {
        // Recursively set all nodes on the sub-tree.
        (*it)->setChecked(checked);
    }

    // If is a leaf node, update parent chain.
    if (0 == childCount())
    {
        // Change parent node state to partially checked accordingly
        updateParentState();
    }
}

//-----------------------------------------------------------------------------
bool CheckableTreeItem::isChecked() const
{
    bool ret = false;

    if (Qt::Checked == m_Data->second)
    {
        ret = true;
    }

    return ret;
}

//-----------------------------------------------------------------------------
Qt::CheckState CheckableTreeItem::getTristate() const
{
    return m_Data->second;
}

//-----------------------------------------------------------------------------
void CheckableTreeItem::updateParentState()
{
    if (NULL != m_pParent)
    {
        std::vector<boost::shared_ptr<CheckableTreeItem> >* pChildList = &m_pParent->m_ChildList;
        std::vector<boost::shared_ptr<CheckableTreeItem> >::iterator it;
        unsigned int checkedCount = 0;

        for (it = pChildList->begin(); it != pChildList->end(); ++it)
        {
            if (Qt::Checked == (*it)->getTristate())
            {
                ++checkedCount;
            }
        }

        if (pChildList->size() == checkedCount)
        {
            // All checked
            m_pParent->m_Data->second = Qt::Checked;
        }
        else if (0 == checkedCount)
        {
            // All unchecked
            m_pParent->m_Data->second = Qt::Unchecked;
        }
        else
        {
            // Partially checked
            m_pParent->m_Data->second = Qt::PartiallyChecked;
        }

        // If is the second bottom layer, should not have partially checked state because the bottom layer has no check box
        if (0 == m_ChildList.size())
        {
            if (Qt::PartiallyChecked == m_pParent->m_Data->second)
            {
                m_pParent->m_Data->second = Qt::Checked;
            }
        }

        // Recursively go up parent chain
        m_pParent->updateParentState();
    }
}

//-----------------------------------------------------------------------------
void CheckableTreeItem::setupData(CheckableTreeItem* pParent, QStringList* pLines)
{
    if (0 < pLines->size())
    {
        CheckableTreeItem* pLocalParent = NULL;
        int localIndentCount = 0;
        bool checked;
        QString dataName;

        // Parse the first line
        QString str = pLines->front();

        // Count how many indents at the front.  The number of indents represents depth of
        // the node on the tree.
        int indentCount = CheckableTreeItem::removeIndents(str);

        while (0 < pLines->size())
        {
            str = pLines->front();
            localIndentCount = CheckableTreeItem::removeIndents(str);

            if (localIndentCount == indentCount)
            {
                // Keep adding node
                CheckableTreeItem::decodeData(str, &dataName, &checked);

                // Create a new node and add it to the tree
                boost::shared_ptr<CheckableTreeItem> item(new CheckableTreeItem(dataName, checked, pParent));
                pParent->appendChild(item);
                item->updateParentState();
                pLocalParent = item.get();

                // Done with the front line.  Remove it.
                pLines->pop_front();
            }
            else if (localIndentCount > indentCount)
            {
                // Recursively go to one more level
                setupData(pLocalParent, pLines);
            }
            else // localIndentCount < indentCount
            {
                // Break the loop and return
                break;
            }
        }
    }
}

//-----------------------------------------------------------------------------
void CheckableTreeItem::updateData(CheckableTreeItem* pParent, QStringList* pLines)
{
    if (0 < pLines->size())
    {
        CheckableTreeItem* pLocalParent = NULL;
        int localIndentCount = 0;
        bool checked;
        QString dataName;
        QString destDataName;

        // Parse the first line
        QString str = pLines->front();

        // Count how many indents at the front.  The number of indents represents depth of
        // the node on the tree.
        int indentCount = CheckableTreeItem::removeIndents(str);
        int i = 0;
        CheckableTreeItem* pItem;

        while (0 < pLines->size())
        {
            str = pLines->front();
            localIndentCount = CheckableTreeItem::removeIndents(str);

            if (localIndentCount == indentCount)
            {
                // Keep updating node
                CheckableTreeItem::decodeData(str, &dataName, &checked);

                assert(i < pParent->childCount());
                pItem = pParent->getChild(i);
                destDataName = pItem->data(0).first;

                assert(dataName == destDataName);

                pItem->setChecked(checked);
                pLocalParent = pItem;

                // Done with the front line.  Remove it.
                pLines->pop_front();
                ++i;
            }
            else if (localIndentCount > indentCount)
            {
                // Recursively go to one more level
                updateData(pLocalParent, pLines);
            }
            else // localIndentCount < indentCount
            {
                // Break the loop and return
                break;
            }
        }
    }
}

//-----------------------------------------------------------------------------
void CheckableTreeItem::getData(CheckableTreeItem* pParent,
                                int depth,
                                QStringList* pLinesOut)
{
    int childCount = pParent->childCount();
    QString tabs;
    QString encodedStr;
    CheckableTreeItem* pChild;
    QString name;

    if (0 < depth)
    {
        tabs = QString(depth, DEPTH_INDENT);
    }

    int i;

    for (i = 0; i < childCount; ++i)
    {
        pChild = pParent->getChild(i);
        name = pChild->data(0).first;
        encodedStr = CheckableTreeItem::encodeData(name, pChild->isChecked());
        QString line = tabs + encodedStr;
        pLinesOut->push_back(line);

        // Recursively go to child nodes.
        getData(pChild, depth + 1, pLinesOut);
    }
}

//-----------------------------------------------------------------------------
void CheckableTreeItem::getCheckedName(const QStringList& linesIn,
                                       int treeDepthLevel,
                                       QStringList* pLinesOut)
{
    pLinesOut->clear();
    QString str;
    bool checked;
    QStringList::const_iterator it;
    int level;

    for (it = linesIn.begin(); it != linesIn.end(); ++it)
    {
        level = decodeData(*it, &str, &checked);

        if (level == treeDepthLevel && true == checked)
        {
            pLinesOut->push_back(str);
        }
    }
}

//-----------------------------------------------------------------------------
QString CheckableTreeItem::encodeData(const QString& strIn, bool checked, int depth)
{
    QString str;

    if (0 < depth)
    {
        str = QString(depth, DEPTH_INDENT) + strIn;
    }
    else
    {
        str = strIn;
    }

    str.append(CHAR_DELIMITER);

    if (true == checked)
    {
        str.append(CHAR_TRUE);
    }
    else
    {
        str.append(CHAR_FALSE);
    }

    return str;
}

//-----------------------------------------------------------------------------
int CheckableTreeItem::decodeData(const QString& strIn, QString* pStrOut, bool* pBoolOut)
{
    QChar lastChar = strIn.at(strIn.length() - 1);

    if (CHAR_FALSE == lastChar)
    {
        *pBoolOut = false;
    }
    else
    {
        *pBoolOut = true;
    }

    if (&strIn != pStrOut)
    {
        *pStrOut = strIn;
    }

    // Chop two chars at the end. The chopped chars include a delimiter and a boolean char.
    pStrOut->chop(2);
    return removeIndents(*pStrOut);
}

//-----------------------------------------------------------------------------
QString CheckableTreeItem::encodeHeaderData(const QString& strIn)
{
    // Pass the string unchanged at this implementation
    return strIn;
}

//-----------------------------------------------------------------------------
int CheckableTreeItem::removeIndents(QString& str)
{
    int ret;

    for (ret = 0; ret < str.length(); ++ret)
    {
        if (DEPTH_INDENT != str.at(ret))
        {
            break;
        }
    }

    // Remove indents
    str = str.right(str.length() - ret);

    return ret;
}

//-----------------------------------------------------------------------------
void CheckableTreeItem::toggleStr(QString& str)
{
    QChar lastChar = str.at(str.length() - 1);

    if (CHAR_FALSE == lastChar)
    {
        lastChar = CHAR_TRUE;
    }
    else
    {
        lastChar = CHAR_FALSE;
    }

    // Replace the last char
    str.chop(1);
    str.append(lastChar);
}

//-----------------------------------------------------------------------------
kaTreeModel::kaTreeModel(const QStringList& lines,  QTreeView* pParent)
    : QAbstractItemModel(pParent)
{
    // Make a copy of QStringList
    QStringList linesCopy(lines);

    if (!linesCopy.isEmpty())
    {
        // Read the first line
        QString str = linesCopy.front();

        // Create root item which has no meaningful check status
        m_pRootItem.reset(new CheckableTreeItem(str, false, NULL));

        // Done with the first line.  Remove it.
        linesCopy.pop_front();
    }

    // Recursively create tree model
    setupModelData(&linesCopy, m_pRootItem.get());

    // The following connection will connect tree view's click event
    // to this model's update procedures.
    connect(pParent, SIGNAL(clicked(const QModelIndex&)),
            this,    SLOT(clicked(const QModelIndex&)));
}

//-----------------------------------------------------------------------------
QVariant kaTreeModel::data(const QModelIndex& index, int role) const
{
    QVariant ret;

    if (true == index.isValid())
    {
        if (Qt::CheckStateRole == role)
        {
            // This role is sent only if Qt::ItemIsUserCheckable is returned in flags()
            CheckableTreeItem* pItem = static_cast<CheckableTreeItem*>(index.internalPointer());

            // Returning checked state here will update check box in a TreeView.
            // The checked state is updated in slot function clicked() when an item is clicked
            // in the TreeView.
            if (0 < pItem->childCount())
            {
                // Return status only for non-leaf nodes.  QT will put a check box for the node.
                // This is where you control whether display a checkbox or not.
                ret = static_cast< int >(pItem->getTristate());
            }

            // else return QVariant() which will make QT not display a check box.
        }
        else if (Qt::DisplayRole == role)
        {
            CheckableTreeItem* pItem = static_cast<CheckableTreeItem*>(index.internalPointer());
            std::pair<QString, Qt::CheckState> myData = pItem->data(index.column());

            // Returning a string here will update item string in a TreeView.
            ret = QVariant(myData.first);
        }
    }

    return ret;
}

//-----------------------------------------------------------------------------
Qt::ItemFlags kaTreeModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags ret = Qt::NoItemFlags;

    if (true == index.isValid())
    {
        ret = Qt::ItemIsEnabled  | Qt::ItemIsUserCheckable;
    }

    return ret;
}

//-----------------------------------------------------------------------------
QVariant kaTreeModel::headerData(int section, Qt::Orientation orientation,
                                 int role) const
{
    QVariant ret;

    if (Qt::Horizontal == orientation && Qt::DisplayRole == role)
    {
        std::pair<QString, Qt::CheckState> sb = m_pRootItem->data(section);
        ret = QVariant(sb.first);
    }

    return ret;
}

//-----------------------------------------------------------------------------
QModelIndex kaTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    QModelIndex ret = QModelIndex();

    if (true == hasIndex(row, column, parent))
    {
        CheckableTreeItem* pParentItem;

        if (false == parent.isValid())
        {
            pParentItem = m_pRootItem.get();
        }
        else
        {
            pParentItem = static_cast<CheckableTreeItem*>(parent.internalPointer());
        }

        CheckableTreeItem* pChildItem = pParentItem->getChild(row);

        if (NULL != pChildItem)
        {
            ret = createIndex(row, column, pChildItem);
        }
    }

    return ret;
}

//-----------------------------------------------------------------------------
QModelIndex kaTreeModel::parent(const QModelIndex& index) const
{
    QModelIndex ret = QModelIndex();

    if (true == index.isValid())
    {
        CheckableTreeItem* pChildItem = static_cast<CheckableTreeItem*>(index.internalPointer());
        CheckableTreeItem* pParentItem = pChildItem->getParent();

        if (pParentItem != m_pRootItem.get())
        {
            ret = createIndex(pParentItem->row(), 0, pParentItem);
        }
    }

    return ret;
}

//-----------------------------------------------------------------------------
int kaTreeModel::rowCount(const QModelIndex& parent) const
{
    int ret = 0;
    CheckableTreeItem* pParentItem;

    if (0 >= parent.column())
    {
        if (false == parent.isValid())
        {
            pParentItem = m_pRootItem.get();
        }
        else
        {
            pParentItem = static_cast<CheckableTreeItem*>(parent.internalPointer());
        }

        if (NULL != pParentItem)
        {
            ret = pParentItem->childCount();
        }
    }

    return ret;
}

//-----------------------------------------------------------------------------
int kaTreeModel::columnCount(const QModelIndex& parent) const
{
    int ret;

    if (true == parent.isValid())
    {
        ret = static_cast<CheckableTreeItem*>(parent.internalPointer())->columnCount();
    }
    else
    {
        ret = m_pRootItem->columnCount();
    }

    return ret;
}

//-----------------------------------------------------------------------------
void kaTreeModel::getStrData(QStringList* pLinesOut) const
{
    pLinesOut->clear();

    QString name = m_pRootItem->data(0).first;

    // Header has no check mark
    pLinesOut->push_back(name);

    // Recursively get data.  The root depth is 0, so the next depth is 1.
    getModelData(m_pRootItem.get(), 1, pLinesOut);
}

//-----------------------------------------------------------------------------
void kaTreeModel::updateData(const QStringList& linesIn)
{
    QStringList sList = linesIn;

    // Skip the first line which is a header
    sList.pop_front();

    CheckableTreeItem::updateData(m_pRootItem.get(), &sList);
}

//-----------------------------------------------------------------------------
void kaTreeModel::checkAll(bool checkOrNot)
{
    m_pRootItem->setChecked(checkOrNot);
    int childCount = m_pRootItem->childCount();
    QModelIndex rootModelIndex;

    for (int i = 0; i < childCount; ++i)
    {
        rootModelIndex = index(i, 0);
        updateSubTree(rootModelIndex);
    }
}

//-----------------------------------------------------------------------------
void kaTreeModel::setupModelData(QStringList* pLines, CheckableTreeItem* pParent)
{
    CheckableTreeItem::setupData(pParent, pLines);
}

//-----------------------------------------------------------------------------
void kaTreeModel::getModelData(CheckableTreeItem* pParent, int depth, QStringList* pLinesOut) const
{
    CheckableTreeItem::getData(pParent, depth, pLinesOut);
}

//-----------------------------------------------------------------------------
void kaTreeModel::updateSubTree(const QModelIndex& node)
{
    if (true == node.isValid())
    {
        // Emit this signal to repaint checkbox images.  This is a signal provided by the parent class.
        emit dataChanged(node, node);

        // Go through sub tree to update.
        int i;
        int j;
        QModelIndex child;

        CheckableTreeItem* pNodeItem = static_cast<CheckableTreeItem*>(node.internalPointer());
        int rowCount = pNodeItem->childCount();
        int colCount = pNodeItem->columnCount();

        for (j = 0; j < colCount; ++j)
        {
            for (i = 0; i < rowCount; ++i)
            {
                child = node.child(i, j);

                // Recursively update child tree.
                updateSubTree(child);
            }
        }
    }
}

//-----------------------------------------------------------------------------
void kaTreeModel::updateParentChain(const QModelIndex& parent)
{
    if (true == parent.isValid())
    {
        // Emit this signal to repaint checkbox images.  This is a signal provided by the parent class.
        emit dataChanged(parent, parent);

        // Recursively go up parent chain
        updateParentChain(parent.parent());
    }
}

//-----------------------------------------------------------------------------
void kaTreeModel::clicked(const QModelIndex& index)
{
    // This slot function should be connected to a QTreeView clicked() signal.
    // QTreeView does not update a checkbox once the checkbox is clicked. This
    // function updates tree item status and emits dataChanged() signal which
    // invokes data() and refresh/paint functions.
    if (true == index.isValid())
    {
        CheckableTreeItem* pChildItem = static_cast<CheckableTreeItem*>(index.internalPointer());

        // Check if this node has a check box
        int role = Qt::CheckStateRole;
        QVariant v = data(index, role);

        // If has a check box, toggle the node value.
        if (true == v.isValid())
        {
            bool checked = pChildItem->isChecked();

            // Toggle the bool value
            checked ^= 1;

            // Note: setChecked() will set all child nodes to the same checked state.
            // This function also update parent node's check state.
            // This function only updates tree data.  The tree image is updated below.
            // This is an opportunity to change child nodes name as well if required.
            pChildItem->setChecked(checked);

            // Recursively update sub tree.  This will repaint sub-tree
            updateSubTree(index);

            // Recursively update parent chain for check state. This will repaint parent chain.
            updateParentChain(index);
        }
    }
}
