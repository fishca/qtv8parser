#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFile>
#include <QByteArray>
#include <QString>
#include <QStringDecoder>


#include <QFileDialog>   // обязательно
#include <QDebug>        // для вывода в консоль

#include <QMenuBar>
#include <QMessageBox>

#include <QStandardItemModel>
#include <QStandardItem>
#include <QIcon>
#include "globals.h"
#include "Parse_tree.h"
#include "guids.h"
#include "cfparserworker.h"
#include <QStyleFactory>

#include <QThread>

#include <QSplitter>
#include <QResizeEvent>
#include <QTimer>
#include <QLabel>
#include <QHeaderView>

#include <QApplication>
#include <QStyle>

typedef QByteArray TBytes;

static int detectEncoding(const QByteArray& data, QStringConverter::Encoding& enc);
tree* _get_treeFromV8file(v8file* f);
tree* get_treeFromV8file(v8file* f);

void get_cf_name(v8catalog* cf);
void get_cf_name(tree* tr, v8catalog *cf);
void fill_md(tree* tr, QString guid_md, v8catalog *cf);



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    ,ui(new Ui::MainWindow)
    ,settings("YourCompany", "YourApp")   // организация и приложение
{
    ui->setupUi(this);

    //ui->treeView->setStyle(QStyleFactory::create("Windows"));
    //populateTree();

    setupFileToolbar();   // <-- СНАЧАЛА тулбар (пока виджеты ещё на месте)
    setupSplitter();   // <-- СНАЧАЛА сплиттер, потом всё остальное

    setupMenuBar();
    loadRecentFiles();
    updateRecentFileMenu();

}

MainWindow::~MainWindow()
{
    delete ui;
}

// Реализация слота
void MainWindow::on_pushButton_clicked()
{
    QString filter = tr("Файлы конфигураций 1С (*.cf);;");

    // Открываем диалог выбора файла
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Выберите файл"),               // заголовок
                                                    QDir::homePath(),                  // стартовая папка (домашняя)
                                                    filter);            // фильтр (можно задать свой)

    // Если пользователь выбрал файл (не отменил диалог)
    if (!fileName.isEmpty()) {
        qDebug() << "Выбран файл:" << fileName;

        // Например, можно вывести путь в строку состояния или в QLabel:
        ui->statusbar->showMessage(fileName);
        ui->lineEdit->setText(fileName);
        // или ui->label->setText(fileName);
    } else {
        qDebug() << "Выбор файла отменён";
    }
}

void MainWindow::setupMenuBar()
{
    // Создаём меню "Файл"
    fileMenu = menuBar()->addMenu(tr("&Файл"));

    // Пункт "Открыть..."
    openAction = new QAction(tr("&Открыть..."), this);
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::openFile);
    fileMenu->addAction(openAction);

    // Подменю "Недавние файлы"
    recentMenu = new QMenu(tr("Недавние файлы"), this);
    fileMenu->addMenu(recentMenu);

    // Разделитель
    fileMenu->addSeparator();

    // Пункт "Выход"
    exitAction = new QAction(tr("В&ыход"), this);
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    fileMenu->addAction(exitAction);

    // ============================================================
    //  Меню «Справка»
    // ============================================================
    helpMenu = menuBar()->addMenu(tr("&Справка"));

    // Пункт «О программе»
    aboutAction = new QAction(tr("О &программе..."), this);
    aboutAction->setShortcut(QKeySequence::HelpContents);
    connect(aboutAction, &QAction::triggered,
            this, &MainWindow::showAboutDialog);
    helpMenu->addAction(aboutAction);

    // Можно добавить «О Qt» — стандартный диалог
    helpMenu->addAction(tr("О &Qt..."), this, []() {
        QApplication::aboutQt();
    });
}

void MainWindow::openFile()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Выберите файл конфигурации"), QDir::homePath(), tr("Конфигурации 1С (*.cf);;Все файлы (*.*)"));

    if (filePath.isEmpty())
        return;

    // Здесь выполняется ваш парсинг / загрузка файла
    loadFile(filePath);  // ваша функция обработки  -------------- надо реализовать

    // Добавляем файл в историю
    addRecentFile(filePath);

    // Например, можно вывести путь в строку состояния или в QLabel:
    ui->statusbar->showMessage(filePath);
    ui->lineEdit->setText(filePath);

}

void MainWindow::addRecentFile(const QString &filePath)
{
    // Удаляем дубликат, если уже есть
    recentFiles.removeAll(filePath);
    // Добавляем в начало
    recentFiles.prepend(filePath);
    // Ограничиваем количество
    while (recentFiles.size() > maxRecentFiles)
        recentFiles.removeLast();

    saveRecentFiles();
    updateRecentFileMenu();
}

void MainWindow::updateRecentFileMenu()
{
    recentMenu->clear();

    if (recentFiles.isEmpty()) {
        QAction *emptyAction = new QAction(tr("(нет файлов)"), this);
        emptyAction->setEnabled(false);
        recentMenu->addAction(emptyAction);
        return;
    }

    for (const QString &filePath : recentFiles) {
        // Для красоты можно показывать только имя файла, а в tooltip – полный путь
        QFileInfo fileInfo(filePath);
        QString displayName = fileInfo.fileName();
        if (displayName.isEmpty())
            displayName = filePath;

        QAction *action = new QAction(displayName, this);
        action->setData(filePath);          // сохраняем полный путь
        action->setToolTip(filePath);

        // Подключаем слот с использованием лямбды, чтобы передать путь
        connect(action, &QAction::triggered, this, [this, filePath]() {
            openRecentFile(filePath);
        });

        recentMenu->addAction(action);
    }
}

void MainWindow::openRecentFile(const QString &filePath)
{
    // Проверяем, существует ли файл
    if (!QFile::exists(filePath)) {
        QMessageBox::warning(this, tr("Файл не найден"), tr("Файл '%1' больше не существует.\nУдаляем из списка.").arg(filePath));
        // Удаляем из списка
        recentFiles.removeAll(filePath);
        saveRecentFiles();
        updateRecentFileMenu();
        return;
    }

    // Загружаем файл
    loadFile(filePath);  //<---------------------------------------------------- надо реализовать

    // Перемещаем в начало списка (если загрузили успешно)
    addRecentFile(filePath);  // она обновит порядок
}

void MainWindow::loadRecentFiles()
{
    QStringList list = settings.value("recentFiles").toStringList();
    // Оставляем только существующие файлы (опционально)
    recentFiles.clear();
    for (const QString &file : list) {
        if (QFile::exists(file))
            recentFiles.append(file);
    }
    // Ограничиваем количество
    while (recentFiles.size() > maxRecentFiles)
        recentFiles.removeLast();
}

void MainWindow::saveRecentFiles()
{
    settings.setValue("recentFiles", recentFiles);
}

void MainWindow::loadFile(const QString &filePath)
{

    /*
    // Здесь ваш код для открытия и парсинга
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, tr("Ошибка"), tr("Не удалось открыть файл"));
        return;
    }

    // Возможно надо бы сделать глобальную???
    //*GlobalCF = std::make_unique<v8catalog>(filePath, true);

    QThread* thread = new QThread;
    CfParserWorker* worker = new CfParserWorker(filePath);
    worker->moveToThread(thread);

    // Соединяем сигнал от парсера со слотом в GUI
    // Qt::QueuedConnection автоматически обеспечит потокобезопасность
    connect(worker, &CfParserWorker::metadataItemParsed, this, &MainWindow::addMetadataItem, Qt::QueuedConnection);

    connect(thread, &QThread::started, worker, &CfParserWorker::process);
    connect(worker, &CfParserWorker::finished, thread, &QThread::quit);
        // ... очистка памяти ...

    thread->start();
    */
    // Закрываем предыдущую сессию, если была
    m_session.reset();

    m_workerThread = new QThread(this);
    m_worker = new CfParserWorker(filePath);
    m_worker->moveToThread(m_workerThread);

    // Когда воркер закончит — принимаем каталог и конфигурацию
    connect(m_worker, &CfParserWorker::parsingFinished, this, &MainWindow::onParsingFinished);

    connect(m_worker, &CfParserWorker::error, this,    [this](const QString& msg) {
                QMessageBox::critical(this, "Ошибка", msg);
            });

    connect(m_workerThread, &QThread::started,       m_worker, &CfParserWorker::process);
    connect(m_worker,       &CfParserWorker::parsingFinished, m_workerThread, &QThread::quit);
    connect(m_workerThread, &QThread::finished,      m_worker, &QObject::deleteLater);
    connect(m_workerThread, &QThread::finished,      m_workerThread, &QObject::deleteLater);

    m_workerThread->start();

}

