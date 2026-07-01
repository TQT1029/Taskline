#ifndef TASKNOTIFIER_H
#define TASKNOTIFIER_H

#include <QSystemTrayIcon>
#include <QIcon>
#include <QObject>
#include <QTimer>
#include <QDateTime>
#include <QSet>
#include <QMenu>        // Thêm thư viện quản lý Menu thả xuống
#include <QAction>      // Thêm thư viện quản lý nút bấm trong Menu
#include <QApplication> // Thêm thư viện để gọi lệnh tắt hẳn App
#include "task.h"       //
#include <QDebug>

class TaskNotifier : public QObject {
    Q_OBJECT
private:
    QSystemTrayIcon *trayIcon = nullptr; //
    QTimer *checkTimer = nullptr; //
    QMenu *trayMenu = nullptr;     // Menu xuất hiện khi click chuột phải
    QAction *closeAction = nullptr; // Nút bấm "Thoát" nằm trong Menu

    QSet<QString> notifiedOverdueTasks; //
    QSet<QString> notifiedUrgentTasks; //

public:
    inline explicit TaskNotifier(QObject *parent = nullptr) : QObject(parent) { //
        trayIcon = new QSystemTrayIcon(parent); //
        trayIcon->setIcon(QIcon(":/Assets/icons/check.png")); //

        // --- 1. THIẾT LẬP MENU CHUỘT PHẢI CHO ICON CHẠY NGẦM ---
        trayMenu = new QMenu();
        // Định dạng style cho menu nhìn hiện đại, chữ to rõ ràng
        trayMenu->setStyleSheet(
            "QMenu {"
            "   background-color: #ffffff; border: 1px solid #dcdcdc;"
            "   border-radius: 6px; padding: 4px;"
            "}"
            "QMenu::item {"
            "   padding: 6px 20px; font-size: 13px; color: #333333; border-radius: 4px;"
            "}"
            "QMenu::item:selected {"
            "   background-color: #e74c3c; color: #ffffff;" // Di chuột vào nút Close sẽ có màu đỏ cảnh báo
            "}"
            );

        // Tạo nút "Thoát ứng dụng"
        closeAction = new QAction("Thoát hoàn toàn", this);
        trayMenu->addAction(closeAction);

        // Gắn Menu này vào Icon khay hệ thống
        trayIcon->setContextMenu(trayMenu);

        // --- 2. XỬ LÝ SỰ KIỆN KHI BẤM NÚT THOÁT ---
        connect(closeAction, &QAction::triggered, this, []() {
            // Lệnh tối cao tắt ứng dụng ngay lập tức, bỏ qua mọi closeEvent chắn đường
            QApplication::quit();
        });

        // --- 3. CLICK TRÁI HOẶC CLICK ĐÚP ĐỂ HIỆN LẠI APP ---
        connect(trayIcon, &QSystemTrayIcon::activated, this, [parent](QSystemTrayIcon::ActivationReason reason) { //
            if (reason == QSystemTrayIcon::DoubleClick || reason == QSystemTrayIcon::Trigger) { //
                QWidget *mainWindow = qobject_cast<QWidget*>(parent); //
                if (mainWindow) { //
                    mainWindow->show(); //
                    mainWindow->raise(); //
                    mainWindow->activateWindow(); //
                }
            }
        });

        trayIcon->show(); //

        // Khởi tạo bộ quét ngầm tự động chạy
        // checkTimer = new QTimer(this); //
        // checkTimer->setInterval(10000); //
        // checkTimer->start(); //
    }

    // Hàm bắn thông báo thủ công
    inline void sendNotification(const QString &title, const QString &message,
                                 QSystemTrayIcon::MessageIcon iconType = QSystemTrayIcon::Information,
                                 int durationMs = 3000) { //
        qDebug() << "[NOTIFICATION TRIGGERED] Cửa sổ thông báo xuất hiện -> Title:" << title << "| Message:" << message;

        if (trayIcon && QSystemTrayIcon::supportsMessages()) { //
            trayIcon->showMessage(title, message, iconType, durationMs); //
        }
    }

    // Hàm tự động quét deadline
    inline void checkDeadlines(const QList<Task> &allTasks) {
        // Lấy thời gian hiện tại và ép chắc chắn về Local Time
        QDateTime now = QDateTime::currentDateTime().toLocalTime();

        qDebug() << "[TIMER] Đang quét kiểm tra các task ngầm lúc:" << now.toString("hh:mm:ss");

        for (const Task &task : allTasks) {
            if (task.getStatus() == TaskStatus::DONE) continue;
            QString taskId = task.getId();

            // ÉP DEADLINE CỦA TASK VỀ CÙNG HỆ LOCAL TIME
            QDateTime deadline = task.getDeadline().toLocalTime();
            if (!deadline.isValid()) continue;

            qDebug() << "   -> Khảo sát Task:" << task.getTitle()
                     << "Status:" << (int)task.getStatus()
                     << "Deadline:" << deadline.toString("yyyy-MM-dd hh:mm:ss")
                     << "Current Now:" << now.toString("yyyy-MM-dd hh:mm:ss");

            if (now >= deadline) { //
                qDebug()<<"bat dau tb";
                if (!notifiedOverdueTasks.contains(taskId)) { //
                    qDebug()<< "da tb";
                    sendNotification("Cảnh Báo Quá Hạn! 🚨", QString("Công việc '%1' đã trễ hạn.").arg(task.getTitle()), QSystemTrayIcon::Critical); //
                    notifiedOverdueTasks.insert(taskId); //
                }
            } else { //
                int64_t secondsToDeadline = now.secsTo(deadline); //
                if (secondsToDeadline > 0 && secondsToDeadline <= 900) { //
                    if (!notifiedUrgentTasks.contains(taskId)) { //
                        sendNotification("Sắp Đến Hạn! ⏰", QString("Công việc '%1' sắp hết hạn!").arg(task.getTitle()), QSystemTrayIcon::Warning); //
                        notifiedUrgentTasks.insert(taskId); //
                    }
                }
            }
        }
    }

    inline void clearTaskCache(const QString &taskId) { //
        notifiedOverdueTasks.remove(taskId); //
        notifiedUrgentTasks.remove(taskId); //
    }

    // Hủy Menu khi giải phóng class để không bị rò rỉ bộ nhớ RAM
    inline ~TaskNotifier() {
        if (trayMenu) {
            delete trayMenu;
        }
    }
};

#endif // TASKNOTIFIER_H