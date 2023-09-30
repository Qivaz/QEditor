#ifndef DIFF_H
#define DIFF_H

#include "diff_match_patch_stl.h"
#include <QTextCharFormat>

using _DiffMatchPatch = diff_match_patch<std::string>;
using _Diff = diff_match_patch<std::string>::Diff;
using _Diffs = std::list<_Diff>;

namespace QEditor {
class FormattedText {
   public:
    FormattedText() = default;
    FormattedText(const QString &txt, const QTextCharFormat &fmt = QTextCharFormat()) : text_(txt), format_(fmt) {}

    QString text_;
    QTextCharFormat format_;

    bool operator==(const FormattedText &other) const { return text_ == other.text_ && format_ == other.format_; }
};

class Diff {
   public:
    Diff();

    void Impose(const QString &before, const QString &after);

    QString ToHtml();
    QString ToLineHtml();
    QList<FormattedText> ToFormattedText();

    inline static const QColor kOldFgColor = QColor(Qt::red);
    inline static const QColor kNewFgColor = QColor(Qt::green);
    inline static const QColor kCommonFgColor = QColor(Qt::darkGray);

    inline static const QColor kOldBgColor = QColor("#ffe6e6");
    inline static const QColor kNewBgColor = QColor("#e6ffe6");

   private:
    _DiffMatchPatch diffMatchPatch_;
    _Diffs diffs_;
};
}  // namespace QEditor

#endif  // DIFF_H
