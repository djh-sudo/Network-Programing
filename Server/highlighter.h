#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H
#include<QSyntaxHighlighter>
class Highlighter: public QSyntaxHighlighter
{
    Q_OBJECT                                         //Qt宏定义

public:
    Highlighter(QTextDocument *parent = 0);      //构造函数，传递一个QTextDocument对象给其父类

protected:
    void highlightBlock(const QString &text) Q_DECL_OVERRIDE;    //块高亮使用的函数，自动调用

private:
    struct HighlightingRule                    //语法规则结构体，包含正则表达式模式串和匹配的样式
    {
        QRegExp pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;    //规则的集合，可以定义多个高亮规则

    QRegExp commentStartExpression;                //注释的高亮，使用highliangBlock()匹配，下文提到
    QRegExp commentEndExpression;

    QTextCharFormat keywordFormat;                //高亮样式，关键词，一下顾名思义
    QTextCharFormat classFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat multiLineCommentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat functionFormat;
};

#endif // HIGHLIGHTER_H
