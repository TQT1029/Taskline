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
public:
    QSpinBox *daySpin;
    QSpinBox *monthSpin;
    QSpinBox *yearSpin;
    QSpinBox *hourSpin;
    QSpinBox *minuteSpin;
    QMap<QSpinBox*, QPair<QToolButton*, QToolButton*>> m_spinToButtonsMap;
    bool m_blockManualSignal = false;

    explicit DeadlineSelectorWidget(QWidget *parent = nullptr);

    void setDateTime(const QDateTime &dt);
    QDateTime getDateTime() const;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

signals:
    void dateTimeChanged(const QDateTime &dateTime);

private slots:
    void onManualSpinChanged();

private:
    QWidget* createColumn(const QString &title, QSpinBox *spin, const QString &type);
    QSpinBox* createSubSelector(int min, int max);
    void modifyDateTime(const QString &type, int val);
    void updateDaysInMonth();
};

// ==========================================
// CLASS LOGIC TASKDEADLINE CHÍNH
// ==========================================
class TaskDeadline {
public:
    TaskDeadline();
    TaskDeadline(const QDateTime &deadline);

    QDateTime getDateTime() const;
    void setDateTime(const QDateTime &deadline);

    bool isOverdue(const QString &currentStatus = "") const;
    QString toString(const QString &format = "dd/MM/yyyy hh:mm") const;
    QString toJsonString() const;

private:
    QDateTime m_deadline;
};

#endif // TASKDEADLINE_H