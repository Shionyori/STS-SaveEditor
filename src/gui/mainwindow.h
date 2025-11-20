#pragma once
#include <QMainWindow>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onEncodeClicked();
    void onDecodeClicked();
    void onOpenSrc();
    void onSaveDst();

private:
    bool allowCodec = true; // 防止自动触发 encode/decode

    QLineEdit *keyEdit = nullptr;
    QPlainTextEdit *srcEdit = nullptr;
    QPlainTextEdit *dstEdit = nullptr;
    QPushButton *openSrc = nullptr;
    QPushButton *encodeBtn = nullptr;
    QPushButton *decodeBtn = nullptr;
    QPushButton *saveDst = nullptr;
};
