#include "tasknotifier.h"
#include <QDebug>

TaskNotifier::TaskNotifier(QObject *parent) : QObject(parent) {
    trayIcon = new QSystemTrayIcon(parent);
    trayIcon->setIcon(QIcon(":/Assets/icons/check.png"));

    // --- 1. THIẾT LẬP MENU CHUỘT PHẢI CHO ICON CHẠY NGẦM ---
    trayMenu = new QMenu();
    trayMenu->setStyleSheet(
        "QMenu {"
        "   background-color: #ffffff; border: 1px solid #dcdcdc;"
        "   border-radius: 6px; padding: 4px;"
        "}"
        "QMenu::item {"
        "   padding: 6px 20px; font-size: 13px; color: #333333; border-radius: 4px;"
        "}"
        "QMenu::item:selected {"
        "   background-color: #e74c3c; color: #ffffff;"
        "}"
        );

    closeAction = new QAction("Thoát hoàn toàn", this);
    trayMenu->addAction(closeAction);
    trayIcon->setContextMenu(trayMenu);

    // --- 2. XỬ LÝ SỰ KIỆN KHI BẤM NÚT THOÁT ---
    connect(closeAction, &QAction::triggered, this, []() {
        QApplication::quit();
    });

    // --- 3. CLICK TRÁI HOẶC CLICK ĐÚP ĐỂ HIỆN LẠI APP ---
    connect(trayIcon, &QSystemTrayIcon::activated, this, [parent](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::DoubleClick || reason == QSystemTrayIcon::Trigger) {
            QWidget *mainWindow = qobject_cast<QWidget*>(parent);
            if (mainWindow) {
                mainWindow->show();
                mainWindow->raise();
                mainWindow->activateWindow();
            }
        }
    });

    trayIcon->show();
}

TaskNotifier::~TaskNotifier() {
    if (trayMenu) {
        delete trayMenu;
    }
}

void TaskNotifier::sendNotification(const QString &title, const QString &message,
                             QSystemTrayIcon::MessageIcon iconType,
                             int durationMs) {
    qDebug() << "[NOTIFICATION TRIGGERED] Cửa sổ thông báo xuất hiện -> Title:" << title << "| Message:" << message;

    if (trayIcon && QSystemTrayIcon::supportsMessages()) {
        trayIcon->showMessage(title, message, iconType, durationMs);
    }
}

void TaskNotifier::checkDeadlines(const QList<Task> &allTasks) {
    QDateTime now = QDateTime::currentDateTime();

    qDebug() << "[TIMER] Đang quét kiểm tra các task ngầm lúc:" << now.toString("hh:mm:ss");

    for (const Task &task : allTasks) {
        if (task.getStatus() == TaskStatus::DONE) continue;
        int taskId = task.getId();

        QDateTime deadline = task.getDeadline();
        if (!deadline.isValid()) continue;

        // Tính toán khoảng cách thời gian tính bằng giây
        int64_t secondsToDeadline = now.secsTo(deadline);

        // 1. KIỂM TRA QUÁ HẠN
        if (secondsToDeadline <= 0) {
            if (!notifiedOverdueTasks.contains(taskId)) {
                sendNotification("Cảnh Báo Quá Hạn! 🚨", QString("Công việc '%1' đã trễ hạn.").arg(task.getTitle()), QSystemTrayIcon::Critical);
                notifiedOverdueTasks.insert(taskId);
            }
        }
        // 2. KIỂM TRA SẮP ĐẾN HẠN
        else {
            // THÊM MỚI: Điều kiện cảnh báo trước 1 tiếng (Từ 55 phút đến 60 phút trước deadline)
            if (secondsToDeadline > 3300 && secondsToDeadline <= 3600) {
                if (!notifiedOneHourWarningTasks.contains(taskId)) {
                    sendNotification("Sắp Đến Hạn (Còn 1 Tiếng)! ⏳",
                                     QString("Công việc '%1' sẽ hết hạn sau 1 tiếng nữa!").arg(task.getTitle()),
                                     QSystemTrayIcon::Information);
                    notifiedOneHourWarningTasks.insert(taskId);
                }
            }
            // Điều kiện cảnh báo gấp trước 15 phút (Giữ nguyên như cũ)
            else if (secondsToDeadline > 0 && secondsToDeadline <= 900) {
                if (!notifiedUrgentTasks.contains(taskId)) {
                    sendNotification("Sắp Đến Hạn Gấp! ⏰", QString("Công việc '%1' còn 15 phút nữa hết hạn!").arg(task.getTitle()), QSystemTrayIcon::Warning);
                    notifiedUrgentTasks.insert(taskId);
                }
            }
        }
    }
}

void TaskNotifier::clearTaskCache(const int &taskId) {
    notifiedOverdueTasks.remove(taskId);
    notifiedUrgentTasks.remove(taskId);
    notifiedOneHourWarningTasks.remove(taskId); // Xóa cache hàng chờ cảnh báo 1 tiếng khi Task được sửa/xóa
}
