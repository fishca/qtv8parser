// ============================================================
// metadata_objects.cpp
// Реализация производных классов объектов метаданных
// C++17 / Qt 6
// ============================================================

#include "metadata_objects.h"

namespace OneC {
namespace Metadata {

// ============================================================
// MetadataWithAttributes
// ============================================================
QList<AttributeMetadata>& MetadataWithAttributes::attributes() { return m_attributes; }
const QList<AttributeMetadata>& MetadataWithAttributes::attributes() const { return m_attributes; }

QList<TablePartMetadata>& MetadataWithAttributes::tableParts() { return m_tableParts; }
const QList<TablePartMetadata>& MetadataWithAttributes::tableParts() const { return m_tableParts; }

QList<FormMetadata>& MetadataWithAttributes::forms() { return m_forms; }
const QList<FormMetadata>& MetadataWithAttributes::forms() const { return m_forms; }

QList<CommandMetadata>& MetadataWithAttributes::commands() { return m_commands; }
const QList<CommandMetadata>& MetadataWithAttributes::commands() const { return m_commands; }

QList<TemplateMetadata>& MetadataWithAttributes::templates() { return m_templates; }
const QList<TemplateMetadata>& MetadataWithAttributes::templates() const { return m_templates; }

// ============================================================
// CatalogMetadata
// ============================================================
QString CatalogMetadata::metadataType() const
{
    return QStringLiteral("Справочник");
}

// ============================================================
// DocumentMetadata
// ============================================================
QString DocumentMetadata::metadataType() const
{
    return QStringLiteral("Документ");
}

// ============================================================
// ReportMetadata
// ============================================================
QString ReportMetadata::metadataType() const
{
    return QStringLiteral("Отчет");
}

// ============================================================
// DataProcessorMetadata
// ============================================================
QString DataProcessorMetadata::metadataType() const
{
    return QStringLiteral("Обработка");
}

// ============================================================
// ChartOfCharacteristicTypesMetadata
// ============================================================
QString ChartOfCharacteristicTypesMetadata::metadataType() const
{
    return QStringLiteral("ПланВидовХарактеристик");
}

// ============================================================
// ChartOfCalculationTypesMetadata
// ============================================================
QString ChartOfCalculationTypesMetadata::metadataType() const
{
    return QStringLiteral("ПланВидовРасчета");
}

// ============================================================
// BusinessProcessMetadata
// ============================================================
QString BusinessProcessMetadata::metadataType() const
{
    return QStringLiteral("БизнесПроцесс");
}

// ============================================================
// ExchangePlanMetadata
// ============================================================
QString ExchangePlanMetadata::metadataType() const
{
    return QStringLiteral("ПланОбмена");
}

// ============================================================
// TaskMetadata
// ============================================================
QString TaskMetadata::metadataType() const
{
    return QStringLiteral("Задача");
}

QList<AttributeMetadata>& TaskMetadata::addressingAttributes()
{
    return m_addressingAttributes;
}

const QList<AttributeMetadata>& TaskMetadata::addressingAttributes() const
{
    return m_addressingAttributes;
}

// ============================================================
// DocumentJournalMetadata
// ============================================================
QString DocumentJournalMetadata::metadataType() const
{
    return QStringLiteral("ЖурналДокументов");
}

QList<ColumnMetadata>& DocumentJournalMetadata::columns() { return m_columns; }
const QList<ColumnMetadata>& DocumentJournalMetadata::columns() const { return m_columns; }

QList<FormMetadata>& DocumentJournalMetadata::forms() { return m_forms; }
const QList<FormMetadata>& DocumentJournalMetadata::forms() const { return m_forms; }

QList<CommandMetadata>& DocumentJournalMetadata::commands() { return m_commands; }
const QList<CommandMetadata>& DocumentJournalMetadata::commands() const { return m_commands; }

// ============================================================
// EnumMetadata
// ============================================================
QString EnumMetadata::metadataType() const
{
    return QStringLiteral("Перечисление");
}

QList<EnumValueMetadata>& EnumMetadata::values() { return m_values; }
const QList<EnumValueMetadata>& EnumMetadata::values() const { return m_values; }

QList<FormMetadata>& EnumMetadata::forms() { return m_forms; }
const QList<FormMetadata>& EnumMetadata::forms() const { return m_forms; }

QList<CommandMetadata>& EnumMetadata::commands() { return m_commands; }
const QList<CommandMetadata>& EnumMetadata::commands() const { return m_commands; }

QList<TemplateMetadata>& EnumMetadata::templates() { return m_templates; }
const QList<TemplateMetadata>& EnumMetadata::templates() const { return m_templates; }

// ============================================================
// ChartOfAccountsMetadata
// ============================================================
QString ChartOfAccountsMetadata::metadataType() const
{
    return QStringLiteral("ПланСчетов");
}

QList<AttributeMetadata>& ChartOfAccountsMetadata::attributes() { return m_attributes; }
const QList<AttributeMetadata>& ChartOfAccountsMetadata::attributes() const { return m_attributes; }

QList<AccountingSignMetadata>& ChartOfAccountsMetadata::accountingSigns() { return m_accountingSigns; }
const QList<AccountingSignMetadata>& ChartOfAccountsMetadata::accountingSigns() const { return m_accountingSigns; }

QList<SubcontoSignMetadata>& ChartOfAccountsMetadata::subcontoSigns() { return m_subcontoSigns; }
const QList<SubcontoSignMetadata>& ChartOfAccountsMetadata::subcontoSigns() const { return m_subcontoSigns; }

QList<TablePartMetadata>& ChartOfAccountsMetadata::tableParts() { return m_tableParts; }
const QList<TablePartMetadata>& ChartOfAccountsMetadata::tableParts() const { return m_tableParts; }

QList<FormMetadata>& ChartOfAccountsMetadata::forms() { return m_forms; }
const QList<FormMetadata>& ChartOfAccountsMetadata::forms() const { return m_forms; }

QList<CommandMetadata>& ChartOfAccountsMetadata::commands() { return m_commands; }
const QList<CommandMetadata>& ChartOfAccountsMetadata::commands() const { return m_commands; }

QList<TemplateMetadata>& ChartOfAccountsMetadata::templates() { return m_templates; }
const QList<TemplateMetadata>& ChartOfAccountsMetadata::templates() const { return m_templates; }

// ============================================================
// RegisterMetadata
// ============================================================
QList<DimensionMetadata>& RegisterMetadata::dimensions() { return m_dimensions; }
const QList<DimensionMetadata>& RegisterMetadata::dimensions() const { return m_dimensions; }

QList<ResourceMetadata>& RegisterMetadata::resources() { return m_resources; }
const QList<ResourceMetadata>& RegisterMetadata::resources() const { return m_resources; }

QList<AttributeMetadata>& RegisterMetadata::attributes() { return m_attributes; }
const QList<AttributeMetadata>& RegisterMetadata::attributes() const { return m_attributes; }

QList<FormMetadata>& RegisterMetadata::forms() { return m_forms; }
const QList<FormMetadata>& RegisterMetadata::forms() const { return m_forms; }

QList<CommandMetadata>& RegisterMetadata::commands() { return m_commands; }
const QList<CommandMetadata>& RegisterMetadata::commands() const { return m_commands; }

QList<TemplateMetadata>& RegisterMetadata::templates() { return m_templates; }
const QList<TemplateMetadata>& RegisterMetadata::templates() const { return m_templates; }

// ============================================================
// InformationRegisterMetadata
// ============================================================
QString InformationRegisterMetadata::metadataType() const
{
    return QStringLiteral("РегистрСведений");
}

// ============================================================
// AccumulationRegisterMetadata
// ============================================================
QString AccumulationRegisterMetadata::metadataType() const
{
    return QStringLiteral("РегистрНакопления");
}

// ============================================================
// AccountingRegisterMetadata
// ============================================================
QString AccountingRegisterMetadata::metadataType() const
{
    return QStringLiteral("РегистрБухгалтерии");
}

// ============================================================
// CalculationRegisterMetadata
// ============================================================
QString CalculationRegisterMetadata::metadataType() const
{
    return QStringLiteral("РегистрРасчета");
}

QList<RecalculationMetadata>& CalculationRegisterMetadata::recalculations()
{
    return m_recalculations;
}

const QList<RecalculationMetadata>& CalculationRegisterMetadata::recalculations() const
{
    return m_recalculations;
}

// ============================================================
// FilterCriteriaMetadata
// ============================================================
QString FilterCriteriaMetadata::metadataType() const
{
    return QStringLiteral("КритерийОтбора");
}

QList<FormMetadata>& FilterCriteriaMetadata::forms() { return m_forms; }
const QList<FormMetadata>& FilterCriteriaMetadata::forms() const { return m_forms; }

QList<CommandMetadata>& FilterCriteriaMetadata::commands() { return m_commands; }
const QList<CommandMetadata>& FilterCriteriaMetadata::commands() const { return m_commands; }

// ============================================================
// SettingsStorageMetadata
// ============================================================
QString SettingsStorageMetadata::metadataType() const
{
    return QStringLiteral("ХранилищеНастроек");
}

QList<FormMetadata>& SettingsStorageMetadata::forms() { return m_forms; }
const QList<FormMetadata>& SettingsStorageMetadata::forms() const { return m_forms; }

QList<TemplateMetadata>& SettingsStorageMetadata::templates() { return m_templates; }
const QList<TemplateMetadata>& SettingsStorageMetadata::templates() const { return m_templates; }


} // namespace Metadata
} // namespace OneC
