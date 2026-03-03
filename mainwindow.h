#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QJsonObject>
#include <QJsonArray>
#include <QTreeWidgetItem> // 添加这个头文件

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // 确保参数类型 QTreeWidgetItem 已识别
    void displayJsonInTree(const QJsonArray &arr, QTreeWidgetItem *parent);

private slots:
    void on_btnDelete_clicked();
    void on_btnAdd_clicked();
    void on_btnConvert_clicked();
    void onItemChanged(QTreeWidgetItem *item, int column);  // 新增
    void on_btnImportFile_clicked();   // 导入文件（自动识别json/mcstructure）
    void on_btnExportJson_clicked();   // 导出为JSON
    // 在 private slots: 中添加

private:
    Ui::MainWindow *ui;
    QJsonObject currentJsonObject;
    QJsonArray treeToJson(QTreeWidgetItem *parent);
};
#endif
