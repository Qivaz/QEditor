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

#ifndef LOGGER_H
#define LOGGER_H

// clang-format off
#include <QDebug>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QMutex>

#if defined (Q_OS_LINUX)
#include <unistd.h>
#endif
#include <thread>
#include <sstream>

#include "Constants.h"
#include "Utils.h"
// clang-format on

#define OUTPUT_LOG
//#define OUTPUT_LOG_FILE

#ifdef OUTPUT_LOG
#undef qDebug
#undef qInfo
#undef qDebug
#undef qCritical
#undef qFatal

#define qDebug QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC).debug
#define qInfo QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC).info
#define qWarning QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC).warning
#define qCritical QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC).critical
#define qFatal QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC).fatal

#ifdef OUTPUT_LOG_FILE
static inline QString GetLogsPath() {
    static QString logPath(Constants::kAppInternalPath);
    static bool init = false;
    if (init) {
        return logPath;
    }

    logPath.append(Constants::kAppInternalLogsRelativePath);
    (void)Utils::mkdir(logPath);
    logPath.append("QEditor_");
    auto time = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");
    logPath.append(time);
    logPath.append(".log");
    logPath = QDir::toNativeSeparators(logPath);
    init = true;
    return logPath;
}
#endif

static std::string GetProcName() {
#if defined(__APPLE__) || defined(__FreeBSD__)
    const std::string appname = getprogname();
#elif defined(_GNU_SOURCE)
    const std::string appname = program_invocation_name;
#else
    const std::string appname = "?";
#endif
    // Sometimes, the app name is an absolute path, it is too long
    std::string app_name(appname);
    std::size_t pos = app_name.rfind("/");
    if (pos == std::string::npos) {
        return app_name;
    }
    if (pos + 1 >= app_name.size()) {
        return app_name;
    }
    return app_name.substr(pos + 1);
}

static inline void OutputMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
    QTextStream cin(stdin, QIODevice::ReadOnly);
    QTextStream cout(stdout, QIODevice::WriteOnly);
    QTextStream cerr(stderr, QIODevice::WriteOnly);
    QString text;
    QByteArray localMsg = msg.toLocal8Bit();
    std::stringstream ss;
#if defined(Q_OS_LINUX)
    ss << "(" << getpid() << ":" << std::this_thread::get_id() << "," << GetProcName() << ")";
#else
    ss << "(" << std::this_thread::get_id() << "," << GetProcName() << ")";
#endif
    QString pidInfo = QString::fromStdString(ss.str());
    const auto& currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    switch (type) {
    case QtDebugMsg:
        return;
        text = QString("[DEBUG] %1:%2 [%3:%4@%5] %6\n")
                   .arg(pidInfo)
                   .arg(currentTime)
                   .arg(context.file)
                   .arg(context.line)
                   .arg(context.function)
                   .arg(localMsg.constData());
        cout << text;
        break;
    case QtInfoMsg:
        return;
        text = QString("[INFO] %1:%2 [%3:%4@%5] %6\n")
                   .arg(pidInfo)
                   .arg(currentTime)
                   .arg(context.file)
                   .arg(context.line)
                   .arg(context.function)
                   .arg(localMsg.constData());
        cout << text;
        break;
    case QtWarningMsg:
        return;
        text = QString("[WARNING] %1:%2 [%3:%4@%5] %6\n")
                   .arg(pidInfo)
                   .arg(currentTime)
                   .arg(context.file)
                   .arg(context.line)
                   .arg(context.function)
                   .arg(localMsg.constData());
        cout << text;
        break;
    case QtCriticalMsg:
        text = QString("[CRITIAL] %1:%2 [%3:%4@%5] %6\n")
                   .arg(pidInfo)
                   .arg(currentTime)
                   .arg(context.file)
                   .arg(context.line)
                   .arg(context.function)
                   .arg(localMsg.constData());
        cerr << text;
        break;
    case QtFatalMsg:
        text = QString("[FATAL] %1:%2 [%3:%4@%5] %6\n")
                   .arg(pidInfo)
                   .arg(currentTime)
                   .arg(context.file)
                   .arg(context.line)
                   .arg(context.function)
                   .arg(localMsg.constData());
        cerr << text;
        break;
    default:
        break;
    }
#ifdef OUTPUT_LOG_FILE
    const auto& logPath = GetLogsPath();
    QFile file(logPath);
    static QMutex mutex;
    mutex.lock();
    if (file.open(QIODevice::WriteOnly | QIODevice::Append)) {
        qDebug() << "Open file success, " << logPath << ", " << QDir::toNativeSeparators(logPath);
        QTextStream fileStream(&file);
        fileStream << text;
        file.flush();
        file.close();
        mutex.unlock();
    } else {
        qDebug() << "Open file failed, " << logPath << ", " << QDir::toNativeSeparators(logPath) << ", "
                 << file.errorString();
        QMessageBox::warning(
            nullptr, QString(Constants::kAppName),
            QString("Cannot read file %1:\n%2.").arg(QDir::toNativeSeparators(logPath), file.errorString()));
        mutex.unlock();
        exit(-1);
    }
#endif
}
#else
#undef qDebug
#undef qInfo
#undef qDebug
//#undef qCritical
//#undef qFatal

#define qDebug QT_NO_QDEBUG_MACRO
#define qInfo QT_NO_QDEBUG_MACRO
#define qDebug QT_NO_QDEBUG_MACRO
//#define qCritical QT_NO_QDEBUG_MACRO
//#define qFatal QT_NO_QDEBUG_MACRO
#endif
#endif // LOGGER_H
