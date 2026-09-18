#include "attribute_metadata.h"

namespace OneC::Metadata {

AttributeMetadataType::AttributeMetadataType(const QString& name,
                                     const QString& synonym,
                                     const QUuid&   uuid)
    : MetadataObject(name, synonym, uuid)
{
}

} // namespace OneC::Metadata