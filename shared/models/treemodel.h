#pragma once

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

#include "models/treeitem.h"

class TreeItem;
class TreeModel : public QAbstractItemModel {
Q_OBJECT

public:
  Q_DISABLE_COPY_MOVE(TreeModel)

  explicit TreeModel(QObject *parent = nullptr);
  ~TreeModel() override;

  void setupModelData(const QString &data, int rows);

  [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
  [[nodiscard]] Qt::ItemFlags flags(const QModelIndex &index) const override;
  [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  [[nodiscard]] QModelIndex index(int row, int column, const QModelIndex &parent = {}) const override;
  [[nodiscard]] QModelIndex parent(const QModelIndex &index) const override;
  [[nodiscard]] int rowCount(const QModelIndex &parent = {}) const override;
  [[nodiscard]] int columnCount(const QModelIndex &parent = {}) const override;

private:
  std::unique_ptr<TreeItem> rootItem;
};
