#ifndef TASKSTATUSBAR_H
#define TASKSTATUSBAR_H

#include <QLabel>
#include <QProgressBar>
#include <QList>
#include <QPropertyAnimation> // Thêm thư viện này để chạy animation trượt
#include "task.h" //

class TaskStatusTracker {
public:
    QLabel *statusLabel = nullptr; //
    QProgressBar *progressBar = nullptr; //

    // Khởi tạo các thành phần UI và ném vào bottomBar của MainWindow
    inline void setupInWidget(QWidget *parent) { //
        statusLabel = new QLabel("Hoàn thành: 0/0", parent); //
        statusLabel->setStyleSheet("font-weight: bold; color: #555555; font-size: 13px; border: none; background: transparent;"); //

        progressBar = new QProgressBar(parent); //
        progressBar->setRange(0, 100); //
        progressBar->setValue(0); //
        progressBar->setTextVisible(true); //
        progressBar->setFormat("%p%"); //

        // --- Cho phép thanh tự động kéo giãn tối đa ---
        progressBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed); //
        progressBar->setMinimumWidth(150); // Chiều rộng tối thiểu khi thu nhỏ app
        progressBar->setMaximumHeight(16); // Giữ độ cao mảnh mai

        progressBar->setStyleSheet(
            "QProgressBar {"
            "   border: 1px solid #bdc3c7;"
            "   border-radius: 4px;"
            "   text-align: center;"
            "   background-color: #ffffff;"
            "   color: #333333;"
            "   font-weight: bold;"
            "   font-size: 11px;"
            "}"
            "QProgressBar::chunk {"
            "   background-color: #2ecc71;"
            "   border-radius: 3px;"
            "}"
            ); //
    }

    // Tính toán % dựa trên file dữ liệu JSON gốc và chạy hiệu ứng trượt từ từ
    inline void updateStatistics(const QList<Task> &taskList) { //
        if (!statusLabel || !progressBar) return; //

        int totalTasks = taskList.size(); //
        int completedTasks = 0; //

        for (const Task &task : taskList) { //
            if (task.getStatus() == TaskStatus::DONE) { //
                completedTasks++; //
            }
        } //

        statusLabel->setText(QString("Hoàn thành: %1/%2").arg(completedTasks).arg(totalTasks)); //

        // Tính toán đích đến phần trăm mới
        int targetPercentage = 0;
        if (totalTasks > 0) { //
            targetPercentage = static_cast<int>((static_cast<double>(completedTasks) / totalTasks) * 100); //
        } //

        // --- HIỆU ỨNG TRƯỢT TỪ TỪ CHO THANH TIẾN TRÌNH ---
        // Lấy giá trị hiện tại của progress bar làm điểm bắt đầu cho animation
        int currentPercentage = progressBar->value();

        // Xóa các animation cũ đang chạy dở trên progressBar (nếu có) để tránh xung đột
        progressBar->setProperty("animatedValue", currentPercentage);

        QPropertyAnimation *anim = new QPropertyAnimation(progressBar, "value");
        anim->setDuration(400); // Tốc độ trượt: 400 mili-giây (thay đổi nếu muốn trượt chậm/nhanh hơn)
        anim->setStartValue(currentPercentage); // Trượt từ vị trí phần trăm cũ
        anim->setEndValue(targetPercentage);    // Trượt tới vị trí phần trăm mới
        anim->setEasingCurve(QEasingCurve::OutCubic); // Hiệu ứng giảm tốc mượt mà khi gần đến đích

        anim->start(QAbstractAnimation::DeleteWhenStopped); // Chạy xong tự giải phóng RAM
    }
};

#endif // TASKSTATUSBAR_H