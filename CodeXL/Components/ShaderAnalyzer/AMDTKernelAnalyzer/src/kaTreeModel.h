//------------------------------ kaTreeModel.h ------------------------------

#ifndef _TREE_MODEL_H_
#define _TREE_MODEL_H_

#include <set>
#include <string>
#include <vector>

#ifdef _WIN32
    #pragma warning(push)
    #pragma warning(disable : 4127)
    #pragma warning(disable : 4251)
    #pragma warning(disable : 4800)
#endif

#include <QAbstractItemModel>
#include <QString>

#ifdef _WIN32
    #pragma warning(pop)
#endif

#include "boost/smart_ptr.hpp"

class QTreeView;

/// Class for checkable tree struct
class CheckableTreeItem
{
public:

    //-----------------------------------------------------------------------------
    /// Constructor
    /// \param[in] itemName     A string for this item.
    /// \param[in] checked      A boolean value
    /// \param[in] pNodeParent  A parent node
    //-----------------------------------------------------------------------------
    CheckableTreeItem(const QString& itemName, bool checked, CheckableTreeItem* pNodeParent = NULL);

    //-----------------------------------------------------------------------------
    /// Append a child node
    /// \param[in] child     A child node
    //-----------------------------------------------------------------------------
    void appendChild(boost::shared_ptr<CheckableTreeItem>& child);

    //-----------------------------------------------------------------------------
    /// Get parent node
    /// \returns  A parent node
    //-----------------------------------------------------------------------------
    CheckableTreeItem*        getParent() const;

    //-----------------------------------------------------------------------------
    /// Get parent node
    /// \returns  A parent node
    //-----------------------------------------------------------------------------
    CheckableTreeItem*        getChild(int row) const;

    //-----------------------------------------------------------------------------
    /// Get number of child nodes
    /// \returns  Number of child nodes
    //-----------------------------------------------------------------------------
    int                       childCount() const;

    //-----------------------------------------------------------------------------
    /// Get row position of this node in the parent node
    /// \returns  A row index
    //-----------------------------------------------------------------------------
    int                       row() const;

    //-----------------------------------------------------------------------------
    /// Get column count
    /// \returns  A column count
    //-----------------------------------------------------------------------------
    int                       columnCount() const;

    //-----------------------------------------------------------------------------
    /// Get data
    /// \param[in] column   A column index.  Ingnored in this implementation
    /// \returns  A data which is the copy of the internal data
    //-----------------------------------------------------------------------------
    std::pair<QString, Qt::CheckState>  data(int column) const;

    //-----------------------------------------------------------------------------
    /// Set check state
    /// \param[in] checked   true for check, false for uncheck.
    //-----------------------------------------------------------------------------
    void                      setChecked(bool checked);

    //-----------------------------------------------------------------------------
    /// Get check state of this node
    /// \returns  true if checked, false if unchecked
    //-----------------------------------------------------------------------------
    bool                      isChecked() const;

    //-----------------------------------------------------------------------------
    /// Get tristate of this node
    /// \returns  Qt::Unchecked, Qt::PartiallyChecked or Qt::Checked
    //-----------------------------------------------------------------------------
    Qt::CheckState            getTristate() const;

    //-----------------------------------------------------------------------------
    /// Set parent state according to child nodes state
    //-----------------------------------------------------------------------------
    void                      updateParentState();

    //-----------------------------------------------------------------------------
    /// Set up data
    /// \param[in] pParent     A pointer to a parent node
    /// \param[in] pLines      A string list which has correct indents and check status.
    //-----------------------------------------------------------------------------
    static void setupData(CheckableTreeItem* pParent, QStringList* pLines);

    //-----------------------------------------------------------------------------
    /// Update data
    /// \param[in] pParent     A pointer to a parent node
    /// \param[in] pLines      A string list which has correct indents and check status.
    //-----------------------------------------------------------------------------
    static void updateData(CheckableTreeItem* pParent, QStringList* pLines);

    //-----------------------------------------------------------------------------
    /// Get data.  This is the reverse of setupData()
    /// \param[in]  pParent     A pointer to a parent node
    /// \param[in]  depth       A depth for tree struct.
    /// \param[out] pLinesOut   A string list to be populated
    //-----------------------------------------------------------------------------
    static void getData(CheckableTreeItem* pParent, int depth, QStringList* pLinesOut);

    //-----------------------------------------------------------------------------
    /// Get a list of checked name on leaf node
    /// \param[in]  linesIn          A list of string with indents and check status information
    /// \param[in]  treeDepthLevel   A tree depth level from which to retrieve the name
    /// \param[out] pLinesOut        A string list to be populated
    //-----------------------------------------------------------------------------
    static void getCheckedName(const QStringList& linesIn, int treeDepthLevel, QStringList* pLinesOut);

    //-----------------------------------------------------------------------------
    /// Encode a string data
    /// \param[in] strIn     A string in
    /// \param[in] checked   A check state
    /// \param[in] depth     A nest depth
    /// \returns  An encoded string
    //-----------------------------------------------------------------------------
    static QString encodeData(const QString& strIn, bool checked, int depth = 0);

    //-----------------------------------------------------------------------------
    /// Decode a string which is encoded by encodeData()
    /// \param[in]  strIn      A string in
    /// \param[out] pStrOut    A string to be filled
    /// \param[out] pBoolOut   A bool value to be filled
    /// \returns  A data depth level
    //-----------------------------------------------------------------------------
    static int decodeData(const QString& strIn, QString* pStrOut, bool* pBoolOut);

