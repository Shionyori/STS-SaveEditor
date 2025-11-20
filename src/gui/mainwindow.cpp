#include "mainwindow.h"
#include "../codec.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("STS-SaveEditor");

    QWidget *central = new QWidget(this);
    auto *mainLay = new QVBoxLayout(central);

    keyEdit = new QLineEdit(central);
    keyEdit->setPlaceholderText("输入密钥（默认 key）");
    mainLay->addWidget(keyEdit);

    srcEdit = new QPlainTextEdit(central);
    srcEdit->setPlaceholderText("原始内容 / TXT / autosave 均可");
    mainLay->addWidget(srcEdit);

    auto *btnLay = new QHBoxLayout();
    openSrc = new QPushButton("Open", central);
    encodeBtn = new QPushButton("Encode", central);
    decodeBtn = new QPushButton("Decode", central);
    saveDst = new QPushButton("Save", central);
    btnLay->addWidget(openSrc);
    btnLay->addWidget(encodeBtn);
    btnLay->addWidget(decodeBtn);
    btnLay->addWidget(saveDst);
    mainLay->addLayout(btnLay);

    dstEdit = new QPlainTextEdit(central);
    dstEdit->setPlaceholderText("结果输出");
    mainLay->addWidget(dstEdit);

    setCentralWidget(central);

    connect(openSrc,   &QPushButton::clicked, this, &MainWindow::onOpenSrc);
    connect(encodeBtn, &QPushButton::clicked, this, &MainWindow::onEncodeClicked);
    connect(decodeBtn, &QPushButton::clicked, this, &MainWindow::onDecodeClicked);
    connect(saveDst,   &QPushButton::clicked, this, &MainWindow::onSaveDst);
}

MainWindow::~MainWindow() {}


// ----------------------------------------
// 安全打开任何文件（TXT/GBK/UTF8/二进制）
// ----------------------------------------
void MainWindow::onOpenSrc()
{
    QString f = QFileDialog::getOpenFileName(this, "打开源文件");
    if (f.isEmpty()) return;

    QFile file(f);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "错误", "无法打开文件");
        return;
    }

    QByteArray raw = file.readAll();

    allowCodec = false; // 防止自动触发 encode/decode

    // 尝试 UTF-8
    QString text = QString::fromUtf8(raw);

    // 如果含有替换符号，说明不是 UTF-8 → 尝试 ANSI/GBK
    if (text.contains(QChar::ReplacementCharacter))
        text = QString::fromLocal8Bit(raw);

    srcEdit->setPlainText(text);

    allowCodec = true;
}


// ----------------------------------------
// Encode：文本 → autosave(二进制)
// ----------------------------------------
void MainWindow::onEncodeClicked()
{
    if (!allowCodec) return;

    QString key = keyEdit->text().trimmed();
    if (key.isEmpty()) {
        QMessageBox::warning(this, "提示", "密钥不能为空");
        return;
    }

    QByteArray src = srcEdit->toPlainText().toUtf8();

    try {
        std::string out = encode_autosave(src.toStdString(), key.toStdString());
        // 二进制 → Latin1 显示，避免 UTF8 崩溃
        dstEdit->setPlainText(QString::fromLatin1(out.data(), out.size()));
    }
    catch (const std::exception &e) {
        QMessageBox::critical(this, "异常", e.what());
    }
}


void MainWindow::onDecodeClicked()
{
    if (!allowCodec) return;

    QString key = keyEdit->text().trimmed();
    if (key.isEmpty()) {
        QMessageBox::warning(this, "提示", "密钥不能为空");
        return;
    }

    QByteArray src = srcEdit->toPlainText().toLatin1();

    try {
        std::string out = decode_autosave(src.toStdString(), key.toStdString());
        // 解码后一般是 UTF-8 文本
        dstEdit->setPlainText(QString::fromUtf8(out.data(), out.size()));
    }
    catch (const std::exception &e) {
        QMessageBox::critical(this, "异常", e.what());
    }
}


// ----------------------------------------
// 保存文本（包括Binary）
// ----------------------------------------
void MainWindow::onSaveDst()
{
    QString f = QFileDialog::getSaveFileName(this, "保存目标文件");
    if (f.isEmpty()) return;

    QFile file(f);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "错误", "无法写入文件");
        return;
    }

    QByteArray out = dstEdit->toPlainText().toLatin1();
    file.write(out);
}
