#ifndef TASKDEADLINE_H
#define TASKDEADLINE_H

#include <QDateTime>
#include <QString>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSpinBox>
#include <QLabel>
#include <QToolButton>
#include <QEvent>
#include <QMap>

// ==========================================
// CUSTOM WIDGET CHO GIAO DIỆN CHỈNH THỜI GIAN
// ==========================================
class DeadlineSelectorWidget : public QWidget {
    Q_OBJECT
private:
    // Hàm tạo cột giao diện như cũ
    QWidget* createColumn(const QString &title, QSpinBox *spin, const QString &type) {
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

        // THAY ĐỔI: Kết nối nút bấm qua hàm xử lý cộng dồn hệ số thông minh riêng biệt
        connect(btnUp, &QToolButton::clicked, this, [this, type]() { modifyDateTime(type, 1); });
        connect(btnDown, &QToolButton::clicked, this, [this, type]() { modifyDateTime(type, -1); });

        spin->installEventFilter(this);
        m_spinToButtonsMap[spin] = qMakePair(btnUp, btnDown);

        return colWidget;
    }

    QSpinBox* createSubSelector(int min, int max) {
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

        // Đồng bộ ngược lại khi người dùng gõ số trực tiếp bằng bàn phím
        connect(spin, QOverload<int>::of(&QSpinBox::valueChanged), this, &DeadlineSelectorWidget::onManualSpinChanged);
        return spin;
    }

    // THÀNH PHẦN CỐT LÕI: Hàm tính toán cộng dồn dựa trên logic QDateTime
    void modifyDateTime(const QString &type, int val) {
        QDateTime currentDt = getDateTime();

        if (type == "DAY") currentDt = currentDt.addDays(val);
        else if (type == "MONTH") currentDt = currentDt.addMonths(val);
        else if (type == "YEAR") currentDt = currentDt.addYears(val);
        else if (type == "HOUR") currentDt = currentDt.addSecs(val * 3600);
        else if (type == "MIN") currentDt = currentDt.addSecs(val * 60);

        setDateTime(currentDt); // Cập nhật lại toàn bộ giao diện 5 hộp
        emit dateTimeChanged(currentDt);
    }

    void updateDaysInMonth() {
        QDate date(yearSpin->value(), monthSpin->value(), 1);
        daySpin->setRange(1, date.daysInMonth());
    }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override {
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

signals:
    void dateTimeChanged(const QDateTime &dateTime);

private slots:
    void onManualSpinChanged() {
        if (m_blockManualSignal) return;
        updateDaysInMonth();
        emit dateTimeChanged(getDateTime());
    }

public:
    QSpinBox *daySpin;
    QSpinBox *monthSpin;
    QSpinBox *yearSpin;
    QSpinBox *hourSpin;
    QSpinBox *minuteSpin;
    QMap<QSpinBox*, QPair<QToolButton*, QToolButton*>> m_spinToButtonsMap;
    bool m_blockManualSignal = false;

    explicit DeadlineSelectorWidget(QWidget *parent = nullptr) : QWidget(parent) {
        QHBoxLayout *layout = new QHBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(2);
        layout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

        // Khởi tạo SpinBox trống biên độ trước
        daySpin = createSubSelector(1, 31);
        monthSpin = createSubSelector(1, 12);
        yearSpin = createSubSelector(2000, 2100);
        hourSpin = createSubSelector(0, 23);
        minuteSpin = createSubSelector(0, 59);

        // Đắp Layout và định nghĩa định danh loại tính toán "DAY", "MONTH", "YEAR"...
        layout->addWidget(createColumn("Ngày", daySpin, "DAY"));

        QLabel *slash1 = new QLabel("/", this);
        slash1->setStyleSheet("padding-top: 20px; color: #bdc3c7; font-weight: bold;");
        layout->addWidget(slash1);

        layout->addWidget(createColumn("Tháng", monthSpin, "MONTH"));

        QLabel *slash2 = new QLabel("/", this);
        slash2->setStyleSheet("padding-top: 20px; color: #bdc3c7; font-weight: bold;");
        layout->addWidget(slash2);

        layout->addWidget(createColumn("Năm", yearSpin, "YEAR"));

        layout->addSpacing(12);

        layout->addWidget(createColumn("Giờ", hourSpin, "HOUR"));

        QLabel *colon = new QLabel(":", this);
        colon->setStyleSheet("padding-top: 20px; color: #bdc3c7; font-weight: bold;");
        layout->addWidget(colon);

        layout->addWidget(createColumn("Phút", minuteSpin, "MIN"));

        layout->addStretch();

        // Gán thời gian mặc định ban đầu là ngày mai
        setDateTime(QDateTime::currentDateTime().addDays(1));
    }

    void setDateTime(const QDateTime &dt) {
        m_blockManualSignal = true; // Chặn xử lý logic manual tạm thời

        // BẬT KHÓA TÍN HIỆU TOÀN BỘ CÁC Ô TRƯỚC KHI SET VALUE
        daySpin->blockSignals(true);
        monthSpin->blockSignals(true);
        yearSpin->blockSignals(true);
        hourSpin->blockSignals(true);
        minuteSpin->blockSignals(true);

        // Cập nhật giá trị an toàn vào hộp nhập
        yearSpin->setValue(dt.date().year());
        monthSpin->setValue(dt.date().month());
        updateDaysInMonth();
        daySpin->setValue(dt.date().day());
        hourSpin->setValue(dt.time().hour());
        minuteSpin->setValue(dt.time().minute());

        // MỞ LẠI TÍN HIỆU SAU KHI ĐÃ CẬP NHẬT XONG
        daySpin->blockSignals(false);
        monthSpin->blockSignals(false);
        yearSpin->blockSignals(false);
        hourSpin->blockSignals(false);
        minuteSpin->blockSignals(false);

        m_blockManualSignal = false; // Mở khóa xử lý manual
    }

    QDateTime getDateTime() const {
        QDate date(yearSpin->value(), monthSpin->value(), daySpin->value());
        QTime time(hourSpin->value(), minuteSpin->value(), 0);
        return QDateTime(date, time);
    }
};

// ==========================================
// CLASS LOGIC TASKDEADLINE CHÍNH
// ==========================================
class TaskDeadline {
public:
    TaskDeadline() : m_deadline(QDateTime::currentDateTime()) {}
    TaskDeadline(const QDateTime &deadline) : m_deadline(deadline) {}

    QDateTime getDateTime() const { return m_deadline; }
    void setDateTime(const QDateTime &deadline) { m_deadline = deadline; }

    bool isOverdue(const QString &currentStatus = "") const {
        if (currentStatus.toLower() == "done" || currentStatus == "hoàn thành") return false;
        return QDateTime::currentDateTime() > m_deadline;
    }

    QString toString(const QString &format = "dd/MM/yyyy hh:mm") const {
        return m_deadline.toString(format);
    }

    QString toJsonString() const {
        return m_deadline.toString(Qt::ISODate);
    }

private:
    QDateTime m_deadline;
};

#endif // TASKDEADLINE_H