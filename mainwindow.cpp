#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QInputDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    leafCountLabel = new QLabel("Количество листьев: 0", this);
    ui->statusbar->addPermanentWidget(leafCountLabel);

    connect(ui->mdiArea, &QMdiArea::subWindowActivated, this, &MainWindow::onSubWindowActivated);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onSubWindowActivated(QMdiSubWindow *window) {
    if (window) {
        QTreeWidget *tree = qobject_cast<QTreeWidget*>(window->widget());
        updateLeafCount(tree);
    } else {
        leafCountLabel->setText("Количество листьев: 0");
        ui->statusbar->showMessage("");
    }
}

void MainWindow::on_actionOpen_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Открыть файл путей", "", "Text Files (*.txt);;All Files (*)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл");
        return;
    }

    QTreeWidget *tree = setupNewSubWindow(fileName);
    tree->setProperty("filePath", fileName);

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList pathElements = line.split(" ", Qt::SkipEmptyParts);
        addPathToTree(pathElements, tree);
    }
    file.close();

    tree->expandAll();
    updateLeafCount(tree);
}

void MainWindow::addPathToTree(const QStringList &pathElements, QTreeWidget *tree)
{
    if (!tree) return;

    QTreeWidgetItem *currentParent = nullptr;

    for (const QString &word : pathElements) {
        QTreeWidgetItem *match = nullptr;

        int childCount = currentParent ? currentParent->childCount() : tree->topLevelItemCount();
        for (int i = 0; i < childCount; ++i) {
            QTreeWidgetItem *child = currentParent ? currentParent->child(i) : tree->topLevelItem(i);
            if (child->text(0) == word) {
                match = child;
                break;
            }
        }

        if (!match) {
            match = new QTreeWidgetItem();
            match->setText(0, word);
            if (currentParent) {
                currentParent->addChild(match);
            } else {
                tree->addTopLevelItem(match);
            }
        }
        currentParent = match;
    }
}


void MainWindow::on_actionAddPath_triggered()
{
    QTreeWidget *tree = activeTreeWidget();
    if (!tree) {
        QMessageBox::information(this, "Внимание", "Создайте или откройте документ.");
        return;
    }

    bool ok;
    QString text = QInputDialog::getText(this, "Добавить путь", "Введите путь (через пробел):", QLineEdit::Normal, "", &ok);

    if (ok && !text.trimmed().isEmpty()) {
        QStringList pathElements = text.trimmed().split(" ", Qt::SkipEmptyParts);
        addPathToTree(pathElements, tree);
        tree->expandAll();
        updateLeafCount(tree);
    }
}

int MainWindow::countLeaves(QTreeWidgetItem *item)
{
    if (!item) return 0;

    if (item->childCount() == 0) return 1;

    int count = 0;
    for (int i = 0; i < item->childCount(); ++i) {
        count += countLeaves(item->child(i));
    }
    return count;
}

void MainWindow::updateLeafCount(QTreeWidget *tree)
{
    if (!tree) return;

    int totalLeaves = 0;
    for (int i = 0; i < tree->topLevelItemCount(); ++i) {
        totalLeaves += countLeaves(tree->topLevelItem(i));
    }
    leafCountLabel->setText("Количество листьев: " + QString::number(totalLeaves));
}

void MainWindow::onCurrentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    Q_UNUSED(previous);
    if (!current) {
        ui->statusbar->showMessage("");
        return;
    }

    QStringList fullPath;
    QTreeWidgetItem *node = current;

    while (node != nullptr) {
        fullPath.prepend(node->text(0));
        node = node->parent();
    }

    ui->statusbar->showMessage("Полный путь: " + fullPath.join(" "));
}

void MainWindow::collectPaths(QTreeWidgetItem *item, QString currentPath, QStringList &results)
{
    QString nodeText = item->text(0);
    QString newPath = currentPath.isEmpty() ? nodeText : currentPath + " " + nodeText;

    if (item->childCount() == 0) {
        results << newPath;
    } else {
        for (int i = 0; i < item->childCount(); ++i) {
            collectPaths(item->child(i), newPath, results);
        }
    }
}

void MainWindow::on_actionSave_triggered()
{
    QTreeWidget *tree = activeTreeWidget();
    if (!tree) return;

    QString fileName = tree->property("filePath").toString();

    if (fileName.isEmpty()) {
        on_actionSaveAs_triggered();
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл для записи");
        return;
    }

    QTextStream out(&file);
    QStringList allPaths;

    for (int i = 0; i < tree->topLevelItemCount(); ++i) {
        collectPaths(tree->topLevelItem(i), "", allPaths);
    }

    for (const QString &path : std::as_const(allPaths)) {
        out << path << "\n";
    }

    file.close();
    ui->statusbar->showMessage("Файл сохранен");
}


void MainWindow::on_actionSaveAs_triggered()
{
    QTreeWidget *tree = activeTreeWidget();
    if (!tree) return;

    QString fileName = QFileDialog::getSaveFileName(this, "Сохранить дерево путей", "", "Text Files (*.txt);;All Files (*)");
    if (fileName.isEmpty()) return;

    tree->setProperty("filePath", fileName);
    ui->mdiArea->activeSubWindow()->setWindowTitle(fileName);

    on_actionSave_triggered();
}


QTreeWidget* MainWindow::setupNewSubWindow(QString title) {
    QTreeWidget *tree = new QTreeWidget();
    tree->setHeaderLabel("Пути");

    QMdiSubWindow *subWindow = ui->mdiArea->addSubWindow(tree);
    subWindow->setWindowTitle(title);
    subWindow->setAttribute(Qt::WA_DeleteOnClose);

    connect(tree, &QTreeWidget::currentItemChanged, this, &MainWindow::onCurrentItemChanged);

    subWindow->show();
    return tree;
}

QTreeWidget* MainWindow::activeTreeWidget() {
    if (QMdiSubWindow *activeSubWindow = ui->mdiArea->activeSubWindow()) {
        return qobject_cast<QTreeWidget*>(activeSubWindow->widget());
    }
    return nullptr;
}

void MainWindow::on_actionNew_triggered() {
    setupNewSubWindow("Новое дерево");
}


void MainWindow::on_actionTile_triggered()
{
    ui->mdiArea->tileSubWindows();
}


void MainWindow::on_actionCascade_triggered()
{
    ui->mdiArea->cascadeSubWindows();
}

