#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "../codec.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("STS-SaveEditor");

    connect(ui->encodeBtn, &QPushButton::clicked, this, &MainWindow::onEncodeClicked);
    connect(ui->decodeBtn, &QPushButton::clicked, this, &MainWindow::onDecodeClicked);
    connect(ui->openSrc,   &QPushButton::clicked, this, &MainWindow::onOpenSrc);
    connect(ui->saveDst,   &QPushButton::clicked, this, &MainWindow::onSaveDst);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::onEncodeClicked()
{
    QString key = ui->keyEdit->text();
    if (key.isEmpty()) { QMessageBox::warning(this, "提示", "密钥不能为空"); return; }
    QString src = ui->srcEdit->toPlainText();
    if (src.isEmpty()) return;

    std::string out = encode_autosave(src.toStdString(), key.toStdString());
    ui->dstEdit->setPlainText(QString::fromStdString(out));
}

void MainWindow::onDecodeClicked()
{
    QString key = ui->keyEdit->text();
    if (key.isEmpty()) { QMessageBox::warning(this, "提示", "密钥不能为空"); return; }
    QString src = ui->srcEdit->toPlainText();
    if (src.isEmpty()) return;

    std::string out = decode_autosave(src.toStdString(), key.toStdString());
    ui->dstEdit->setPlainText(QString::fromStdString(out));
}

void MainWindow::onOpenSrc()
{
    QString f = QFileDialog::getOpenFileName(this, "打开源文件");
    if (f.isEmpty()) return;
    QFile file(f);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        ui->srcEdit->setPlainText(file.readAll());
}

void MainWindow::onSaveDst()
{
    QString f = QFileDialog::getSaveFileName(this, "保存目标文件");
    if (f.isEmpty()) return;
    QFile file(f);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
        file.write(ui->dstEdit->toPlainText().toUtf8());
}