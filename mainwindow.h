#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    bool insertNewItem(QString name, float weight, QString description, int quantity, QString character);
    bool insertNewCharacter(QString name, float maxCarryWeight);
    QList<QString> getCharacters();
    void updateInventoryWindow();
    ~MainWindow();    

private:
    Ui::MainWindow *ui;
    float currentWeight;
    float maxWeight;
    bool addOrRemoveItems(QString itemName, QString charName, int quantity);

private slots:
    void MenuAboutQtPressed();
    void on_pushButton_clicked();
    void on_actionAdd_Character_triggered();
    void on_cb_characterNames_currentIndexChanged(int index);
    void on_pushButton_3_clicked();
    void on_btn_removeOne_clicked();
    void on_btn_addX_clicked();
};
#endif // MAINWINDOW_H
