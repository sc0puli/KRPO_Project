#include "Highlighter.h"
#include <QRegularExpression>

SpiceHighlighter::SpiceHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent) {

    // Инициализация форматов
    elementFormat.setForeground(Qt::darkBlue);
    elementFormat.setFontWeight(QFont::Bold);

    commentFormat.setForeground(Qt::gray);
    commentFormat.setFontItalic(true);

    directiveFormat.setForeground(Qt::darkGreen);
    directiveFormat.setFontWeight(QFont::Bold);

    valueFormat.setForeground(Qt::darkMagenta);

    modelFormat.setForeground(Qt::darkCyan);
    modelFormat.setFontItalic(true);

    // Инициализация правил
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

// Правила для элементов (R1, C3, L5 и т.д.)
void SpiceHighlighter::initElementRules() {
    HighlightRule rule;

    QStringList elementPatterns = {
        R"(\bR\d+\w*\b)",    // Резисторы
        R"(\bC\d+\w*\b)",    // Конденсаторы
        R"(\bL\d+\w*\b)",    // Индуктивности
        R"(\bV\d+\w*\b)",    // Источники напряжения
        R"(\bI\d+\w*\b)",    // Источники тока
    };

    for (const QString& pattern : elementPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = elementFormat;
        highlightingRules.append(rule);
    }
}

// Правила для комментариев
void SpiceHighlighter::initCommentRules() {
    HighlightRule rule;
    rule.pattern = QRegularExpression(R"((\*|;).*)");
    rule.format = commentFormat;
    highlightingRules.append(rule);
}

// Правила для директив (.model, .tran и т.д.)
void SpiceHighlighter::initDirectiveRules() {
    HighlightRule rule;
    rule.pattern = QRegularExpression(R"(\.\w+\b)");
    rule.format = directiveFormat;
    highlightingRules.append(rule);
}

// Правила для числовых значений
void SpiceHighlighter::initValueRules() {
    HighlightRule rule;
    rule.pattern = QRegularExpression(R"(\b\d+[eE]?[+-]?\d*[GMkmuµnpf]?)");
    rule.format = valueFormat;
    highlightingRules.append(rule);
}

// Правила для моделей (MODEL XYZ)
void SpiceHighlighter::initModelRules() {
    HighlightRule rule;
    rule.pattern = QRegularExpression(R"(\bMODEL\s+\w+)", QRegularExpression::CaseInsensitiveOption);
    rule.format = modelFormat;
    highlightingRules.append(rule);
}