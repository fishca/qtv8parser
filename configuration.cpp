// ============================================================
// configuration.cpp
// Реализация контейнера конфигурации
// C++17 / Qt 6
// ============================================================

#include "configuration.h"
#include "metadata_repository.h"   // <-- тут можно, цикла нет

namespace OneC {
namespace Metadata {

Configuration::Configuration()
{
}

Configuration::~Configuration() = default;   // ✅ Вот эта строка пропущена

QString Configuration::configurationName() const
{
    return m_configurationName;
}

void Configuration::setConfigurationName(const QString& name)
{
    m_configurationName = name;
}

QString Configuration::configurationSynonym() const
{
    return m_configurationSynonym;
}

void Configuration::setConfigurationSynonym(const QString& synonym)
{
    m_configurationSynonym = synonym;
}

// ---- Справочники ----
QList<MetadataDescriptor>& Configuration::catalogs() { return m_catalogs; }
const QList<MetadataDescriptor> &Configuration::catalogs() const { return m_catalogs; }

// ---- Документы ----
QList<MetadataDescriptor>& Configuration::documents() { return m_documents; }
const QList<MetadataDescriptor>& Configuration::documents() const { return m_documents; }

// ---- Отчеты ----
QList<MetadataDescriptor>& Configuration::reports() { return m_reports; }
const QList<MetadataDescriptor>& Configuration::reports() const { return m_reports; }

// ---- Обработки ----
QList<MetadataDescriptor>& Configuration::dataProcessors() { return m_dataProcessors; }
const QList<MetadataDescriptor>& Configuration::dataProcessors() const { return m_dataProcessors; }

// ---- Планы видов характеристик ----
QList<MetadataDescriptor>& Configuration::chartsOfCharacteristicTypes() { return m_chartsOfCharacteristicTypes; }
const QList<MetadataDescriptor>& Configuration::chartsOfCharacteristicTypes() const { return m_chartsOfCharacteristicTypes; }

// ---- Планы видов расчета ----
QList<MetadataDescriptor>& Configuration::chartsOfCalculationTypes() { return m_chartsOfCalculationTypes; }
const QList<MetadataDescriptor>& Configuration::chartsOfCalculationTypes() const { return m_chartsOfCalculationTypes; }

// ---- Бизнес-процессы ----
QList<MetadataDescriptor>& Configuration::businessProcesses() { return m_businessProcesses; }
const QList<MetadataDescriptor>& Configuration::businessProcesses() const { return m_businessProcesses; }

// ---- Планы обмена ----
QList<MetadataDescriptor>& Configuration::exchangePlans() { return m_exchangePlans; }
const QList<MetadataDescriptor>& Configuration::exchangePlans() const { return m_exchangePlans; }

// ---- Задачи ----
QList<MetadataDescriptor>& Configuration::tasks() { return m_tasks; }
const QList<MetadataDescriptor>& Configuration::tasks() const { return m_tasks; }

// ---- Журналы документов ----
QList<MetadataDescriptor>& Configuration::documentJournals() { return m_documentJournals; }
const QList<MetadataDescriptor>& Configuration::documentJournals() const { return m_documentJournals; }

// ---- Перечисления ----
QList<MetadataDescriptor>& Configuration::enums() { return m_enums; }
const QList<MetadataDescriptor>& Configuration::enums() const { return m_enums; }

// ---- Планы счетов ----
QList<MetadataDescriptor>& Configuration::chartsOfAccounts() { return m_chartsOfAccounts; }
const QList<MetadataDescriptor>& Configuration::chartsOfAccounts() const { return m_chartsOfAccounts; }

// ---- Регистры сведений ----
QList<MetadataDescriptor>& Configuration::informationRegisters() { return m_informationRegisters; }
const QList<MetadataDescriptor>& Configuration::informationRegisters() const { return m_informationRegisters; }

// ---- Регистры накопления ----
QList<MetadataDescriptor>& Configuration::accumulationRegisters() { return m_accumulationRegisters; }
const QList<MetadataDescriptor>& Configuration::accumulationRegisters() const { return m_accumulationRegisters; }

// ---- Регистры бухгалтерии ----
QList<MetadataDescriptor>& Configuration::accountingRegisters() { return m_accountingRegisters; }
const QList<MetadataDescriptor>& Configuration::accountingRegisters() const { return m_accountingRegisters; }

// ---- Регистры расчета ----
QList<MetadataDescriptor>& Configuration::calculationRegisters() { return m_calculationRegisters; }
const QList<MetadataDescriptor>& Configuration::calculationRegisters() const { return m_calculationRegisters; }

// ---- Критерии отбора ----
QList<MetadataDescriptor>& Configuration::filterCriteria() { return m_filterCriteria; }
const QList<MetadataDescriptor>& Configuration::filterCriteria() const { return m_filterCriteria; }

// ---- Хранилища настроек ----
QList<MetadataDescriptor>& Configuration::settingsStorages() { return m_settingsStorages; }
const QList<MetadataDescriptor>& Configuration::settingsStorages() const { return m_settingsStorages; }

/*
// ---- Поиск объекта метаданных по имени ----
MetadataObject* Configuration::findObjectByName(const QString& name) const
{
    // Поиск в справочниках
    for (const auto& obj : m_catalogs)
        if (obj->objectName() == name) return obj.get();

    // Поиск в документах
    for (const auto& obj : m_documents)
        if (obj->objectName() == name) return obj.get();

    // Поиск в отчетах
    for (const auto& obj : m_reports)
        if (obj->objectName() == name) return obj.get();

    // Поиск в обработках
    for (const auto& obj : m_dataProcessors)
        if (obj->objectName() == name) return obj.get();

    // Поиск в планах видов характеристик
    for (const auto& obj : m_chartsOfCharacteristicTypes)
        if (obj->objectName() == name) return obj.get();

    // Поиск в планах видов расчета
    for (const auto& obj : m_chartsOfCalculationTypes)
        if (obj->objectName() == name) return obj.get();

    // Поиск в бизнес-процессах
    for (const auto& obj : m_businessProcesses)
        if (obj->objectName() == name) return obj.get();

    // Поиск в планах обмена
    for (const auto& obj : m_exchangePlans)
        if (obj->objectName() == name) return obj.get();

    // Поиск в задачах
    for (const auto& obj : m_tasks)
        if (obj->objectName() == name) return obj.get();

    // Поиск в журналах документов
    for (const auto& obj : m_documentJournals)
        if (obj->objectName() == name) return obj.get();

    // Поиск в перечислениях
    for (const auto& obj : m_enums)
        if (obj->objectName() == name) return obj.get();

    // Поиск в планах счетов
    for (const auto& obj : m_chartsOfAccounts)
        if (obj->objectName() == name) return obj.get();

    // Поиск в регистрах сведений
    for (const auto& obj : m_informationRegisters)
        if (obj->objectName() == name) return obj.get();

    // Поиск в регистрах накопления
    for (const auto& obj : m_accumulationRegisters)
        if (obj->objectName() == name) return obj.get();

    // Поиск в регистрах бухгалтерии
    for (const auto& obj : m_accountingRegisters)
        if (obj->objectName() == name) return obj.get();

    // Поиск в регистрах расчета
    for (const auto& obj : m_calculationRegisters)
        if (obj->objectName() == name) return obj.get();

    // Поиск в критериях отбора
    for (const auto& obj : m_filterCriteria)
        if (obj->objectName() == name) return obj.get();

    // Поиск в хранилищах настроек
    for (const auto& obj : m_settingsStorages)
        if (obj->objectName() == name) return obj.get();

    return nullptr;
}
*/


// ---- Общее количество объектов метаданных ----
int Configuration::totalObjectCount() const
{
    return m_catalogs.size()
         + m_documents.size()
         + m_reports.size()
         + m_dataProcessors.size()
         + m_chartsOfCharacteristicTypes.size()
         + m_chartsOfCalculationTypes.size()
         + m_businessProcesses.size()
         + m_exchangePlans.size()
         + m_tasks.size()
         + m_documentJournals.size()
         + m_enums.size()
         + m_chartsOfAccounts.size()
         + m_informationRegisters.size()
         + m_accumulationRegisters.size()
         + m_accountingRegisters.size()
         + m_calculationRegisters.size()
         + m_filterCriteria.size()
         + m_settingsStorages.size();
}

void Configuration::setRepository(std::unique_ptr<MetadataRepository> repo)
{
    m_repository = std::move(repo);
}

const MetadataDescriptor *Configuration::findByUuid(const QUuid &uuid) const
{
    auto it = m_uuidIndex.constFind(uuid);
    return it == m_uuidIndex.constEnd() ? nullptr : it.value();
}

const MetadataDescriptor *Configuration::findByName(const QString &name) const
{
    auto it = m_nameIndex.constFind(name);
    return it == m_nameIndex.constEnd() ? nullptr : it.value();
}

void Configuration::rebuildIndices()
{
    m_uuidIndex.clear();
    m_nameIndex.clear();

    // Лямбда обходит один список дескрипторов и кладёт указатели в индексы
    auto addRange = [this](const QList<MetadataDescriptor>& list) {
        for (const auto& d : list) {
            if (!d.uuid().isNull()) {
                m_uuidIndex.insert(d.uuid(), &d);
            }
            if (!d.name().isEmpty()) {
                m_nameIndex.insert(d.name(), &d);
            }
        }
    };

    addRange(m_catalogs);
    addRange(m_documents);
    //addRange(m_commonModules);
    // ... добавьте здесь ВСЕ остальные списки дескрипторов ...
    // Например:
    // addRange(m_reports);
    // addRange(m_dataProcessors);
    // addRange(m_chartsOfCharacteristicTypes);
    // addRange(m_chartsOfCalculationTypes);
    // addRange(m_businessProcesses);
    // addRange(m_exchangePlans);
    // addRange(m_tasks);
    // addRange(m_documentJournals);
    // addRange(m_enums);
    // addRange(m_chartsOfAccounts);
    // addRange(m_informationRegisters);
    // addRange(m_accumulationRegisters);
    // addRange(m_accountingRegisters);
    // addRange(m_calculationRegisters);
    // addRange(m_filterCriteria);
    // addRange(m_settingsStorages);
}

void Configuration::addCatalog(const MetadataDescriptor &d)
{
    m_catalogs.append(d);
}

void Configuration::addDocument(const MetadataDescriptor &d)
{
    m_documents.append(d);
}

void Configuration::addReport(const MetadataDescriptor &d)
{
    m_reports.append(d);
}

void Configuration::addDataProcessor(const MetadataDescriptor &d)
{
    m_dataProcessors.append(d);
}


} // namespace Metadata
} // namespace OneC
