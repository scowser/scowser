#pragma once

#include <QDockWidget>
#include <QPlainTextEdit>
#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QToolButton>
#include <QLabel>
#include <QMutex>

class LogHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    explicit LogHighlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct Rule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<Rule> m_rules;
};

class LogPanel : public QDockWidget {
    Q_OBJECT

public:
    explicit LogPanel(QWidget *parent = nullptr);
    ~LogPanel() override;

    void appendMessage(QtMsgType type, const QString &message);
    bool isDockedBottom() const;

public slots:
    void toggleDockOrientation();
    void clearLog();

private:
    void setupUI();

    QPlainTextEdit *m_logView;
    QToolButton *m_orientationButton;
    QToolButton *m_clearButton;
    LogHighlighter *m_highlighter;

    static LogPanel *s_instance;
    static QtMessageHandler s_previousHandler;
    static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
    static QMutex s_mutex;
};
