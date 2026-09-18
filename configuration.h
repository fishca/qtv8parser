#ifndef CONFIGURATION_H
#define CONFIGURATION_H
// ============================================================
// configuration.h
// Контейнер конфигурации — хранит все объекты метаданных
// C++17 / Qt 6
// ============================================================

#pragma once

//#include "metadata_objects.h"
#include <QList>
#include <memory>
#include <QHash>
//#include "metadata_repository.h"   // <-- нужен для MetadataRepository
#include "metadata_descriptor.h"

namespace OneC {
namespace Metadata {

class MetadataRepository;   // <-- предварительное объявление

// ============================================================
// Контейнер конфигурации
// Хранит все объекты метаданных конфигурации 1С
// ============================================================
class Configuration {
public:
    Configuration();
    ~Configuration();

    // Имя конфигурации
    QString configurationName() const;
    void setConfigurationName(const QString& name);

    // Синоним конфигурации
    QString configurationSynonym() const;
    void setConfigurationSynonym(const QString& synonym);

    // ---- Справочники ----
    QList<MetadataDescriptor>& catalogs();
    const QList<MetadataDescriptor>& catalogs() const;

    // ---- Документы ----
    QList<MetadataDescriptor>& documents();
    const QList<MetadataDescriptor>& documents() const;

    // ---- Отчеты ----
    QList<MetadataDescriptor>& reports();
    const QList<MetadataDescriptor>& reports() const;

    // ---- Обработки ----
    QList<MetadataDescriptor>& dataProcessors();
    const QList<MetadataDescriptor>& dataProcessors() const;

    // ---- Планы видов характеристик ----
    QList<MetadataDescriptor>& chartsOfCharacteristicTypes();
    const QList<MetadataDescriptor>& chartsOfCharacteristicTypes() const;

    // ---- Планы видов расчета ----
    QList<MetadataDescriptor>& chartsOfCalculationTypes();
    const QList<MetadataDescriptor>& chartsOfCalculationTypes() const;

    // ---- Бизнес-процессы ----
    QList<MetadataDescriptor>& businessProcesses();
    const QList<MetadataDescriptor>& businessProcesses() const;

    // ---- Планы обмена ----
    QList<MetadataDescriptor>& exchangePlans();
    const QList<MetadataDescriptor>& exchangePlans() const;

    // ---- Задачи ----
    QList<MetadataDescriptor>& tasks();
    const QList<MetadataDescriptor>& tasks() const;

    // ---- Журналы документов ----
    QList<MetadataDescriptor>& documentJournals();
    const QList<MetadataDescriptor>& documentJournals() const;

    // ---- Перечисления ----
    QList<MetadataDescriptor>& enums();
    const QList<MetadataDescriptor>& enums() const;

    // ---- Планы счетов ----
    QList<MetadataDescriptor>& chartsOfAccounts();
    const QList<MetadataDescriptor>& chartsOfAccounts() const;

    // ---- Регистры сведений ----
    QList<MetadataDescriptor>& informationRegisters();
    const QList<MetadataDescriptor>& informationRegisters() const;

    // ---- Регистры накопления ----
    QList<MetadataDescriptor>& accumulationRegisters();
    const QList<MetadataDescriptor>& accumulationRegisters() const;

    // ---- Регистры бухгалтерии ----
    QList<MetadataDescriptor>& accountingRegisters();
    const QList<MetadataDescriptor>& accountingRegisters() const;

    // ---- Регистры расчета ----
    QList<MetadataDescriptor>& calculationRegisters();
    const QList<MetadataDescriptor>& calculationRegisters() const;

    // ---- Критерии отбора ----
    QList<MetadataDescriptor>& filterCriteria();
    const QList<MetadataDescriptor>& filterCriteria() const;

    // ---- Хранилища настроек ----
    QList<MetadataDescriptor>& settingsStorages();
    const QList<MetadataDescriptor>& settingsStorages() const;

    // ---- Поиск объекта метаданных по имени ----
    //MetadataObject* findObjectByName(const QString& name) const;

    // ---- Общее количество объектов метаданных ----
    int totalObjectCount() const;

    // ---- Репозиторий полных объектов ----
    MetadataRepository* repository() const { return m_repository.get(); }

    /// Принять владение репозиторием.
    /// Вызывается один раз — после открытия каталога.
    void setRepository(std::unique_ptr<MetadataRepository> repo);

    // ---- Индексы для быстрого поиска ----
    const MetadataDescriptor* findByUuid(const QUuid& uuid) const;
    const MetadataDescriptor* findByName(const QString& name) const;
    /// Перестроить индексы поиска.
    /// Вызывать один раз после завершения парсинга, когда списки
    /// дескрипторов больше не меняются.
    void rebuildIndices();

    // ---- Методы добавления (вызываются парсером) ----
    void addCatalog(const MetadataDescriptor& d);
    void addDocument(const MetadataDescriptor& d);
    //void addCommonModule(const MetadataDescriptor& d);
    void addReport(const MetadataDescriptor& d);
    void addDataProcessor(const MetadataDescriptor& d);

private:
    QString m_configurationName;
    QString m_configurationSynonym;

    std::unique_ptr<MetadataRepository> m_repository;   // <-- новое поле

    QList<MetadataDescriptor> m_catalogs;
    QList<MetadataDescriptor> m_documents;
    QList<MetadataDescriptor> m_reports;
    QList<MetadataDescriptor> m_dataProcessors;
    QList<MetadataDescriptor> m_chartsOfCharacteristicTypes;
    QList<MetadataDescriptor> m_chartsOfCalculationTypes;
    QList<MetadataDescriptor> m_businessProcesses;
    QList<MetadataDescriptor> m_exchangePlans;
    QList<MetadataDescriptor> m_tasks;
    QList<MetadataDescriptor> m_documentJournals;
    QList<MetadataDescriptor> m_enums;
    QList<MetadataDescriptor> m_chartsOfAccounts;
    QList<MetadataDescriptor> m_informationRegisters;
    QList<MetadataDescriptor> m_accumulationRegisters;
    QList<MetadataDescriptor> m_accountingRegisters;
    QList<MetadataDescriptor> m_calculationRegisters;
    QList<MetadataDescriptor> m_filterCriteria;
    QList<MetadataDescriptor> m_settingsStorages;

    // Индексы (указатели валидны, пока QList не перевыделяет память)
    QHash<QUuid,   const MetadataDescriptor*> m_uuidIndex;
    QHash<QString, const MetadataDescriptor*> m_nameIndex;

};

} // namespace Metadata
} // namespace OneC

#endif // CONFIGURATION_H
