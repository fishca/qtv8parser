#include "MessageRegistration.h"
#include <QDebug>
#include <QTextStream>
#include <QDateTime>

MessageRegistrator::MessageRegistrator(QObject *parent)
    : QObject(parent)
{
}

void MessageRegistrator::setDebugMode(bool enabled)
{
    m_debugMode = enabled;
}

bool MessageRegistrator::debugMode() const
{
    return m_debugMode;
}

// ===== ConsoleMessageRegistrator =====

ConsoleMessageRegistrator::ConsoleMessageRegistrator(QObject *parent)
    : MessageRegistrator(parent)
{
}

void ConsoleMessageRegistrator::addMessage(const QString &description,
                                           MessageState state,
                                           const QVariantMap &params)
{
    // Формируем строку для вывода
    QString stateStr;
    switch (state) {
    case msError:      stateStr = "[ERROR]"; break;
    case msWarning:    stateStr = "[WARN]"; break;
    case msInfo:       stateStr = "[INFO]"; break;
    case msSuccesfull: stateStr = "[OK]"; break;
    default:           stateStr = "[MSG]"; break;
    }

    QString output = QString("%1 %2: %3")
                         .arg(QDateTime::currentDateTime().toString("hh:mm:ss.zzz"))
                         .arg(stateStr)
                         .arg(description);

    if (!params.isEmpty()) {
        QStringList paramLines;
        for (auto it = params.begin(); it != params.end(); ++it) {
            paramLines << QString("    %1 = %2").arg(it.key()).arg(it.value().toString());
        }
        output += "\n" + paramLines.join("\n");
    }

    // Вывод в зависимости от состояния
    if (state == msError) {
        qCritical() << output;
    } else if (state == msWarning) {
        qWarning() << output;
    } else {
        qDebug() << output;
    }

    // Испускаем сигнал (для GUI)
    emit messageAdded(description, state, params);
}

void ConsoleMessageRegistrator::status(const QString &message)
{
    QString output = QString("[STATUS] %1: %2")
    .arg(QDateTime::currentDateTime().toString("hh:mm:ss.zzz"))
        .arg(message);
    qDebug() << output;
    emit statusChanged(message);
}