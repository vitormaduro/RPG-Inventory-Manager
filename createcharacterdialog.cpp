#include "createcharacterdialog.h"
#include "ui_createcharacterdialog.h"
#include "mainwindow.h"

CreateCharacterDialog::CreateCharacterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateCharacterDialog)
{
    ui->setupUi(this);
}

CreateCharacterDialog::~CreateCharacterDialog()
{
    delete ui;
}

void CreateCharacterDialog::on_pushButton_clicked()
{
    MainWindow mainWindow;
    if(mainWindow.insertNewCharacter(ui->charName->toPlainText(), ui->carryWeight->toPlainText().toFloat())) {
        close();
    }
}
