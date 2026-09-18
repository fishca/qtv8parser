#ifndef ATTRIBUTE_METADATA_H
#define ATTRIBUTE_METADATA_H


#include <QString>
#include <QUuid>

#include "metadata_objects.h"   // если наследуемся от MetadataObject

namespace OneC::Metadata {

/**
 * @brief Метаданные одного реквизита объекта 1С.
 *
 * Соответствует узлу TRequisite в C++Builder-версии.
 * Содержит имя, синоним, UUID, тип значения.
 */
class AttributeMetadataType : public MetadataObject {
public:
    AttributeMetadataType() = default;

    AttributeMetadataType(const QString& name,
                      const QString& synonym = {},
                      const QUuid&   uuid    = {});

    QString metadataType() const override {
        return QStringLiteral("Реквизит");
    }

    // ---- Тип значения реквизита ----
    // В 1С: "S" — строка, "N" — число, "D" — дата, "B" — булево,
    //       "CatalogRef.XXX" — ссылка, и т.д.
    QString valueType() const          { return m_valueType; }
    void setValueType(const QString& t){ m_valueType = t; }

    int  stringLength() const          { return m_stringLength; }
    void setStringLength(int len)      { m_stringLength = len; }

    int  numberPrecision() const       { return m_numberPrecision; }
    void setNumberPrecision(int p)     { m_numberPrecision = p; }

    int  numberScale() const           { return m_numberScale; }
    void setNumberScale(int s)         { m_numberScale = s; }

private:
    QString m_valueType;          // "S", "N", "D", "B", "CatalogRef...."
    int     m_stringLength   = 0;
    int     m_numberPrecision = 0;
    int     m_numberScale    = 0;
};

} // namespace OneC::Metadata

#endif // ATTRIBUTE_METADATA_H
