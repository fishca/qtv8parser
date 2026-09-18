#include "tree_utils.h"
#include "APIcfBase.h"
#include "v8file.h"
#include "Parse_tree.h"

//#include "Common.h"

#include <QDebug>

namespace OneC::Metadata::TreeUtils {

tree* getNodeByPath(tree* root, std::initializer_list<int> path)
{
    tree* cur = root;
    if (!cur) return nullptr;
    for (int idx : path) {
        if (!cur || idx < 0 || idx >= cur->get_num_subnode()) return nullptr;
        cur = cur->get_subnode(idx);
    }
    return cur;
}

tree* getNodeByPath(tree* root, const QVector<int>& path)
{
    tree* cur = root;
    if (!cur) return nullptr;
    for (int idx : path) {
        if (!cur || idx < 0 || idx >= cur->get_num_subnode()) return nullptr;
        cur = cur->get_subnode(idx);
    }
    return cur;
}

tree* findSubnodeByGuid(tree* root, const QString& guid)
{
    if (!root) return nullptr;
    for (int i = 0; i < root->get_num_subnode(); ++i) {
        tree* child = root->get_subnode(i);
        if (!child) continue;
        if (child->get_num_subnode() > 0) {
            tree* first = child->get_subnode(0);
            if (first && first->get_value() == guid) {
                return child;
            }
        }
    }
    return nullptr;
}

tree* findCollectionBlock(tree* root, const QString& collectionGuid)
{
    // Внутри узла справочника блок коллекции обычно находится среди детей.
    // Пробуем найти по прямому совпадению первого узла.
    if (tree* direct = findSubnodeByGuid(root, collectionGuid)) {
        return direct;
    }

    // Иногда нужен рекурсивный поиск на 1-2 уровня вглубь.
    for (int i = 0; i < root->get_num_subnode(); ++i) {
        tree* child = root->get_subnode(i);
        if (tree* found = findSubnodeByGuid(child, collectionGuid)) {
            return found;
        }
    }
    return nullptr;
}

QString readLocalString(tree* node, const QString& lang)
{
    // Ожидаемая структура: {1,"ru","Синoним"} или {0} (пусто)
    if (!node) return {};
    if (node->get_num_subnode() < 3) return {};
    // [0] = количество, [1] = код языка, [2] = текст
    if (node->get_subnode(1)->get_value() == lang) {
        return node->get_subnode(2)->get_value();
    }
    // Если языка нет — попробуем первый доступный
    for (int i = 1; i + 1 < node->get_num_subnode(); i += 2) {
        return node->get_subnode(i + 1)->get_value();
    }
    return {};
}

QString readItemName(tree* itemNode, const QVector<int>& namePath)
{
    tree* nameNode = getNodeByPath(itemNode, namePath);
    return nameNode ? nameNode->get_value() : QString();
}

QString readItemGuid(tree* itemNode, const QVector<int>& guidPath)
{
    tree* guidNode = getNodeByPath(itemNode, guidPath);
    return guidNode ? guidNode->get_value().trimmed() : QString();
}

} // namespace OneC::Metadata::TreeUtils