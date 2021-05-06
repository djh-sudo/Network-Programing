#ifndef NOFOCUSDELEGATE_H
#define NOFOCUSDELEGATE_H

#include<QStyleOptionViewItem>
#include<QPainter>
#include<QStyledItemDelegate>
class NoFocusDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    NoFocusDelegate(){};
    ~NoFocusDelegate(){};
    void NoFocusDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // NOFOCUSDELEGATE_H