void MainWindow::onParsingFinished(std::shared_ptr<v8catalog> catalog,
                                   std::shared_ptr<OneC::Metadata::Configuration> config)
{
    // Теперь каталог принадлежит GUI-потоку.
    m_session = std::make_unique<OneC::Metadata::ConfigurationSession>();
    m_session->setCatalog(std::move(catalog));
    m_session->setConfiguration(std::move(config));
    m_session->initializeRepository(/*cacheSize=*/300);

    // Строим дерево по дескрипторам
    populateTreeFromConfiguration(*m_session->configuration());

    statusBar()->showMessage(
        tr("Загружено объектов: %1")
            .arg(m_session->configuration()->totalObjectCount()),
        3000
    );
}

void MainWindow::onTreeItemExpanded(const QModelIndex &index)
{
    // Здесь позже можно лениво грузить реквизиты/формы и добавлять
    // их как дочерние узлы к выбранному объекту.
    Q_UNUSED(index);
}

void MainWindow::onTreeItemDoubleClicked(const QModelIndex &index)
{
    const QUuid uuid = index.data(Qt::UserRole).value<QUuid>();

    if (uuid.isNull())
        return;   // кликнули на категорию — ничего не делаем

    const auto* desc = m_session->configuration()->findByUuid(uuid);
    if (!desc) {
        qWarning() << "Дескриптор не найден:" << uuid;
        return;
    }

    /*
    // Ленивая загрузка полного объекта
    auto obj = m_session->loadObject(*desc);
    if (!obj) {
        QMessageBox::warning(this, tr("Ошибка"),
                             tr("Не удалось загрузить объект %1").arg(desc->name()));
        return;
    }



    // Показать свойства объекта — здесь ваша логика:
    // showObjectProperties(obj, *desc);
    qDebug() << "Загружен объект:" << desc->name()
             << "тип:" << obj->metadataType();
             */
}

void MainWindow::populateTree()
{
    m_model = new QStandardItemModel(this);
    m_model->setHorizontalHeaderLabels(QStringList() << "Имя");

    // Создаем корневой узел
    m_rootItem = new QStandardItem(QIcon(":/icons/folder.png"), "Конфигурация");
    m_model->appendRow(m_rootItem);

    // Лямбда-функция для быстрого создания категорий и сохранения их в карту
    auto addCategory = [this](const QString& name, QStandardItem* parent = nullptr) -> QStandardItem* {
        if (!parent) parent = m_rootItem;
        QStandardItem* item = new QStandardItem(name);
        parent->appendRow(item);
        m_categoryNodes.insert(name, item); // Сохраняем указатель по имени
        return item;
    };

    // ----- Создаем категории верхнего уровня -----
    QStandardItem* generalNode = addCategory("Общие");
    addCategory("Константы");
    addCategory("Справочники");
    addCategory("Документы");
    addCategory("Журналы документов");
    addCategory("Перечисления");
    addCategory("Отчеты");
    addCategory("Обработки");
    addCategory("Планы видов характеристик");
    addCategory("Планы счетов");
    addCategory("Планы видов расчета");
    addCategory("Регистры сведений");
    addCategory("Регистры накопления");
    addCategory("Регистры бухгалтерии");
    addCategory("Регистры расчета");
    addCategory("Бизнес-процессы");
    addCategory("Задачи");
    addCategory("Внешние источники данных");

    // ----- Добавляем подкатегории для "Общие" -----
    if (generalNode) {
        addCategory("Подсистемы", generalNode);
        addCategory("Общие модули", generalNode);
        addCategory("Параметры сеанса", generalNode);
        addCategory("Роли", generalNode);
        addCategory("Общие реквизиты", generalNode);
        addCategory("Планы обмена", generalNode);
        addCategory("Критерии отбора", generalNode);
        addCategory("Подписки на события", generalNode);
        addCategory("Регламентные задания", generalNode);
        addCategory("Функциональные опции", generalNode);
        addCategory("Параметры функциональных опций", generalNode);
        addCategory("Определяемые типы", generalNode);
        addCategory("Хранилища настроек", generalNode);
        addCategory("Общие формы", generalNode);
        addCategory("Общие команды", generalNode);
        addCategory("Группы команд", generalNode);
        addCategory("Интерфейсы", generalNode);
        addCategory("Общие макеты", generalNode);
        addCategory("Общие картинки", generalNode);
        addCategory("XDTO-пакеты", generalNode);
        addCategory("Web-сервисы", generalNode);
        addCategory("HTTP-сервисы", generalNode);
        addCategory("WS-ссылки", generalNode);
        addCategory("Сервисы интеграции", generalNode);
        addCategory("Элементы стиля", generalNode);
        addCategory("Стили", generalNode);
        addCategory("Языки", generalNode);
    }

    // Устанавливаем модель в QTreeView
    ui->treeView->setModel(m_model);
    ui->treeView->resizeColumnToContents(0);

}

