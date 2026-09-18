#ifndef METADATA_OBJECTS_H
#define METADATA_OBJECTS_H

// ============================================================
// metadata_objects.h
// Производные классы объектов метаданных конфигурации 1С
// C++17 / Qt 6
// ============================================================

#pragma once

#include <functional>
#include "metadata_base.h"
#include "guids.h"
#include "APIcfBase.h"

namespace OneC {
namespace Metadata {

// ============================================================
// Промежуточный класс: объект с реквизитами и табличными частями
// Используется для: Справочники, Документы, Отчеты, Обработки,
//   Планы видов характеристик, Планы видов расчета,
//   Бизнес-процессы, Планы обмена, Задачи
// ============================================================
class MetadataWithAttributes : public MetadataObject {
public:
    using MetadataObject::MetadataObject;

    // Реквизиты
    QList<AttributeMetadata>& attributes();
    const QList<AttributeMetadata>& attributes() const;

    // Табличные части
    QList<TablePartMetadata>& tableParts();
    const QList<TablePartMetadata>& tableParts() const;

    // Формы
    QList<FormMetadata>& forms();
    const QList<FormMetadata>& forms() const;

    // Команды
    QList<CommandMetadata>& commands();
    const QList<CommandMetadata>& commands() const;

    // Макеты
    QList<TemplateMetadata>& templates();
    const QList<TemplateMetadata>& templates() const;

protected:
    QList<AttributeMetadata> m_attributes;
    QList<TablePartMetadata> m_tableParts;
    QList<FormMetadata> m_forms;
    QList<CommandMetadata> m_commands;
    QList<TemplateMetadata> m_templates;
};

/**
 * @brief The MetadataTreePaths class
 * Конфигурация путей в дереве парсинга для разных типов метаданных 1С
 */
//---------------------------------------------------------------------------
//
struct MetadataTreePaths
{
    int attIdx;                      // индекс: root[0][attIdx][1] = количество реквизитов
    std::vector<int> attItemPath;    // путь к имени реквизита от узла элемента
    int tabIdx;                      // индекс для табличных частей
    std::vector<int> tabItemPath;    // путь к имени табличной части
    int formsIdx;                    // индекс: root[0][formsIdx][0] = узел списка форм
    int cmdIdx;                      // индекс: root[0][cmdIdx][1] = количество команд
    std::vector<int> cmdItemPath;    // путь к имени команды от узла элемента
    int moxIdx;                      // индекс: root[0][moxIdx][0] = узел списка макетов
    std::function<QString(v8catalog*, QString&)> getFormNameFunc;
    bool hasTabulars = true;         // у журналов нет табличных частей
};

/*
template<typename Derived, MetadataTreePaths (*GetPathsFunc)()>
class MDWithSectionsT : public MetadataWithAttributes
{
    MDWithSectionsT() : MDWithSectionsT() {}
    MDWithSectionsT(v8catalog* _parent, const QString& _guid) : MetadataWithAttributes()
}

*/


// ============================================================
// Общие
// ============================================================
/**
 * @brief Подсистемы
 */
class Subsystem: public MetadataObject {
public:
    using MetadataObject::MetadataObject;
    QString metadataType() const override
    {
        return QStringLiteral("Подсистема");
    }
};

/**
 * @brief Общие модули
 */
class CommonModules: public MetadataObject {
public:
    using MetadataObject::MetadataObject;
    QString metadataType() const override
    {
        return QStringLiteral("ОбщийМодуль");
    }
};

/**
 * @brief Параметры сеанса
 */
class SessionParameters: public MetadataObject {
public:
    using MetadataObject::MetadataObject;
    QString metadataType() const override
    {
        return QStringLiteral("ПараметрыСеанса");
    }
};

/**
 * @brief Роли
 */
class Roles: public MetadataObject {
public:
    using MetadataObject::MetadataObject;
    QString metadataType() const override
    {
        return QStringLiteral("Роли");
    }
};

/**
 * @brief Общие реквизиты
 */
class CommonAttributes: public MetadataObject {
public:
    using MetadataObject::MetadataObject;
    QString metadataType() const override
    {
        return QStringLiteral("ОбщиеРеквизиты");
    }
};

/**
 * @brief Подписки на события
 */
class EventSubscriptions: public MetadataObject {
public:
    using MetadataObject::MetadataObject;
    QString metadataType() const override
    {
        return QStringLiteral("ПодпискиНаСобытия");
    }
};

/**
 * @brief Регламентные задания
 */
class SheduledJob: public MetadataObject {
public:
    using MetadataObject::MetadataObject;
    QString metadataType() const override
    {
        return QStringLiteral("РегламентноеЗадание");
    }
};

/**
 * @brief Боты
 */
class Bots: public MetadataObject {
public:
    using MetadataObject::MetadataObject;
    QString metadataType() const override
    {
        return QStringLiteral("Боты");
    }
};

/**
 * @brief Функциональные опции
 */
class FunctionalOptions: public MetadataObject {
public:
    using MetadataObject::MetadataObject;
    QString metadataType() const override
    {
        return QStringLiteral("ФункциональныеОпции");
    }
};

/**
 * @brief Параметры функциональных опций
 */
class ParamFunctionalOptions: public MetadataObject {
public:
    using MetadataObject::MetadataObject;
    QString metadataType() const override
    {
        return QStringLiteral("ПараметрыФункциональныхОпций");
    }
};

/**
 * @brief Определяемые типы
 */
class DefinedTypes: public MetadataObject {
public:
    using MetadataObject::MetadataObject;
    QString metadataType() const override
    {
        return QStringLiteral("ОпределяемыеТипы");
    }
};

/**
 * @brief Общие команды
 */
class CommonComands: public MetadataObject {
public:
    using MetadataObject::MetadataObject;
    QString metadataType() const override
    {
        return QStringLiteral("ОбщиеКоманды");
    }
};

/**
 * @brief Группы команд
 */
class GroupComands: public MetadataObject {
public:
    using MetadataObject::MetadataObject;
    QString metadataType() const override
    {
        return QStringLiteral("ГруппыКоманд");
    }
};


/**
 * @brief Общие формы
 */
class CommonForms: public MetadataObject {
public:
    using MetadataObject::MetadataObject;
    QString metadataType() const override
    {
        return QStringLiteral("ОбщиеФормы");
    }
};

/**
 * @brief Интерфейсы
 */
class Interfaces: public MetadataObject {
public:
    using MetadataObject::MetadataObject;
    QString metadataType() const override
    {
        return QStringLiteral("Интерфейсы");
    }
};

/**
 * @brief Общие макеты
 */
class CommonTemplates: public MetadataObject {
public:
    using MetadataObject::MetadataObject;
    QString metadataType() const override
    {
        return QStringLiteral("ОбщиеМакеты");
    }
};

/**
 * @brief Общие картинки
 */
class CommonPictures: public MetadataObject {
public:
    using MetadataObject::MetadataObject;
    QString metadataType() const override
    {
        return QStringLiteral("ОбщиеКартинки");
    }
};

/**
 * @brief XDTO
 */
class PackXDTO: public MetadataObject {
public:
    using MetadataObject::MetadataObject;
    QString metadataType() const override
    {
        return QStringLiteral("XDTO-пакеты");
    }
};

/**
 * @brief Web - сервисы
 */
class WebService: public MetadataObject {
public:
    using MetadataObject::MetadataObject;
    QString metadataType() const override
    {
        return QStringLiteral("Web-сервисы");
    }
};

/**
 * @brief HTTP - сервисы
 */
class HTTPService: public MetadataObject {
public:
    using MetadataObject::MetadataObject;
    QString metadataType() const override
    {
        return QStringLiteral("HTTP-сервисы");
    }
};

/**
 * @brief WS - ссылки
 */
class WSRefs: public MetadataObject {
public:
    using MetadataObject::MetadataObject;
    QString metadataType() const override
    {
        return QStringLiteral("WS-ссылки");
    }
};

/**
 * @brief Web socket - клиенты
 */
class WebSocketClients: public MetadataObject {
public:
    using MetadataObject::MetadataObject;
    QString metadataType() const override
    {
        return QStringLiteral("WebSocket-клиенты");
    }
};

/**
 * @brief Сервисы интеграции
 */
class IntegrationServices: public MetadataObject {
public:
    using MetadataObject::MetadataObject;
    QString metadataType() const override
    {
        return QStringLiteral("СервисыИнтеграции");
    }
};

/**
 * @brief Элементы стиля
 */
class ItemStyles: public MetadataObject {
public:
    using MetadataObject::MetadataObject;
    QString metadataType() const override
    {
        return QStringLiteral("ЭлементыСтиля");
    }
};

/**
 * @brief Стили
 */
class Styles: public MetadataObject {
public:
    using MetadataObject::MetadataObject;
    QString metadataType() const override
    {
        return QStringLiteral("Стили");
    }
};

/**
 * @brief Языки
 */
class Languages: public MetadataObject {
public:
    using MetadataObject::MetadataObject;
    QString metadataType() const override
    {
        return QStringLiteral("Языки");
    }
};


// ============================================================
// Справочник
// ============================================================
class CatalogMetadata final : public MetadataWithAttributes {
public:
    using MetadataWithAttributes::MetadataWithAttributes;
    QString metadataType() const override;
};

// ============================================================
// Документ
// ============================================================
class DocumentMetadata final : public MetadataWithAttributes {
public:
    using MetadataWithAttributes::MetadataWithAttributes;
    QString metadataType() const override;
};

// ============================================================
// Отчет
// ============================================================
class ReportMetadata final : public MetadataWithAttributes {
public:
    using MetadataWithAttributes::MetadataWithAttributes;
    QString metadataType() const override;
};

// ============================================================
// Обработка
// ============================================================
class DataProcessorMetadata final : public MetadataWithAttributes {
public:
    using MetadataWithAttributes::MetadataWithAttributes;
    QString metadataType() const override;
};

// ============================================================
// План видов характеристик
// ============================================================
class ChartOfCharacteristicTypesMetadata final : public MetadataWithAttributes {
public:
    using MetadataWithAttributes::MetadataWithAttributes;
    QString metadataType() const override;
};

// ============================================================
// План видов расчета
// ============================================================
class ChartOfCalculationTypesMetadata final : public MetadataWithAttributes {
public:
    using MetadataWithAttributes::MetadataWithAttributes;
    QString metadataType() const override;
};

// ============================================================
// Бизнес-процесс
// ============================================================
class BusinessProcessMetadata final : public MetadataWithAttributes {
public:
    using MetadataWithAttributes::MetadataWithAttributes;
    QString metadataType() const override;
};

// ============================================================
// План обмена
// ============================================================
class ExchangePlanMetadata final : public MetadataWithAttributes {
public:
    using MetadataWithAttributes::MetadataWithAttributes;
    QString metadataType() const override;
};

// ============================================================
// Задача (дополнительно: реквизиты адресации)
// ============================================================
class TaskMetadata final : public MetadataWithAttributes {
public:
    using MetadataWithAttributes::MetadataWithAttributes;
    QString metadataType() const override;

