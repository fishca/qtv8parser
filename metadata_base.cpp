// ============================================================
// metadata_base.cpp
// Реализация базового класса и подэлементов метаданных
// C++17 / Qt 6
// ============================================================

#include "metadata_base.h"

namespace OneC {
namespace Metadata {

// ============================================================
// MetadataObject
// ============================================================
MetadataObject::MetadataObject(const QString& name,
                               const QString& synonym,
                               const QUuid& uuid)
    : m_objectName(name)
    , m_objectSynonym(synonym)
    , m_objectUuid(uuid)
{
}

QString MetadataObject::objectName() const
{
    return m_objectName;
}

void MetadataObject::setObjectName(const QString& name)
{
    m_objectName = name;
}

QString MetadataObject::objectSynonym() const
{
    return m_objectSynonym;
}

void MetadataObject::setObjectSynonym(const QString& synonym)
{
    m_objectSynonym = synonym;
}

QUuid MetadataObject::objectUuid() const { return m_objectUuid; }
void MetadataObject::setObjectUuid(const QUuid& uuid) { m_objectUuid = uuid; }

// ============================================================
// AttributeMetadata
// ============================================================
AttributeMetadata::AttributeMetadata(const QString& name, const QString& synonym, const QString& typeName)
    : m_name(name)
    , m_synonym(synonym)
    , m_typeName(typeName)
{
}

QString AttributeMetadata::name() const { return m_name; }
void AttributeMetadata::setName(const QString& name) { m_name = name; }

QString AttributeMetadata::synonym() const { return m_synonym; }
void AttributeMetadata::setSynonym(const QString& synonym) { m_synonym = synonym; }

QString AttributeMetadata::typeName() const { return m_typeName; }
void AttributeMetadata::setTypeName(const QString& typeName) { m_typeName = typeName; }

// ============================================================
// TablePartMetadata
// ============================================================
TablePartMetadata::TablePartMetadata(const QString& name, const QString& synonym)
    : m_name(name)
    , m_synonym(synonym)
{
}

QString TablePartMetadata::name() const { return m_name; }
void TablePartMetadata::setName(const QString& name) { m_name = name; }

QString TablePartMetadata::synonym() const { return m_synonym; }
void TablePartMetadata::setSynonym(const QString& synonym) { m_synonym = synonym; }

QList<AttributeMetadata>& TablePartMetadata::attributes() { return m_attributes; }
const QList<AttributeMetadata>& TablePartMetadata::attributes() const { return m_attributes; }

// ============================================================
// FormMetadata
// ============================================================
FormMetadata::FormMetadata(const QString& name, const QString& synonym, const QString& formType)
    : m_name(name)
    , m_synonym(synonym)
    , m_formType(formType)
{
}

QString FormMetadata::name() const { return m_name; }
void FormMetadata::setName(const QString& name) { m_name = name; }

QString FormMetadata::synonym() const { return m_synonym; }
void FormMetadata::setSynonym(const QString& synonym) { m_synonym = synonym; }

QString FormMetadata::formType() const { return m_formType; }
void FormMetadata::setFormType(const QString& formType) { m_formType = formType; }

// ============================================================
// CommandMetadata
// ============================================================
CommandMetadata::CommandMetadata(const QString& name, const QString& synonym)
    : m_name(name)
    , m_synonym(synonym)
{
}

QString CommandMetadata::name() const { return m_name; }
void CommandMetadata::setName(const QString& name) { m_name = name; }

QString CommandMetadata::synonym() const { return m_synonym; }
void CommandMetadata::setSynonym(const QString& synonym) { m_synonym = synonym; }

// ============================================================
// TemplateMetadata
// ============================================================
TemplateMetadata::TemplateMetadata(const QString& name, const QString& synonym, const QString& templateType)
    : m_name(name)
    , m_synonym(synonym)
    , m_templateType(templateType)
{
}

QString TemplateMetadata::name() const { return m_name; }
void TemplateMetadata::setName(const QString& name) { m_name = name; }

QString TemplateMetadata::synonym() const { return m_synonym; }
void TemplateMetadata::setSynonym(const QString& synonym) { m_synonym = synonym; }

QString TemplateMetadata::templateType() const { return m_templateType; }
void TemplateMetadata::setTemplateType(const QString& templateType) { m_templateType = templateType; }

// ============================================================
// ColumnMetadata
// ============================================================
ColumnMetadata::ColumnMetadata(const QString& name, const QString& synonym)
    : m_name(name)
    , m_synonym(synonym)
{
}

QString ColumnMetadata::name() const { return m_name; }
void ColumnMetadata::setName(const QString& name) { m_name = name; }

QString ColumnMetadata::synonym() const { return m_synonym; }
void ColumnMetadata::setSynonym(const QString& synonym) { m_synonym = synonym; }

// ============================================================
// EnumValueMetadata
// ============================================================
EnumValueMetadata::EnumValueMetadata(const QString& name, const QString& synonym)
    : m_name(name)
    , m_synonym(synonym)
{
}

QString EnumValueMetadata::name() const { return m_name; }
void EnumValueMetadata::setName(const QString& name) { m_name = name; }

QString EnumValueMetadata::synonym() const { return m_synonym; }
void EnumValueMetadata::setSynonym(const QString& synonym) { m_synonym = synonym; }

// ============================================================
// AccountingSignMetadata
// ============================================================
AccountingSignMetadata::AccountingSignMetadata(const QString& name, const QString& synonym)
    : m_name(name)
    , m_synonym(synonym)
{
}

QString AccountingSignMetadata::name() const { return m_name; }
void AccountingSignMetadata::setName(const QString& name) { m_name = name; }

QString AccountingSignMetadata::synonym() const { return m_synonym; }
void AccountingSignMetadata::setSynonym(const QString& synonym) { m_synonym = synonym; }

// ============================================================
// SubcontoSignMetadata
// ============================================================
SubcontoSignMetadata::SubcontoSignMetadata(const QString& name, const QString& synonym)
    : m_name(name)
    , m_synonym(synonym)
{
}

QString SubcontoSignMetadata::name() const { return m_name; }
void SubcontoSignMetadata::setName(const QString& name) { m_name = name; }

QString SubcontoSignMetadata::synonym() const { return m_synonym; }
void SubcontoSignMetadata::setSynonym(const QString& synonym) { m_synonym = synonym; }

// ============================================================
// DimensionMetadata
// ============================================================
DimensionMetadata::DimensionMetadata(const QString& name, const QString& synonym,
                                     const QString& typeName)
    : m_name(name)
    , m_synonym(synonym)
    , m_typeName(typeName)
{
}

QString DimensionMetadata::name() const { return m_name; }
void DimensionMetadata::setName(const QString& name) { m_name = name; }

QString DimensionMetadata::synonym() const { return m_synonym; }
void DimensionMetadata::setSynonym(const QString& synonym) { m_synonym = synonym; }

QString DimensionMetadata::typeName() const { return m_typeName; }
void DimensionMetadata::setTypeName(const QString& typeName) { m_typeName = typeName; }

// ============================================================
// ResourceMetadata
// ============================================================
ResourceMetadata::ResourceMetadata(const QString& name, const QString& synonym, const QString& typeName)
    : m_name(name)
    , m_synonym(synonym)
    , m_typeName(typeName)
{
}

QString ResourceMetadata::name() const { return m_name; }
void ResourceMetadata::setName(const QString& name) { m_name = name; }

QString ResourceMetadata::synonym() const { return m_synonym; }
void ResourceMetadata::setSynonym(const QString& synonym) { m_synonym = synonym; }

QString ResourceMetadata::typeName() const { return m_typeName; }
void ResourceMetadata::setTypeName(const QString& typeName) { m_typeName = typeName; }

// ============================================================
// RecalculationMetadata
// ============================================================
RecalculationMetadata::RecalculationMetadata(const QString& name, const QString& synonym)
    : m_name(name)
    , m_synonym(synonym)
{
}

QString RecalculationMetadata::name() const { return m_name; }
void RecalculationMetadata::setName(const QString& name) { m_name = name; }

QString RecalculationMetadata::synonym() const { return m_synonym; }
void RecalculationMetadata::setSynonym(const QString& synonym) { m_synonym = synonym; }

} // namespace Metadata
} // namespace OneC
