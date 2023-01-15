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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>

class Settings final
{
public:
    Settings();
    ~Settings() {
        delete settings_;
    }

    void Set(const QString &nodeName, const QString &keyName, const QVariant &var)
    {
        settings_->setValue(QString("/%1/%2").arg(nodeName).arg(keyName), var);
    }

    QVariant Get(const QString &nodeName, const QString &keyName)
    {
        QVariant var = settings_->value(QString("/%1/%2").arg(nodeName).arg(keyName));
        return var;
    }

private:
    QString settingsFile_;
    QSettings *settings_{nullptr};
};

#endif // SETTINGS_H