    // Реквизиты адресации
    QList<AttributeMetadata>& addressingAttributes();
    const QList<AttributeMetadata>& addressingAttributes() const;

private:
    QList<AttributeMetadata> m_addressingAttributes;
};

// ============================================================
// Журнал документов
// ============================================================
class DocumentJournalMetadata final : public MetadataObject {
public:
    using MetadataObject::MetadataObject;
    QString metadataType() const override;

    // Графы
    QList<ColumnMetadata>& columns();
    const QList<ColumnMetadata>& columns() const;

    // Формы
    QList<FormMetadata>& forms();
    const QList<FormMetadata>& forms() const;

    // Команды
    QList<CommandMetadata>& commands();
    const QList<CommandMetadata>& commands() const;

private:
    QList<ColumnMetadata> m_columns;
    QList<FormMetadata> m_forms;
    QList<CommandMetadata> m_commands;
};

// ============================================================
// Перечисление
// ============================================================
class EnumMetadata final : public MetadataObject {
public:
    using MetadataObject::MetadataObject;
    QString metadataType() const override;

    // Значения
    QList<EnumValueMetadata>& values();
    const QList<EnumValueMetadata>& values() const;

    // Формы
    QList<FormMetadata>& forms();
    const QList<FormMetadata>& forms() const;

