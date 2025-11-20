#include "mainwindow.h"
#include "../codec.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("STS-SaveEditor");

    // Central widget and main layout
    QWidget *central = new QWidget(this);
    auto *mainLay = new QVBoxLayout(central);

    // Key input
    keyEdit = new QLineEdit(central);
    keyEdit->setPlaceholderText("输入密钥（默认 key）");
    mainLay->addWidget(keyEdit);

    // Source text
    srcEdit = new QPlainTextEdit(central);
    srcEdit->setPlaceholderText("原始内容 / 拖入文件");
    mainLay->addWidget(srcEdit);

    // Buttons row
    auto *btnLay = new QHBoxLayout();
    openSrc = new QPushButton(tr("打开"), central);
    encodeBtn = new QPushButton(tr("Encode"), central);
    decodeBtn = new QPushButton(tr("Decode"), central);
    saveDst = new QPushButton(tr("保存"), central);
    btnLay->addWidget(openSrc);
    btnLay->addWidget(encodeBtn);
    btnLay->addWidget(decodeBtn);
    btnLay->addWidget(saveDst);
    mainLay->addLayout(btnLay);

    // Destination text
    dstEdit = new QPlainTextEdit(central);
    dstEdit->setPlaceholderText("结果输出");
    mainLay->addWidget(dstEdit);

    setCentralWidget(central);

    // Connections
    connect(encodeBtn, &QPushButton::clicked, this, &MainWindow::onEncodeClicked);
    connect(decodeBtn, &QPushButton::clicked, this, &MainWindow::onDecodeClicked);
    connect(openSrc,   &QPushButton::clicked, this, &MainWindow::onOpenSrc);
    connect(saveDst,   &QPushButton::clicked, this, &MainWindow::onSaveDst);
}

MainWindow::~MainWindow() = default;

void MainWindow::onEncodeClicked()
{
    QString key = keyEdit->text();
    if (key.isEmpty()) { QMessageBox::warning(this, "提示", "密钥不能为空"); return; }
    QString src = srcEdit->toPlainText();
    if (src.isEmpty()) return;

    std::string out = encode_autosave(src.toStdString(), key.toStdString());
    dstEdit->setPlainText(QString::fromStdString(out));
}

void MainWindow::onDecodeClicked()
{
    QString key = keyEdit->text();
    if (key.isEmpty()) { QMessageBox::warning(this, "提示", "密钥不能为空"); return; }
    QString src = srcEdit->toPlainText();
    if (src.isEmpty()) return;

    std::string out = decode_autosave(src.toStdString(), key.toStdString());
    dstEdit->setPlainText(QString::fromStdString(out));
}


void MainWindow::onOpenSrc()
{
    QString f = QFileDialog::getOpenFileName(this, "打开源文件");
    if (f.isEmpty()) return;
    QFile file(f);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        srcEdit->setPlainText(file.readAll());
}

void MainWindow::onSaveDst()
{
    QString f = QFileDialog::getSaveFileName(this, "保存目标文件");
    if (f.isEmpty()) return;
    QFile file(f);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
        file.write(dstEdit->toPlainText().toUtf8());
}