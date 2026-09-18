#include "cf_loader.h"
//#include "v8catalog.h"    // ваш заголовок
//#include "v8file.h"
#include <QDebug>

namespace OneC::Metadata {

CfLoader::CfLoader(v8catalog* catalog)
    : m_catalog(catalog)
{
    Q_ASSERT(m_catalog);
}

std::shared_ptr<MetadataObject>
CfLoader::load(const MetadataDescriptor& desc)
{
    if (!m_catalog || !desc.isValid()) {
        return nullptr;
    }

    // 1. Получаем внутренний "файл" из каталога
    v8file* file = m_catalog->GetFile(desc.internalFileName());
    if (!file) {
        qWarning() << "Внутренний файл не найден:" << desc.internalFileName();
        return nullptr;
    }

    // 2. Создаём нужный объект в зависимости от типа
    std::shared_ptr<MetadataObject> obj;
    const QString type = desc.type();

    if (type == "Catalog") {
        auto cat = std::make_shared<CatalogMetadata>();
        // Лёгкие поля — из дескриптора, не читаем повторно
        cat->setObjectName(desc.name());
        cat->setObjectSynonym(desc.synonym());
        cat->setObjectUuid(desc.uuid());

        if (!readCatalogBody(file, *cat)) return nullptr;
        obj = cat;
    }
    else if (type == "CommonModule") {
        auto mod = std::make_shared<CommonModules>();
        mod->setObjectName(desc.name());
        mod->setObjectSynonym(desc.synonym());
        mod->setObjectUuid(desc.uuid());

        if (!readCommonModuleBody(file, *mod)) return nullptr;
        obj = mod;
    }
    else if (type == "Document") {
        auto doc = std::make_shared<DocumentMetadata>();
        doc->setObjectName(desc.name());
        doc->setObjectSynonym(desc.synonym());
        doc->setObjectUuid(desc.uuid());

        if (!readDocumentBody(file, *doc)) return nullptr;
        obj = doc;
    }
    // ... остальные типы ...

    return obj;
}

bool CfLoader::readCatalogBody(v8file* file, CatalogMetadata& cat)
{
    // Здесь — ваш существующий код парсинга тела справочника.
    // Вместо QDataStream над QFile используйте поток v8file.
    //
    // v8file — судя по API, содержит поток с данными объекта.
    // Точный способ получить поток зависит от вашей библиотеки:
    // возможно, file->data, или file->Open(), или file->Stream().

    // Пример (синтаксис условный — подставьте свой):
    // vcl_utils::TStream* stream = file->GetStream();
    // parseCatalogStream(stream, cat);

    return true;
}

bool CfLoader::readCommonModuleBody(v8file* file, CommonModules& mod)
{
    // Аналогично — читаем текст модуля, ставим в mod.setModuleText(...)
    return true;
}

bool CfLoader::readDocumentBody(v8file* file, DocumentMetadata& doc)
{
    return true;
}

}
