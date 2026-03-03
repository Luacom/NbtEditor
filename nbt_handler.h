#ifndef NBT_HANDLER_H
#define NBT_HANDLER_H

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QDataStream>
#include <QFile>
#include <QDebug>

class NbtHandler {
public:
    // 写入 .mcstructure 文件（已有，保持不变）
    static bool saveAsMcStructure(const QJsonObject &root, const QString &filePath) {
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly)) return false;

        QDataStream out(&file);
        out.setByteOrder(QDataStream::LittleEndian);
        out.setFloatingPointPrecision(QDataStream::SinglePrecision);

        // 写入根节点：无名 Compound (Type 10)
        writeTag(out, 10, "", root["value"].toArray());

        file.close();
        return true;
    }

    // 新增：从 .mcstructure 文件读取
    static QJsonObject loadFromMcStructure(const QString &filePath) {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) return {};

        QDataStream in(&file);
        in.setByteOrder(QDataStream::LittleEndian);
        in.setFloatingPointPrecision(QDataStream::SinglePrecision);

        // 读取根标签（应为无名 Compound）
        return readTag(in);
    }

private:
    // 写入标签（已有，保持不变）
    static void writeTag(QDataStream &s, int type, const QString &name, const QJsonValue &value) {
        s << (quint8)type;
        if (type != 0) {
            QByteArray nameData = name.toUtf8();
            s << (quint16)nameData.length();
            s.writeRawData(nameData.data(), nameData.length());
        }
        writeValue(s, type, value);
    }

    // 写入值（已有，保持不变）
    static void writeValue(QDataStream &s, int type, const QJsonValue &val) {
        switch (type) {
        case 1: s << (qint8)val.toInt(); break;
        case 2: s << (qint16)val.toInt(); break;
        case 3: s << (qint32)val.toInt(); break;
        case 4: s << (qint64)val.toVariant().toLongLong(); break;
        case 5: s << (float)val.toDouble(); break;
        case 6: s << val.toDouble(); break;
        case 8: {
            QByteArray str = val.toString().toUtf8();
            s << (quint16)str.length();
            s.writeRawData(str.data(), str.length());
            break;
        }
        case 9: {
            QJsonArray arr = val.toArray();
            int subType = arr.isEmpty() ? 0 : arr[0].toObject()["type"].toInt();
            s << (quint8)subType;
            s << (qint32)arr.size();
            for (const QJsonValue &v : arr)
                writeValue(s, subType, v.toObject()["value"]);
            break;
        }
        case 10: {
            QJsonArray arr = val.toArray();
            for (const QJsonValue &v : arr) {
                QJsonObject obj = v.toObject();
                writeTag(s, obj["type"].toInt(), obj["name"].toString(), obj["value"]);
            }
            s << (quint8)0; // End Tag
            break;
        }
        }
    }

    // 读取一个完整的标签（包含类型、名称、值）
    static QJsonObject readTag(QDataStream &s) {
        quint8 type;
        s >> type;
        if (type == 0) return {}; // End Tag

        quint16 nameLen;
        s >> nameLen;
        QByteArray nameData(nameLen, Qt::Uninitialized);
        s.readRawData(nameData.data(), nameLen);
        QString name = QString::fromUtf8(nameData);

        QJsonObject obj;
        obj["name"] = name;
        obj["type"] = type;
        obj["value"] = readValue(s, type);
        return obj;
    }

    // 读取值（根据类型）
    static QJsonValue readValue(QDataStream &s, int type) {
        switch (type) {
        case 1: { // Byte (有符号)
            qint8 val;
            s >> val;
            return val;
        }
        case 2: { // Short
            qint16 val;
            s >> val;
            return val;
        }
        case 3: { // Int
            qint32 val;
            s >> val;
            return val;
        }
        case 4: { // Long
            qint64 val;
            s >> val;
            return val;
        }
        case 5: { // Float
            float val;
            s >> val;
            return val;
        }
        case 6: { // Double
            double val;
            s >> val;
            return val;
        }
        case 8: { // String
            quint16 len;
            s >> len;
            QByteArray strData(len, Qt::Uninitialized);
            s.readRawData(strData.data(), len);
            return QString::fromUtf8(strData);
        }
        case 9: { // List —— 关键修改处
            quint8 subType;
            s >> subType;
            qint32 size;
            s >> size;

            QJsonArray listArr;
            for (int i = 0; i < size; ++i) {
                QJsonValue elementValue = readValue(s, subType);
                // 为每个元素创建包裹对象（无名称）
                QJsonObject elemObj;
                elemObj["name"] = "";
                elemObj["type"] = subType;
                elemObj["value"] = elementValue;
                listArr.append(elemObj);
            }
            return listArr;
        }
        case 10: { // Compound
            QJsonArray compoundArr;
            while (true) {
                QJsonObject subTag = readTag(s);
                if (subTag.isEmpty()) break; // End Tag
                compoundArr.append(subTag);
            }
            return compoundArr;
        }
        default:
            return {};
        }
    }
};

#endif // NBT_HANDLER_H
