#ifndef CF_LOADER_H
#define CF_LOADER_H
#include <QString>
#include <memory>

#include "metadata_descriptor.h"
#include "metadata_objects.h"
#include "APIcfBase.h"

// Предварительное объявление класса из вашей библиотеки
//class v8catalog;
//class v8file;

namespace OneC::Metadata {

/**
 * @brief Загрузчик полных объектов метаданных из открытой конфигурации.
 *
 * Использует v8catalog для доступа к внутренним "файлам" 1Cv8.cf.
 * Не владеет каталогом — каталог должен жить дольше лоадера.
 */
class CfLoader {
public:
    /// @param catalog Указатель на уже открытый v8catalog. Не может быть nullptr.
    explicit CfLoader(v8catalog* catalog);

    /// Загружает объект по дескриптору.
    /// @return shared_ptr на объект или nullptr при ошибке.
    std::shared_ptr<MetadataObject> load(const MetadataDescriptor& desc);

private:
    bool readCatalogBody(v8file* file, CatalogMetadata& cat);
    bool readCommonModuleBody(v8file* file, CommonModules& mod);
    bool readDocumentBody(v8file* file, DocumentMetadata& doc);
    // ... остальные read* ...

    v8catalog* m_catalog = nullptr;   // не владеет!
};

} // namespace OneC::Metadata
#endif // CF_LOADER_H
