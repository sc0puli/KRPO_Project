#pragma once

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>

class SpiceHighlighter : public QSyntaxHighlighter {
public:
    SpiceHighlighter(QTextDocument* parent = nullptr);

private:
    struct HighlightRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    QVector<HighlightRule> highlightingRules;

    // Форматы
    QTextCharFormat elementFormat;
    QTextCharFormat commentFormat;
    QTextCharFormat directiveFormat;
    QTextCharFormat valueFormat;
    QTextCharFormat modelFormat;

    void initElementRules();
    void initCommentRules();
    void initDirectiveRules();
    void initValueRules();
    void initModelRules();

    void highlightBlock(const QString& text) override;

};
