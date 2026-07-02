#include "taskdeadline.h"
#include <QDate>
#include <QTime>

QWidget* DeadlineSelectorWidget::createColumn(const QString &title, QSpinBox *spin, const QString &type) {
    QWidget *colWidget = new QWidget(this);
    QVBoxLayout *colLayout = new QVBoxLayout(colWidget);
    colLayout->setContentsMargins(0, 0, 0, 0);
    colLayout->setSpacing(3);

    QLabel *titleLabel = new QLabel(title, colWidget);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 11px; color: #7f8c8d; font-weight: normal; background: transparent;");
    colLayout->addWidget(titleLabel);
    colLayout->addWidget(spin);

    QWidget *btnContainer = new QWidget(colWidget);
    QHBoxLayout *btnLayout = new QHBoxLayout(btnContainer);
    btnLayout->setContentsMargins(0, 0, 0, 0);
    btnLayout->setSpacing(2);

    QString btnStyle =
        "QToolButton {"
        "   border: 1px solid #dcdde1;"
        "   border-radius: 3px;"
        "   background-color: #f8f9fa;"
        "   color: #57606f;"
        "   font-size: 9px;"
        "   min-width: 18px;"
        "   max-width: 22px;"
        "   min-height: 14px;"
        "   max-height: 16px;"
        "}"
        "QToolButton:hover { background-color: #e4e7eb; color: #2f3542; }"
        "QToolButton:pressed { background-color: #dcdde1; }"
        "QToolButton:disabled { background-color: #f1f2f6; color: #ced6e0; border-color: #f1f2f6; }";

    QToolButton *btnUp = new QToolButton(btnContainer);
    btnUp->setText("▲");
    btnUp->setStyleSheet(btnStyle);

    QToolButton *btnDown = new QToolButton(btnContainer);
    btnDown->setText("▼");
    btnDown->setStyleSheet(btnStyle);

    btnLayout->addWidget(btnDown);
    btnLayout->addWidget(btnUp);
    colLayout->addWidget(btnContainer);

    connect(btnUp, &QToolButton::clicked, this, [this, type]() { modifyDateTime(type, 1); });
    connect(btnDown, &QToolButton::clicked, this, [this, type]() { modifyDateTime(type, -1); });

    spin->installEventFilter(this);
    m_spinToButtonsMap[spin] = qMakePair(btnUp, btnDown);

    return colWidget;
}

QSpinBox* DeadlineSelectorWidget::createSubSelector(int min, int max) {
    QSpinBox *spin = new QSpinBox(this);
    spin->setRange(min, max);
    spin->setAlignment(Qt::AlignCenter);
    spin->setButtonSymbols(QAbstractSpinBox::NoButtons);

    spin->setStyleSheet(
        "QSpinBox {"
        "   border: 1px solid #bdc3c7;"
        "   border-radius: 4px;"
        "   padding: 2px 0px;"
        "   min-width: 38px;"
        "   max-width: 45px;"
        "   font-size: 14px;"
        "   background-color: #ffffff;"
        "   color: #333333;"
        "}"
        "QSpinBox:focus {"
        "   border: 1px solid #3498db;"
        "}"
        "QSpinBox:disabled {"
        "   background-color: #f5f5f5;"
        "   color: #7f8c8d;"
        "}"
        );

    if (max > 2000) {
        spin->setMinimumWidth(58);
        spin->setMaximumWidth(65);
    }

    connect(spin, QOverload<int>::of(&QSpinBox::valueChanged), this, &DeadlineSelectorWidget::onManualSpinChanged);
    return spin;
}

void DeadlineSelectorWidget::modifyDateTime(const QString &type, int val) {
    QDateTime currentDt = getDateTime();

    if (type == "DAY") currentDt = currentDt.addDays(val);
    else if (type == "MONTH") currentDt = currentDt.addMonths(val);
    else if (type == "YEAR") currentDt = currentDt.addYears(val);
    else if (type == "HOUR") currentDt = currentDt.addSecs(val * 3600);
    else if (type == "MIN") currentDt = currentDt.addSecs(val * 60);

    setDateTime(currentDt); 
    emit dateTimeChanged(currentDt);
}

void DeadlineSelectorWidget::updateDaysInMonth() {
    QDate date(yearSpin->value(), monthSpin->value(), 1);
    daySpin->setRange(1, date.daysInMonth());
}

