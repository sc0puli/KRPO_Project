#include "Highlighter.h"
#include <QRegularExpression>

SpiceHighlighter::SpiceHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent) {

    // ������������� ��������
    elementFormat.setForeground(Qt::darkBlue);
    elementFormat.setFontWeight(QFont::Bold);

    commentFormat.setForeground(Qt::gray);
    commentFormat.setFontItalic(true);

    directiveFormat.setForeground(Qt::darkGreen);
    directiveFormat.setFontWeight(QFont::Bold);

    valueFormat.setForeground(Qt::darkMagenta);

    modelFormat.setForeground(Qt::darkCyan);
    modelFormat.setFontItalic(true);

    // ������������� ������
    initElementRules();
    initCommentRules();
    initDirectiveRules();
    initValueRules();
    initModelRules();
}

void SpiceHighlighter::highlightBlock(const QString& text) {
    for (const auto& rule : highlightingRules) {
        QRegularExpressionMatchIterator it = rule.pattern.globalMatch(text);
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}

// ������� ��� ��������� (R1, C3, L5 � �.�.)
void SpiceHighlighter::initElementRules() {
    HighlightRule rule;

    QStringList elementPatterns = {
        R"(\bR\d+\w*\b)",    // ���������
        R"(\bC\d+\w*\b)",    // ������������
        R"(\bL\d+\w*\b)",    // �������������
        R"(\bV\d+\w*\b)",    // ��������� ����������
        R"(\bI\d+\w*\b)",    // ��������� ����
    };

    for (const QString& pattern : elementPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = elementFormat;
        highlightingRules.append(rule);
    }
}

// ������� ��� ������������
void SpiceHighlighter::initCommentRules() {
    HighlightRule rule;
    rule.pattern = QRegularExpression(R"((\*|;).*)");
    rule.format = commentFormat;
    highlightingRules.append(rule);
}

// ������� ��� �������� (.model, .tran � �.�.)
void SpiceHighlighter::initDirectiveRules() {
    HighlightRule rule;
    rule.pattern = QRegularExpression(R"(\.\w+\b)");
    rule.format = directiveFormat;
    highlightingRules.append(rule);
}

// ������� ��� �������� ��������
void SpiceHighlighter::initValueRules() {
    HighlightRule rule;
    rule.pattern = QRegularExpression(R"(\b\d+[eE]?[+-]?\d*[GMkmu�npf]?)");
    rule.format = valueFormat;
    highlightingRules.append(rule);
}

// ������� ��� ������� (MODEL XYZ)
void SpiceHighlighter::initModelRules() {
    HighlightRule rule;
    rule.pattern = QRegularExpression(R"(\bMODEL\s+\w+)", QRegularExpression::CaseInsensitiveOption);
    rule.format = modelFormat;
    highlightingRules.append(rule);
}