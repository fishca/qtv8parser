#include "configurationsession.h"
#include "APIcfBase.h"
#include "cf_loader.h"

namespace OneC::Metadata {

ConfigurationSession::~ConfigurationSession() = default;

void ConfigurationSession::setCatalog(std::shared_ptr<v8catalog> catalog)
{
    m_catalog = std::move(catalog);
}

void ConfigurationSession::setConfiguration(std::shared_ptr<Configuration> config)
{
    m_configuration = std::move(config);
}

void ConfigurationSession::initializeRepository(int cacheSize)
{
    Q_ASSERT(m_catalog);
    Q_ASSERT(m_configuration);

    auto loader = std::make_shared<CfLoader>(m_catalog.get());

    m_repository = std::make_unique<MetadataRepository>(
        [loader](const MetadataDescriptor& d) { return loader->load(d); },
        cacheSize
    );

    m_configuration->setRepository(std::move(m_repository));
}

std::shared_ptr<const MetadataObject>
ConfigurationSession::loadObject(const MetadataDescriptor& desc)
{
    if (!m_configuration) return nullptr;
    auto* repo = m_configuration->repository();
    return repo ? repo->get(desc) : nullptr;
}

} // namespace OneC::Metadata