void MainWindow::populateTreeFromConfiguration(const OneC::Metadata::Configuration &config)
{
    // 1. Создаём модель один раз
    m_model = new QStandardItemModel(this);
    m_model->setHorizontalHeaderLabels(QStringList() << tr("Имя"));

    // 2. Корневой узел
    QStandardItem* root = new QStandardItem(QIcon(":/icons/icons/brightness.png"), tr("Конфигурация"));
    root->setEditable(false);
    m_model->appendRow(root);

    // 3. Лямбда: создать категорию и запомнить её указатель
    auto addCategory = [this](const QString& key,
                              const QString& title,
                              QStandardItem* parent) -> QStandardItem* {
        QStandardItem* node = new QStandardItem(iconForCategory(key), title);
        node->setEditable(false);
        parent->appendRow(node);
        m_categoryNodes.insert(key, node);
        return node;
    };

    // 4. Создаём категории верхнего уровня
    QStandardItem* generalNode    = addCategory("Общие",               tr("Общие"),               root);
    addCategory("Константы",              tr("Константы"),              root);
    addCategory("Справочники",            tr("Справочники"),            root);
    addCategory("Документы",              tr("Документы"),              root);
    addCategory("ЖурналыДокументов",      tr("Журналы документов"),     root);
    addCategory("Перечисления",           tr("Перечисления"),           root);
    addCategory("Отчеты",                 tr("Отчеты"),                 root);
    addCategory("Обработки",              tr("Обработки"),              root);
    addCategory("ПланыВидовХарактеристик",tr("Планы видов характеристик"), root);
    addCategory("ПланыСчетов",            tr("Планы счетов"),           root);
    addCategory("ПланыВидовРасчета",      tr("Планы видов расчета"),    root);
    addCategory("РегистрыСведений",       tr("Регистры сведений"),      root);
    addCategory("РегистрыНакопления",     tr("Регистры накопления"),    root);
    addCategory("РегистрыБухгалтерии",    tr("Регистры бухгалтерии"),   root);
    addCategory("РегистрыРасчета",        tr("Регистры расчета"),       root);
    addCategory("БизнесПроцессы",         tr("Бизнес-процессы"),        root);
    addCategory("Задачи",                 tr("Задачи"),                 root);
    addCategory("ВнешниеИсточникиДанных", tr("Внешние источники данных"), root);

    // 5. Подкатегории "Общие"
    if (generalNode) {
        addCategory("Подсистемы",                tr("Подсистемы"),                generalNode);
        addCategory("ОбщиеМодули",               tr("Общие модули"),              generalNode);
        addCategory("ПараметрыСеанса",           tr("Параметры сеанса"),          generalNode);
        addCategory("Роли",                      tr("Роли"),                      generalNode);
        addCategory("ОбщиеРеквизиты",            tr("Общие реквизиты"),           generalNode);
        addCategory("ПланыОбмена",               tr("Планы обмена"),              generalNode);
        addCategory("КритерииОтбора",            tr("Критерии отбора"),           generalNode);
        addCategory("ПодпискиНаСобытия",         tr("Подписки на события"),       generalNode);
        addCategory("РегламентныеЗадания",       tr("Регламентные задания"),      generalNode);
        addCategory("ФункциональныеОпции",       tr("Функциональные опции"),      generalNode);
        addCategory("ПараметрыФункциональныхОпций", tr("Параметры функциональных опций"), generalNode);
        addCategory("ОпределяемыеТипы",          tr("Определяемые типы"),         generalNode);
        addCategory("ХранилищаНастроек",         tr("Хранилища настроек"),        generalNode);
        addCategory("ОбщиеФормы",                tr("Общие формы"),               generalNode);
        addCategory("ОбщиеКоманды",              tr("Общие команды"),             generalNode);
        addCategory("ГруппыКоманд",              tr("Группы команд"),             generalNode);
        addCategory("Интерфейсы",                tr("Интерфейсы"),                generalNode);
        addCategory("ОбщиеМакеты",               tr("Общие макеты"),              generalNode);
        addCategory("ОбщиеКартинки",             tr("Общие картинки"),            generalNode);
        addCategory("XDTOПакеты",                tr("XDTO-пакеты"),               generalNode);
        addCategory("WebСервисы",                tr("Web-сервисы"),               generalNode);
        addCategory("HTTPСервисы",               tr("HTTP-сервисы"),              generalNode);
        addCategory("WSСсылки",                  tr("WS-ссылки"),                 generalNode);
        addCategory("СервисыИнтеграции",         tr("Сервисы интеграции"),        generalNode);
        addCategory("ЭлементыСтиля",             tr("Элементы стиля"),            generalNode);
        addCategory("Стили",                     tr("Стили"),                     generalNode);
        addCategory("Языки",                     tr("Языки"),                     generalNode);
    }

    // 6. Лямбда: заполнить категорию дескрипторами
    auto fillCategory = [this](const QString& categoryKey,
                               const QList<OneC::Metadata::MetadataDescriptor>& descriptors) {
        auto it = m_categoryNodes.constFind(categoryKey);
        if (it == m_categoryNodes.constEnd()) {
            qWarning() << "Категория не найдена:" << categoryKey;
            return;
        }
        QStandardItem* parent = it.value();

        for (const auto& desc : descriptors) {
            auto* item = new QStandardItem(iconForType(desc.type()), desc.name());
            item->setEditable(false);

            // Сохраняем UUID и тип — пригодятся при клике
            item->setData(desc.uuid(), Qt::UserRole);        // UUID
            item->setData(desc.type(), Qt::UserRole + 1);    // тип

            // Тултип с синонимом и UUID
            QString tooltip = desc.name();
            if (!desc.synonym().isEmpty() && desc.synonym() != desc.name()) {
                tooltip += "\n" + desc.synonym();
            }
            tooltip += "\n" + desc.uuid().toString(QUuid::WithoutBraces);
            item->setToolTip(tooltip);

            parent->appendRow(item);
        }

        // Показать количество в заголовке категории
        if (!descriptors.isEmpty()) {
            parent->setText(QString("%1 (%2)")
                            .arg(parent->text().section(' ', 0, -1).section('(', 0, 0).trimmed())
                            .arg(descriptors.size()));
        }
    };

    // 7. Заполняем категории дескрипторами из Configuration
    fillCategory("Справочники",             config.catalogs());
    fillCategory("Документы",               config.documents());
    fillCategory("ЖурналыДокументов",       config.documentJournals());
    fillCategory("Перечисления",            config.enums());
    fillCategory("Отчеты",                  config.reports());
    fillCategory("Обработки",               config.dataProcessors());
    fillCategory("ПланыВидовХарактеристик", config.chartsOfCharacteristicTypes());
    fillCategory("ПланыСчетов",             config.chartsOfAccounts());
    fillCategory("ПланыВидовРасчета",       config.chartsOfCalculationTypes());
    fillCategory("РегистрыСведений",        config.informationRegisters());
    fillCategory("РегистрыНакопления",      config.accumulationRegisters());
    fillCategory("РегистрыБухгалтерии",     config.accountingRegisters());
    fillCategory("РегистрыРасчета",         config.calculationRegisters());
    fillCategory("БизнесПроцессы",          config.businessProcesses());
    fillCategory("Задачи",                  config.tasks());
    fillCategory("КритерииОтбора",          config.filterCriteria());
    fillCategory("ХранилищаНастроек",       config.settingsStorages());
    //fillCategory("ОбщиеМодули",             config.commonModules());
    // ... остальные — по аналогии с вашими геттерами ...

    // 8. Устанавливаем модель в QTreeView
    ui->treeView->setModel(m_model);
    ui->treeView->setUniformRowHeights(true);          // ускорение для больших деревьев
    ui->treeView->resizeColumnToContents(0);

    ui->treeView->setRootIsDecorated(true);   // линии для корневых
    ui->treeView->setIndentation(20);          // отступ между уровнями
    ui->treeView->setAnimated(false);          // быстрее для больших деревьев

    // 9. Подключаем обработчики (один раз!)
    connect(ui->treeView, &QTreeView::expanded, this, &MainWindow::onTreeItemExpanded);
    connect(ui->treeView, &QTreeView::doubleClicked, this, &MainWindow::onTreeItemDoubleClicked);

    // Колонка растягивается на всю ширину дерева — без серой полосы справа
    ui->treeView->header()->setStretchLastSection(true);
    ui->treeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);

    qDebug() << "Дерево построено. Всего объектов:" << config.totalObjectCount();
}

QIcon MainWindow::iconForType(const QString& metadataType) const
{
    auto* style = QApplication::style();

    // Справочники, документы, отчёты — «файлы и папки»
    if (metadataType == "Справочник")              return QIcon(":/icons/icons/abacus.png");
    if (metadataType == "Документ")             return QIcon(":/icons/icons/blue-document--pencil.png");
    if (metadataType == "ЖурналДокументов")      return style->standardIcon(QStyle::SP_FileDialogListView);
    if (metadataType == "Отчет")               return style->standardIcon(QStyle::SP_FileDialogDetailedView);
    if (metadataType == "Обработка")        return style->standardIcon(QStyle::SP_ComputerIcon);

    // Общие объекты — «служебные» иконки
    if (metadataType == "ОбщийМодуль")         return style->standardIcon(QStyle::SP_FileDialogInfoView);
    if (metadataType == "ОбщаяФорма")           return style->standardIcon(QStyle::SP_FileDialogContentsView);
    if (metadataType == "ОбщийМакет")       return style->standardIcon(QStyle::SP_FileDialogNewFolder);
    if (metadataType == "ОбщаяКоманда")        return style->standardIcon(QStyle::SP_CommandLink);
    if (metadataType == "ОбщаяКартинка")        return style->standardIcon(QStyle::SP_FileDialogStart);
    if (metadataType == "Роль")                 return style->standardIcon(QStyle::SP_DialogYesButton);
    if (metadataType == "Подсистема")            return style->standardIcon(QStyle::SP_DirOpenIcon);

    // Перечисления, константы, регистры
    if (metadataType == "Перечисление")                 return style->standardIcon(QStyle::SP_FileDialogListView);
    if (metadataType == "Константа")             return style->standardIcon(QStyle::SP_DialogSaveButton);
    if (metadataType == "РегистрСведений")  return style->standardIcon(QStyle::SP_DriveHDIcon);
    if (metadataType == "РегистрНакопления") return style->standardIcon(QStyle::SP_DriveFDIcon);
    if (metadataType == "РегистрБухгалтерии")   return style->standardIcon(QStyle::SP_DriveNetIcon);
    if (metadataType == "РегистрРасчета")  return style->standardIcon(QStyle::SP_DriveDVDIcon);

    // Планы, бизнес-процессы, задачи
    if (metadataType == "ПланВидовХарактеристик") return style->standardIcon(QStyle::SP_FileDialogBack);
    if (metadataType == "ПланСчетов")            return style->standardIcon(QStyle::SP_FileDialogToParent);
    if (metadataType == "ПланВидовРасчета")    return style->standardIcon(QStyle::SP_ArrowForward);
    if (metadataType == "ПланОбмена")               return style->standardIcon(QStyle::SP_BrowserReload);
    if (metadataType == "БизнесПроцесс")            return style->standardIcon(QStyle::SP_MediaPlay);
    if (metadataType == "Задача")                       return style->standardIcon(QStyle::SP_MessageBoxQuestion);

    // Внешние источники, сервисы
    if (metadataType == "ВнешнийИсточникДанных")   return style->standardIcon(QStyle::SP_DriveNetIcon);
    if (metadataType == "WebService")           return style->standardIcon(QStyle::SP_DriveNetIcon);
    if (metadataType == "HTTPService")          return style->standardIcon(QStyle::SP_DriveNetIcon);
    if (metadataType == "СервисИнтеграции")   return style->standardIcon(QStyle::SP_DriveNetIcon);

    // Иконка по умолчанию
    return style->standardIcon(QStyle::SP_FileIcon);
}



