#ifndef ATTRIBUTE_READER_H
#define ATTRIBUTE_READER_H
#include <QList>
#include <QString>
#include <QVector>

class tree;

namespace OneC::Metadata {

class AttributeMetadata;

namespace AttributeReader {

/// Читает все атрибуты из блока с указанным GUID коллекции.
/// @param rootNode       узел, внутри которого искать блок
/// @param collectionGuid GUID коллекции (cf4abea7-... для справочников,
///                       или 888744e1-... для табличных частей)
QList<AttributeMetadata> readAttributes(tree* rootNode, const QString& collectionGuid);

} // namespace AttributeReader

} // namespace OneC::Metadata
#endif // ATTRIBUTE_READER_H
