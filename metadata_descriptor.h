#ifndef METADATA_DESCRIPTOR_H
#define METADATA_DESCRIPTOR_H
#include <QString>
#include <QUuid>

namespace OneC::Metadata {

/**
 * @brief Лёгкий дескриптор объекта метаданных.
 *
 * Хранит только минимально необходимое для дерева и поиска.
 * Не содержит модулей, макетов, реквизитов.
 */
class MetadataDescriptor {
public:
    MetadataDescriptor() = default;

    MetadataDescriptor(const QString& name,
                       const QString& synonym,
                       const QUuid&   uuid,
                       const QString& type,
                       const QString& internalFileName = {})
        : m_name(name)
        , m_synonym(synonym)
        , m_uuid(uuid)
        , m_type(type)
        , m_internalFileName(internalFileName)
    {}

    QString name()              const { return m_name; }
    QString synonym()           const { return m_synonym; }
    QUuid   uuid()              const { return m_uuid; }
    QString type()              const { return m_type; }
    QString internalFileName()  const { return m_internalFileName; }

    void setInternalFileName(const QString& fn) {
        m_internalFileName = fn;
    }


    bool isValid() const { return !m_uuid.isNull() && !m_internalFileName.isEmpty(); }

private:
    QString m_name;
    QString m_synonym;
    QUuid   m_uuid;
    QString m_type;              // "Catalog", "Document", "CommonModule", ...
    QString m_internalFileName;  // имя "файла" внутри v8catalog

};

} // namespace OneC::Metadata
#endif // METADATA_DESCRIPTOR_H
