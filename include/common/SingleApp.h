/**
 * Copyright 2022 QEditor QH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SINGLEAPP_H
#define SINGLEAPP_H

#include <QFile>
#include <QLocalSocket>
#include <QLocalServer>

#include "Constants.h"
#include "MainWindow.h"
#include "Toast.h"

class SingleAppServer : public QObject
{
    Q_OBJECT
public:
    SingleAppServer() : server_(new QLocalServer()) {
        connect(server_, SIGNAL(newConnection()), this, SLOT(HandleNewConnection()));
    }

    ~SingleAppServer()
    {
        server_->close();
        delete server_;
        qDebug() << "Server removed.";
    }

    void RunServer()
    {
        if (QLocalServer::removeServer(Constants::kSingleAppHostName)) {
            qDebug() << "Remove server failure.";
        }
        bool success = server_->listen(Constants::kSingleAppHostName);
        if (!success) {
            qDebug() << "Listen failure.";
            return;
        }
        if (!server_->isListening()) {
            qDebug() << "Listen failure.";
            return;
        }
        qDebug() << "Run server success.";
     }

public slots:
    void HandleNewConnection()
    {
        qDebug() << "New Connection.";
        qDebug() << "listen: " << server_->serverName();
        qDebug() << "hasPendingConnections: " << server_->hasPendingConnections();

        QLocalSocket* socket = server_->nextPendingConnection();
        connect(socket, SIGNAL(readyRead()), this, SLOT(HandleReadyRead()));
        connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));
    }

    void HandleReadyRead()
    {
        QLocalSocket* socket = static_cast<QLocalSocket*>(sender());
        if (socket == nullptr) {
            qDebug() << "socket is null.";
            return;
        }
        QTextStream stream(socket);
        QString text = stream.readAll();
        qDebug() << "Read data from client: " << text;
        QFileInfo fileInfo(text);
        if (fileInfo.isFile()) {
            qDebug() << "Open file: " << fileInfo.filePath();
            MainWindow::Instance().show();
            MainWindow::Instance().tabView()->OpenFile(fileInfo.filePath());
        } else {
            qDebug() << "Fail to open file: " << fileInfo.filePath();
            Toast::Instance().Show(Toast::kError, QString("Can't open %1").arg(fileInfo.filePath()));
        }

        QString response = "FIN";
        socket->write(response.toUtf8());
        socket->flush();
    }

private:
    QLocalServer *server_;

};

class SingleAppClient : public QObject
{
    Q_OBJECT
public:
    SingleAppClient() : socket_(new QLocalSocket())
    {
        connect(socket_, SIGNAL(connected()), SLOT(HandleConnected()));
        connect(socket_, SIGNAL(disconnected()), SLOT(HandleDisConnected()));
        connect(socket_, SIGNAL(error(QLocalSocket::LocalSocketError)), SLOT(HandleError(QLocalSocket::LocalSocketError)));
    }

    ~SingleAppClient()
    {
        socket_->disconnectFromServer();
        delete socket_;
    }

public:
    bool ConnectToServer(const QString &message, const QString &serverName = Constants::kSingleAppHostName)
    {
        socket_->connectToServer(serverName);
        if (socket_->waitForConnected()) {
            qDebug() << "connect server success.";
            SendMessage(message);
            return true;
        } else {
            qDebug() << "connect server failure.";
            return false;
        }
    }

private:
    void SendMessage(const QString &msg)
    {
        // To support unicode chars, should not use 'msg.toStdString().c_str()'
        socket_->write(msg.toLocal8Bit());
        socket_->flush();

        if (!socket_->bytesAvailable()) {
            socket_->waitForReadyRead();
        }

        QTextStream stream(socket_);
        QString respond = stream.readAll();
        qDebug() << "Read data from server: " << respond;
    }

private slots:
    void HandleConnected()
    {
        qDebug() << "connected.";
    }


    void HandleDisConnected()
    {
        qDebug() << "disconnected.";
    }

    void HandleError(QLocalSocket::LocalSocketError error)
    {
        qDebug() << error;
    }

private:
    QLocalSocket *socket_;

};

class SingleApp : public QObject
{
    Q_OBJECT
public:
    SingleApp() = default;
    ~SingleApp() = default;

    bool TryRun(const QString &filePath)
    {
        // Already running.
        if (client_.ConnectToServer(filePath.isEmpty() ? "[NO_FILE]" : filePath)) {
            return false;
        }
        // Start a server.
        server_.RunServer();
        return true;
    }

private:
    SingleAppClient client_;
    SingleAppServer server_;
};

#endif // SINGLEAPP_H