// Этот метод будет вызываться по сигналу от парсера
void MainWindow::addMetadataItem(const QString& categoryName, const QString& itemName, const QString& uuid)
{
    // Ищем нужную ветку в нашей карте
    if (m_categoryNodes.contains(categoryName)) {
        QStandardItem* parentNode = m_categoryNodes.value(categoryName);

        // Создаем новый элемент
        QStandardItem* newItem = new QStandardItem(itemName);

        // Сохраняем UUID в данных элемента (пригодится при клике)
        newItem->setData(uuid, Qt::UserRole);

        // Можно добавить иконку в зависимости от типа
        // newItem->setIcon(QIcon(":/icons/document.png"));

        parentNode->appendRow(newItem);
    } else {
        qWarning() << "Категория не найдена:" << categoryName;
    }
}

// Процедура заполняет метаданные по корневому гуиду
void fill_md(tree* tr, QString guid_md, v8catalog *cf)
{
    v8file *filedata;
    tree* tree_md;
    tree* node;

    QString s;

    //v8catalog *cf = MainForm->GlobalCF.get();
    //msreg->AddMessage(L"fill_md: Начало обработки GUID: " + guid_md, MessageState::msInfo);

    // Карта путей для извлечения имен
    std::unordered_map<QString, std::vector<int>> namePaths = {
        {GUID_Catalogs,          {0,1,9,1,2}},
        {GUID_Languages,         {0,1,1,2}},
        {GUID_CommonModules,     {0,1,1,2}},
        {GUID_Roles,             {0,1,1,2}},
        {GUID_CommonTemplates,   {0,1,1,2}},
        {GUID_HTTPServices,      {0,1,2,2}},
        {GUID_ScheduledJobs,     {0,1,1,2}},
        {GUID_CommonAttributes,  {0,1,1,1,1,2}},
        {GUID_SessionParameters, {0,1,1,1,2}},
        {GUID_FunctionalOptionsParameters, {0,1,1,2}},
        {GUID_Subsystems,           {0,1,1,2}},
        {GUID_Interfaces,           {0,1,2,2}},
        {GUID_Styles,               {0,1,1,2}},
        {GUID_FilterCriteria,       {0,1,5,1,2}},
        {GUID_SettingsStorages,     {0,1,1,1,2}},
        {GUID_EventSubscriptions,   {0,1,1,2}},
        {GUID_StyleItems,           {0,1,3,2}},
        {GUID_CommonPictures,       {0,1,1,2}},
        {GUID_ExchangePlans,        {0,1,12,2}},
        {GUID_WebServices,          {0,1,2,2}},
        {GUID_WSReferences,         {1,2,2}},
        {GUID_WebSocketClients,     {0,1,1,2}},
        {GUID_IntegrationServices,  {0,1,1,2}},
        {GUID_FunctionalOptions,    {0,1,1,2}},
        {GUID_DefinedTypes,         {0,1,3,2}},
        {GUID_XDTOPackages,         {0,1,1,2}},
        {GUID_Constants,            {0,1,1,1,1,2}},
        {GUID_Documents,            {0,1,9,1,2}},
        {GUID_InformationRegisters, {0,1,15,1,2}},
        {GUID_CalculationRegisters, {0,1,15,1,2}},
        {GUID_BusinessProcesses,    {0,1,1,2}},
        {GUID_Tasks,                {0,1,1,2}},
        {GUID_AccountingRegisters,  {0,1,16,1,2}},
        {GUID_CommandGroups,        {0,1,6,2}},
        {GUID_CommonCommands,       {0,1,1,2,9,2}},
        {GUID_CommonForms,         {0,1,1,1,2}},
        {GUID_Numerators,           {0,1,1,2}},
        {GUID_JournDocuments,       {0,1,3,1,2}},
        {GUID_Reports,              {0,1,3,1,2}},
        {GUID_ChartOfCharacteristicTypes, {0,1,13,1,2}},
        {GUID_ChartsOfAccounts,           {0,1,15,1,2}},
        {GUID_ChartsOfCalculationTypes,   {0,1,1,1,2}},
        {GUID_AccumulationRegisters,      {0,1,13,1,2}},
        {GUID_Sequences,                  {0,1,7,1,2}},
        {GUID_ExternalDataSources,        {1,1,1,2}},
        {GUID_DataProcessors,             {0,1,3,1,2}},
        {GUID_Enums,                      {0,1,5,1,2}},
        {GUID_DefinedTypes,               {0,1,3,2}},
        {GUID_Bots,                       {0,1,1,2}}
    };

    auto pathIt = namePaths.find(guid_md);

    auto tryGetNodeByPath = [](tree* startNode, const std::vector<int>& candidatePath) -> tree*
    {
        tree* currentNode = startNode;
        if (!currentNode)
            return nullptr;

        for (size_t i = 0; i < candidatePath.size(); i++)
        {
            int idx = candidatePath[i];
            if (idx < 0 || idx >= currentNode->get_num_subnode())
                return nullptr;

            currentNode = currentNode->get_subnode(idx);
            if (!currentNode)
                return nullptr;
        }

        return currentNode;
    };

    if (pathIt == namePaths.end()) {
        // GUID не найден в карте путей, пропустить
        return;
    }

    tree* node_md = find_metadata_node_by_guid(tr, guid_md);
    if (!node_md) {
        return; // Защита от nullptr
    }

    const std::vector<int>& path = pathIt->second;

    tree* nextNode = node_md->get_next();
    if (!nextNode) {
        return;
    }

    //int CountMD = std::stoi(nextNode->get_value());
    int CountMD = nextNode->get_value().toInt();

    tree* curNode = node_md->get_next();

    int processedCount = 0;

    while (curNode)
    {
        curNode = curNode->get_next();
        if (curNode)
        {
            processedCount++;
            QString curNodeValue = curNode->get_value();

            filedata = cf->GetFile(curNodeValue);

            if(!filedata)
            {
                continue; // Продолжить с следующим
            }

            try
            {
                tree_md = get_treeFromV8file(filedata);
            }
            catch (...)
            {
                continue;
            }
            if(!tree_md)
            {
                continue;
            }

            node = nullptr;

            try {
                std::vector<std::vector<int>> candidatePaths = {path};

                if (guid_md == GUID_BusinessProcesses || guid_md == GUID_Tasks)
                {
                    candidatePaths.push_back({0,1,1});
                    candidatePaths.push_back({0,1,2});
                }
                else if (guid_md == GUID_ExternalDataSources)
                {
                    candidatePaths.push_back({1,0,1,2});
                    candidatePaths.push_back({0,1,1,2});
                }

                for (size_t candidateIndex = 0; candidateIndex < candidatePaths.size() && !node; candidateIndex++)
                {
                    const std::vector<int>& candidatePath = candidatePaths[candidateIndex];
                    node = tryGetNodeByPath(tree_md, candidatePath);
                }
            }
            catch (...) {
                delete tree_md;
                continue;
            }

            if (!node) {
                continue;
            }

            // Проверка типа узла перед получением значения
            if (node->get_type() == node_type::nd_empty || node->get_type() == node_type::nd_unknown) {
                delete tree_md;
                continue;
            }

            QString val = node->get_value();

            if (val.length() == 0) {
                if (guid_md == GUID_ExternalDataSources)
                {
                    val = curNodeValue;
                }
                else
                {
                    delete tree_md;
                    continue;
                }
            }

            // Создание объектов для специфических типов
            try {
                if (guid_md == GUID_Catalogs)
                {

                    //MainForm->mdCatalogs.push_back(std::make_unique<TCatalogs>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_Languages)
                {
//					msreg->AddMessage(L"fill_md: Создание языка: " + val, MessageState::msInfo);
//					MainForm->mdLanguages.push_back(std::make_unique<TLangs>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_CommonModules)
                {
//					msreg->AddMessage(L"fill_md: Создание общего модуля: " + val, MessageState::msInfo);
//					MainForm->mdCommonModules.push_back(std::make_unique<TCommonModules>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_Roles)
                {
//					msreg->AddMessage(L"fill_md: Создание роли: " + val, MessageState::msInfo);
//					MainForm->mdRoles.push_back(std::make_unique<TRoles>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_CommonTemplates)
                {
//					msreg->AddMessage(L"fill_md: Создание общего макета: " + val, MessageState::msInfo);
//					MainForm->mdCommonTemplates.push_back(std::make_unique<TCommonTemplates>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_HTTPServices)
                {
//					msreg->AddMessage(L"fill_md: Создание HTTP-сервиса: " + val, MessageState::msInfo);
//					MainForm->mdHTTPServices.push_back(std::make_unique<THTTPServices>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_ScheduledJobs)
                {
//					msreg->AddMessage(L"fill_md: Создание регламентного задания: " + val, MessageState::msInfo);
//					MainForm->mdScheduledJobs.push_back(std::make_unique<TScheduledJobs>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_CommonAttributes)
                {
//					msreg->AddMessage(L"fill_md: Создание общего реквизита: " + val, MessageState::msInfo);
//					MainForm->mdCommonAttributes.push_back(std::make_unique<TCommonAttributes>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_SessionParameters)
                {
//					msreg->AddMessage(L"fill_md: Создание параметра сеанса: " + val, MessageState::msInfo);
//					MainForm->mdSessionParameters.push_back(std::make_unique<TSessionParameters>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_FunctionalOptionsParameters)
                {
//					msreg->AddMessage(L"fill_md: Создание параметра функциональной опции: " + val, MessageState::msInfo);
//					MainForm->mdFunctionalOptionsParameters.push_back(std::make_unique<TFunctionalOptionsParameters>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_Subsystems)
                {
                    //msreg->AddMessage(L"fill_md: Пропуск подсистемы: " + val, MessageState::msInfo);
//					msreg->AddMessage(L"fill_md: Создание подсистемы: " + val, MessageState::msInfo);
//					MainForm->mdSubsystems.push_back(std::make_unique<TSubsystem>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_Interfaces)
                {
//					msreg->AddMessage(L"fill_md: Создание интерфейса: " + val, MessageState::msInfo);
//					MainForm->mdInterfaces.push_back(std::make_unique<TInterfaces>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_Styles)
                {
//					msreg->AddMessage(L"fill_md: Создание стиля: " + val, MessageState::msInfo);
//					MainForm->mdStyles.push_back(std::make_unique<TStyles>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_FilterCriteria)
                {
//					msreg->AddMessage(L"fill_md: Создание критерия отбора: " + val, MessageState::msInfo);
//					MainForm->mdFilterCriteria.push_back(std::make_unique<TFilterCriteria>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_SettingsStorages)
                {
//					msreg->AddMessage(L"fill_md: Создание хранилища настроек: " + val, MessageState::msInfo);
//					MainForm->mdSettingsStorages.push_back(std::make_unique<TSettingsStorages>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_StyleItems)
                {
//					msreg->AddMessage(L"fill_md: Создание элемента стиля: " + val, MessageState::msInfo);
//					MainForm->mdStyleItems.push_back(std::make_unique<TStyleItems>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_CommonPictures)
                {
//					msreg->AddMessage(L"fill_md: Создание общей картинки: " + val, MessageState::msInfo);
//					MainForm->mdCommonPictures.push_back(std::make_unique<TCommonPictures>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_ExchangePlans)
                {
//					msreg->AddMessage(L"fill_md: Создание плана обмена: " + val, MessageState::msInfo);
//					MainForm->mdExchangePlans.push_back(std::make_unique<TExchangePlans>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_EventSubscriptions)
                {
//					msreg->AddMessage(L"fill_md: Создание подписки на событие: " + val, MessageState::msInfo);
//					MainForm->mdEventSubscriptions.push_back(std::make_unique<TEventSubscriptions>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_WebServices)
                {
//					msreg->AddMessage(L"fill_md: Создание веб-сервиса: " + val, MessageState::msInfo);
//					MainForm->mdWebServices.push_back(std::make_unique<TWebServices>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_FunctionalOptions)
                {
//					msreg->AddMessage(L"fill_md: Создание функциональной опции: " + val, MessageState::msInfo);
//					MainForm->mdFunctionalOptions.push_back(std::make_unique<TFunctionalOptions>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_DefinedTypes)
                {
//					msreg->AddMessage(L"fill_md: Создание определяемого типа: " + val, MessageState::msInfo);
//					MainForm->mdDefinedTypes.push_back(std::make_unique<TDefinedTypes>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_XDTOPackages)
                {
//					msreg->AddMessage(L"fill_md: Создание XDTO-пакета: " + val, MessageState::msInfo);
//					MainForm->mdXDTOPackages.push_back(std::make_unique<TXDTOPackages>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_WSReferences)
                {
//					msreg->AddMessage(L"fill_md: Создание WS-ссылки: " + val, MessageState::msInfo);
//					MainForm->mdWSReferences.push_back(std::make_unique<TWSReferences>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_WebSocketClients)
                {
//					msreg->AddMessage(L"fill_md: Создание WebSocket-клиента: " + val, MessageState::msInfo);
//					MainForm->mdWebSocketClients.push_back(std::make_unique<TWebSocketClients>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_IntegrationServices)
                {
//					msreg->AddMessage(L"fill_md: Создание сервиса интеграции: " + val, MessageState::msInfo);
//					MainForm->mdIntegrationServices.push_back(std::make_unique<TIntegrationServices>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_Constants)
                {
//					msreg->AddMessage(L"fill_md: Создание константы: " + val, MessageState::msInfo);
//					MainForm->mdConstants.push_back(std::make_unique<TConstants>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_Documents)
                {
//					msreg->AddMessage(L"fill_md: Создание документа: " + val, MessageState::msInfo);
//					MainForm->mdDocuments.push_back(std::make_unique<TDocuments>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_CommonForms)
                {
//					msreg->AddMessage(L"fill_md: Создание общей формы: " + val, MessageState::msInfo);
//					MainForm->mdCommonForms.push_back(std::make_unique<TCommonForms>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_InformationRegisters)
                {
//					msreg->AddMessage(L"fill_md: Создание регистра сведений: " + val, MessageState::msInfo);
//					MainForm->mdInformationRegisters.push_back(std::make_unique<TInformationRegisters>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_CalculationRegisters)
                {
//					msreg->AddMessage(L"fill_md: Создание регистра расчета: " + val, MessageState::msInfo);
//					MainForm->mdCalculationRegisters.push_back(std::make_unique<TCalculationRegisters>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_BusinessProcesses)
                {
//					msreg->AddMessage(L"fill_md: Создание бизнес-процесса: " + val, MessageState::msInfo);
//					MainForm->mdBusinessProcesses.push_back(std::make_unique<TBusinessProceses>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_Tasks)
                {
//					msreg->AddMessage(L"fill_md: Создание задачи: " + val, MessageState::msInfo);
//					MainForm->mdTasks.push_back(std::make_unique<TTasks>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_AccountingRegisters)
                {
//					msreg->AddMessage(L"fill_md: Создание регистра бухгалтерии: " + val, MessageState::msInfo);
//					MainForm->mdAccountingRegisters.push_back(std::make_unique<TAccountingRegisters>(cf, curNode->get_value(), val));
//					msreg->AddMessage(L"fill_md: Регистр бухгалтерии создан успешно: " + val, MessageState::msInfo);
                }
                else if (guid_md == GUID_CommandGroups)
                {
//					msreg->AddMessage(L"fill_md: Создание группы команд: " + val, MessageState::msInfo);
//					MainForm->mdCommandGroups.push_back(std::make_unique<TCommandGroups>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_CommonCommands)
                {
//					msreg->AddMessage(L"fill_md: Создание общей команды: " + val, MessageState::msInfo);
//					MainForm->mdCommonCommands.push_back(std::make_unique<TCommonCommands>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_Numerators)
                {
//					msreg->AddMessage(L"fill_md: Создание нумератора: " + val, MessageState::msInfo);
//					MainForm->mdDocumentNumerators.push_back(std::make_unique<TNumerators>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_JournDocuments)
                {
//					msreg->AddMessage(L"fill_md: Создание журнала документов: " + val, MessageState::msInfo);
//					MainForm->mdDocumentJournals.push_back(std::make_unique<TJournals>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_Reports)
                {
//					msreg->AddMessage(L"fill_md: Создание отчета: " + val, MessageState::msInfo);
//					MainForm->mdReports.push_back(std::make_unique<TReports>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_ChartOfCharacteristicTypes)
                {
//					msreg->AddMessage(L"fill_md: Создание ПВХ: " + val, MessageState::msInfo);
//					MainForm->mdChartsOfCharacteristicTypes.push_back(std::make_unique<TChartOfCharacteristicTypes>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_ChartsOfAccounts)
                {
//					msreg->AddMessage(L"fill_md: Создание плана счетов: " + val, MessageState::msInfo);
//					MainForm->mdChartOfAccounts.push_back(std::make_unique<TChartOfAccounts>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_ChartsOfCalculationTypes)
                {
//					msreg->AddMessage(L"fill_md: Создание ПВР: " + val, MessageState::msInfo);
//					MainForm->mdChartOfCalculationTypes.push_back(std::make_unique<TChartOfCalculationTypes>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_AccumulationRegisters)
                {
//					msreg->AddMessage(L"fill_md: Создание регистра накопления: " + val, MessageState::msInfo);
//					MainForm->mdAccumulationRegisters.push_back(std::make_unique<TAccumulationRegisters>(cf, curNode->get_value(), val));
//					msreg->AddMessage(L"fill_md: Регистр накопления создан успешно: " + val, MessageState::msInfo);
                }
                else if (guid_md == GUID_Sequences)
                {
//					msreg->AddMessage(L"fill_md: Создание последовательности: " + val, MessageState::msInfo);
//					MainForm->mdSequences.push_back(std::make_unique<TSequences>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_DataProcessors)
                {
//					msreg->AddMessage(L"fill_md: Создание обработки: " + val, MessageState::msInfo);
//					MainForm->mdDataProcessors.push_back(std::make_unique<TDataProcessors>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_Enums)
                {
//					msreg->AddMessage(L"fill_md: Создание перечисления: " + val, MessageState::msInfo);
//					MainForm->mdEnums.push_back(std::make_unique<TEnums>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_Bots)
                {
//					msreg->AddMessage(L"fill_md: Создание бота: " + val, MessageState::msInfo);
//					MainForm->mdBots.push_back(std::make_unique<TBots>(cf, curNode->get_value(), val));
                }
                else if (guid_md == GUID_ExternalDataSources)
                {
//					msreg->AddMessage(L"fill_md: Создание внешнего источника данных: " + val, MessageState::msInfo);
//					MainForm->mdExternalDataSources.push_back(std::make_unique<TExternalDataSources>(cf, curNode->get_value(), val));
                }
                else
                {
//					msreg->AddMessage(L"fill_md: Неизвестный GUID для объекта: " + val, MessageState::msWarning);
                }
            }

            catch (...) {
//				msreg->AddMessage_(L"fill_md: неизвестное исключение при создании объекта", msError,
//						L"Name", val,
//						L"GUID", guid_md,
//						L"File", curNodeValue);
//				LogHeapStatus(L"fill_md: память при неизвестном exception создания объекта", guid_md, curNodeValue, processedCount, CountMD);
            }
            delete tree_md;
        }
    }


}



void get_cf_name(v8catalog* cf)
{
    std::unique_ptr<tree> tr;
    tree* node;
    v8catalog* cat;
    v8file* filedata;
    QString meta;
    QString s;
    int ver;

    if(!cf)
    {
        return;
    }

    filedata = cf->GetFile("version");
    if(!filedata)
    {
        return;
    }

    tr.reset(get_treeFromV8file(filedata));
    if(!tr)
    {
        return;
    }

    node = tr.get();

    node = &(*node)[0][0][0];
    if(node->get_type() != node_type::nd_number)
    {
        return;
    }

    //ver = std::stoi(node->get_value());
    ver = node->get_value().toInt();
    tr.reset();

    if(ver < 100)
    {
        // 8.0
        filedata = cf->GetFile("metadata");
        if(!filedata)
        {
            return;
        }

        cat = filedata->GetCatalog();
        if(!cat)
        {
            return;
        }

    }
    else
    {
        //8.1 или 8.2
        cat = cf;
    }

    filedata = cat->GetFile("root");
    if(!filedata)
    {
        return;
    }

    tr.reset(get_treeFromV8file(filedata));
    if(!tr)
    {
        return;
    }

    node = tr.get();
    node = &(*node)[0][1];

    if(node->get_type() != node_type::nd_guid)
    {
        return;
    }

    meta = node->get_value();
    tr.reset();

    filedata = cat->GetFile(meta);
    if(!filedata)
    {
        return;
    }

    tr.reset(get_treeFromV8file(filedata));
    if(!tr)
    {
        return;
    }

    get_cf_name(tr.release(), cf);
}



void get_cf_name(tree* tr, v8catalog *cf)
{
    int j, k;
    tree* node;
    tree* node2;
    tree* node3;
    tree* node_Catalogs;
    tree* node_md;
    tree* curNode;
    QString cf_synonym;
    QString cf_version;
    QString s;
    bool ok;
    int structver;
    int offset_name;
    int offset_ver;
    int CountCatalogs;

    node = tr;
    node = &(*node)[0][3][1][1];

    node3 = tr;

    // Заполняем справочники

    fill_md(tr, GUID_Catalogs, cf);


    structver = (*node)[0].get_value().toInt();

    if(structver <= 15)
    {
        offset_name = 1;
        offset_ver = 12;
    }
    else
    {
        offset_name = 1;
        offset_ver = 15;
    }


    node2 = &(*node)[offset_name][1][3];

    if(node2->get_type() != node_type::nd_list)
    {

        delete tr;
        return;
    }
    if((*node2)[0].get_type() != node_type::nd_number)
    {
        delete tr;
        return;
    }

    j = (*node2)[0].get_value().toInt();

    if(j == 0)
        cf_synonym = (*node)[offset_name][1][2].get_value();
    else
    {
        ok = false;
        for(k = 0; k < j; k++)
        {
            //if((*node2)[k * 2 + 1].get_value().CompareIC(L"ru") == 0)
            if (QString::compare((*node2)[k * 2 + 1].get_value(), QStringLiteral("ru"), Qt::CaseInsensitive) == 0)
            {
                cf_synonym = (*node2)[k * 2 + 2].get_value();
                ok = true;
                break;
            }
        }

        if(!ok)
            cf_synonym = (*node2)[2].get_value();
    }

    cf_version = (*node)[offset_ver].get_value();

    delete tr;

    //mess->AddMessage(cf_synonym + " (" + cf_version + ")", msEmpty);
    //MainForm->ConfigName = cf_synonym + " (" + cf_version + ")";
    //ConfigName = cf_synonym + " (" + cf_version + ")";
    //	if (mess && mess->getUiMessagesEnabled()) mess->AddMessage("Прочитана конфигурация: " + cf_synonym + " (" + cf_version + ")", msInfo);


}

// Определяем кодировку по BOM и первым байтам.
// Возвращает:
//   offset — сколько байт BOM надо пропустить (0 — BOM нет)
//   codec  — выбранный декодер
static int detectEncoding(const QByteArray& data, QStringConverter::Encoding& enc)
{
    if (data.size() >= 3 &&
        (uchar)data[0] == 0xEF &&
        (uchar)data[1] == 0xBB &&
        (uchar)data[2] == 0xBF)
    {
        enc = QStringConverter::Utf8;
        return 3;
    }

    if (data.size() >= 2)
    {
        const uchar b0 = (uchar)data[0];
        const uchar b1 = (uchar)data[1];
        if (b0 == 0xFF && b1 == 0xFE) { enc = QStringConverter::Utf16LE; return 2; }
        if (b0 == 0xFE && b1 == 0xFF) { enc = QStringConverter::Utf16BE; return 2; }
    }

    // Эвристика: попробуем UTF-8 строгим декодером
    if (!data.isEmpty())
    {
        QStringDecoder test(QStringDecoder::Utf8, QStringDecoder::Flag::Stateless);
        QString s = test(data);
        if (!test.hasError()) {
            enc = QStringConverter::Utf8;
            return 0;
        }
    }

    // Иначе — Latin1 (аналог ANSI в 1С-контейнерах)
    enc = QStringConverter::Latin1;
    return 0;
}


// Предполагается, что класс v8file имеет методы:
// - std::string GetFullName() const;   // возвращает путь к файлу
// - (методы GetFileLength и Read больше не используются)
// Функция error() определена где-то в другом месте.

//---------------------------------------------------------------------------
tree* get_treeFromV8file(v8file* f)
{

    //TBytes sourceBytes;
    QByteArray sourceBytes;


    if (!f)
        return nullptr;

    const qint64 fileSize = f->GetFileLength();
    if (fileSize <= 0)
    {
        //error(QStringLiteral("Пустой файл контейнера"), QStringLiteral("Файл"), f->GetFullName());
        return nullptr;
    }

    sourceBytes.resize(fileSize);

    if (f->Read(sourceBytes, 0, fileSize) != fileSize)
    {
        //error(QStringLiteral("Ошибка чтения файла контейнера"), QStringLiteral("Файл"), f->GetFullName());
        return nullptr;
    }


    // Определяем кодировку
    QStringConverter::Encoding enc;
    const int off = detectEncoding(sourceBytes, enc);
    if (off < 0 || off > sourceBytes.size())
    {
        //error(QStringLiteral("Ошибка определения кодировки файла контейнера"), QStringLiteral("Файл"), f->GetFullName());
        return nullptr;
    }

    // Конвертируем в Unicode
    const QByteArray payload = sourceBytes.mid(off);

    QStringDecoder decoder(enc, QStringDecoder::Flag::Stateless);
    QString unicodeText = decoder(payload);

    if (decoder.hasError())
    {
        //error(QStringLiteral("Ошибка конвертации файла контейнера в Unicode"), QStringLiteral("Файл"), f->GetFullName());
        return nullptr;
    }

    if (unicodeText.isEmpty())
    {
        //error(QStringLiteral("Ошибка конвертации файла контейнера в Unicode"), QStringLiteral("Файл"), f->GetFullName());
        return nullptr;
    }

    // Если файл открывался через поток v8file — позиция уже не нужна
    tree* rt = parse_1Ctext(unicodeText, f->GetFullName());
    return rt;
}

tree* _get_treeFromV8file(v8file* f)
{
    /*


    // 1. Получаем имя файла (уже не VCL-строка)
    std::wstring fileName = f->GetFullName();

    // 2. Открываем файл в бинарном режиме
    std::ifstream file(fileName, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return NULL;
    }

    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    if (fileSize <= 0) {
        return NULL;
    }

    // 3. Читаем всё содержимое в vector<unsigned char>
    std::vector<unsigned char> sourceBytes(static_cast<size_t>(fileSize));
    if (!file.read(reinterpret_cast<char*>(sourceBytes.data()), fileSize)) {
        return NULL;
    }

    // 4. Определяем кодировку по BOM
    enum Encoding { UTF8, UTF16LE, UTF16BE, Unknown };
    Encoding enc = Unknown;
    int off = 0;  // смещение после BOM

    if (fileSize >= 2) {
        if (sourceBytes[0] == 0xFE && sourceBytes[1] == 0xFF) {
            enc = UTF16BE;
            off = 2;
        } else if (sourceBytes[0] == 0xFF && sourceBytes[1] == 0xFE) {
            enc = UTF16LE;
            off = 2;
        } else if (fileSize >= 3 &&
                   sourceBytes[0] == 0xEF &&
                   sourceBytes[1] == 0xBB &&
                   sourceBytes[2] == 0xBF) {
            enc = UTF8;
            off = 3;
        }
    }

    if (enc == Unknown) {
        return NULL;
    }

    // 5. Конвертируем данные (после BOM) в UTF-16 (std::wstring)
    const unsigned char* data = sourceBytes.data() + off;
    size_t dataSize = static_cast<size_t>(fileSize - off);

    std::wstring unicodeText;

    if (enc == UTF8) {
        // Преобразование UTF-8 → UTF-16 через стандартный конвертер
        // (std::wstring_convert объявлен deprecated в C++17, но в учебных целях допустим)
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        try {
            unicodeText = converter.from_bytes(
                reinterpret_cast<const char*>(data),
                reinterpret_cast<const char*>(data + dataSize)
            );
        } catch (...) {
            return NULL;
        }
    } else if (enc == UTF16LE || enc == UTF16BE) {
        // Проверка чётности размера
        if (dataSize % 2 != 0) {
            return NULL;
        }
        size_t numChars = dataSize / 2;
        unicodeText.resize(numChars);

        if (enc == UTF16LE) {
            // Для little-endian платформ (x86) просто копируем
            // (если нужна переносимость, можно проверить порядок байт в системе)
            std::copy(
                reinterpret_cast<const wchar_t*>(data),
                reinterpret_cast<const wchar_t*>(data + dataSize),
                unicodeText.begin()
            );
        } else { // UTF16BE – меняем порядок байт
            for (size_t i = 0; i < numChars; ++i) {
                unsigned char low  = data[2*i];
                unsigned char high = data[2*i + 1];
                unicodeText[i] = (static_cast<wchar_t>(high) << 8) | low;
            }
        }
    }

    if (unicodeText.empty()) {
        return NULL;
    }

    // 6. Вызов функции разбора (она должна быть адаптирована под std::wstring)
    tree* rt = parse_1Ctext(unicodeText, fileName.c_str());
    */
    tree* rt = parse_1Ctext("", "");
    return rt;
}


void MainWindow::setupSplitter()
{
    // ---- 1. Находим существующие виджеты ----
    QWidget* treeWidget  = ui->treeView;
    QWidget* tabWidget   = ui->tabWidget;   // ← ЗАМЕНИТЕ на реальное имя из .ui

    if (!treeWidget || !tabWidget) {
        qWarning() << "Не найдены treeView или tabWidget в .ui";
        return;
    }

    // ---- 2. Определяем layout, где лежит treeView ----
    QWidget* treeParent = treeWidget->parentWidget();
    QLayout* treeLayout = treeParent ? treeParent->layout() : nullptr;

    // ---- 3. Отвязываем оба виджета от их текущих layout ----
    if (treeLayout) {
        treeLayout->removeWidget(treeWidget);
    }

    if (QWidget* tabParent = tabWidget->parentWidget()) {
        if (QLayout* tabLayout = tabParent->layout()) {
            tabLayout->removeWidget(tabWidget);
        }
    }

    // ---- Создаём QSplitter ----
    m_splitter = new QSplitter(Qt::Horizontal, this);
    m_splitter->setChildrenCollapsible(false);
    m_splitter->setHandleWidth(6);                    // шире — легче попасть мышью
    m_splitter->setOpaqueResize(true);

    // Стилизуем ручку, чтобы она была явно видна и подсвечивалась при наведении
    m_splitter->setStyleSheet(QStringLiteral(
        "QSplitter::handle:horizontal {"
        "    background-color: #d0d0d0;"
        "    border-left: 1px solid #a0a0a0;"
        "    border-right: 1px solid #a0a0a0;"
        "}"
        "QSplitter::handle:horizontal:hover {"
        "    background-color: #a8c8e8;"                 // подсветка при наведении
        "}"
    ));

    // ---- 5. Левая панель — treeView ----
    treeWidget->setParent(m_splitter);
    treeWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    treeWidget->setMinimumWidth(150);
    treeWidget->setMaximumWidth(QWIDGETSIZE_MAX);   // сбросить старый лимит, если был
    m_splitter->addWidget(treeWidget);

    // ---- 6. Правая панель — QTabWidget ----
    m_rightPanel = tabWidget;
    m_rightPanel->setParent(m_splitter);
    m_rightPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_rightPanel->setMinimumWidth(200);
    m_splitter->addWidget(m_rightPanel);



    // ---- 7. Пропорции ----
    m_splitter->setStretchFactor(0, 0);   // дерево не растягивается
    m_splitter->setStretchFactor(1, 1);   // правая панель растягивается

    // ---- 8. Куда вставлять сплиттер в основном layout? ----
    //     Логично поставить его на место, где раньше было дерево.
    //     Если treeLayout существует и это QVBox/QHBox — вставляем
    //     на освободившееся место. Если layout не найден — добавляем
    //     сплиттер в centralWidget.
    if (treeLayout) {
        if (auto* vbox = qobject_cast<QVBoxLayout*>(treeLayout)) {
            vbox->insertWidget(0, m_splitter, 1);
        } else if (auto* hbox = qobject_cast<QHBoxLayout*>(treeLayout)) {
            hbox->insertWidget(0, m_splitter, 1);
        } else {
            treeLayout->addWidget(m_splitter);
        }
    } else if (auto* central = centralWidget()) {
        auto* vbox = new QVBoxLayout(central);
        vbox->addWidget(m_splitter, 1);
    }

    // ---- 9. Подключаем сигнал ручки сплиттера ----
    connect(m_splitter, &QSplitter::splitterMoved,
            this, &MainWindow::on_splitterMoved);

    // ---- 10. Отложенная установка размеров ----
    QTimer::singleShot(0, this, [this]() {
        const int total = m_splitter->width();
        const int limit = qMax(200, static_cast<int>(total * 0.25));
        m_splitter->setSizes({ limit, total - limit });
    });
}


void MainWindow::updateTreeMaxWidth()
{
    if (!m_splitter)
        return;

    const int totalWidth = m_splitter->width();
    if (totalWidth <= 0)
        return;

    const int limit = qBound(200, static_cast<int>(totalWidth * 0.25), 500);

    QList<int> sizes = m_splitter->sizes();
    if (sizes.size() != 2)
        return;

    bool needUpdate = false;

    // Если дерево вылезло за 25% — подрезаем
    if (sizes[0] > limit) {
        sizes[0] = limit;
        sizes[1] = totalWidth - limit;
        needUpdate = true;
    }

    // Если суммарная ширина не совпадает с текущей (после ресайза) — синхронизируем
    if (sizes[0] + sizes[1] != totalWidth) {
        sizes[1] = totalWidth - sizes[0];
        needUpdate = true;
    }

    if (needUpdate)
        m_splitter->setSizes(sizes);
}

void MainWindow::on_splitterMoved(int /*pos*/, int /*index*/)
{
    if (!m_splitter)
        return;

    QList<int> sizes = m_splitter->sizes();
    if (sizes.size() != 2)
        return;

    const int totalWidth = m_splitter->width();
    const int limit      = qBound(200, static_cast<int>(totalWidth * 0.25), 500);

    if (sizes[0] > limit) {
        sizes[0] = limit;
        sizes[1] = totalWidth - limit;
        m_splitter->setSizes(sizes);
    }
}
void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    updateTreeMaxWidth();
}

void MainWindow::setupFileToolbar()
{
    // ---- 1. Создаём тулбар. Он автоматически встанет под menuBar. ----
    auto* toolbar = addToolBar(tr("Панель файла"));
    toolbar->setMovable(false);
    toolbar->setFloatable(false);
    toolbar->setAllowedAreas(Qt::TopToolBarArea);
    toolbar->setIconSize(QSize(16, 16));

    // ---- 2. Метка «Файл:» ----
    auto* label = new QLabel(tr("Файл:"), toolbar);
    label->setContentsMargins(6, 0, 6, 0);
    toolbar->addWidget(label);

    // ---- 3. Поле ввода — переносим из .ui ----
    if (ui->lineEdit) {
        // Отвязываем от старого layout
        if (QWidget* oldParent = ui->lineEdit->parentWidget()) {
            if (QLayout* oldLayout = oldParent->layout()) {
                oldLayout->removeWidget(ui->lineEdit);
            }
        }

        ui->lineEdit->setParent(toolbar);
        ui->lineEdit->setReadOnly(true);
        ui->lineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        ui->lineEdit->setMinimumWidth(300);
        ui->lineEdit->setPlaceholderText(tr("Файл не выбран"));

        toolbar->addWidget(ui->lineEdit);
    }

    // ---- 4. Кнопка «Выбрать» — переносим из .ui ----
    if (ui->pushButton) {
        if (QWidget* oldParent = ui->pushButton->parentWidget()) {
            if (QLayout* oldLayout = oldParent->layout()) {
                oldLayout->removeWidget(ui->pushButton);
            }
        }

        ui->pushButton->setParent(toolbar);
        ui->pushButton->setText(tr("Выбрать..."));
        ui->pushButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

        toolbar->addWidget(ui->pushButton);
    }
}

void MainWindow::showAboutDialog()
{
    const QString appName = QApplication::applicationName();
    const QString version = QApplication::applicationVersion();

    const QString text = tr(
        "<h3>%1</h3>"
        "<p>Версия: <b>%2</b></p>"
        "<p>Просмотр и разбор файлов конфигураций 1С в скобочном формате.</p>"

        "<h4>Сторонние компоненты</h4>"
        "<p>"
        "  <b>Fugue Icons</b><br>"
        "  Автор: Yusuke Kamiyamane<br>"
        "  Сайт: <a href='https://p.yusukekamiyamane.com/'>"
        "  p.yusukekamiyamane.com</a><br>"
        "  Лицензия: <a href='https://creativecommons.org/licenses/by/3.0/'>"
        "  CC BY 3.0</a>"
        "</p>"

        "<p>Собрано на Qt %3</p>"
        "<p>© 2026 Ваша организация</p>"
    ).arg(appName, version, QString::fromLatin1(qVersion()));

    QMessageBox box(this);
    box.setWindowTitle(tr("О программе"));
    box.setTextFormat(Qt::RichText);
    box.setText(text);
    box.setIcon(QMessageBox::Information);
    box.setStandardButtons(QMessageBox::Ok);
    box.exec();
}



QIcon MainWindow::iconForCategory(const QString& categoryKey) const
{
    auto* style = QApplication::style();

    // Верхний уровень — крупные категории
    if (categoryKey == "Общие")                 return QIcon(":/icons/icons/block-small.png");
    if (categoryKey == "Константы")             return QIcon(":/icons/icons/universal.png");
    if (categoryKey == "Справочники")           return QIcon(":/icons/icons/clipboard-invoice.png");
    if (categoryKey == "Документы")             return QIcon(":/icons/icons/blue-document--pencil.png");
    if (categoryKey == "ЖурналыДокументов")     return QIcon(":/icons/icons/photo-album.png");
    if (categoryKey == "Перечисления")          return QIcon(":/icons/icons/layers-ungroup.png");
    if (categoryKey == "Отчеты")                return QIcon(":/icons/icons/report.png");
    if (categoryKey == "Обработки")             return QIcon(":/icons/icons/gear--arrow.png");
    if (categoryKey == "ПланыВидовХарактеристик") return QIcon(":/icons/icons/edit-padding-left.png");
    if (categoryKey == "ПланыСчетов")           return QIcon(":/icons/icons/edit.png");
    if (categoryKey == "ПланыВидовРасчета")     return QIcon(":/icons/icons/drive--plus.png");

    if (categoryKey == "РегистрыСведений")      return QIcon(":/icons/icons/document-hf-insert.png");
    if (categoryKey == "РегистрыНакопления")    return QIcon(":/icons/icons/calculator--pencil.png");
    if (categoryKey == "РегистрыБухгалтерии")   return QIcon(":/icons/icons/books.png");
    if (categoryKey == "РегистрыРасчета")       return QIcon(":/icons/icons/web-slice-document.png");
    if (categoryKey == "БизнесПроцессы")        return QIcon(":/icons/icons/credit-cards.png");
    if (categoryKey == "Задачи")                return QIcon(":/icons/icons/clipboard-list.png");
    if (categoryKey == "ВнешниеИсточникиДанных")return QIcon(":/icons/icons/blue-folder-network.png");

    // Подкатегории «Общие»
    if (categoryKey == "Подсистемы")            return QIcon(":/icons/icons/node-select-all.png");
    if (categoryKey == "ОбщиеМодули")           return QIcon(":/icons/icons/scripts.png");
    if (categoryKey == "ПараметрыСеанса")       return QIcon(":/icons/icons/control-stop-square-small.png");
    if (categoryKey == "Роли")                  return QIcon(":/icons/icons/key.png");
    if (categoryKey == "ОбщиеРеквизиты")        return QIcon(":/icons/icons/minus.png");
    if (categoryKey == "ПланыОбмена")           return QIcon(":/icons/icons/sitemap.png");
    if (categoryKey == "КритерииОтбора")        return QIcon(":/icons/icons/funnel--arrow.png");
    if (categoryKey == "ПодпискиНаСобытия")     return QIcon(":/icons/icons/lightning--arrow.png");
    if (categoryKey == "РегламентныеЗадания")   return QIcon(":/icons/icons/alarm-clock-select.png");
    if (categoryKey == "ФункциональныеОпции")   return QIcon(":/icons/icons/application-blog.png");
    if (categoryKey == "ПараметрыФункциональныхОпций") return QIcon(":/icons/icons/application-detail.png");
    if (categoryKey == "ОпределяемыеТипы")      return QIcon(":/icons/icons/category.png");
    if (categoryKey == "ХранилищаНастроек")     return QIcon(":/icons/icons/database-cloud.png");
    if (categoryKey == "ОбщиеФормы")            return QIcon(":/icons/icons/zone-label.png");
    if (categoryKey == "ОбщиеКоманды")          return QIcon(":/icons/icons/task--plus.png");
    if (categoryKey == "ГруппыКоманд")          return QIcon(":/icons/icons/folders-stack.png");
    if (categoryKey == "Интерфейсы")            return QIcon(":/icons/icons/piano.png");
    if (categoryKey == "ОбщиеМакеты")           return QIcon(":/icons/icons/table--pencil.png");
    if (categoryKey == "ОбщиеКартинки")         return QIcon(":/icons/icons/pictures.png");
    if (categoryKey == "XDTOПакеты")            return QIcon(":/icons/icons/wooden-box--plus.png");
    if (categoryKey == "WebСервисы")            return QIcon(":/icons/icons/webcam-share.png");
    if (categoryKey == "HTTPСервисы")           return QIcon(":/icons/icons/globe-network.png");
    if (categoryKey == "WSСсылки")              return QIcon(":/icons/icons/globe-place.png");
    if (categoryKey == "СервисыИнтеграции")     return QIcon(":/icons/icons/globe-share.png");
    if (categoryKey == "ЭлементыСтиля")         return QIcon(":/icons/icons/molecule.png");
    if (categoryKey == "Стили")                 return QIcon(":/icons/icons/color--pencil.png");
    if (categoryKey == "Языки")                 return QIcon(":/icons/icons/globe-model.png");

    // Fallback — обычная папка
    return style->standardIcon(QStyle::SP_DirIcon);
}