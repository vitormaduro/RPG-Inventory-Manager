#include "createitemdialog.h"
#include "ui_createitemdialog.h"
#include "mainwindow.h"

CreateItemDialog::CreateItemDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateItemDialog)
{
    ui->setupUi(this);

    MainWindow mainWindow;
    QList<QString> charNames = mainWindow.getCharacters();

    for(int i = 0; i < charNames.size(); i++) {
        ui->characterName->addItem(charNames[i]);
    }
}

CreateItemDialog::~CreateItemDialog()
{
    delete ui;
}

void CreateItemDialog::on_createButton_clicked()
{
    MainWindow mainWindow;

    if(mainWindow.insertNewItem(ui->itemName->toPlainText(), ui->itemWeight->toPlainText().toFloat(), ui->itemDesc->toPlainText(), ui->itemQuantity->toPlainText().toInt(), ui->characterName->currentText())) {
        mainWindow.updateInventoryWindow();
        close();
    }
}
