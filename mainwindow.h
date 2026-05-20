#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidgetItem>
#include <QLabel>

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

private slots:
    void on_actionOpen_triggered();

    void on_actionAddPath_triggered();

    void on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

    void on_actionSave_triggered();

    void on_actionSaveAs_triggered();

private:
    Ui::MainWindow *ui;

    void addPathToTree(const QStringList &pathElements);

    QLabel *leafCountLabel;
    int countLeaves(QTreeWidgetItem *item);
    void updateLeafCount();

    QString currentFilePath;
    void collectPaths(QTreeWidgetItem *item, QString currentPath, QStringList &results);
};
#endif // MAINWINDOW_H
