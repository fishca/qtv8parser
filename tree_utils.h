#ifndef TREE_UTILS_H
#define TREE_UTILS_H
#include <QString>
#include <QUuid>
#include <QVector>
#include <memory>

class v8catalog;
class tree;

namespace OneC::Metadata::TreeUtils {

/// Возвращает подузел, значение первого дочернего узла которого равно указанному GUID.
/// Возвращает nullptr, если не найден.
tree* findSubnodeByGuid(tree* root, const QString& guid);

/// Возвращает подузел, чей первый дочерний узел — заданный GUID,
/// НЕ создавая копию дерева (используется для навигации внутри уже загруженного root).
tree* findCollectionBlock(tree* root, const QString& collectionGuid);

/// Безопасный обход дерева по пути индексов.
tree* getNodeByPath(tree* root, std::initializer_list<int> path);
tree* getNodeByPath(tree* root, const QVector<int>& path);

/// Читает строку "синонима" из узла вида {1,"ru","Тестовый справочник"}.
/// Возвращает строку для указанного языка (по умолчанию "ru"), или пустую.
QString readLocalString(tree* node, const QString& lang = QStringLiteral("ru"));

/// Читает имя реквизита/ТЧ/команды/формы из стандартной структуры.
/// @param itemNode  узел одного элемента коллекции
/// @param namePath  путь до узла со значением имени (по умолчанию {0,0,0,0,1})
QString readItemName(tree* itemNode, const QVector<int>& namePath);

/// Читает GUID элемента (обычно лежит в {..,"Pattern",...} или рядом с именем).
QString readItemGuid(tree* itemNode, const QVector<int>& guidPath);

} // namespace OneC::Metadata::TreeUtils

#endif // TREE_UTILS_H
