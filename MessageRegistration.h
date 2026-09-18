#ifndef MESSAGEREGISTRATION_H
#define MESSAGEREGISTRATION_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QPair>
#include <QVariantMap>
#include <memory>

// Типы сообщений (оставлены совместимыми с оригиналом)
enum MessageState {
    msEmpty      = -1,
    msSuccesfull = 0,
    msWarning    = 1,
    msInfo       = 2,
    msError      = 3,
    msWait       = 4,
    msHint       = 5
};

// Вспомогательный тип для хранения параметров (имя + значение)
using MessageParam = QPair<QString, QString>;
using MessageParams = QVector<MessageParam>;

/**
 * Базовый класс регистратора сообщений.
 * Наследует QObject для поддержки сигналов/слотов.
 * Предоставляет шаблонные методы для удобного добавления сообщений с параметрами.
 */
class MessageRegistrator : public QObject
{
    Q_OBJECT
public:
    explicit MessageRegistrator(QObject *parent = nullptr);

    // Установка режима отладки
    void setDebugMode(bool enabled);
    bool debugMode() const;

    // ---- Основные методы (виртуальные для переопределения) ----

    // Добавить сообщение с параметрами (передаётся QVariantMap для гибкости)
    virtual void addMessage(const QString &description,
                            MessageState state,
                            const QVariantMap &params = QVariantMap()) = 0;

    // Сообщить о статусе (прогресс)
    virtual void status(const QString &message) = 0;

    // ---- Удобные обёртки с использованием вариативных шаблонов ----

    // Добавить ошибку (state = msError)
    template<typename... Args>
    void addError(const QString &description, Args&&... args) {
        addMessage(description, msError, makeParams(std::forward<Args>(args)...));
    }

    // Добавить сообщение с произвольным состоянием и параметрами
    template<typename... Args>
    void addMessage(const QString &description, MessageState state, Args&&... args) {
        addMessage(description, state, makeParams(std::forward<Args>(args)...));
    }

    // Добавить отладочное сообщение (выводится только если debugMode включён)
    template<typename... Args>
    void addDebug(const QString &description, MessageState state, Args&&... args) {
        if (m_debugMode) {
            addMessage(description, state, makeParams(std::forward<Args>(args)...));
        }
    }

signals:
    // Сигнал, испускаемый при каждом добавленном сообщении
    void messageAdded(const QString &description, MessageState state, const QVariantMap &params);
    // Сигнал для статуса
    void statusChanged(const QString &message);

protected:
    // Вспомогательная функция для сборки параметров из пар "имя, значение"
    // Работает с произвольным числом аргументов, которые должны чередоваться: name, value, name, value, ...
    template<typename... Args>
    static QVariantMap makeParams(Args&&... args) {
        QVariantMap map;
        // Рекурсивно обрабатываем пары
        // Используем вспомогательную структуру
        buildParams(map, std::forward<Args>(args)...);
        return map;
    }

private:
    bool m_debugMode = false;

    // Вспомогательные функции для разбора пар (рекурсивные шаблоны)
    static void buildParams(QVariantMap &map) {
        // Базовый случай: ничего не делаем
    }

    template<typename T, typename U, typename... Rest>
    static void buildParams(QVariantMap &map, T&& name, U&& value, Rest&&... rest) {
        map.insert(QString::fromUtf8(name), QVariant::fromValue(QString::fromUtf8(value)));
        buildParams(map, std::forward<Rest>(rest)...);
    }

    // Перегрузка для случая, когда параметры передаются как QVariantMap (уже готовый)
    static void buildParams(QVariantMap &map, const QVariantMap &prebuilt) {
        for (auto it = prebuilt.begin(); it != prebuilt.end(); ++it)
            map.insert(it.key(), it.value());
    }
};

// ============ Конкретная реализация для вывода в консоль (пример) ============

class ConsoleMessageRegistrator : public MessageRegistrator
{
    Q_OBJECT
public:
    explicit ConsoleMessageRegistrator(QObject *parent = nullptr);

    void addMessage(const QString &description,
                    MessageState state,
                    const QVariantMap &params = QVariantMap()) override;

    void status(const QString &message) override;
};

#endif // MESSAGEREGISTRATION_H