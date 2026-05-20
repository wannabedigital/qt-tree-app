#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidgetItem>
#include <QLabel>
#include <QMdiSubWindow>
#include <QDropEvent>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class CustomTreeWidget : public QTreeWidget {
public:
    CustomTreeWidget(QWidget *parent = nullptr) : QTreeWidget(parent) {}

protected:
    void dropEvent(QDropEvent *event) override {
        QTreeWidget *sourceTree = qobject_cast<QTreeWidget*>(event->source());

        if (sourceTree) {
            QList<QTreeWidgetItem*> items = sourceTree->selectedItems();
            if (items.isEmpty()) return;

            QTreeWidgetItem *targetItem = itemAt(event->position().toPoint());
            DropIndicatorPosition dropPos = dropIndicatorPosition();

            QTreeWidgetItem *temp = targetItem;
            while (temp) {
                if (items.contains(temp)) {
                    event->ignore();
                    return;
                }
                temp = temp->parent();
            }

            if (sourceTree == this) {
                for (QTreeWidgetItem *item : std::as_const(items)) {
                    QTreeWidgetItem *currentParent = item->parent();

                    if (targetItem && targetItem == currentParent && (dropPos == QAbstractItemView::OnItem || dropPos == QAbstractItemView::OnViewport)) {
                        event->ignore();
                        return;
                    }

                    if (!targetItem && currentParent == nullptr) {
                        event->ignore();
                        return;
                    }
                }
            }

            for (QTreeWidgetItem *item : std::as_const(items)) {
                QTreeWidgetItem *clone = item->clone();

                if (targetItem) {
                    if (dropPos == QAbstractItemView::OnItem || dropPos == QAbstractItemView::OnViewport) {
                        targetItem->addChild(clone);
                        targetItem->setExpanded(true);
                    } else {
                        QTreeWidgetItem *parent = targetItem->parent();
                        if (parent) {
                            int index = parent->indexOfChild(targetItem);
                            if (dropPos == QAbstractItemView::BelowItem) index++;
                            parent->insertChild(index, clone);
                        } else {
                            int index = indexOfTopLevelItem(targetItem);
                            if (dropPos == QAbstractItemView::BelowItem) index++;
                            insertTopLevelItem(index, clone);
                        }
                    }
                } else {
                    addTopLevelItem(clone);
                }
            }
            event->acceptProposedAction();
            return;
        }
        QTreeWidget::dropEvent(event);
    }
};

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
