#ifndef METADATA_REPOSITORY_H
#define METADATA_REPOSITORY_H

#include <QCache>
#include <QMutex>
#include <QMutexLocker>
#include <QUuid>
#include <functional>
#include <memory>

#include "metadata_descriptor.h"
#include "metadata_objects.h"

namespace OneC::Metadata {

/**
 * @brief Репозиторий полных объектов метаданных с ленивой загрузкой.
 *
 * Хранит в памяти только те объекты, которые реально запрашивались.
 * При превышении лимита кэша самые старые записи вытесняются.
 *
 * Потокобезопасен: можно вызывать из GUI-потока и из рабочих потоков.
 */
class MetadataRepository {
public:
    /// Функция загрузки полного объекта по дескриптору.
    /// Реализуется тем, кто знает формат 1Cv8.cf.
    using Loader = std::function<std::shared_ptr<MetadataObject>(const MetadataDescriptor&)>;

    /// @param loader      Функция загрузки (не может быть пустой).
    /// @param maxObjects  Максимальное количество объектов в кэше.
    explicit MetadataRepository(Loader loader, int maxObjects = 500);

    MetadataRepository(const MetadataRepository&)            = delete;
    MetadataRepository& operator=(const MetadataRepository&) = delete;

    /// Получить полный объект по дескриптору (загружает при первом обращении).
    /// @return shared_ptr на объект или nullptr, если загрузка не удалась.
    std::shared_ptr<const MetadataObject> get(const MetadataDescriptor& desc);

    /// Типизированный доступ: сразу приводит к нужному классу.
    template<typename T>
    std::shared_ptr<const T> getAs(const MetadataDescriptor& desc) {
        return std::dynamic_pointer_cast<const T>(get(desc));
    }

    /// Полностью очистить кэш (например, при закрытии конфигурации).
    void clearCache();

    /// Изменить лимит объектов в кэше.
    void setMaxObjects(int count);

    /// Текущее количество объектов в кэше.
    int cachedCount() const;

private:
    Loader m_loader;
    mutable QMutex m_mutex;

    // QCache владеет указателями, мы храним shared_ptr в куче,
    // чтобы вызывающий код мог безопасно пережить вытеснение из кэша.
    QCache<QUuid, std::shared_ptr<MetadataObject>> m_cache;
};

} // namespace OneC::Metadata

#endif // METADATA_REPOSITORY_H
