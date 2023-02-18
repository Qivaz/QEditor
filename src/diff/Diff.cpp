#include "Diff.h"

namespace QEditor {
Diff::Diff() {}

void Diff::Impose(const QString &before, const QString &after) {
    diffs_ = diffMatchPatch_.diff_main(before, after);
}

QString Diff::ToHtml() {
    return diffMatchPatch_.diff_prettyHtml(diffs_);
}

QString Diff::ToLineHtml() {
    QString html;
    QString oldHtml;
    QString newHtml;
    QString text;
    constexpr auto html_line_feed = "&para;<br>";
    const auto html_line_feed_len = strlen(html_line_feed);
    foreach(_Diff aDiff, diffs_) {
        text = aDiff.text;
        qDebug() << "diff text: " << text << ", op: " << aDiff.operation;
        text.replace("&", "&amp;").replace("<", "&lt;")
            .replace(">", "&gt;").replace("\n", html_line_feed);
        switch (aDiff.operation) {
            case DELETE:
                oldHtml += QString("<font color=\"red\">") + text + QString("</font>");
                break;
            case INSERT:
                newHtml += QString("<font color=\"green\">") + text + QString("</font>");
                break;
            case EQUAL:
                // Find \n from start.
                auto pos = text.indexOf(html_line_feed);
                if (pos != -1) {
                    // If difference exists before.
                    if ((!oldHtml.isEmpty() || !newHtml.isEmpty())) {
                        auto first = text.mid(0, pos + html_line_feed_len);
                        if (oldHtml != newHtml) {  // Add the first line's text to old line and new line.
                            oldHtml += QString("<span>") + first + QString("</span>");
                            newHtml += QString("<span>") + first + QString("</span>");
                            html += "<span style=\"background:#ffe6e6;\">" + oldHtml + "</span>";
                            html += "<span style=\"background:#e6ffe6;\">" + newHtml + "</span>";
                        } else {  // Add to common line.
                            oldHtml += QString("<span>") + first + QString("</span>");
                            html += oldHtml;
                        }
                        oldHtml.clear();
                        newHtml.clear();
                        // Find \n from end.
                        auto lastPos = text.lastIndexOf(html_line_feed, -1);
                        if (lastPos == -1) {  // No new line in text.
                            html += QString("<span>") + text.mid(pos + html_line_feed_len) + QString("</span>");
                        } else if (pos == lastPos) {  // Only one \n at the start of text.
                            oldHtml += QString("<span>") + text.mid(pos + html_line_feed_len) + QString("</span>");
                            newHtml += QString("<span>") + text.mid(pos + html_line_feed_len) + QString("</span>");
                        } else {  // More than one \n exist, append middle to common line, and append the last line to old line and new line.
                            html += QString("<span>") + text.mid(pos + html_line_feed_len, lastPos - pos) + QString("</span>");
                            oldHtml += QString("<span>") + text.mid(lastPos + html_line_feed_len) + QString("</span>");
                            newHtml += QString("<span>") + text.mid(lastPos + html_line_feed_len) + QString("</span>");
                        }
                    } else {  // No difference, append to common line.
                        html += QString("<span>") + text + QString("</span>");
                    }
                } else {  // Gather old line text and new line text.
                    newHtml += QString("<span>") + text + QString("</span>");
                    oldHtml += QString("<span>") + text + QString("</span>");
                }
                break;
        }
    }
    // Append the accumulated old line text and new line text.
    if (oldHtml != newHtml) {  // Add the two lines for either old line and new line.
        html += "<span style=\"background:#ffe6e6;\">" + oldHtml + "</span>";
        html += html_line_feed;
        html += "<span style=\"background:#e6ffe6;\">" + newHtml + "</span>";
    } else {  // Add to common line.
        html += oldHtml;
    }
    return html;
}
}  // namespace QEditor
