#include "models/GenericTableModel.h"

GenericTableModel::GenericTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int GenericTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_data.size();
}

int GenericTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_headers.size();
}

QVariant GenericTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_data.size() || index.column() >= m_headers.size())
        return QVariant();

    const QVariantMap &rowData = m_data.at(index.row());

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        // 使用列映射获取属性名
        QString propertyName = m_columnMapping.value(index.column());
        if (!propertyName.isEmpty() && rowData.contains(propertyName)) {
            return rowData.value(propertyName);
        }
    }

    // 可以添加其他角色的处理，如对齐、颜色等

    return QVariant();
}

QVariant GenericTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        if (section < m_headers.size())
            return m_headers.at(section);
    }

    return QVariant();
}

void GenericTableModel::setData(const QVector<QVariantMap> &data)
{
    beginResetModel();
    m_data = data;
    endResetModel();

    emit dataRefreshed();
}

QVariantMap GenericTableModel::getRowData(int row) const
{
    if (row >= 0 && row < m_data.size())
        return m_data.at(row);

    return QVariantMap();
}

void GenericTableModel::setHeaders(const QStringList &headers)
{
    beginResetModel();
    m_headers = headers;
    endResetModel();
}

QStringList GenericTableModel::getHeaders() const
{
    return m_headers;
}

void GenericTableModel::setColumnMapping(const QHash<int, QString> &mapping)
{
    m_columnMapping = mapping;
}

void GenericTableModel::setColumnMapping(int column, const QString &dataProperty)
{
    m_columnMapping[column] = dataProperty;
}

void GenericTableModel::refresh()
{
    emit refreshStarted();
    // 子类应重写此方法以实际刷新数据
    // 通常涉及API调用
    emit refreshFinished(true);
}
