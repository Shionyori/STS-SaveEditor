#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QPlainTextEdit>   // 1. 纯文本控件
#include <QLineEdit>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QFile>
#include "codec.h"

class MainWindow : public QWidget
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr) : QWidget(parent)
    {
        setWindowTitle("STSSaveEditor");
        resize(800, 600);

        /* ---- 控件 ---- */
        auto *vLay = new QVBoxLayout(this);

        auto *h1 = new QHBoxLayout();
        btnOpen  = new QPushButton("打开.autosave");
        btnDec   = new QPushButton("Decode → JSON");
        btnEnc   = new QPushButton("Encode ← JSON");
        btnSave  = new QPushButton("保存.autosave");
        h1->addWidget(btnOpen);
        h1->addWidget(btnDec);
        h1->addWidget(btnEnc);
        h1->addWidget(btnSave);
        vLay->addLayout(h1);

        pathEdit = new QLineEdit();
        pathEdit->setReadOnly(true);
        vLay->addWidget(pathEdit);

        editor = new QPlainTextEdit();          // 2. 纯文本
        editor->setPlaceholderText("JSON 编辑区");
        vLay->addWidget(editor);

        status = new QLabel("就绪");
        vLay->addWidget(status);

        /* ---- 信号 ---- */
        connect(btnOpen, &QPushButton::clicked, this, &MainWindow::openFile);
        connect(btnDec,  &QPushButton::clicked, this, &MainWindow::decode);
        connect(btnEnc,  &QPushButton::clicked, this, &MainWindow::encode);
        connect(btnSave, &QPushButton::clicked, this, &MainWindow::save);
    }

private slots:
    void openFile()
    {
        QString f = QFileDialog::getOpenFileName(this, "选择文件", "",
                                                 "autosave (*.autosave);;JSON (*.json)");
        if (f.isEmpty()) return;

        QFile file(f);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(this, "错误", "无法打开文件");
            return;
        }
        QByteArray blob = file.readAll();
        file.close();
        if (blob.isEmpty()) {
            QMessageBox::warning(this, "提示", "文件为空");
            return;
        }

        path = f;
        pathEdit->setText(path);
        rawAutosave = std::string(blob.begin(), blob.end()); // 内存保留原始二进制

        if (f.endsWith(".json", Qt::CaseInsensitive)) {
            /* 文本文件：直接显示 */
            QString txt = QString::fromUtf8(blob); // JSON 必须是 UTF-8
            editor->setPlainText(txt);
            status->setText("JSON 已加载");
        } else {
            /* 二进制：清空编辑区，等待 Decode */
            editor->clear();
            status->setText("已加载 autosave，请点击 Decode");
        }
    }

    void decode()
    {
        if (rawAutosave.empty()) {
            QMessageBox::warning(this, "提示", "请先打开 autosave 文件");
            return;
        }
        try {
            std::string json = decode_autosave(rawAutosave);
            QString txt = QString::fromUtf8(json.c_str(), json.size());
            if (txt.isEmpty() && !json.empty())
                txt = QString::fromLocal8Bit(json.c_str(), json.size());
            editor->setPlainText(txt);            // 3. 纯文本设置
            status->setText("解码成功");
        } catch (...) {
            QMessageBox::critical(this, "错误", "解码失败");
            status->setText("解码失败");
        }
    }

    void encode()
    {
        QString txt = editor->toPlainText();      // 纯文本读取
        if (txt.isEmpty()) {
            QMessageBox::warning(this, "提示", "JSON 内容为空");
            return;
        }
        try {
            rawAutosave = encode_autosave(txt.toStdString());
            status->setText("编码完成，可保存");
        } catch (...) {
            QMessageBox::critical(this, "错误", "编码失败");
            status->setText("编码失败");
        }
    }

    void save()
    {
        if (rawAutosave.empty()) {
            QMessageBox::warning(this, "提示", "尚未编码，请先 Encode");
            return;
        }
        QString def = QFileInfo(path).absolutePath() + "/modified.autosave";
        QString out = QFileDialog::getSaveFileName(this, "保存 autosave", def,
                                                   "autosave (*.autosave)");
        if (out.isEmpty()) return;
        if (write_file(out.toStdString(), rawAutosave)) {
            status->setText("已保存：" + out);
        } else {
            QMessageBox::critical(this, "错误", "写入失败");
        }
    }

private:
    QPushButton *btnOpen, *btnDec, *btnEnc, *btnSave;
    QLineEdit   *pathEdit;
    QPlainTextEdit *editor;   // 纯文本
    QLabel *status;
    QString path;
    std::string rawAutosave;  // 始终保留二进制
};

#include "gui_main.moc"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}