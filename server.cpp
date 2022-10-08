#include "server.h"
#include "QDebug"

Server::Server()
{
    if (this->listen(QHostAddress::Any, 5151)) {
       qDebug() << "Server has started";
    }
    else {
       qDebug() << "Server startup error";
    }

    blockSize = 0;
}

void Server::sendToClient(QString msg, qintptr senderDesc, qintptr recipientDesc)
{

    if (recipientDesc != NULL) { // Передача сообщения

        QTcpSocket* addressSocket = sockets.find(recipientDesc).value();
        writeDataToSocket(addressSocket, false, msg);

    }
    else { // Пополнение списка адресатов (для всех клиентов)

        foreach (QTcpSocket* sck, sockets.values()) {

            if (senderDesc == sck->socketDescriptor()) { // Для нового клиента добавить весь список клиентов

                foreach (QTcpSocket* tempSck, sockets.values()) {

                    if (senderDesc != tempSck->socketDescriptor()) { // Клиент сам себе не может отправлять сообщения
                        writeDataToSocket(sck, true, QString::number(tempSck->socketDescriptor()));
                    }

                }
            }
            else { // Для существующих клиентов только добавить нового
                writeDataToSocket(sck, true, msg);
            }
        }

    }

}

void Server::writeDataToSocket(QTcpSocket* sck, bool typeInfo, QString msg) // typeInfo = false - отправка сообщения, true - передача списка адресатов
{
    data.clear();
    QDataStream output(&data, QIODevice::WriteOnly);
    output.setVersion(QDataStream::Qt_6_3);

    output << quint16(0) << quint64(0) << typeInfo << msg; // Выделить место под размер сообщения в начале блока данных
    output.device()->seek(0); // Переместиться в начало блока данных
    output << quint16(data.size() - sizeof(quint16)); // Записать размер сообщения

    sck->write(data);
}

void Server::incomingConnection(qintptr descriptor)
{
    socket = new QTcpSocket();
    socket->setSocketDescriptor(descriptor);
    connect(socket, &QTcpSocket::readyRead, this, &Server::slotReadyRead);
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);

    sockets.insert(socket->socketDescriptor(), socket);

    QString socketDescStr = QString::number(socket->socketDescriptor());

    sendToClient(socketDescStr, descriptor, NULL);

    qDebug() << descriptor << " - connection has completed";
}

void Server::slotReadyRead()
{
    socket = (QTcpSocket*)sender();
    QDataStream input(socket);
    input.setVersion(QDataStream::Qt_6_3);

    if (input.status() == QDataStream::Ok) {
        qDebug() << socket->socketDescriptor() << " - Reading...";

        while (true) {

            if (blockSize == 0) { // Проверка наличия размера блока данных

                if (socket->bytesAvailable() < 2) {
                    qDebug() << "Block size < 2";
                    break;
                }

                input >> blockSize; // Запись размера блока данных из потока в переменную
                qDebug() << "Block size = " << blockSize;
            }

            if (socket->bytesAvailable() < blockSize) { // Проверка, имеются ли в потоке данные заявленного размера
                qDebug() << "Data isn't full";
                break;
            }


            quint64 recipientDesc;
            QString msg;
            input >> recipientDesc;
            input >> msg;

            blockSize = 0;
            sendToClient(msg, socket->socketDescriptor(), recipientDesc);

        }

    }
    else {
        qDebug() << "Read error";
    }

}
