#include "cfparserworker.h"
#include "APIcfBase.h"
#include "Parse_tree.h"
#include "guids.h"
#include <QDebug>
#include <memory>
#include "configuration.h"

bool get_cf_name_work(v8catalog* cf, OneC::Metadata::Configuration& config);

void get_cf_name_work(tree* tr, v8catalog *cf, OneC::Metadata::Configuration &config);

tree* get_treeFromV8file_work(v8file* f);

void fill_md_work(tree* tr, QString guid_md, v8catalog *cf, OneC::Metadata::Configuration &config);

static int detectEncoding_work(const QByteArray& data, QStringConverter::Encoding& enc);

CfParserWorker::CfParserWorker(const QString &filePath, QObject *parent)
    : QObject(parent), m_filePath(filePath) // Сохраняем путь в переменную класса
{
}

void CfParserWorker::process()
{
    try {
        // 1. Открываем каталог в рабочем потоке
        auto catalog = std::make_shared<v8catalog>(m_filePath, true);
        if (!catalog->isOpen()) {
            emit error("Не удалось открыть конфигурацию: " + m_filePath);
            return;
        }

        // 2. Парсим только дескрипторы (лёгкие данные)
        auto config = std::make_shared<OneC::Metadata::Configuration>();

        // get_cf_name_work должна заполнять Configuration дескрипторами,
        // а не читать все модули в память!
        if (!get_cf_name_work(catalog.get(), *config)) {
            emit error("Ошибка парсинга конфигурации");
            return;
        }

        config->rebuildIndices();

        // 3. Передаём владение в GUI-поток
        emit parsingFinished(std::move(catalog), std::move(config));
    }
    catch (const std::exception& e) {
        emit error(QString::fromUtf8(e.what()));
    }
}

void CfParserWorker::parseConfiguration()
{
    // ... ваш тяжелый код чтения файла ...

    /*
    QString FileName = m_filePath;

    std::unique_ptr<v8catalog> GlobalCF = std::make_unique<v8catalog>(FileName, true);

    get_cf_name_work(GlobalCF.get());

    // В любом месте метода можно испускать сигнал:
    //emit metadataItemParsed("Справочники", "Товары", "{uuid-товаров}");

    // ... продолжаем парсинг ...

    emit metadataItemParsed("Документы", "РеализацияТоваровУслуг", "{uuid-документа}");
    */
}


bool get_cf_name_work(v8catalog* cf, OneC::Metadata::Configuration& config)
{
    std::unique_ptr<tree> tr;
    tree* node;
    v8catalog* cat;
    v8file* filedata;
    QString meta;
    QString s;
    int ver;

    if(!cf)
    {
        return false;
    }

    filedata = cf->GetFile("version");
    if(!filedata)
    {
        return false;
    }

    tr.reset(get_treeFromV8file_work(filedata));
    if(!tr)
    {
        return false;
    }

    node = tr.get();

    node = &(*node)[0][0][0];
    if(node->get_type() != node_type::nd_number)
    {
        return false;
    }

    //ver = std::stoi(node->get_value());
    ver = node->get_value().toInt();
    tr.reset();

    if(ver < 100)
    {
        // 8.0
        filedata = cf->GetFile("metadata");
        if(!filedata)
        {
            return false;
        }

        cat = filedata->GetCatalog();
        if(!cat)
        {
            return false;
        }

    }
    else
    {
        //8.1 или 8.2
        cat = cf;
    }

    filedata = cat->GetFile("root");
    if(!filedata)
    {
        return false;
    }

    tr.reset(get_treeFromV8file_work(filedata));
    if(!tr)
    {
        return false;
    }

    node = tr.get();
    node = &(*node)[0][1];

    if(node->get_type() != node_type::nd_guid)
    {
        return false;
    }

    meta = node->get_value();
    tr.reset();

    filedata = cat->GetFile(meta);
    if(!filedata)
    {
        return false;
    }

    tr.reset(get_treeFromV8file_work(filedata));
    if(!tr)
    {
        return false;
    }

    get_cf_name_work(tr.release(), cf, config);

    return true;
}



