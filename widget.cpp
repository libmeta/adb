#include "widget.hpp"
#include "ui_widget.h"

//#include <QGraphicsProxyWidget>
//#include <QGraphicsView>
//#include <QScreen>

//#include <QJniEnvironment>
//#include <QJniObject>

#include <QEventLoop>
#include <QFileDialog>
#include <QLineEdit>
#include <QPushButton>

#include <thread>

class QEventLoopThread {
public:
    using CallBackType = std::function<void(void)>;
    explicit QEventLoopThread() = delete;
    explicit QEventLoopThread(const CallBackType& callback)
    {
        QEventLoop loop;
        auto th = std::thread([&] {
            callback();
            loop.quit();
        });
        loop.exec();
        if (th.joinable()) {
            th.join();
        }
    }
};
/**运行命令
 *1. cmd    要执行的命令
 *2. 参数type可使用“r”代表读取，“w”代表写入。
 */

static inline int RunShell(const char* cmd, const char* type, char* resultBuffer, int len)
{
    int ret;

    FILE* pp = popen(cmd, type);
    ret = fread(resultBuffer, 1, len, pp);
    pclose(pp);

    return ret;
}

Widget::Widget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , timer(new QTimer(this))
{
    ui->setupUi(this);
    this->showMaximized();
    //    this->resize(qApp->primaryScreen()->availableSize().height(),qApp->primaryScreen()->availableSize().width());
    //    QGraphicsScene* scene = new QGraphicsScene;
    //    QGraphicsProxyWidget* w = scene->addWidget(this);
    //    w->setRotation(270);
    //    QGraphicsView* view = new QGraphicsView(scene);
    //    view->showMaximized();

    ui->lineEditCMD->setText("ls /dev/video*");
//    ui->lineEditCMD->setText("");
    ui->textEditDisplay->setLineWrapMode(QTextEdit::NoWrap);
    ui->textEditDisplay->setReadOnly(true);
    ui->textEditDisplay->moveCursor(QTextCursor::End);
    QFont font;
    font.setPointSize(6);
    ui->textEditDisplay->setFont(font);
    onCmdFunc = [this] {
        const auto cmd = ui->lineEditCMD->text();
        if (cmd.isEmpty()) {
            return;
        }

        std::vector<char> msg(1024 * 1024, '\0');
        do {
            int ret = 0;
            QEventLoopThread eventLoop([&] {
                ret = RunShell(cmd.toStdString().c_str(), "r", msg.data(), msg.size());
            });
            if (ret <= 0) {
                break;
            }

            msg.resize(ret);
            ui->textEditDisplay->append(QString(msg.data()).toLocal8Bit());
            ui->textEditDisplay->update();
            //            qDebug() << msg;
        } while (false);
    };

    onUpdateFunc = [this] {
        const auto cmd = ui->lineEditCMD->text();
        if (cmd.isEmpty()) {
            return;
        }

        std::vector<char> msg(1024 * 1024, '\0');
        do {
            int ret = 0;
            QEventLoopThread eventLoop([&] {
                ret = RunShell(cmd.toStdString().c_str(), "r", msg.data(), msg.size());
            });
            if (ret <= 0) {
                break;
            }

            msg.resize(ret);
            ui->textEditDisplay->setText(QString(msg.data()).toLocal8Bit());
            ui->textEditDisplay->update();
        } while (false);
    };

    connect(ui->pushButtonApply, &QPushButton::clicked, onCmdFunc);
    connect(ui->lineEditCMD, &QLineEdit::returnPressed, ui->pushButtonApply, &QPushButton::click);
    connect(ui->pushButtonClear, &QPushButton::clicked, this, [this] {
        ui->textEditDisplay->clear();
    });

    connect(ui->pushButtonLoopApply, &QPushButton::clicked, this, [this] {
        static bool isLoop = false;
        isLoop = !isLoop;
        if (isLoop) {
            connect(timer, &QTimer::timeout, ui->pushButtonApply, &QPushButton::click);
            timer->start(2000);
            ui->pushButtonLoopApply->setText("stop loop ");
        } else {
            timer->stop();
            ui->pushButtonLoopApply->setText("loop apply");
            disconnect(timer, nullptr, nullptr, nullptr);
        }
    });
    connect(ui->pushButtonLoopUpdate, &QPushButton::clicked, this, [this] {
        static bool isLoop = false;
        isLoop = !isLoop;
        if (isLoop) {
            connect(timer, &QTimer::timeout, onUpdateFunc);
            timer->start(500);
            ui->pushButtonLoopUpdate->setText("stop loop  ");
        } else {
            timer->stop();
            ui->pushButtonLoopUpdate->setText("loop update ");
            disconnect(timer, nullptr, nullptr, nullptr);
        }
    });

    connect(ui->pushButtonOpenFile, &QPushButton::clicked, this, [this] {
        const auto& file_name = QFileDialog::getOpenFileName(this, tr("Get file path"), "", tr("All file(*.*)"), 0);
        if (!file_name.isNull()) {
            ui->textEditDisplay->clear();
            ui->lineEditCMD->setText("");

            QFile text(file_name);
            text.open(QIODevice::ReadOnly | QIODevice::Text);
            while (!text.atEnd()) {
                ui->textEditDisplay->append(QString(text.readLine()).toLocal8Bit());
                ui->textEditDisplay->update();
            }
            text.close();
        }
    });
}

Widget::~Widget()
{
    delete ui;
}
