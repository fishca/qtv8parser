#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenu>
#include <QAction>
#include <QSettings>

#include <QStandardItemModel>
#include <QMap>

#include <QSplitter>          // <-- добавить
#include <QResizeEvent>       // <-- добавить


#include <QToolBar>
#include <QSizePolicy>

#include "cfparserworker.h"
#include "configurationsession.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

public slots:
    // Слот, который будет вызываться, когда парсер найдет объект
    void addMetadataItem(const QString& categoryName, const QString& itemName, const QString& uuid);

protected:
    void resizeEvent(QResizeEvent* event) override;   // <-- добавить

private slots:
    // Этот слот будет автоматически вызван при клике на pushButton
    void on_pushButton_clicked();

    void openFile();                         // выбор файла через диалог
    void openRecentFile(const QString &filePath);                   // слот для пунктов истории
    void onParsingFinished(std::shared_ptr<v8catalog> catalog,
                               std::shared_ptr<OneC::Metadata::Configuration> config);
    void onTreeItemExpanded(const QModelIndex& index);
    void onTreeItemDoubleClicked(const QModelIndex& index);
    void on_splitterMoved(int pos, int index);        // <-- добавить
    void showAboutDialog();   // <-- добавить

private:
    Ui::MainWindow *ui;

    void setupMenuBar();
    void updateRecentFileMenu();             // обновить подменю
    void addRecentFile(const QString &filePath);
    void loadRecentFiles();                  // загрузить из QSettings
    void saveRecentFiles();                  // сохранить в QSettings
    void loadFile(const QString &filePath);
    void populateTree();
    void populateTreeFromConfiguration(const OneC::Metadata::Configuration& config);


    QMenu *fileMenu;
    QMenu *recentMenu;
    QAction *openAction;
    QAction *exitAction;

    QMenu*   helpMenu    = nullptr;   // <-- добавить
    QAction* aboutAction = nullptr;   // <-- добавить

    QStringList recentFiles;                 // список путей (от новых к старым)
    static const int maxRecentFiles = 10;    // максимальное количество
    QSettings settings;                      // для хранения настроек

    QStandardItemModel *m_model    = nullptr;
    QStandardItem      *m_rootItem = nullptr;

    // Карта: Имя категории -> Указатель на узел в дереве
    QMap<QString, QStandardItem*> m_categoryNodes;

    std::unique_ptr<OneC::Metadata::ConfigurationSession> m_session;
    CfParserWorker* m_worker = nullptr;
    QThread*        m_workerThread = nullptr;

    void setupSplitter();                             // <-- добавить
    void updateTreeMaxWidth();                        // <-- добавить

    QSplitter* m_splitter    = nullptr;               // <-- добавить
    QWidget*   m_rightPanel  = nullptr;               // <-- добавить

    void setupFileToolbar();   // <-- добавить

    QIcon iconForType(const QString& metadataType) const;
    QIcon iconForCategory(const QString& categoryKey) const;   // <-- добавить


};
#endif // MAINWINDOW_H
