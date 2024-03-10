#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    //Исходное состояние виджетов
    ui->setupUi(this);
    ui->lb_statusConnect->setStyleSheet("color:red");
    ui->pb_request->setEnabled(false);

    /*
     * Выделим память под необходимые объекты. Все они наследники
     * QObject, поэтому воспользуемся иерархией.
    */
    dataDb = new DbData(this);
    dataBase = new DataBase(this);
    msg = new QMessageBox(this);

    //Установим размер вектора данных для подключения к БД
    dataForConnect.resize(NUM_DATA_FOR_CONNECT_TO_DB);

    /*
     * Добавим БД используя стандартный драйвер PSQL и зададим имя.
    */
    dataBase->AddDataBase(POSTGRE_DRIVER, DB_NAME);

    /*
     * Устанавливаем данные для подключениея к БД.
     * Поскольку метод небольшой используем лямбда-функцию.
     */
    connect(dataDb, &DbData::sig_sendData, this, [&](QVector<QString> receivData){
        dataForConnect = receivData;
    });

    /*
     *  Сигнал для подключения к БД
     */
    connect(dataBase, &DataBase::sig_SendStatusConnection, this, &MainWindow::ReceiveStatusConnectionToDB);
    connect(dataBase, &DataBase::sig_SendStatusRequest, this, &MainWindow::ReceiveStatusRequestToDB);

}

MainWindow::~MainWindow()
{
    delete ui;
}

/*!
 * @brief Слот отображает форму для ввода данных подключения к БД
 */
void MainWindow::on_act_addData_triggered()
{
    //Отобразим диалоговое окно. Какой метод нужно использовать?
    dataDb->show();
}

/*!
 * @brief Слот выполняет подключение к БД. И отображает ошибки.
 */

void MainWindow::on_act_connect_triggered()
{
    /*
     * Обработчик кнопки у нас должен подключаться и отключаться от БД.
     * Можно привязаться к надписи лейбла статуса. Если он равен
     * "Отключено" мы осуществляем подключение, если "Подключено" то
     * отключаемся
    */

    if(ui->lb_statusConnect->text() == "Отключено"){

       ui->lb_statusConnect->setText("Подключение");
       ui->lb_statusConnect->setStyleSheet("color : black");
       auto conn = [&]{dataBase->ConnectToDataBase(dataForConnect);};
       QtConcurrent::run(conn);

    }
    else{
        dataBase->DisconnectFromDataBase(DB_NAME);
        ui->lb_statusConnect->setText("Отключено");
        ui->act_connect->setText("Подключиться");
        ui->lb_statusConnect->setStyleSheet("color:red");
        ui->pb_request->setEnabled(false);
    }
}
/*!
 * \brief Обработчик кнопки "Получить"
 */
void MainWindow::on_pb_request_clicked()
{
    dataBase->sqlTableModel->clear();
    dataBase->sqlQueryModel->clear();
    ui->pb_request->setEnabled(false);
    ui->pb_clear->setEnabled(false);
    if(!boolGenres)
    {
        QtConcurrent::run(
            [&]
            {
                dataBase->RequestToDB(requests.allGenres, requestAllFilms);
                ui->pb_request->setEnabled(true);
                ui->pb_clear->setEnabled(true);
            });
    }else if((boolGenres >> 0) & 1)
    {
        QtConcurrent::run(
            [&]
            {
                dataBase->RequestToDB(requests.ComedyGenre, requestComedy);
                ui->pb_request->setEnabled(true);
                ui->pb_clear->setEnabled(true);
            });
    }else if((boolGenres >> 1) & 1)
    {
        QtConcurrent::run(
            [&]
            {
                dataBase->RequestToDB(requests.HorrorGenre, requestHorrors);
                ui->pb_request->setEnabled(true);
                ui->pb_clear->setEnabled(true);
            });
    }
}
/*!
 * \brief Метод изменяет стотояние формы в зависимости от статуса подключения к БД
 * \param status
 */
void MainWindow::ReceiveStatusConnectionToDB(bool status)
{
    if(status){
        ui->act_connect->setText("Отключиться");
        ui->lb_statusConnect->setText("Подключено к БД");
        ui->lb_statusConnect->setStyleSheet("color:green");
        ui->pb_request->setEnabled(true);
    }
    else{
        dataBase->DisconnectFromDataBase(DB_NAME);
        msg->setIcon(QMessageBox::Critical);
        msg->setText(dataBase->GetLastError().text());
        ui->lb_statusConnect->setText("Отключено");
        ui->lb_statusConnect->setStyleSheet("color:red");
        msg->exec();
    }

}

void MainWindow::ReceiveStatusRequestToDB(QSqlError err,int typeRequest)
{
    if(err.type() != QSqlError::NoError){
        msg->setIcon(QMessageBox::Critical);
        msg->setText(err.text());
        msg->exec();
        return;
    }

    if (typeRequest == requestAllFilms)
    {
        ui->tv_result->setModel(dataBase->sqlTableModel);
        ui->tv_result->hideColumn(0);
    }
    else
    {
        ui->tv_result->setModel(dataBase->sqlQueryModel);
    }

    ui->pb_clear->setEnabled(true);
}


void MainWindow::on_cb_category_currentIndexChanged(int index)
{
    boolGenres = 0;
    switch (index) {
    case 0:
        boolGenres = index;
        break;
    case 1:
        boolGenres = boolGenres | (1 << (index - 1));
        break;
    case 2:
        boolGenres = boolGenres | (1 << (index - 1));
        break;
    }
}


void MainWindow::on_pb_clear_clicked()
{
    dataBase->sqlTableModel->clear();
    dataBase->sqlQueryModel->clear();
    ui->tv_result->update();
    ui->pb_clear->setEnabled(false);
}

