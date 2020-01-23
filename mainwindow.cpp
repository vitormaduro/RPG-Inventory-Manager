#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "createitemdialog.h"
#include "createcharacterdialog.h"
#include "QMessageBox"
#include "QInputDialog"
#include "QSqlDatabase"
#include "QSqlDriver"
#include "QSqlError"
#include "QSqlQuery"
#include "QDebug"
#include "QSqlRelationalTableModel"
#include "QSqlQueryModel"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->actionAbout_Qt, SIGNAL(triggered()), this, SLOT(MenuAboutQtPressed()));

    const QString DRIVER("QSQLITE");

    if(QSqlDatabase::isDriverAvailable(DRIVER)) {
        QSqlDatabase db = QSqlDatabase::addDatabase(DRIVER);
        db.setDatabaseName(QCoreApplication::applicationDirPath() + "dbInventory");
        if(!db.open()) {
            qWarning() << "ERROR: " << db.lastError();
        }

        QSqlQuery query;

        if(!db.tables(QSql::AllTables).contains(QLatin1String("Inventory")) && !db.tables(QSql::AllTables).contains(QLatin1String("Items")) && !db.tables(QSql::AllTables).contains(QLatin1String("Characters"))) {
            query.exec("CREATE TABLE Items (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, weight FLOAT, description TEXT)");
            qWarning() << query.lastError().text();

            query.exec("CREATE TABLE Characters (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, maxCarryWeight FLOAT)");
            qWarning() << query.lastError().text();

            query.exec("CREATE TABLE Parties (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT)");
            qWarning() << query.lastError().text();

            query.exec("CREATE TABLE PartyComposition (charID INTEGER, partyID INTEGER)");
            qWarning() << query.lastError().text();

            query.exec("CREATE TABLE Inventory (charID INTEGER, itemID INTEGER, quantity INTEGER)");
            qWarning() << query.lastError().text();
        }        

        query.exec("SELECT name FROM Characters");
        while(query.next()) {
            ui->cb_characterNames->addItem(query.value(0).toString());
        }

        updateInventoryWindow();
        ui->tv_Inventory->setSelectionBehavior(QAbstractItemView::SelectRows);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

/*
    Insere um novo item no banco de dados. Se um item com o mesmo nome já existir, é possível inserí-lo novamente ou cancelar a operação
    Entradas: name (nome do item a ser inserido), weight (peso do item), description (descrição to item, character (personagem que possui o item)
    Retorno: true, caso a inserção tenha sucesso. Caso contrário, retorna false
*/
bool MainWindow::insertNewItem(QString name, float weight, QString description, int quantity, QString character) {
    QSqlQuery query;
    int charIndex;
    int itemIndex;
    bool error = false;

    query.prepare("SELECT name FROM Items WHERE name = ?");
    query.addBindValue(name);
    query.exec();
    if(query.next()) {
        QMessageBox::StandardButton answer;
        answer = QMessageBox::question(this, "Item already in database", "The item '" + name + "' is already present in the database. Do you want to create a new copy of it?");

        if(answer == QMessageBox::No) {
            return false;
        }
    }

    // Encontra o id do personagem no banco de dados Characters
    query.prepare("SELECT id FROM Characters WHERE name = ?");
    query.addBindValue(character);
    query.exec();
    query.first();
    charIndex = query.value(0).toInt();
    if(query.lastError().text() != "") {
        error = true;
    }

    // Insere o item no banco de dados Items
    query.prepare("INSERT INTO Items (name, weight, description) VALUES (?, ?, ?)");
    query.addBindValue(name);
    query.addBindValue(weight);
    query.addBindValue(description);
    query.exec();
    if(query.lastError().text() != "") {
        error = true;
    }

    // Encontra o id do item no banco de dados Items
    query.prepare("SELECT id FROM Items WHERE name = ?");
    query.addBindValue(name);
    query.exec();
    query.first();
    itemIndex = query.value(0).toInt();

    // Insere o id do personagem, o id do item, e a quantidade no banco de dados Inventory
    query.prepare("INSERT INTO Inventory VALUES (?, ?, ?)");
    query.addBindValue(charIndex);
    query.addBindValue(itemIndex);
    query.addBindValue(quantity);
    query.exec();
    if(query.lastError().text() != "") {
        error = true;
    }

    if(!error) {
        QMessageBox::about(this, "Success", "The item '" + name + "' was added successfully to '" + character + "'s inventory");
        return true;
    } else {
        QMessageBox::about(this, "Error", "Error when adding new item");
        return false;
    }
}

/*
    Insere um novo personagem no banco de dados. Caso um personagem com o mesmo nome já exista, é possível inserí-lo novamente ou cancelar a operação
    Entradas: name (nome do personagem), maxCarryWeight (capacidade máxima que o personagem aguenta carregar)
    Retorno: true, caso a inserção tenha sucesso. Caso contrário, retorna false
*/
bool MainWindow::insertNewCharacter(QString name, float maxCarryWeight) {
    QSqlQuery query;

    query.prepare("SELECT name FROM Characters WHERE name = ?");
    query.addBindValue(name);
    query.exec();
    if(query.next()) {
        QMessageBox::StandardButton answer;
        answer = QMessageBox::question(this, "Character already in database", "The character '" + name + "' is already present in the database. Do you want to create a new copy of it?");

        if(answer == QMessageBox::No) {
            return false;
        }
    }

    query.prepare("INSERT INTO Characters (name, maxCarryWeight) VALUES (?, ?)");
    query.addBindValue(name);
    query.addBindValue(maxCarryWeight);
    query.exec();
    if(query.lastError().text() == "") {
        QMessageBox::about(this, "Success", "The character '" + name + "' was added to the party successfully");
    }

    query.exec("SELECT name FROM Characters");
    ui->cb_characterNames->clear();
    while(query.next()) {
        ui->cb_characterNames->addItem(query.value(0).toString());
        qWarning() << query.value(0).toString();
    }

    return true;
}

/*
    Retorna uma lista com o nome de todos os personagens atualmente disponíveis
    Entrada: void
    Retorno: lista de strings com os nomes de todos os personagens disponíveis
*/
QList<QString> MainWindow::getCharacters() {
    QSqlQuery query;
    QList<QString> returnList;

    query.exec("SELECT name FROM Characters");
    while(query.next()) {
        returnList.append(query.value(0).toString());
    }

    return returnList;
}


/*
    Atualiza a tela principal com as informações do banco de dados
    Entrada: void
    Retorno: void
*/
void MainWindow::updateInventoryWindow() {
    QSqlQuery query;
    QString currentChar;

    // Atualiza a tabela com todos os itens do personagem
    currentChar = ui->cb_characterNames->currentText();
    QSqlQueryModel *model = new QSqlQueryModel;
    query.prepare("SELECT Items.name, Items.weight, Inventory.quantity, Items.description FROM Items, Characters, Inventory WHERE charID = Characters.id AND itemID = Items.id AND Characters.name = ?");
    query.addBindValue(currentChar);
    query.exec();
    model->setQuery(query);
    ui->tv_Inventory->setModel(model);
    ui->tv_Inventory->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Atualiza a caixa dizendo a capacidade atual e total de carga
    query.prepare("SELECT SUM(weight * quantity) FROM Items, Characters, Inventory WHERE charID = Characters.id AND itemID = Items.id AND Characters.name = ?");
    query.addBindValue(currentChar);
    query.exec();
    query.first();
    currentWeight = query.value(0).toFloat();

    query.prepare("SELECT maxCarryWeight FROM Characters WHERE name = ?");
    query.addBindValue(currentChar);
    query.exec();
    query.first();
    maxWeight = query.value(0).toFloat();

    ui->currentMaxWeight->setText(QString::number(currentWeight) + "/" + QString::number(maxWeight) + " kg");

    // Atualiza a caixa com a porcentagem atual de carga
    ui->currentPercentage->setText(QString::number((100 / maxWeight) * currentWeight, 'F', 2) + "%");
}

/*
    About Qt (botão do menu)
*/
void MainWindow::MenuAboutQtPressed() {
    QMessageBox::aboutQt(this);
}

/*
    Adicionar novo item (botão da tela principal)
*/
void MainWindow::on_pushButton_clicked() {
    CreateItemDialog itemDialog;
    itemDialog.exec();
}


/*
    Adicionar novo personagem (botão do menu)
*/
void MainWindow::on_actionAdd_Character_triggered()
{
    CreateCharacterDialog charDialog;
    charDialog.setWindowTitle("Add New Character");
    charDialog.exec();
}

/*
    Quando o personagem atual é trocado (ComboBox da tela principal)
*/
void MainWindow::on_cb_characterNames_currentIndexChanged(int index) {
    updateInventoryWindow();
}

/*
    Adiciona 1 unidade do item selecionado (botão Add 1 da tela principal)
*/
void MainWindow::on_pushButton_3_clicked() {
    int row;
    QString itemName;
    QString currentChar;

    // Encontra o nome do item atualmente selecionado
    row = ui->tv_Inventory->selectionModel()->selectedRows().at(0).row();
    itemName = ui->tv_Inventory->model()->index(row, 0).data().toString();

    // Encontra o nome do personagem atualmente selecionado
    currentChar = ui->cb_characterNames->currentText();

    // Adiciona 1 unidade do item do inventário do personagem
    addOrRemoveItems(itemName, currentChar, 1);
}

/*
    Remove 1 unidade do item selecionado (botão Remove 1 da tela principal)
*/
void MainWindow::on_btn_removeOne_clicked()
{
    int row;
    QString itemName;
    QString currentChar;

    // Encontra o nome do item atualmente selecionado
    row = ui->tv_Inventory->selectionModel()->selectedRows().at(0).row();
    itemName = ui->tv_Inventory->model()->index(row, 0).data().toString();

    // Encontra o nome do personagem atualmente selecionado
    currentChar = ui->cb_characterNames->currentText();

    // Remove 1 unidade do item do inventário do personagem
    addOrRemoveItems(itemName, currentChar, -1);
}

void MainWindow::on_btn_addX_clicked()
{
    bool ok;
    int i;
    int row;
    QString itemName;
    QString currentChar;
    QSqlQuery query;

    // Encontra o nome do item atualmente selecionado
    row = ui->tv_Inventory->selectionModel()->selectedRows().at(0).row();
    itemName = ui->tv_Inventory->model()->index(row, 0).data().toString();

    // Encontra o nome do personagem atualmente selecionado
    currentChar = ui->cb_characterNames->currentText();

    // Pergunta ao usuário a quantidade a ser adiciona
    i = QInputDialog::getInt(this, "Add Multiple", "Enter quantity of '" + itemName + "' to add", 0, 0, 2147483647, 1, &ok);

    // Adiciona i unidades do item ao inventário do personagem
    if(ok) {
        addOrRemoveItems(itemName, currentChar, i);
    }
}

/*
    Adiciona ou remove items do inventário de um personagem
    Entrada: itemName (nome do item a ser alterado), charName (nome do personagem dono do inventário), quantity (valor a ser somado ou subtraído da quantidade de itens)
    Retorno: true, caso a alteração tenha sucesso. Caso contrário, retorna false
*/
bool MainWindow::addOrRemoveItems(QString itemName, QString charName, int quantity) {
    bool error = false;
    int itemID;
    int charID;
    QSqlQuery query;

    // Encontra o id do personagem selecionado na tabela Characters
    query.prepare("SELECT id FROM Characters WHERE name = ?");
    query.addBindValue(charName);
    query.exec();
    query.first();
    charID = query.value(0).toInt();
    if(query.lastError().text() != "")
        error = true;

    // Encontra o id do item na tabela Items
    query.prepare("SELECT id FROM Items WHERE name = ?");
    query.addBindValue(itemName);
    query.exec();
    query.first();
    itemID = query.value(0).toInt();
    if(query.lastError().text() != "")
        error = true;

    //Atualiza a tabela Inventory com a nova quantidade (quantidade antiga + quantity)
    query.prepare("UPDATE Inventory SET quantity = quantity + ? WHERE itemID = ? AND charID = ?");
    query.addBindValue(quantity);
    query.addBindValue(itemID);
    query.addBindValue(charID);
    query.exec();
    if(query.lastError().text() != "")
        error = true;

    if(!error)
        return true;
    else
        return false;
}
