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

#include <QApplication>
#include <QFile>
#include <QLocalSocket>
#include <QLocalServer>
#include <QWindow>

#include "Constants.h"
#include "MainWindow.h"
#include "Toast.h"

namespace QEditor {
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

    void ShowWindow() {
#if defined(Q_OS_WIN)
        // https://stackoverflow.com/questions/6087887/bring-window-to-front-raise-show-activatewindow-don-t-work
        // I don't know why have to use two steps.
        // Step 1:
        auto eFlags = MainWindow::Instance().windowFlags();
        MainWindow::Instance().setWindowFlags(eFlags|Qt::WindowStaysOnTopHint);
        MainWindow::Instance().show();
        MainWindow::Instance().setWindowFlags(eFlags);
        MainWindow::Instance().show();
        // Step 2:
        for ( QWindow* appWindow : QGuiApplication::allWindows() )
        {
          appWindow->show(); // Bring window to top on OSX
          appWindow->raise(); // Bring window from minimized state on OSX

          appWindow->requestActivate(); // Bring window to front/unminimize on windows.
        }
#else // defined(Q_OS_LINUX) or defined(Q_OS_OSX)
        // Not work in Windows, only blink on taskbar.
         MainWindow::Instance().raise();  // Mac OS
         MainWindow::Instance().activateWindow(); // Linux OS
#endif
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
            qDebug() << "Open file: " << fileInfo.canonicalFilePath();
            ShowWindow();

            MainWindow::Instance().tabView()->OpenFile(fileInfo.canonicalFilePath());
        } else {
            qDebug() << "Fail to open file: " << fileInfo.canonicalFilePath();
            ShowWindow();

            // Toast::Instance().Show(Toast::kError, QString("Can't open %1").arg(fileInfo.canonicalFilePath()));
            Toast::Instance().Show(Toast::kError, QString("Can't open multiple window."));
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
}  // namespace QEditor

#endif // SINGLEAPP_H
