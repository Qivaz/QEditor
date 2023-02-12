#ifndef DIFF_H
#define DIFF_H

#include "diff_match_patch.h"

using _Diff = Diff;

namespace QEditor {
class Diff
{
public:
    Diff();

    void Impose(const QString &before, const QString &after);

    QString ToHtml();

private:
    diff_match_patch diffMatchPatch_;
    QList<_Diff> diffs_;
};
}  // namespace QEditor

#endif // DIFF_H