bool DeadlineSelectorWidget::eventFilter(QObject *watched, QEvent *event) {
    if (event->type() == QEvent::EnabledChange) {
        QSpinBox *spin = qobject_cast<QSpinBox*>(watched);
        if (spin && m_spinToButtonsMap.contains(spin)) {
            bool enabled = spin->isEnabled();
            m_spinToButtonsMap[spin].first->setEnabled(enabled);
            m_spinToButtonsMap[spin].second->setEnabled(enabled);
        }
    }
    return QWidget::eventFilter(watched, event);
}

void DeadlineSelectorWidget::onManualSpinChanged() {
    if (m_blockManualSignal) return;
    updateDaysInMonth();
    emit dateTimeChanged(getDateTime());
}

DeadlineSelectorWidget::DeadlineSelectorWidget(QWidget *parent) : QWidget(parent) {
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(2);
    layout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter); 

    daySpin = createSubSelector(1, 31);
    monthSpin = createSubSelector(1, 12);
    yearSpin = createSubSelector(2000, 2100);
    hourSpin = createSubSelector(0, 23);
    minuteSpin = createSubSelector(0, 59);

    auto createSeparatorColumn = [this](const QString &symbol) -> QWidget* {
        QWidget *sepWidget = new QWidget(this);
        QVBoxLayout *sepLayout = new QVBoxLayout(sepWidget);
        sepLayout->setContentsMargins(0, 0, 0, 0);
        sepLayout->setSpacing(3);

        sepLayout->addSpacing(14);

        QLabel *label = new QLabel(symbol, sepWidget);
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet("color: #bdc3c7; font-weight: bold; font-size: 16px; background: transparent;");
        sepLayout->addWidget(label);

        sepLayout->addSpacing(16);

        return sepWidget;
    };

    layout->addWidget(createColumn("Ngày", daySpin, "DAY"));
    layout->addWidget(createSeparatorColumn("/"));

    layout->addWidget(createColumn("Tháng", monthSpin, "MONTH"));
    layout->addWidget(createSeparatorColumn("/"));

    layout->addWidget(createColumn("Năm", yearSpin, "YEAR"));

    layout->addSpacing(10); 

    layout->addWidget(createColumn("Giờ", hourSpin, "HOUR"));
    layout->addWidget(createSeparatorColumn(":"));

    layout->addWidget(createColumn("Phút", minuteSpin, "MIN"));

    layout->addStretch();

    setDateTime(QDateTime::currentDateTime().addDays(1));
}

void DeadlineSelectorWidget::setDateTime(const QDateTime &dt) {
    m_blockManualSignal = true;

    daySpin->blockSignals(true);
    monthSpin->blockSignals(true);
    yearSpin->blockSignals(true);
    hourSpin->blockSignals(true);
    minuteSpin->blockSignals(true);

    yearSpin->setValue(dt.date().year());
    monthSpin->setValue(dt.date().month());
    updateDaysInMonth();
    daySpin->setValue(dt.date().day());
    hourSpin->setValue(dt.time().hour());
    minuteSpin->setValue(dt.time().minute());

    daySpin->blockSignals(false);
    monthSpin->blockSignals(false);
    yearSpin->blockSignals(false);
    hourSpin->blockSignals(false);
    minuteSpin->blockSignals(false);

    m_blockManualSignal = false;
}

QDateTime DeadlineSelectorWidget::getDateTime() const {
    QDate date(yearSpin->value(), monthSpin->value(), daySpin->value());
    QTime time(hourSpin->value(), minuteSpin->value(), 0);
    return QDateTime(date, time);
}

TaskDeadline::TaskDeadline() : m_deadline(QDateTime::currentDateTime()) {}

TaskDeadline::TaskDeadline(const QDateTime &deadline) : m_deadline(deadline) {}

QDateTime TaskDeadline::getDateTime() const { return m_deadline; }

void TaskDeadline::setDateTime(const QDateTime &deadline) { m_deadline = deadline; }

bool TaskDeadline::isOverdue(const QString &currentStatus) const {
    if (currentStatus.toLower() == "done" || currentStatus == "hoàn thành") return false;
    return QDateTime::currentDateTime() > m_deadline;
}

QString TaskDeadline::toString(const QString &format) const {
    return m_deadline.toString(format);
}

QString TaskDeadline::toJsonString() const {
    return m_deadline.toString(Qt::ISODate);
}
