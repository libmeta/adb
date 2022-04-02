#pragma once

#include <QKeyEvent>
#include <QWidget>
#include <QTimer>

#include <functional>

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget {
    Q_OBJECT

public:
    Widget(QWidget* parent = nullptr);
    ~Widget();

private:
    Ui::Widget* ui;
    QTimer *timer;
private:
    std::function<void(void)> onCmdFunc;
    std::function<void(void)> onUpdateFunc;

};