    // Команды
    QList<CommandMetadata>& commands();
    const QList<CommandMetadata>& commands() const;

    // Макеты
    QList<TemplateMetadata>& templates();
    const QList<TemplateMetadata>& templates() const;

private:
    QList<EnumValueMetadata> m_values;
    QList<FormMetadata> m_forms;
    QList<CommandMetadata> m_commands;
    QList<TemplateMetadata> m_templates;
};

// ============================================================
// План счетов
// ============================================================
class ChartOfAccountsMetadata final : public MetadataObject {
public:
    using MetadataObject::MetadataObject;
    QString metadataType() const override;

    // Реквизиты
    QList<AttributeMetadata>& attributes();
    const QList<AttributeMetadata>& attributes() const;

    // Признаки учета
    QList<AccountingSignMetadata>& accountingSigns();
    const QList<AccountingSignMetadata>& accountingSigns() const;

    // Признаки учета субконто
    QList<SubcontoSignMetadata>& subcontoSigns();
    const QList<SubcontoSignMetadata>& subcontoSigns() const;

    // Табличные части
    QList<TablePartMetadata>& tableParts();
    const QList<TablePartMetadata>& tableParts() const;

    // Формы
    QList<FormMetadata>& forms();
    const QList<FormMetadata>& forms() const;

