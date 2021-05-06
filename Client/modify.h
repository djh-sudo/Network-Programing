#ifndef MODIFY_H
#define MODIFY_H

#include <QWidget>

namespace Ui {
class Modify;
}

class Modify : public QWidget
{
    Q_OBJECT

public:
    explicit Modify(QWidget *parent = nullptr);
    ~Modify();

private:
    Ui::Modify *ui;
};

#endif // MODIFY_H