void get_cf_name_work(tree* tr, v8catalog *cf, OneC::Metadata::Configuration& config)
{
    int j, k;
    tree* node;
    tree* node2;
    tree* node3;
    tree* node_Catalogs;
    tree* node_md;
    tree* curNode;
    QString cf_synonym;
    QString cf_version;
    QString s;
    bool ok;
    int structver;
    int offset_name;
    int offset_ver;
    int CountCatalogs;

    node = tr;
    node = &(*node)[0][3][1][1];

    node3 = tr;

    // Заполняем справочники

    fill_md_work(tr, GUID_Catalogs, cf, config);


    structver = (*node)[0].get_value().toInt();

    if(structver <= 15)
    {
        offset_name = 1;
        offset_ver = 12;
    }
    else
    {
        offset_name = 1;
        offset_ver = 15;
    }


    node2 = &(*node)[offset_name][1][3];

    if(node2->get_type() != node_type::nd_list)
    {

        delete tr;
        return;
    }
    if((*node2)[0].get_type() != node_type::nd_number)
    {
        delete tr;
        return;
    }

    j = (*node2)[0].get_value().toInt();

    if(j == 0)
        cf_synonym = (*node)[offset_name][1][2].get_value();
    else
    {
        ok = false;
        for(k = 0; k < j; k++)
        {
            if (QString::compare((*node2)[k * 2 + 1].get_value(), QStringLiteral("ru"), Qt::CaseInsensitive) == 0)
            {
                cf_synonym = (*node2)[k * 2 + 2].get_value();
                ok = true;
                break;
            }
        }

        if(!ok)
            cf_synonym = (*node2)[2].get_value();
    }

    cf_version = (*node)[offset_ver].get_value();

    delete tr;


}


tree* get_treeFromV8file_work(v8file* f)
{

    //TBytes sourceBytes;
    QByteArray sourceBytes;


    if (!f)
        return nullptr;

    const qint64 fileSize = f->GetFileLength();
    if (fileSize <= 0)
    {
        //error(QStringLiteral("Пустой файл контейнера"), QStringLiteral("Файл"), f->GetFullName());
        return nullptr;
    }

    sourceBytes.resize(fileSize);

    if (f->Read(sourceBytes, 0, fileSize) != fileSize)
    {
        //error(QStringLiteral("Ошибка чтения файла контейнера"), QStringLiteral("Файл"), f->GetFullName());
        return nullptr;
    }


    // Определяем кодировку
    QStringConverter::Encoding enc;
    const int off = detectEncoding_work(sourceBytes, enc);
    if (off < 0 || off > sourceBytes.size())
    {
        //error(QStringLiteral("Ошибка определения кодировки файла контейнера"), QStringLiteral("Файл"), f->GetFullName());
        return nullptr;
    }

    // Конвертируем в Unicode
    const QByteArray payload = sourceBytes.mid(off);

    QStringDecoder decoder(enc, QStringDecoder::Flag::Stateless);
    QString unicodeText = decoder(payload);

    if (decoder.hasError())
    {
        //error(QStringLiteral("Ошибка конвертации файла контейнера в Unicode"), QStringLiteral("Файл"), f->GetFullName());
        return nullptr;
    }

    if (unicodeText.isEmpty())
    {
        //error(QStringLiteral("Ошибка конвертации файла контейнера в Unicode"), QStringLiteral("Файл"), f->GetFullName());
        return nullptr;
    }

    // Если файл открывался через поток v8file — позиция уже не нужна
    tree* rt = parse_1Ctext(unicodeText, f->GetFullName());
    return rt;
}

