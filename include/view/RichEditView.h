#ifndef RICHEDITVIEW_H
#define RICHEDITVIEW_H

#include <QTextEdit>

namespace QEditor {
class RichEditView : public QTextEdit
{
    Q_OBJECT
public:
    RichEditView(QWidget *parent = nullptr);
    ~RichEditView() = default;

    void ZoomIn();
    void ZoomOut();

protected:
    void wheelEvent(QWheelEvent *event);

private:
    int currentFontSize_{font().pointSize()};
};
}  // namespace QEditor

#endif // RICHEDITVIEW_H
