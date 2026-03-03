#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "nbt_handler.h"
#include <QFileDialog>
#include <QJsonDocument>
#include <QMessageBox>
#include <QTreeWidget> // 必须包含这个来操作树状节点
#include <QInputDialog>
#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QMessageBox>

// 在 mainwindow.cpp 的构造函数中
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    ui->treeWidget->setIconSize(QSize(24, 24));  // 或 24,24
    setWindowTitle("NBT 转换器");
    setWindowIcon(QIcon(":/icons/NbtConverter.png"));  // 需要添加图标到资源文件

    ui->treeWidget->setColumnCount(3);
    ui->treeWidget->setHeaderLabels({"名称 (Name)", "类型 (Type)", "数值 (Value)"});

    // 监听项目修改信号
    connect(ui->treeWidget, &QTreeWidget::itemChanged, this, &MainWindow::onItemChanged);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::displayJsonInTree(const QJsonArray &arr, QTreeWidgetItem *parent) {
    for (const QJsonValue &val : arr) {
        QJsonObject obj = val.toObject();
        int type = obj["type"].toInt();

        QTreeWidgetItem *item = new QTreeWidgetItem(parent);
        item->setText(0, obj["name"].toString());
        item->setText(1, QString::number(type));

        // === 新增：根据 type 设置图标 ===
        QString iconPath;
        switch (type) {
        case 1: iconPath = ":/icons/type_1.png"; break;   // Byte
        case 2: iconPath = ":/icons/type_2.png"; break;   // Short
        case 3: iconPath = ":/icons/type_3.png"; break;   // Int
        case 4: iconPath = ":/icons/type_4.png"; break;   // Long
        case 5: iconPath = ":/icons/type_5.png"; break;   // Float
        case 6: iconPath = ":/icons/type_6.png"; break;   // Double
        case 8: iconPath = ":/icons/type_8.png"; break;   // String
        case 9: iconPath = ":/icons/type_9.png"; break;   // List
        case 10: iconPath = ":/icons/type_10.png"; break; // Compound
        default: iconPath = ":/icons/type_unknown.png"; break;
        }
        if (!iconPath.isEmpty()) {
            item->setIcon(0, QIcon(iconPath));
        }
        // ==============================

        item->setFlags(item->flags() | Qt::ItemIsEditable);

        if (type == 10 || type == 9) {
            displayJsonInTree(obj["value"].toArray(), item);
        } else {
            item->setText(2, obj["value"].toVariant().toString());
        }
    }
}


// 修改后的转换按钮逻辑
void MainWindow::on_btnConvert_clicked() {
    // 1. 从当前的 Tree 状态构建最新的 JSON
    QJsonArray latestContent = treeToJson(ui->treeWidget->invisibleRootItem());
    QJsonObject root;
    root["value"] = latestContent;

    // 2. 导出为小端序二进制 [cite: 1]
    QString savePath = QFileDialog::getSaveFileName(this, "保存", "visha.mcstructure", "*.mcstructure");
    if (!savePath.isEmpty()) {
        if (NbtHandler::saveAsMcStructure(root, savePath)) {
            QMessageBox::information(this, "完成", "修改已保存并导出为二进制文件");
        }
    }
}

// 递归将 TreeWidget 还原为 JSON 数组
QJsonArray MainWindow::treeToJson(QTreeWidgetItem *parent) {
    QJsonArray arr;
    for (int i = 0; i < parent->childCount(); ++i) {
        QTreeWidgetItem *item = parent->child(i);
        QJsonObject obj;
        obj["name"] = item->text(0);
        int type = item->text(1).toInt();
        obj["type"] = type;

        if (type == 10 || type == 9) {
            obj["value"] = treeToJson(item); // 递归处理 Compound/List [cite: 1]
        } else {
            // 根据类型还原数据类型（防止数字变成字符串）
            QString valStr = item->text(2);
            if (type == 1 || type == 2 || type == 3) obj["value"] = valStr.toInt();
            else if (type == 5 || type == 6) obj["value"] = valStr.toDouble();
            else obj["value"] = valStr;
        }
        arr.append(obj);
    }
    return arr;
}

void MainWindow::onItemChanged(QTreeWidgetItem *item, int column)
{
    // 如果需要实时处理编辑事件，可以在这里编写逻辑
    // 如果暂时不需要，可以留空或添加调试信息
    Q_UNUSED(item);
    Q_UNUSED(column);
    // qDebug() << "Item changed:" << item->text(0) << "column" << column;
}


void MainWindow::on_btnDelete_clicked()
{
    QTreeWidgetItem *currentItem = ui->treeWidget->currentItem();
    if (!currentItem) {
        QMessageBox::warning(this, "警告", "请先选中要删除的节点");
        return;
    }

    // 不能删除根节点（invisibleRootItem 不可见，但 currentItem 不会是它）
    QTreeWidgetItem *parent = currentItem->parent();
    if (parent) {
        parent->removeChild(currentItem);
        delete currentItem; // removeChild 后仍需 delete 释放内存
    } else {
        // 如果节点是顶层项（父节点为 invisibleRootItem），也需要删除
        int index = ui->treeWidget->indexOfTopLevelItem(currentItem);
        if (index != -1) {
            delete ui->treeWidget->takeTopLevelItem(index);
        }
    }
}