// Процедура заполняет метаданные по корневому гуиду
void fill_md_work(tree* tr, QString guid_md, v8catalog *cf, OneC::Metadata::Configuration& config)
{
    v8file *filedata;
    tree* tree_md;
    tree* node;

    QString s;

    //v8catalog *cf = MainForm->GlobalCF.get();
    //msreg->AddMessage(L"fill_md: Начало обработки GUID: " + guid_md, MessageState::msInfo);

    // Карта путей для извлечения имен
    std::unordered_map<QString, std::vector<int>> namePaths = {
        {GUID_Catalogs,          {0,1,9,1,2}},
        {GUID_Languages,         {0,1,1,2}},
        {GUID_CommonModules,     {0,1,1,2}},
        {GUID_Roles,             {0,1,1,2}},
        {GUID_CommonTemplates,   {0,1,1,2}},
        {GUID_HTTPServices,      {0,1,2,2}},
        {GUID_ScheduledJobs,     {0,1,1,2}},
        {GUID_CommonAttributes,  {0,1,1,1,1,2}},
        {GUID_SessionParameters, {0,1,1,1,2}},
        {GUID_FunctionalOptionsParameters, {0,1,1,2}},
        {GUID_Subsystems,           {0,1,1,2}},
        {GUID_Interfaces,           {0,1,2,2}},
        {GUID_Styles,               {0,1,1,2}},
        {GUID_FilterCriteria,       {0,1,5,1,2}},
        {GUID_SettingsStorages,     {0,1,1,1,2}},
        {GUID_EventSubscriptions,   {0,1,1,2}},
        {GUID_StyleItems,           {0,1,3,2}},
        {GUID_CommonPictures,       {0,1,1,2}},
        {GUID_ExchangePlans,        {0,1,12,2}},
        {GUID_WebServices,          {0,1,2,2}},
        {GUID_WSReferences,         {1,2,2}},
        {GUID_WebSocketClients,     {0,1,1,2}},
        {GUID_IntegrationServices,  {0,1,1,2}},
        {GUID_FunctionalOptions,    {0,1,1,2}},
        {GUID_DefinedTypes,         {0,1,3,2}},
        {GUID_XDTOPackages,         {0,1,1,2}},
        {GUID_Constants,            {0,1,1,1,1,2}},
        {GUID_Documents,            {0,1,9,1,2}},
        {GUID_InformationRegisters, {0,1,15,1,2}},
        {GUID_CalculationRegisters, {0,1,15,1,2}},
        {GUID_BusinessProcesses,    {0,1,1,2}},
        {GUID_Tasks,                {0,1,1,2}},
        {GUID_AccountingRegisters,  {0,1,16,1,2}},
        {GUID_CommandGroups,        {0,1,6,2}},
        {GUID_CommonCommands,       {0,1,1,2,9,2}},
        {GUID_CommonForms,         {0,1,1,1,2}},
        {GUID_Numerators,           {0,1,1,2}},
        {GUID_JournDocuments,       {0,1,3,1,2}},
        {GUID_Reports,              {0,1,3,1,2}},
        {GUID_ChartOfCharacteristicTypes, {0,1,13,1,2}},
        {GUID_ChartsOfAccounts,           {0,1,15,1,2}},
        {GUID_ChartsOfCalculationTypes,   {0,1,1,1,2}},
        {GUID_AccumulationRegisters,      {0,1,13,1,2}},
        {GUID_Sequences,                  {0,1,7,1,2}},
        {GUID_ExternalDataSources,        {1,1,1,2}},
        {GUID_DataProcessors,             {0,1,3,1,2}},
        {GUID_Enums,                      {0,1,5,1,2}},
        {GUID_DefinedTypes,               {0,1,3,2}},
        {GUID_Bots,                       {0,1,1,2}}
    };

    auto pathIt = namePaths.find(guid_md);

    auto tryGetNodeByPath = [](tree* startNode, const std::vector<int>& candidatePath) -> tree*
    {
        tree* currentNode = startNode;
        if (!currentNode)
            return nullptr;

        for (size_t i = 0; i < candidatePath.size(); i++)
        {
            int idx = candidatePath[i];
            if (idx < 0 || idx >= currentNode->get_num_subnode())
                return nullptr;

            currentNode = currentNode->get_subnode(idx);
            if (!currentNode)
                return nullptr;
        }

        return currentNode;
    };

    if (pathIt == namePaths.end()) {
        // GUID не найден в карте путей, пропустить
        return;
    }

    tree* node_md = find_metadata_node_by_guid(tr, guid_md);
    if (!node_md) {
        return; // Защита от nullptr
    }

    const std::vector<int>& path = pathIt->second;

    tree* nextNode = node_md->get_next();
    if (!nextNode) {
        return;
    }

    //int CountMD = std::stoi(nextNode->get_value());
    int CountMD = nextNode->get_value().toInt();

    tree* curNode = node_md->get_next();

    int processedCount = 0;

    while (curNode)
    {
        curNode = curNode->get_next();
        if (curNode)
        {
            processedCount++;
            QString curNodeValue = curNode->get_value();

            filedata = cf->GetFile(curNodeValue);

            if(!filedata)
            {
                continue; // Продолжить с следующим
            }

            try
            {
                tree_md = get_treeFromV8file_work(filedata);
            }
            catch (...)
            {
                continue;
            }
            if(!tree_md)
            {
                continue;
            }

            node = nullptr;

            try {
                std::vector<std::vector<int>> candidatePaths = {path};

                if (guid_md == GUID_BusinessProcesses || guid_md == GUID_Tasks)
                {
                    candidatePaths.push_back({0,1,1});
                    candidatePaths.push_back({0,1,2});
                }
                else if (guid_md == GUID_ExternalDataSources)
                {
                    candidatePaths.push_back({1,0,1,2});
                    candidatePaths.push_back({0,1,1,2});
                }

                for (size_t candidateIndex = 0; candidateIndex < candidatePaths.size() && !node; candidateIndex++)
                {
                    const std::vector<int>& candidatePath = candidatePaths[candidateIndex];
                    node = tryGetNodeByPath(tree_md, candidatePath);
                }
            }
            catch (...) {
                delete tree_md;
                continue;
            }

            if (!node) {
                continue;
            }

            // Проверка типа узла перед получением значения
            if (node->get_type() == node_type::nd_empty || node->get_type() == node_type::nd_unknown) {
                delete tree_md;
                continue;
            }

            QString val = node->get_value();

            if (val.length() == 0) {
                if (guid_md == GUID_ExternalDataSources)
                {
                    val = curNodeValue;
                }
                else
                {
                    delete tree_md;
                    continue;
                }
            }

            // Создание объектов для специфических типов
            try {
                if (guid_md == GUID_Catalogs)
                {

                    //MainForm->mdCatalogs.push_back(std::make_unique<TCatalogs>(cf, curNode->get_value(), val));
                    //emit metadataItemParsed("Справочники", curNode->get_value(), "{uuid-товаров}");
                    qDebug() << "Найден справочник:" << val << " c id: " << curNode->get_value();
                    //OneC::Metadata::MetadataDescriptor out = std::make_unique<OneC::Metadata::MetadataDescriptor>(val, val, curNode->get_value(), "Справочник", curNode->get_value());
                    // ✅ Просто создаём объект на стеке
                    OneC::Metadata::MetadataDescriptor desc(
                        val,                            // name
                        val,                            // synonym
                        QUuid::fromString(curNode->get_value()),           // uuid
                        "Справочник",                   // type
                        curNode->get_value()            // internalFileName
                    );
                    config.addCatalog(desc);
                }
                else if (guid_md == GUID_Languages)
                {
//					msreg->AddMessage(L"fill_md: Создание языка: " + val, MessageState::msInfo);
//					MainForm->mdLanguages.push_back(std::make_unique<TLangs>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_CommonModules)
                {
//					msreg->AddMessage(L"fill_md: Создание общего модуля: " + val, MessageState::msInfo);
//					MainForm->mdCommonModules.push_back(std::make_unique<TCommonModules>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_Roles)
                {
//					msreg->AddMessage(L"fill_md: Создание роли: " + val, MessageState::msInfo);
//					MainForm->mdRoles.push_back(std::make_unique<TRoles>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_CommonTemplates)
                {
//					msreg->AddMessage(L"fill_md: Создание общего макета: " + val, MessageState::msInfo);
//					MainForm->mdCommonTemplates.push_back(std::make_unique<TCommonTemplates>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_HTTPServices)
                {
//					msreg->AddMessage(L"fill_md: Создание HTTP-сервиса: " + val, MessageState::msInfo);
//					MainForm->mdHTTPServices.push_back(std::make_unique<THTTPServices>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_ScheduledJobs)
                {
//					msreg->AddMessage(L"fill_md: Создание регламентного задания: " + val, MessageState::msInfo);
//					MainForm->mdScheduledJobs.push_back(std::make_unique<TScheduledJobs>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_CommonAttributes)
                {
//					msreg->AddMessage(L"fill_md: Создание общего реквизита: " + val, MessageState::msInfo);
//					MainForm->mdCommonAttributes.push_back(std::make_unique<TCommonAttributes>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_SessionParameters)
                {
//					msreg->AddMessage(L"fill_md: Создание параметра сеанса: " + val, MessageState::msInfo);
//					MainForm->mdSessionParameters.push_back(std::make_unique<TSessionParameters>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_FunctionalOptionsParameters)
                {
//					msreg->AddMessage(L"fill_md: Создание параметра функциональной опции: " + val, MessageState::msInfo);
//					MainForm->mdFunctionalOptionsParameters.push_back(std::make_unique<TFunctionalOptionsParameters>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_Subsystems)
                {
                    //msreg->AddMessage(L"fill_md: Пропуск подсистемы: " + val, MessageState::msInfo);
//					msreg->AddMessage(L"fill_md: Создание подсистемы: " + val, MessageState::msInfo);
//					MainForm->mdSubsystems.push_back(std::make_unique<TSubsystem>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_Interfaces)
                {
//					msreg->AddMessage(L"fill_md: Создание интерфейса: " + val, MessageState::msInfo);
//					MainForm->mdInterfaces.push_back(std::make_unique<TInterfaces>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_Styles)
                {
//					msreg->AddMessage(L"fill_md: Создание стиля: " + val, MessageState::msInfo);
//					MainForm->mdStyles.push_back(std::make_unique<TStyles>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_FilterCriteria)
                {
//					msreg->AddMessage(L"fill_md: Создание критерия отбора: " + val, MessageState::msInfo);
//					MainForm->mdFilterCriteria.push_back(std::make_unique<TFilterCriteria>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_SettingsStorages)
                {
//					msreg->AddMessage(L"fill_md: Создание хранилища настроек: " + val, MessageState::msInfo);
//					MainForm->mdSettingsStorages.push_back(std::make_unique<TSettingsStorages>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_StyleItems)
                {
//					msreg->AddMessage(L"fill_md: Создание элемента стиля: " + val, MessageState::msInfo);
//					MainForm->mdStyleItems.push_back(std::make_unique<TStyleItems>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_CommonPictures)
                {
//					msreg->AddMessage(L"fill_md: Создание общей картинки: " + val, MessageState::msInfo);
//					MainForm->mdCommonPictures.push_back(std::make_unique<TCommonPictures>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_ExchangePlans)
                {
//					msreg->AddMessage(L"fill_md: Создание плана обмена: " + val, MessageState::msInfo);
//					MainForm->mdExchangePlans.push_back(std::make_unique<TExchangePlans>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_EventSubscriptions)
                {
//					msreg->AddMessage(L"fill_md: Создание подписки на событие: " + val, MessageState::msInfo);
//					MainForm->mdEventSubscriptions.push_back(std::make_unique<TEventSubscriptions>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_WebServices)
                {
//					msreg->AddMessage(L"fill_md: Создание веб-сервиса: " + val, MessageState::msInfo);
//					MainForm->mdWebServices.push_back(std::make_unique<TWebServices>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_FunctionalOptions)
                {
//					msreg->AddMessage(L"fill_md: Создание функциональной опции: " + val, MessageState::msInfo);
//					MainForm->mdFunctionalOptions.push_back(std::make_unique<TFunctionalOptions>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_DefinedTypes)
                {
//					msreg->AddMessage(L"fill_md: Создание определяемого типа: " + val, MessageState::msInfo);
//					MainForm->mdDefinedTypes.push_back(std::make_unique<TDefinedTypes>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_XDTOPackages)
                {
//					msreg->AddMessage(L"fill_md: Создание XDTO-пакета: " + val, MessageState::msInfo);
//					MainForm->mdXDTOPackages.push_back(std::make_unique<TXDTOPackages>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_WSReferences)
                {
//					msreg->AddMessage(L"fill_md: Создание WS-ссылки: " + val, MessageState::msInfo);
//					MainForm->mdWSReferences.push_back(std::make_unique<TWSReferences>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_WebSocketClients)
                {
//					msreg->AddMessage(L"fill_md: Создание WebSocket-клиента: " + val, MessageState::msInfo);
//					MainForm->mdWebSocketClients.push_back(std::make_unique<TWebSocketClients>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_IntegrationServices)
                {
//					msreg->AddMessage(L"fill_md: Создание сервиса интеграции: " + val, MessageState::msInfo);
//					MainForm->mdIntegrationServices.push_back(std::make_unique<TIntegrationServices>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_Constants)
                {
//					msreg->AddMessage(L"fill_md: Создание константы: " + val, MessageState::msInfo);
//					MainForm->mdConstants.push_back(std::make_unique<TConstants>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_Documents)
                {
//					msreg->AddMessage(L"fill_md: Создание документа: " + val, MessageState::msInfo);
//					MainForm->mdDocuments.push_back(std::make_unique<TDocuments>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_CommonForms)
                {
//					msreg->AddMessage(L"fill_md: Создание общей формы: " + val, MessageState::msInfo);
//					MainForm->mdCommonForms.push_back(std::make_unique<TCommonForms>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_InformationRegisters)
                {
//					msreg->AddMessage(L"fill_md: Создание регистра сведений: " + val, MessageState::msInfo);
//					MainForm->mdInformationRegisters.push_back(std::make_unique<TInformationRegisters>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_CalculationRegisters)
                {
//					msreg->AddMessage(L"fill_md: Создание регистра расчета: " + val, MessageState::msInfo);
//					MainForm->mdCalculationRegisters.push_back(std::make_unique<TCalculationRegisters>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_BusinessProcesses)
                {
//					msreg->AddMessage(L"fill_md: Создание бизнес-процесса: " + val, MessageState::msInfo);
//					MainForm->mdBusinessProcesses.push_back(std::make_unique<TBusinessProceses>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_Tasks)
                {
//					msreg->AddMessage(L"fill_md: Создание задачи: " + val, MessageState::msInfo);
//					MainForm->mdTasks.push_back(std::make_unique<TTasks>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_AccountingRegisters)
                {
//					msreg->AddMessage(L"fill_md: Создание регистра бухгалтерии: " + val, MessageState::msInfo);
//					MainForm->mdAccountingRegisters.push_back(std::make_unique<TAccountingRegisters>(cf, curNode->get_value(), val));
//					msreg->AddMessage(L"fill_md: Регистр бухгалтерии создан успешно: " + val, MessageState::msInfo);
                }
                else if (guid_md == GUID_CommandGroups)
                {
//					msreg->AddMessage(L"fill_md: Создание группы команд: " + val, MessageState::msInfo);
//					MainForm->mdCommandGroups.push_back(std::make_unique<TCommandGroups>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_CommonCommands)
                {
//					msreg->AddMessage(L"fill_md: Создание общей команды: " + val, MessageState::msInfo);
//					MainForm->mdCommonCommands.push_back(std::make_unique<TCommonCommands>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_Numerators)
                {
//					msreg->AddMessage(L"fill_md: Создание нумератора: " + val, MessageState::msInfo);
//					MainForm->mdDocumentNumerators.push_back(std::make_unique<TNumerators>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_JournDocuments)
                {
//					msreg->AddMessage(L"fill_md: Создание журнала документов: " + val, MessageState::msInfo);
//					MainForm->mdDocumentJournals.push_back(std::make_unique<TJournals>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_Reports)
                {
//					msreg->AddMessage(L"fill_md: Создание отчета: " + val, MessageState::msInfo);
//					MainForm->mdReports.push_back(std::make_unique<TReports>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_ChartOfCharacteristicTypes)
                {
//					msreg->AddMessage(L"fill_md: Создание ПВХ: " + val, MessageState::msInfo);
//					MainForm->mdChartsOfCharacteristicTypes.push_back(std::make_unique<TChartOfCharacteristicTypes>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_ChartsOfAccounts)
                {
//					msreg->AddMessage(L"fill_md: Создание плана счетов: " + val, MessageState::msInfo);
//					MainForm->mdChartOfAccounts.push_back(std::make_unique<TChartOfAccounts>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_ChartsOfCalculationTypes)
                {
//					msreg->AddMessage(L"fill_md: Создание ПВР: " + val, MessageState::msInfo);
//					MainForm->mdChartOfCalculationTypes.push_back(std::make_unique<TChartOfCalculationTypes>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_AccumulationRegisters)
                {
//					msreg->AddMessage(L"fill_md: Создание регистра накопления: " + val, MessageState::msInfo);
//					MainForm->mdAccumulationRegisters.push_back(std::make_unique<TAccumulationRegisters>(cf, curNode->get_value(), val));
//					msreg->AddMessage(L"fill_md: Регистр накопления создан успешно: " + val, MessageState::msInfo);
                }
                else if (guid_md == GUID_Sequences)
                {
//					msreg->AddMessage(L"fill_md: Создание последовательности: " + val, MessageState::msInfo);
//					MainForm->mdSequences.push_back(std::make_unique<TSequences>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_DataProcessors)
                {
//					msreg->AddMessage(L"fill_md: Создание обработки: " + val, MessageState::msInfo);
//					MainForm->mdDataProcessors.push_back(std::make_unique<TDataProcessors>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_Enums)
                {
//					msreg->AddMessage(L"fill_md: Создание перечисления: " + val, MessageState::msInfo);
//					MainForm->mdEnums.push_back(std::make_unique<TEnums>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_Bots)
                {
//					msreg->AddMessage(L"fill_md: Создание бота: " + val, MessageState::msInfo);
//					MainForm->mdBots.push_back(std::make_unique<TBots>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_ExternalDataSources)
                {
//					msreg->AddMessage(L"fill_md: Создание внешнего источника данных: " + val, MessageState::msInfo);
//					MainForm->mdExternalDataSources.push_back(std::make_unique<TExternalDataSources>(cf, curNode->get_value(), val));
                }
                else
                {
//					msreg->AddMessage(L"fill_md: Неизвестный GUID для объекта: " + val, MessageState::msWarning);
                }
            }

            catch (...) {
//				msreg->AddMessage_(L"fill_md: неизвестное исключение при создании объекта", msError,
//						L"Name", val,
//						L"GUID", guid_md,
//						L"File", curNodeValue);
//				LogHeapStatus(L"fill_md: память при неизвестном exception создания объекта", guid_md, curNodeValue, processedCount, CountMD);
            }
            delete tree_md;
        }
    }


}

// Определяем кодировку по BOM и первым байтам.
// Возвращает:
//   offset — сколько байт BOM надо пропустить (0 — BOM нет)
//   codec  — выбранный декодер
static int detectEncoding_work(const QByteArray& data, QStringConverter::Encoding& enc)
{
    if (data.size() >= 3 &&
        (uchar)data[0] == 0xEF &&
        (uchar)data[1] == 0xBB &&
        (uchar)data[2] == 0xBF)
    {
        enc = QStringConverter::Utf8;
        return 3;
    }

    if (data.size() >= 2)
    {
        const uchar b0 = (uchar)data[0];
        const uchar b1 = (uchar)data[1];
        if (b0 == 0xFF && b1 == 0xFE) { enc = QStringConverter::Utf16LE; return 2; }
        if (b0 == 0xFE && b1 == 0xFF) { enc = QStringConverter::Utf16BE; return 2; }
    }

    // Эвристика: попробуем UTF-8 строгим декодером
    if (!data.isEmpty())
    {
        QStringDecoder test(QStringDecoder::Utf8, QStringDecoder::Flag::Stateless);
        QString s = test(data);
        if (!test.hasError()) {
            enc = QStringConverter::Utf8;
            return 0;
        }
    }

    // Иначе — Latin1 (аналог ANSI в 1С-контейнерах)
    enc = QStringConverter::Latin1;
    return 0;
}


