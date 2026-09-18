#include "metadata_repository.h"

namespace OneC::Metadata {

MetadataRepository::MetadataRepository(Loader loader, int maxObjects)
    : m_loader(std::move(loader))
{
    m_cache.setMaxCost(maxObjects);
}

std::shared_ptr<const MetadataObject>
MetadataRepository::get(const MetadataDescriptor& desc)
{
    if (!desc.isValid() || !m_loader) {
        return nullptr;
    }

    QMutexLocker locker(&m_mutex);

    // 1. Проверяем кэш
    if (auto* cached = m_cache.object(desc.uuid())) {
        return *cached; // разыменовываем shared_ptr
    }

    // 2. Загружаем (внутри блокировки, чтобы избежать гонок)
    auto obj = m_loader(desc);
    if (!obj) {
        return nullptr;
    }

    // 3. Кладём в кэш. cost = 1 на объект.
    //    При желании можно использовать desc.dataSize() / 1024 как cost.
    auto* holder = new std::shared_ptr<MetadataObject>(obj);
    m_cache.insert(desc.uuid(), holder, 1);

    return obj;
}

void MetadataRepository::clearCache()
{
    QMutexLocker locker(&m_mutex);
    m_cache.clear();
}

void MetadataRepository::setMaxObjects(int count)
{
    QMutexLocker locker(&m_mutex);
    m_cache.setMaxCost(count);
}

int MetadataRepository::cachedCount() const
{
    QMutexLocker locker(&m_mutex);
    return m_cache.count();
}

} // namespace OneC::Metadata