    // Команды
    QList<CommandMetadata>& commands();
    const QList<CommandMetadata>& commands() const;

    // Макеты
    QList<TemplateMetadata>& templates();
    const QList<TemplateMetadata>& templates() const;

private:
    QList<AttributeMetadata> m_attributes;
    QList<AccountingSignMetadata> m_accountingSigns;
    QList<SubcontoSignMetadata> m_subcontoSigns;
    QList<TablePartMetadata> m_tableParts;
    QList<FormMetadata> m_forms;
    QList<CommandMetadata> m_commands;
    QList<TemplateMetadata> m_templates;
};

// ============================================================
// Промежуточный класс: базовый регистр
// (измерения, ресурсы, реквизиты)
// Используется для: Регистры сведений, Регистры накопления,
//   Регистры бухгалтерии, Регистры расчета
// ============================================================
class RegisterMetadata : public MetadataObject {
public:
    using MetadataObject::MetadataObject;

    // Измерения
    QList<DimensionMetadata>& dimensions();
    const QList<DimensionMetadata>& dimensions() const;

    // Ресурсы
    QList<ResourceMetadata>& resources();
    const QList<ResourceMetadata>& resources() const;

    // Реквизиты
    QList<AttributeMetadata>& attributes();
    const QList<AttributeMetadata>& attributes() const;

    // Формы
    QList<FormMetadata>& forms();
    const QList<FormMetadata>& forms() const;

    // Команды
    QList<CommandMetadata>& commands();
    const QList<CommandMetadata>& commands() const;

    // Макеты
    QList<TemplateMetadata>& templates();
    const QList<TemplateMetadata>& templates() const;

protected:
    QList<DimensionMetadata> m_dimensions;
    QList<ResourceMetadata> m_resources;
    QList<AttributeMetadata> m_attributes;
    QList<FormMetadata> m_forms;
    QList<CommandMetadata> m_commands;
    QList<TemplateMetadata> m_templates;
};

// ============================================================
// Регистр сведений
// ============================================================
class InformationRegisterMetadata final : public RegisterMetadata {
public:
    using RegisterMetadata::RegisterMetadata;
    QString metadataType() const override;
};

// ============================================================
// Регистр накопления
// ============================================================
class AccumulationRegisterMetadata final : public RegisterMetadata {
public:
    using RegisterMetadata::RegisterMetadata;
    QString metadataType() const override;
};

// ============================================================
// Регистр бухгалтерии
// ============================================================
class AccountingRegisterMetadata final : public RegisterMetadata {
public:
    using RegisterMetadata::RegisterMetadata;
    QString metadataType() const override;
};

// ============================================================
// Регистр расчета (дополнительно: перерасчеты)
// ============================================================
class CalculationRegisterMetadata final : public RegisterMetadata {
public:
    using RegisterMetadata::RegisterMetadata;
    QString metadataType() const override;

    // Перерасчеты
    QList<RecalculationMetadata>& recalculations();
    const QList<RecalculationMetadata>& recalculations() const;

private:
    QList<RecalculationMetadata> m_recalculations;
};

// ============================================================
// Критерий отбора
// ============================================================
class FilterCriteriaMetadata final : public MetadataObject {
public:
    using MetadataObject::MetadataObject;
    QString metadataType() const override;

    // Формы
    QList<FormMetadata>& forms();
    const QList<FormMetadata>& forms() const;

    // Команды
    QList<CommandMetadata>& commands();
    const QList<CommandMetadata>& commands() const;

private:
    QList<FormMetadata> m_forms;
    QList<CommandMetadata> m_commands;
};

// ============================================================
// Хранилище настроек
// ============================================================
class SettingsStorageMetadata final : public MetadataObject {
public:
    using MetadataObject::MetadataObject;
    QString metadataType() const override;

    // Формы
    QList<FormMetadata>& forms();
    const QList<FormMetadata>& forms() const;

    // Макеты
    QList<TemplateMetadata>& templates();
    const QList<TemplateMetadata>& templates() const;

private:
    QList<FormMetadata> m_forms;
    QList<TemplateMetadata> m_templates;
};

} // namespace Metadata
} // namespace OneC


#endif // METADATA_OBJECTS_H
