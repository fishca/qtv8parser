#include "attribute_reader.h"
#include "attribute_metadata.h"
#include "tree_utils.h"
#include "Parse_tree.h"


#include <QDebug>

namespace OneC::Metadata::AttributeReader {

namespace {
// Пути внутри элемента коллекции
const QVector<int> kNamePath    = {0, 0, 0, 0, 1};
const QVector<int> kSynonymPath = {0, 0, 0, 0, 2};
}

QList<AttributeMetadata> readAttributes(tree* rootNode, const QString& collectionGuid)
{
    QList<AttributeMetadata> result;
    if (!rootNode) return result;

    tree* block = TreeUtils::findCollectionBlock(rootNode, collectionGuid);
    if (!block) {
        qWarning() << "Блок коллекции не найден:" << collectionGuid;
        return result;
    }

    if (block->get_num_subnode() < 2) return result;
    const int count = block->get_subnode(1)->get_value().toInt();

    result.reserve(count);

    for (int i = 0; i < count; ++i) {
        const int itemIndex = 2 + i;
        if (itemIndex >= block->get_num_subnode()) break;

        tree* itemNode = block->get_subnode(itemIndex);
        if (!itemNode) continue;

        QString name = TreeUtils::readItemName(itemNode, kNamePath);
        if (name.isEmpty()) continue;

        QString synonym;
        if (tree* synNode = TreeUtils::getNodeByPath(itemNode, kSynonymPath)) {
            synonym = TreeUtils::readLocalString(synNode);
        }

        result.append(AttributeMetadata(name, synonym));
    }

    return result;
}

} // namespace OneC::Metadata::AttributeReader