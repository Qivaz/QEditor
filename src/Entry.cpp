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

#include "Constants.h"
#include "Logger.h"
#include "MainWindow.h"
#include "SingleApp.h"
#include "Utils.h"
#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QLockFile>
#include <QTranslator>

// Q_IMPORT_PLUGIN(QIbusPlatformInputContextPlugin)
// Q_IMPORT_PLUGIN(StaticQIbusPlatformInputContextPluginInstance)

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QEditor::Constants::kAppPath = QApplication::applicationDirPath();
    QEditor::Constants::kAppInternalPath =
        QEditor::Constants::kAppPath + "/" + QEditor::Constants::kAppInternalRelativePath;
    qDebug() << "Constants::kAppInternalPath: " << QEditor::Constants::kAppInternalPath;
    (void)QEditor::Utils::mkdir(QEditor::Constants::kAppInternalPath);
    Q_INIT_RESOURCE(QEditor);
#ifdef OUTPUT_LOG
    qInstallMessageHandler(OutputMessageOutput);
#endif

    qDebug() << "currentPath: " << QDir::currentPath();
    QTranslator translator;
    translator.load("zh_CN");
    app.installTranslator(&translator);

    QCoreApplication::setOrganizationName(QEditor::Constants::kOrgName);
    QCoreApplication::setApplicationName(QEditor::Constants::kAppName);
    QCoreApplication::setApplicationVersion(QEditor::Constants::kVersionStr);
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
    QEditor::SingleApp singleApp;
    if (!singleApp.TryRun(filePath)) {
        qDebug() << "Already run.";
        return 0;
    }

    // Double single run check with lock file.
    QString singleRunFilePath(QEditor::Constants::kAppInternalPath);
    singleRunFilePath.append(QEditor::Constants::kAppInternalSingleRunFile);
    QLockFile singleRunLockFile(singleRunFilePath);
    if (!singleRunLockFile.tryLock(1000)) {
        qCritical() << "Already run, lock error: " << singleRunLockFile.error();
        QMessageBox::warning(nullptr, QString(QEditor::Constants::kAppName),
                             QString(QObject::tr("Cannot read file %1:\n%2."))
                                 .arg(QDir::toNativeSeparators(singleRunFilePath), singleRunLockFile.error()));
        return 0;
    }
    qDebug() << "start running";

    if (!filePath.isEmpty() && QFileInfo(filePath).isFile()) {
        QEditor::MainWindow::Instance().tabView()->OpenFile(filePath);
    }

    QEditor::MainWindow::Instance().setWindowIcon(QIcon(":/images/QEditorIcon.webp"));
    //    QEditor::MainWindow::Instance().show();
    QEditor::MainWindow::Instance().showMaximized();
    //    app.setQuitOnLastWindowClosed(false);
    return app.exec();
}
