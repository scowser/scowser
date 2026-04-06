#include "ui/LogPanel.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolBar>
#include <QDateTime>
#include <QRegularExpression>
#include <QFont>
#include <QMainWindow>
#include <QScrollBar>
#include <QLabel>
#include <QApplication>

// --- LogHighlighter ---

LogHighlighter::LogHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    // Timestamp: [HH:MM:SS.zzz]
    {
        QTextCharFormat fmt;
        fmt.setForeground(QColor("#a6adc8")); // Subtext
        m_rules.push_back(Rule{QRegularExpression(R"(\[\d{2}:\d{2}:\d{2}\.\d{3}\])"), fmt});
    }

    // Log level: DEBUG
    {
        QTextCharFormat fmt;
        fmt.setForeground(QColor("#89b4fa")); // Blue
        fmt.setFontWeight(QFont::Bold);
        m_rules.push_back(Rule{QRegularExpression(R"(\bDEBUG\b)"), fmt});
    }

    // Log level: INFO
    {
        QTextCharFormat fmt;
        fmt.setForeground(QColor("#a6e3a1")); // Green
        fmt.setFontWeight(QFont::Bold);
        m_rules.push_back(Rule{QRegularExpression(R"(\bINFO\b)"), fmt});
    }

    // Log level: WARNING
    {
        QTextCharFormat fmt;
        fmt.setForeground(QColor("#f9e2af")); // Yellow (Catppuccin Mocha)
        fmt.setFontWeight(QFont::Bold);
        m_rules.push_back(Rule{QRegularExpression(R"(\bWARNING\b)"), fmt});
    }

    // Log level: CRITICAL
    {
        QTextCharFormat fmt;
        fmt.setForeground(QColor("#f38ba8")); // Red
        fmt.setFontWeight(QFont::Bold);
        m_rules.push_back(Rule{QRegularExpression(R"(\bCRITICAL\b)"), fmt});
    }

    // Log level: FATAL
    {
        QTextCharFormat fmt;
        fmt.setForeground(QColor("#f38ba8")); // Red
        fmt.setFontWeight(QFont::Bold);
        fmt.setFontUnderline(true);
        m_rules.push_back(Rule{QRegularExpression(R"(\bFATAL\b)"), fmt});
    }

    // Source component in brackets: [AdBlocker], [DnsOverHttps], etc.
    {
        QTextCharFormat fmt;
        fmt.setForeground(QColor("#cba6f7")); // Mauve (Catppuccin Mocha)
        m_rules.push_back(Rule{QRegularExpression(R"(\] \[(\w+)\])"), fmt});
    }

    // Quoted strings
    {
        QTextCharFormat fmt;
        fmt.setForeground(QColor("#fab387")); // Peach (Catppuccin Mocha)
        m_rules.push_back(Rule{QRegularExpression(R"("[^"]*")"), fmt});
    }

    // URLs
    {
        QTextCharFormat fmt;
        fmt.setForeground(QColor("#89dceb")); // Sky (Catppuccin Mocha)
        fmt.setFontUnderline(true);
        m_rules.push_back(Rule{QRegularExpression(R"(https?://\S+)"), fmt});
    }

    // Numbers
    {
        QTextCharFormat fmt;
        fmt.setForeground(QColor("#fab387")); // Peach
        m_rules.push_back(Rule{QRegularExpression(R"(\b\d+\.?\d*\b)"), fmt});
    }
}

void LogHighlighter::highlightBlock(const QString &text)
{
    for (const auto &rule : m_rules) {
        auto it = rule.pattern.globalMatch(text);
        while (it.hasNext()) {
            auto match = it.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}

// --- LogPanel ---

LogPanel *LogPanel::s_instance = nullptr;
QtMessageHandler LogPanel::s_previousHandler = nullptr;
QMutex LogPanel::s_mutex;

LogPanel::LogPanel(QWidget *parent)
    : QDockWidget("Logs", parent)
{
    setObjectName("logPanel");
    setupUI();

    // Install as global message handler
    s_instance = this;
    s_previousHandler = qInstallMessageHandler(messageHandler);

    // Default to right dock (vertical pane)
    setAllowedAreas(Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
    if (auto *mainWin = qobject_cast<QMainWindow *>(parent)) {
        mainWin->addDockWidget(Qt::RightDockWidgetArea, this);
    }
}

LogPanel::~LogPanel()
{
    // Restore previous handler
    qInstallMessageHandler(s_previousHandler);
    s_instance = nullptr;
    s_previousHandler = nullptr;
}

void LogPanel::setupUI()
{
    auto *container = new QWidget(this);
    container->setObjectName("logPanelContainer");
    auto *layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Toolbar
    auto *toolbar = new QWidget(container);
    toolbar->setObjectName("logPanelToolbar");
    auto *toolbarLayout = new QHBoxLayout(toolbar);
    toolbarLayout->setContentsMargins(8, 4, 8, 4);
    toolbarLayout->setSpacing(4);

    auto *titleLabel = new QLabel("Logs", toolbar);
    titleLabel->setObjectName("logPanelTitle");
    toolbarLayout->addWidget(titleLabel);
    toolbarLayout->addStretch();

    m_clearButton = new QToolButton(toolbar);
    m_clearButton->setObjectName("logClearButton");
    m_clearButton->setText("Clear");
    m_clearButton->setToolTip("Clear log");
    connect(m_clearButton, &QToolButton::clicked, this, &LogPanel::clearLog);
    toolbarLayout->addWidget(m_clearButton);

    m_orientationButton = new QToolButton(toolbar);
    m_orientationButton->setObjectName("logOrientationButton");
    m_orientationButton->setText("⇊");
    m_orientationButton->setToolTip("Dock to bottom");
    connect(m_orientationButton, &QToolButton::clicked, this, &LogPanel::toggleDockOrientation);
    toolbarLayout->addWidget(m_orientationButton);

    layout->addWidget(toolbar);

    // Log text view
    m_logView = new QPlainTextEdit(container);
    m_logView->setObjectName("logView");
    m_logView->setReadOnly(true);
    m_logView->setLineWrapMode(QPlainTextEdit::NoWrap);
    m_logView->setMaximumBlockCount(10000);

    QFont monoFont("Menlo, Monaco, Consolas, monospace");
    monoFont.setStyleHint(QFont::Monospace);
    monoFont.setPointSize(11);
    m_logView->setFont(monoFont);

    m_highlighter = new LogHighlighter(m_logView->document());

    layout->addWidget(m_logView);

    setWidget(container);

    // Hide the default dock widget title bar since we have our own toolbar
    setTitleBarWidget(new QWidget(this));
}

void LogPanel::appendMessage(QtMsgType type, const QString &message)
{
    QString timestamp = QDateTime::currentDateTime().toString("[HH:mm:ss.zzz]");

    QString level;
    switch (type) {
    case QtDebugMsg:    level = "DEBUG";    break;
    case QtInfoMsg:     level = "INFO";     break;
    case QtWarningMsg:  level = "WARNING";  break;
    case QtCriticalMsg: level = "CRITICAL"; break;
    case QtFatalMsg:    level = "FATAL";    break;
    }

    QString formatted = QString("%1 %2 %3").arg(timestamp, level, message);

    // Auto-scroll only if already at the bottom
    auto *scrollBar = m_logView->verticalScrollBar();
    bool atBottom = scrollBar->value() >= scrollBar->maximum() - 4;

    m_logView->appendPlainText(formatted);

    if (atBottom) {
        scrollBar->setValue(scrollBar->maximum());
    }
}

bool LogPanel::isDockedBottom() const
{
    auto *mainWin = qobject_cast<QMainWindow *>(parentWidget());
    if (!mainWin) return false;
    return mainWin->dockWidgetArea(const_cast<LogPanel *>(this)) == Qt::BottomDockWidgetArea;
}

void LogPanel::toggleDockOrientation()
{
    auto *mainWin = qobject_cast<QMainWindow *>(parentWidget());
    if (!mainWin) return;

    if (isDockedBottom()) {
        mainWin->addDockWidget(Qt::RightDockWidgetArea, this);
        m_orientationButton->setText("⇊");
        m_orientationButton->setToolTip("Dock to bottom");
    } else {
        mainWin->addDockWidget(Qt::BottomDockWidgetArea, this);
        m_orientationButton->setText("⇉");
        m_orientationButton->setToolTip("Dock to right");
    }
}

void LogPanel::clearLog()
{
    m_logView->clear();
}

void LogPanel::messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context);

    QMutexLocker locker(&s_mutex);

    // Forward to previous handler (stderr output)
    if (s_previousHandler) {
        s_previousHandler(type, context, msg);
    }

    // Send to panel if it exists
    if (s_instance) {
        // Use QMetaObject::invokeMethod for thread safety
        QMetaObject::invokeMethod(s_instance, [instance = s_instance, type, message = msg]() {
            instance->appendMessage(type, message);
        }, Qt::QueuedConnection);
    }
}
