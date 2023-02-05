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

#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QLockFile>

#include "Constants.h"
#include "MainWindow.h"
#include "SingleApp.h"
#include "Utils.h"
#include "Logger.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Constants::kAppPath = QApplication::applicationDirPath();
    Constants::kAppInternalPath = Constants::kAppPath + "/" + Constants::kAppInternalRelativePath;
    qDebug() << "Constants::kAppInternalPath: " << Constants::kAppInternalPath;
    (void)Utils::mkdir(Constants::kAppInternalPath);
    Q_INIT_RESOURCE(QEditor);
#ifdef OUTPUT_LOG
    qInstallMessageHandler(OutputMessageOutput);
#endif

    QCoreApplication::setOrganizationName(Constants::kOrgName);
    QCoreApplication::setApplicationName(Constants::kAppName);
    QCoreApplication::setApplicationVersion(Constants::kVersionStr);
    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::applicationName());
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", "The file to open.");
    parser.process(app);
    QString filePath;
    if (!parser.positionalArguments().isEmpty()) {
        filePath = parser.positionalArguments().first();
        qDebug() << "filePath: " << filePath;
    }

    // Single run check.
    SingleApp singleApp;
    if (!singleApp.TryRun(filePath)) {
        qDebug() << "Already run.";
        return 0;
    }

    // Double single run check with lock file.
    QString singleRunFilePath(Constants::kAppInternalPath);
    singleRunFilePath.append(Constants::kAppInternalSingleRunFile);
    QLockFile singleRunLockFile(singleRunFilePath);
    if (!singleRunLockFile.tryLock(1000)) {
        qCritical() << "Already run, lock error: " << singleRunLockFile.error();
        QMessageBox::warning(nullptr, QString(Constants::kAppName),
                             QString("Cannot read file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(singleRunFilePath), singleRunLockFile.error()));
        return 0;
    }
    qDebug() << "start running";

    if (!filePath.isEmpty() && QFileInfo(filePath).isFile()) {
        MainWindow::Instance().tabView()->OpenFile(filePath);
    }

    MainWindow::Instance().setWindowIcon(QIcon(":/images/QEditorIcon.webp"));
    // MainWindow::Instance().show();
    MainWindow::Instance().showMaximized();
    return app.exec();
}
