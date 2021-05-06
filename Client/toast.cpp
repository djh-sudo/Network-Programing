#include "toast.h"

#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QEvent>

class ToastDlg: public QDialog
{
private:
    QLabel* mLabel;
    QLabel* mCloseBtn;
protected:
    bool eventFilter(QObject *obj, QEvent *ev) override
    {
        if (obj == mCloseBtn)
        {
            if (ev->type() == QEvent::MouseButtonRelease)
            {
                accept();
            }
        }
        return QObject::eventFilter(obj, ev);
    }
public:
    ToastDlg()
    {
        auto layout = new QHBoxLayout;//水平布局
        mLabel = new QLabel;
        mLabel->setStyleSheet("color: white; background:transparent");
        layout->addWidget(mLabel,1);
        mCloseBtn = new QLabel;
        mCloseBtn->installEventFilter(this);
        mCloseBtn->setStyleSheet("background:transparent");
        layout->addWidget(mCloseBtn);

        setLayout(layout);
        setWindowFlag(Qt::FramelessWindowHint);//生成一个无边界窗口。用户不能通过窗口系统移动或调整无边界窗口的大小。
        setAttribute(Qt::WA_ShowWithoutActivating,true);//Show the widget without making it active.
//        setAttribute(Qt::WA_TranslucentBackground, true);   // 背景透明
    }

    void show(Toast::Level level, const QString& text)
    {
        QPalette p = palette();
        p.setColor(QPalette::Window, QColor(0,0,0,100));
        if (level == Toast::INFO)
        {
            p.setColor(QPalette::Window, QColor(150,100,34,100));
        }
        else if (level == Toast::WARN)
        {
            p.setColor(QPalette::Window, QColor(0,0,255,100));
        }
        else//ERROR
        {
            p.setColor(QPalette::Window, QColor(255,0,0,100));
        }
        setPalette(p);//set widget's palette
        mLabel->setText(text);
        setWindowFlag(Qt::WindowStaysOnTopHint);//通知窗口系统该窗口应保持在所有其他窗口的顶部。
        QDialog::show();
    }
};


Toast::Toast(){//构造函数
    mDlg = new ToastDlg;
}

//返回一个实例(instance)
Toast &Toast::instance()
{
    static Toast thiz;//这种实例化方法会自动回收内存
    return thiz;
}

void Toast::show(Toast::Level level, const QString &text){
    mDlg->show(level, text);//ToastDlg.show方法
    if (mTimerId != 0)//int mTimerId
    {
        killTimer(mTimerId);
    }
    mTimerId = startTimer(2000);//启动定时器，每2s触发定时器事件，直到调用killTimer
}

//重写定时器事件回调函数
void Toast::timerEvent(QTimerEvent *event){
    killTimer(mTimerId);
    mTimerId = 0;
    mDlg->accept();//隐藏模态对话框

}
