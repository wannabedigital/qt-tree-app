#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidgetItem>
#include <QLabel>
#include <QMdiSubWindow>

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

    void on_actionSave_triggered();

    void on_actionSaveAs_triggered();

    void on_actionNew_triggered();

    void onCurrentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

    void onSubWindowActivated(QMdiSubWindow *window);

    void on_actionTile_triggered();

    void on_actionCascade_triggered();

private:
    Ui::MainWindow *ui;
    QLabel *leafCountLabel;

    QTreeWidget* setupNewSubWindow(QString title);
    QTreeWidget* activeTreeWidget();

    void addPathToTree(const QStringList &pathElements, QTreeWidget *tree);
    int countLeaves(QTreeWidgetItem *item);
    void updateLeafCount(QTreeWidget *tree);
    void collectPaths(QTreeWidgetItem *item, QString currentPath, QStringList &results);
};
#endif // MAINWINDOW_H
