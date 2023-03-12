#ifndef DIFF_H
#define DIFF_H

#include "diff_match_patch.h"

#include <QTextCharFormat>

using _Diff = Diff;

namespace QEditor {
class FormattedText {
public:
    FormattedText() = default;
    FormattedText(const QString &txt, const QTextCharFormat &fmt = QTextCharFormat())
        : text(txt), format(fmt) {}

    QString text;
    QTextCharFormat format;

    bool operator==(const FormattedText &other) const { return text == other.text && format == other.format; }
};

class Diff
{
public:
    Diff();

    void Impose(const QString &before, const QString &after);

    QString ToHtml();
    QString ToLineHtml();
    QList<FormattedText> ToFormattedText();

private:
    diff_match_patch diffMatchPatch_;
    QList<_Diff> diffs_;
};
}  // namespace QEditor

#endif // DIFF_H
