#ifndef METADATA_BASE_H
#define METADATA_BASE_H

// ============================================================
// metadata_base.h
// Базовый класс и подэлементы метаданных конфигурации 1С
// C++17 / Qt 6
// ============================================================

#include <QString>
#include <QList>
#include <QVariant>
#include <QUuid>
#include <memory>
#include <vector>

namespace OneC {
namespace Metadata {

// ============================================================
// Базовый абстрактный класс для всех объектов метаданных
// ============================================================
class MetadataObject {
public:
    explicit MetadataObject(const QString& name = {}, const QString& synonym = {}, const QUuid& uuid = {});
    virtual ~MetadataObject() = default;

    // Имя объекта метаданных
    QString objectName() const;
    void setObjectName(const QString& name);

    // Синоним объекта метаданных
    QString objectSynonym() const;
    void setObjectSynonym(const QString& synonym);

    // Идентификатор (UUID)
    QUuid objectUuid() const;
    void setObjectUuid(const QUuid& uuid);

    // Тип метаданных (для идентификации)
    virtual QString metadataType() const = 0;

private:
    QString m_objectName;
    QString m_objectSynonym;
    QUuid   m_objectUuid;   // <-- Новое поле
};

// ============================================================
// Реквизит
// ============================================================
class AttributeMetadata {
public:
    explicit AttributeMetadata(const QString& name = {}, const QString& synonym = {}, const QString& typeName = {});

    QString name() const;
    void setName(const QString& name);

    QString synonym() const;
    void setSynonym(const QString& synonym);

    QString typeName() const;
    void setTypeName(const QString& typeName);

private:
    QString m_name;
    QString m_synonym;
    QString m_typeName;
};

// ============================================================
// Табличная часть
// ============================================================
class TablePartMetadata {
public:
    explicit TablePartMetadata(const QString& name = {}, const QString& synonym = {});

    QString name() const;
    void setName(const QString& name);

    QString synonym() const;
    void setSynonym(const QString& synonym);

    QList<AttributeMetadata>& attributes();
    const QList<AttributeMetadata>& attributes() const;

private:
    QString m_name;
    QString m_synonym;
    QList<AttributeMetadata> m_attributes;
};

// ============================================================
// Форма
// ============================================================
class FormMetadata {
public:
    explicit FormMetadata(const QString& name = {}, const QString& synonym = {}, const QString& formType = {});

    QString name() const;
    void setName(const QString& name);

    QString synonym() const;
    void setSynonym(const QString& synonym);

    QString formType() const;
    void setFormType(const QString& formType);

private:
    QString m_name;
    QString m_synonym;
    QString m_formType;
};

// ============================================================
// Команда
// ============================================================
class CommandMetadata {
public:
    explicit CommandMetadata(const QString& name = {}, const QString& synonym = {});

    QString name() const;
    void setName(const QString& name);

    QString synonym() const;
    void setSynonym(const QString& synonym);

private:
    QString m_name;
    QString m_synonym;
};

// ============================================================
// Макет
// ============================================================
class TemplateMetadata {
public:
    explicit TemplateMetadata(const QString& name = {}, const QString& synonym = {}, const QString& templateType = {});

    QString name() const;
    void setName(const QString& name);

    QString synonym() const;
    void setSynonym(const QString& synonym);

    QString templateType() const;
    void setTemplateType(const QString& templateType);

private:
    QString m_name;
    QString m_synonym;
    QString m_templateType;
};

// ============================================================
// Графа журнала документов
// ============================================================
class ColumnMetadata {
public:
    explicit ColumnMetadata(const QString& name = {}, const QString& synonym = {});

    QString name() const;
    void setName(const QString& name);

    QString synonym() const;
    void setSynonym(const QString& synonym);

private:
    QString m_name;
    QString m_synonym;
};

// ============================================================
// Значение перечисления
// ============================================================
class EnumValueMetadata {
public:
    explicit EnumValueMetadata(const QString& name = {}, const QString& synonym = {});

    QString name() const;
    void setName(const QString& name);

    QString synonym() const;
    void setSynonym(const QString& synonym);

private:
    QString m_name;
    QString m_synonym;
};

// ============================================================
// Признак учета (для плана счетов)
// ============================================================
class AccountingSignMetadata {
public:
    explicit AccountingSignMetadata(const QString& name = {}, const QString& synonym = {});

    QString name() const;
    void setName(const QString& name);

    QString synonym() const;
    void setSynonym(const QString& synonym);

private:
    QString m_name;
    QString m_synonym;
};

// ============================================================
// Признак учета субконто (для плана счетов)
// ============================================================
class SubcontoSignMetadata {
public:
    explicit SubcontoSignMetadata(const QString& name = {}, const QString& synonym = {});

    QString name() const;
    void setName(const QString& name);

    QString synonym() const;
    void setSynonym(const QString& synonym);

private:
    QString m_name;
    QString m_synonym;
};

// ============================================================
// Измерение регистра
// ============================================================
class DimensionMetadata {
public:
    explicit DimensionMetadata(const QString& name = {}, const QString& synonym = {}, const QString& typeName = {});

    QString name() const;
    void setName(const QString& name);

    QString synonym() const;
    void setSynonym(const QString& synonym);

    QString typeName() const;
    void setTypeName(const QString& typeName);

private:
    QString m_name;
    QString m_synonym;
    QString m_typeName;
};

// ============================================================
// Ресурс регистра
// ============================================================
class ResourceMetadata {
public:
    explicit ResourceMetadata(const QString& name = {}, const QString& synonym = {}, const QString& typeName = {});

    QString name() const;
    void setName(const QString& name);

    QString synonym() const;
    void setSynonym(const QString& synonym);

    QString typeName() const;
    void setTypeName(const QString& typeName);

private:
    QString m_name;
    QString m_synonym;
    QString m_typeName;
};

// ============================================================
// Перерасчет (для регистра расчета)
// ============================================================
class RecalculationMetadata {
public:
    explicit RecalculationMetadata(const QString& name = {}, const QString& synonym = {});

    QString name() const;
    void setName(const QString& name);

    QString synonym() const;
    void setSynonym(const QString& synonym);

private:
    QString m_name;
    QString m_synonym;
};

} // namespace Metadata
} // namespace OneC


#endif // METADATA_BASE_H
