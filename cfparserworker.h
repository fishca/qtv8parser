#ifndef CFPARSERWORKER_H
#define CFPARSERWORKER_H

#include <QObject>
#include <QString>
#include "APIcfBase.h"
#include "configuration.h"

class CfParserWorker : public QObject
{
    Q_OBJECT

public:
    // Конструктор, который принимает путь к файлу
    explicit CfParserWorker(const QString &filePath, QObject *parent = nullptr);

public slots:
    // Слот, который запустит процесс парсинга (вызовется при старте потока)
    void process();

signals:
    // Сигналы, которые worker будет отправлять в главный поток (GUI)
    void metadataItemParsed(const QString& categoryName, const QString& itemName, const QString& uuid);
    // Передаём каталог и готовую конфигурацию в GUI-поток.
    // shared_ptr безопасно копируется между потоками.

    void parsingFinished(std::shared_ptr<v8catalog> catalog, std::shared_ptr<OneC::Metadata::Configuration> config);
    void finished();
    void error(const QString& message);

private:
    // Ваши существующие функции парсинга
    void parseConfiguration();
    QString m_filePath; // Храним путь к файлу внутри класса

};

#endif // CFPARSERWORKER_H
