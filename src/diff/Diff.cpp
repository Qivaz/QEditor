#include "Diff.h"

namespace QEditor {
Diff::Diff() {}

void Diff::Impose(const QString &before, const QString &after) {
    diffs_ = diffMatchPatch_.diff_main(before, after);
}

QString Diff::ToHtml() {
    return diffMatchPatch_.diff_prettyHtml(diffs_);
}
}  // namespace QEditor