void MainWindow::on_btnAdd_clicked()
{
    QDialog dialog(this);
    dialog.setWindowTitle("添加节点");

    // 创建下拉列表并设置图标大小
    QComboBox *typeCombo = new QComboBox(&dialog);
    typeCombo->setIconSize(QSize(16, 16)); // 与树控件图标尺寸一致

    // 添加选项，同时设置图标和用户数据
    auto addTypeItem = [&](const QString &text, int type, const QString &iconPath) {
        typeCombo->addItem(QIcon(iconPath), text, type);
    };

    addTypeItem("Byte (1)", 1, ":/icons/type_1.png");
    addTypeItem("Short (2)", 2, ":/icons/type_2.png");
    addTypeItem("Int (3)", 3, ":/icons/type_3.png");
    addTypeItem("Long (4)", 4, ":/icons/type_4.png");
    addTypeItem("Float (5)", 5, ":/icons/type_5.png");
    addTypeItem("Double (6)", 6, ":/icons/type_6.png");
    addTypeItem("String (8)", 8, ":/icons/type_8.png");
    addTypeItem("List (9)", 9, ":/icons/type_9.png");
    addTypeItem("Compound (10)", 10, ":/icons/type_10.png");

    QLineEdit *nameEdit = new QLineEdit(&dialog);
    QLineEdit *valueEdit = new QLineEdit(&dialog);
    valueEdit->setPlaceholderText("对于 List/Compound 可留空");

    QFormLayout *layout = new QFormLayout(&dialog);
    layout->addRow("类型:", typeCombo);
    layout->addRow("名称:", nameEdit);
    layout->addRow("数值:", valueEdit);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
    layout->addRow(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted)
        return;

    int type = typeCombo->currentData().toInt();
    QString name = nameEdit->text();
    QString valueStr = valueEdit->text();

    // 确定插入位置（与之前相同）
    QTreeWidgetItem *parentItem = nullptr;
    QTreeWidgetItem *currentItem = ui->treeWidget->currentItem();
    if (!currentItem) {
        parentItem = ui->treeWidget->invisibleRootItem();
    } else {
        int currentType = currentItem->text(1).toInt();
        if (currentType == 9 || currentType == 10) {
            parentItem = currentItem;
        } else {
            parentItem = currentItem->parent();
            if (!parentItem)
                parentItem = ui->treeWidget->invisibleRootItem();
        }
    }

    QTreeWidgetItem *newItem = new QTreeWidgetItem(parentItem);
    newItem->setText(0, name);
    newItem->setText(1, QString::number(type));

    // 设置图标（复用之前的图标映射）
    QString iconPath;
    switch (type) {
    case 1: iconPath = ":/icons/type_1.png"; break;
    case 2: iconPath = ":/icons/type_2.png"; break;
    case 3: iconPath = ":/icons/type_3.png"; break;
    case 4: iconPath = ":/icons/type_4.png"; break;
    case 5: iconPath = ":/icons/type_5.png"; break;
    case 6: iconPath = ":/icons/type_6.png"; break;
    case 8: iconPath = ":/icons/type_8.png"; break;
    case 9: iconPath = ":/icons/type_9.png"; break;
    case 10: iconPath = ":/icons/type_10.png"; break;
    default: iconPath = ":/icons/type_unknown.png"; break;
    }
    if (!iconPath.isEmpty())
        newItem->setIcon(0, QIcon(iconPath));

    newItem->setFlags(newItem->flags() | Qt::ItemIsEditable);

    if (type != 9 && type != 10) {
        newItem->setText(2, valueStr);
    } else {
        newItem->setText(2, "");
    }

    if (parentItem != ui->treeWidget->invisibleRootItem())
        parentItem->setExpanded(true);
}

void MainWindow::on_btnImportFile_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "导入文件", "", "支持格式 (*.json *.mcstructure)");
    if (path.isEmpty()) return;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "错误", "无法打开文件");
        return;
    }

    QJsonObject root;
    if (path.endsWith(".json", Qt::CaseInsensitive)) {
        // 解析JSON文件
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        if (doc.isNull()) {
            QMessageBox::warning(this, "错误", "JSON 格式无效");
            return;
        }
        root = doc.object();
    } else if (path.endsWith(".mcstructure", Qt::CaseInsensitive)) {
        // 使用NBT解析
        root = NbtHandler::loadFromMcStructure(path);
        if (root.isEmpty()) {
            QMessageBox::warning(this, "错误", "无法读取 .mcstructure 文件或格式无效");
            return;
        }
    } else {
        QMessageBox::warning(this, "错误", "不支持的文件类型");
        return;
    }

    currentJsonObject = root;
    ui->treeWidget->clear();
    displayJsonInTree(currentJsonObject["value"].toArray(), ui->treeWidget->invisibleRootItem());
    QMessageBox::information(this, "成功", "文件已成功导入");
}

void MainWindow::on_btnExportJson_clicked()
{
    // 从当前树构建JSON
    QJsonArray latestContent = treeToJson(ui->treeWidget->invisibleRootItem());
    QJsonObject root;
    root["value"] = latestContent;

    QString savePath = QFileDialog::getSaveFileName(this, "导出为JSON", "", "JSON 文件 (*.json)");
    if (savePath.isEmpty()) return;

    QFile file(savePath);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "错误", "无法创建文件");
        return;
    }

    QJsonDocument doc(root);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    QMessageBox::information(this, "成功", "JSON 文件已保存");
}
