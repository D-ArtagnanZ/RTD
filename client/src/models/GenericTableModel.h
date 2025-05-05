#pragma once

#include <QAbstractTableModel>
#include <QStringList>
#include <QVariantMap>
#include <QVector>

/**
 * @brief 通用表格模型类，可适应动态数据结构
 *
 * 这个类能够处理服务端返回的任何表格型数据结构，无需预先定义固定字段
 */
class GenericTableModel: public QAbstractTableModel {
        Q_OBJECT

    public:
        explicit GenericTableModel(QObject *parent = nullptr);

        // 基础模型接口实现
        int      rowCount(const QModelIndex &parent = QModelIndex()) const override;
        int      columnCount(const QModelIndex &parent = QModelIndex()) const override;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

        // 设置数据
        void setData(const QVector<QVariantMap> &data);

        // 获取特定行的全部数据
        QVariantMap getRowData(int row) const;

        // 获取/设置列头
        void        setHeaders(const QStringList &headers);
        QStringList getHeaders() const;

        // 获取/设置列名到属性名的映射
        void setColumnMapping(const QHash<int, QString> &mapping);
        void setColumnMapping(int column, const QString &dataProperty);

        // 刷新数据
        virtual void refresh();

    signals:
        void dataRefreshed();
        void refreshStarted();
        void refreshFinished(bool success);

    protected:
        QVector<QVariantMap> m_data;             // 存储的表格数据
        QStringList          m_headers;          // 列头名称
        QHash<int, QString>  m_columnMapping;    // 列索引到数据属性的映射
};
