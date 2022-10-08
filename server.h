#ifndef SERVER_H
#define SERVER_H

#include "QTcpServer"
#include "QTcpSocket"
#include "QMap"

class Server : public QTcpServer
{

    Q_OBJECT

public:
    Server();
    QTcpSocket* socket;

private:
    QMap<qintptr,QTcpSocket*> sockets;
    QByteArray data;
    void sendToClient(QString, qintptr, qintptr);
    quint16 blockSize;
    void writeDataToSocket(QTcpSocket*, bool, QString);

public slots:
    void incomingConnection(qintptr);
    void slotReadyRead();

};

#endif // SERVER_H
