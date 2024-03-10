#include "database.h"

DataBase::DataBase(QObject *parent)
    : QObject{parent}
{
    dataBase = new QSqlDatabase();
    sqlQueryModel = new QSqlQueryModel();
}

DataBase::~DataBase()
{
    delete dataBase;
    delete sqlQueryModel;
    delete sqlTableModel;
}

/*!
 * \brief Метод добавляет БД к экземпляру класса QSqlDataBase
 * \param driver драйвер БД
 * \param nameDB имя БД (Если отсутствует Qt задает имя по умолчанию)
 */
void DataBase::AddDataBase(QString driver, QString nameDB)
{
    *dataBase = QSqlDatabase::addDatabase(driver, nameDB);
    sqlTableModel = new QSqlTableModel(this,*dataBase);
}

/*!
 * \brief Метод подключается к БД
 * \param для удобства передаем контейнер с данными необходимыми для подключения
 * \return возвращает тип ошибки
 */
void DataBase::ConnectToDataBase(QVector<QString> data)
{

    dataBase->setHostName(data[hostName]);
    dataBase->setDatabaseName(data[dbName]);
    dataBase->setUserName(data[login]);
    dataBase->setPassword(data[pass]);
    dataBase->setPort(data[port].toInt());

    bool status;
    status = dataBase->open( );
    emit sig_SendStatusConnection(status);

}
/*!
 * \brief Метод производит отключение от БД
 * \param Имя БД
 */
void DataBase::DisconnectFromDataBase(QString nameDb)
{
    *dataBase = QSqlDatabase::database(nameDb);
    dataBase->close();
}
/*!
 * \brief Метод формирует запрос к БД.
 * \param request - SQL запрос
 * \return
 */
void DataBase::RequestToDB(const QString& request, const int& requestType)
{
    QSqlError err;
    if(requestType == requestAllFilms)
    {
        sqlTableModel->setTable("film");
        sqlTableModel->removeColumns(3,11);
        sqlTableModel->select();
        sqlTableModel->setHeaderData(1, Qt::Horizontal, tr("Название"));
        sqlTableModel->setHeaderData(2, Qt::Horizontal, tr("Описание"));
        if (sqlTableModel->lastError().isValid())
        {
            err = sqlTableModel->lastError();
        }
    }
    else
    {
        sqlQueryModel->setQuery(request, *dataBase);
        sqlQueryModel->setHeaderData(0, Qt::Horizontal, tr("Название"));
        sqlQueryModel->setHeaderData(1, Qt::Horizontal, tr("Описание"));
        if (sqlQueryModel->lastError().isValid())
        {
            err = sqlQueryModel->lastError();
        }
    }
    emit sig_SendStatusRequest(std::move(err), requestType);
}

/*!
 * @brief Метод возвращает последнюю ошибку БД
 */
QSqlError DataBase::GetLastError()
{
    return dataBase->lastError();
}
