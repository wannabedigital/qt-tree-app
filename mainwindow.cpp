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
}

MainWindow::~MainWindow()
{
    delete ui;
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

    ui->treeWidget->clear();

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList pathElements = line.split(" ", Qt::SkipEmptyParts);
        addPathToTree(pathElements);
    }
    file.close();

    ui->treeWidget->expandAll();
    updateLeafCount();

    currentFilePath = fileName;
}

void MainWindow::addPathToTree(const QStringList &pathElements)
{
    QTreeWidgetItem *currentParent = nullptr;
    QTreeWidget *tree = ui->treeWidget;

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
    bool ok;
    QString text = QInputDialog::getText(this, "Добавить путь", "Введите путь (через пробел):", QLineEdit::Normal, "", &ok);

    if (ok && !text.trimmed().isEmpty()) {
        QStringList pathElements = text.trimmed().split(" ", Qt::SkipEmptyParts);
        addPathToTree(pathElements);
        ui->treeWidget->expandAll();
        updateLeafCount();
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

void MainWindow::updateLeafCount()
{
    int totalLeaves = 0;
    for (int i = 0; i < ui->treeWidget->topLevelItemCount(); ++i) {
        totalLeaves += countLeaves(ui->treeWidget->topLevelItem(i));
    }
    leafCountLabel->setText("Количество листьев: " + QString::number(totalLeaves));
}

void MainWindow::on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    Q_UNUSED(previous);
    if (!current) return;

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
    if (currentFilePath.isEmpty()) {
        on_actionSaveAs_triggered();
        return;
    }

    QFile file(currentFilePath);
    if (!file.open(QIODevice::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл для записи");
        return;
    }

    QTextStream out(&file);
    QStringList allPaths;

    for (int i = 0; i < ui->treeWidget->topLevelItemCount(); ++i) {
        collectPaths(ui->treeWidget->topLevelItem(i), "", allPaths);
    }

    for (const QString &path : std::as_const(allPaths)) {
        out << path << "\n";
    }

    file.close();
    ui->statusbar->showMessage("Файл сохранен");
}


void MainWindow::on_actionSaveAs_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Сохранить дерево путей", "", "Text Files (*.txt);;All Files (*)");
    if (fileName.isEmpty()) return;

    currentFilePath = fileName;
    on_actionSave_triggered();
}

