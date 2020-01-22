#ifndef CREATECHARACTERDIALOG_H
#define CREATECHARACTERDIALOG_H

#include <QDialog>

namespace Ui {
class CreateCharacterDialog;
}

class CreateCharacterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateCharacterDialog(QWidget *parent = nullptr);
    ~CreateCharacterDialog();

private slots:
    void on_pushButton_clicked();

private:
    Ui::CreateCharacterDialog *ui;
};

#endif // CREATECHARACTERDIALOG_H
