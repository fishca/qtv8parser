#ifndef CONFIGURATIONSESSION_H
#define CONFIGURATIONSESSION_H


#include <QString>
#include <memory>

#include "configuration.h"
#include "metadata_repository.h"

class v8catalog;

namespace OneC::Metadata {

/**
 * @brief Владелец открытой конфигурации.
 *
 * Владеет v8catalog, дескрипторами и репозиторием полных объектов.
 * Создаётся в GUI-потоке, живёт всё время работы с конфигурацией.
 */
class ConfigurationSession {
public:
    /// Создаёт сессию (каталог ещё не открыт).
    ConfigurationSession() = default;
    ~ConfigurationSession();

    ConfigurationSession(const ConfigurationSession&)            = delete;
    ConfigurationSession& operator=(const ConfigurationSession&) = delete;

    /// Вызывается из воркера после парсинга. Принимает владение каталогом.
    /// На этот момент воркер уже не должен обращаться к каталогу.
    void setCatalog(std::shared_ptr<v8catalog> catalog);

    /// Заполняет Configuration из уже распарсенных дескрипторов.
    void setConfiguration(std::shared_ptr<Configuration> config);

    /// Инициализирует репозиторий с лоадером поверх каталога.
    /// Вызывать только после setCatalog().
    void initializeRepository(int cacheSize = 300);

    // ---- Доступ для GUI ----
    const Configuration* configuration() const { return m_configuration.get(); }
    Configuration*       configuration()       { return m_configuration.get(); }

    std::shared_ptr<const MetadataObject> loadObject(const MetadataDescriptor& desc);

    bool isReady() const { return m_catalog && m_configuration && m_repository; }

private:
    std::shared_ptr<v8catalog>                 m_catalog;
    std::shared_ptr<Configuration>             m_configuration;
    std::unique_ptr<MetadataRepository>        m_repository;
};

} // namespace OneC::Metadata
#endif // CONFIGURATIONSESSION_H