    //-----------------------------------------------------------------------------
    /// Encode header data
    /// \param[in]  strIn      A string in
    /// \returns  An encoded header string
    //-----------------------------------------------------------------------------
    static QString encodeHeaderData(const QString& strIn);

    //-----------------------------------------------------------------------------
    /// Remove indents
    /// \param[in]  str      A string which is modified
    /// \returns  Number of indents removed.
    //-----------------------------------------------------------------------------
    static int     removeIndents(QString& str);

    //-----------------------------------------------------------------------------
    /// Toggle string check state
    /// \param[in]  str      A string encoded by encodeData()
    //-----------------------------------------------------------------------------
    static void    toggleStr(QString& str);

private:

    /// A list of child nodes
    std::vector<boost::shared_ptr<CheckableTreeItem> > m_ChildList;

    /// Node string data.  Can be changed to QVariant for various data type.
    boost::shared_ptr<std::pair<QString, Qt::CheckState> >  m_Data;

    /// Pointer to the parent node.
    CheckableTreeItem*                             m_pParent;
};

/// Class for a tree model
class kaTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:

    //-----------------------------------------------------------------------------
    /// Constructor
    /// \param[in] lines     A list of string.  Each line is encoded by CheckableTreeItem::encodeData()
    /// \param[in] pParent   A tree view
    //-----------------------------------------------------------------------------
    kaTreeModel(const QStringList& lines, QTreeView* pParent);

    //-----------------------------------------------------------------------------
    /// Implement parent class's pure virtual function
    /// \param[in] index  A tree view node index
    /// \param[in] role   A role indicating what the caller wants
    //-----------------------------------------------------------------------------
    virtual QVariant data(const QModelIndex& index, int role) const;

    //-----------------------------------------------------------------------------
    /// Implement parent class's pure virtual function
    /// \param[in] index  A tree view node index
    /// \returns  An int which has several flags ORd.
    //-----------------------------------------------------------------------------
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;

    //-----------------------------------------------------------------------------
    /// Implement parent class's pure virtual function
    /// \param[in] section      A column index
    /// \param[in] orientation  An orientation
    /// \param[in] role         A role indicating what the caller wants
    /// \returns  A header data
    //-----------------------------------------------------------------------------
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const;

    //-----------------------------------------------------------------------------
    /// Implement parent class's pure virtual function
    /// \param[in] row      A row index
    /// \param[in] column   A column index
    /// \param[in] parent   An index of a parent
    /// \returns  A model index of the node on the row and column
    //-----------------------------------------------------------------------------
    virtual QModelIndex index(int row, int column,
                              const QModelIndex& parent = QModelIndex()) const;

    //-----------------------------------------------------------------------------
    /// Implement parent class's pure virtual function
    /// \param[in] index     An index of a trew veiw node
    /// \returns  A parent index of the node.
    //-----------------------------------------------------------------------------
    virtual QModelIndex parent(const QModelIndex& index) const;

    //-----------------------------------------------------------------------------
    /// Implement parent class's pure virtual function
    /// \param[in] parent    An index of a parent node on a tree view
    /// \returns  Number of child nodes the parent has
    //-----------------------------------------------------------------------------
    virtual int  rowCount(const QModelIndex& parent = QModelIndex()) const;

    //-----------------------------------------------------------------------------
    /// Implement parent class's pure virtual function
    /// \param[in] parent    An index of a parent node on a tree view
    /// \returns  Number of column the parent has
    //-----------------------------------------------------------------------------
    virtual int  columnCount(const QModelIndex& parent = QModelIndex()) const;

    //-----------------------------------------------------------------------------
    /// Convert internal tree model to a string list which is good for constructing another tree model
    /// \param[out] pLinesOut    A string list to be populated
    //-----------------------------------------------------------------------------
    void getStrData(QStringList* pLinesOut) const;

    //-----------------------------------------------------------------------------
    /// Update internal data using an external string list.
    /// \param[in] linesIn   A string list to update internal data
    //-----------------------------------------------------------------------------
    void updateData(const QStringList& linesIn);

    //-----------------------------------------------------------------------------
    /// Check or uncheck all nodes
    /// \param[in] checkOrNot   true for check all, false for uncheck all
    //-----------------------------------------------------------------------------
    void checkAll(bool checkOrNot);

private:

    //-----------------------------------------------------------------------------
    /// Set up model data
    /// \param[in] pLines   A string list which has tree info, usually created by getStrData()
    /// \param[in] pParent  A pointer to a parent node
    //-----------------------------------------------------------------------------
    void setupModelData(QStringList* pLines, CheckableTreeItem* pParent);

    //-----------------------------------------------------------------------------
    /// Get model data
    /// \param[in]  pParent     A pointer to a parent node
    /// \param[in]  depth       A depth for tree struct.
    /// \param[out] pLinesOut   A string list to be populated
    //-----------------------------------------------------------------------------
    void getModelData(CheckableTreeItem* pParent, int depth, QStringList* pLinesOut) const;

    //-----------------------------------------------------------------------------
    /// Update sub tree
    /// \param[in] node    A tree node
    //-----------------------------------------------------------------------------
    void updateSubTree(const QModelIndex& node);

    //-----------------------------------------------------------------------------
    /// Update parent chain
    /// \param[in] parent    A parent node
    //-----------------------------------------------------------------------------
    void updateParentChain(const QModelIndex& parent);

    /// Root node
    boost::shared_ptr<CheckableTreeItem> m_pRootItem;

private slots:

    //-----------------------------------------------------------------------------
    /// Slot function for receiving a TreeView clicked() signal
    /// \param[in] index    An index of the clicked node.
    //-----------------------------------------------------------------------------
    void clicked(const QModelIndex& index);
};

#endif // _TREE_MODEL_H_
