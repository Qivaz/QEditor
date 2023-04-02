#include "Diff.h"

namespace QEditor {
Diff::Diff() {}

void Diff::Impose(const QString &before, const QString &after) { diffs_ = diffMatchPatch_.diff_main(before, after); }

QString Diff::ToHtml() { return diffMatchPatch_.diff_prettyHtml(diffs_); }

QList<FormattedText> Diff::ToFormattedText() {
    QString text;
    QList<FormattedText> fts;
    QList<FormattedText> oldFts;
    QList<FormattedText> newFts;
    constexpr auto line_feed = "\n";
    const auto line_feed_len = strlen(line_feed);
    foreach(_Diff aDiff, diffs_) {
        text = aDiff.text;
        qDebug() << "diff text: " << text << ", op: " << aDiff.operation;
        switch (aDiff.operation) {
            case DELETE: {
                FormattedText oldFt;
                oldFt.text_ = text;
                oldFt.format_.setForeground(kOldFgColor);
                oldFts << oldFt;
                break;
            }
            case INSERT: {
                FormattedText newFt;
                newFt.text_ = text;
                newFt.format_.setForeground(kNewFgColor);
                newFts << newFt;
                break;
            }
            case EQUAL: {
                // Find \n from start.
                auto pos = text.indexOf(line_feed);
                if (pos != -1) {
                    // If previous differences exist.
                    if (!oldFts.isEmpty() || !newFts.isEmpty()) {
                        auto first = text.mid(0, pos + line_feed_len);
                        if (oldFts != newFts) {  // Add the first line's text to old line and new line.
                            // Old part.
                            for (const auto &oldFt : oldFts) {
                                auto ft = QTextCharFormat();
                                ft.setForeground(oldFt.format_.foreground());
                                ft.setBackground(kOldBgColor);
                                fts << FormattedText(oldFt.text_, ft);
                            }
                            auto firstFt = FormattedText(first);
                            firstFt.format_.setForeground(kCommonFgColor);
                            firstFt.format_.setBackground(kOldBgColor);
                            fts << firstFt;
                            // New part.
                            for (const auto &newFt : newFts) {
                                auto ft = QTextCharFormat();
                                ft.setForeground(newFt.format_.foreground());
                                ft.setBackground(kNewBgColor);
                                fts << FormattedText(newFt.text_, ft);
                            }
                            firstFt = FormattedText(first);
                            firstFt.format_.setForeground(kCommonFgColor);
                            firstFt.format_.setBackground(kNewBgColor);
                            fts << firstFt;
                        } else {  // Add to common line.
                            fts << FormattedText(first);
                        }
                        oldFts.clear();
                        newFts.clear();

                        // Find \n from end.
                        auto lastPos = text.lastIndexOf(line_feed, -1);
                        if (lastPos == -1) {  // No new line in text.
                            fts << FormattedText(text.mid(pos + line_feed_len));
                        } else if (pos == lastPos) {  // Only one \n at the start of text.
                            const auto &tmp = text.mid(pos + line_feed_len);
                            auto tmpFt = FormattedText(tmp);
                            tmpFt.format_.setForeground(kCommonFgColor);
                            oldFts << tmpFt;
                            newFts << tmpFt;
                        } else {  // More than one \n exist, append middle to common line, and append the last line to
                                  // old line and new line.
                            fts << FormattedText(text.mid(pos + line_feed_len, lastPos - pos));
                            const auto &tmp = text.mid(lastPos + line_feed_len);
                            auto tmpFt = FormattedText(tmp);
                            tmpFt.format_.setForeground(kCommonFgColor);
                            oldFts << tmpFt;
                            newFts << tmpFt;
                        }
                    } else {  // No difference before, append to common line.
                        // Handle common line's tail spaces.
                        // Find \n from end.
                        auto lastPos = text.lastIndexOf(line_feed, -1);
                        if (lastPos == -1) {
                            fts << FormattedText(text);
                        } else {
                            fts << FormattedText(text.mid(0, lastPos + line_feed_len));
                            const auto &tmp = text.mid(lastPos + line_feed_len);
                            auto tmpFt = FormattedText(tmp);
                            tmpFt.format_.setForeground(kCommonFgColor);
                            oldFts << tmpFt;
                            newFts << tmpFt;
                        }
                    }
                } else {  // Gather old line text and new line text.
                    auto tmpFt = FormattedText(text);
                    tmpFt.format_.setForeground(kCommonFgColor);
                    oldFts << tmpFt;
                    newFts << tmpFt;
                }
                break;
            }
        }
    }
    // Append the accumulated old line text and new line text.
    if (oldFts != newFts) {  // Add two lines for both old line and new line.
        for (const auto &oldFt : oldFts) {
            auto ft = FormattedText(oldFt.text_);
            ft.format_.setForeground(oldFt.format_.foreground());
            ft.format_.setBackground(kOldBgColor);
            fts << ft;
        }
        fts << FormattedText(line_feed);
        for (const auto &newFt : newFts) {
            auto ft = FormattedText(newFt.text_);
            ft.format_.setForeground(newFt.format_.foreground());
            ft.format_.setBackground(kNewBgColor);
            fts << ft;
        }
    } else {  // Add to common line.
        fts << oldFts;
    }
    return fts;
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
        text.replace("&", "&amp;")
            .replace("<", "&lt;")
            .replace(">", "&gt;")
            .replace("\u0020", "&nbsp;")
            .replace("\n", html_line_feed);
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
                    // If previous differences exist.
                    if ((!oldHtml.isEmpty() || !newHtml.isEmpty())) {
                        auto first = text.mid(0, pos + html_line_feed_len);
                        if (oldHtml != newHtml) {  // Add the first line's text to old line and new line.
                            oldHtml += QString("<span>") + first + QString("</span>");
                            newHtml += QString("<span>") + first + QString("</span>");
                            html += "<span style=\"background:#ffe6e6;\">" + oldHtml + "</span>";
                            html += "<span style=\"background:#e6ffe6;\">" + newHtml + "</span>";
                        } else {  // Add to common line.
                            html += QString("<span>") + first + QString("</span>");
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
                        } else {  // More than one \n exist, append middle to common line, and append the last line to
                                  // old line and new line.
                            html += QString("<span>") + text.mid(pos + html_line_feed_len, lastPos - pos) +
                                    QString("</span>");
                            oldHtml += QString("<span>") + text.mid(lastPos + html_line_feed_len) + QString("</span>");
                            newHtml += QString("<span>") + text.mid(lastPos + html_line_feed_len) + QString("</span>");
                        }
                    } else {  // No difference, append to common line.
                        // Handle common line's tail spaces.
                        // Find \n from end.
                        auto lastPos = text.lastIndexOf(html_line_feed, -1);
                        if (lastPos == -1) {
                            html += QString("<span>") + text + QString("</span>");
                        } else {
                            html += QString("<span>") + text.mid(0, lastPos + html_line_feed_len) + QString("</span>");
                            oldHtml += QString("<span>") + text.mid(lastPos + html_line_feed_len) + QString("</span>");
                            newHtml += QString("<span>") + text.mid(lastPos + html_line_feed_len) + QString("</span>");
                        }
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
