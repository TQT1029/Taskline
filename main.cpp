#include "taskmanager.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QDebug>

void runTests() {
    qDebug() << "--- BẮT ĐẦU TEST TASKMANAGER ---";
    TaskManager manager;

    // 1. Thêm công việc
    manager.addTask("Học C++", TaskStatus::TODO, 1, QDateTime::currentDateTime().addDays(1));
    manager.addTask("Làm UI Qt", TaskStatus::IN_PROGRESS, 2, QDateTime::currentDateTime().addSecs(3600 * 5));
    manager.addTask("Đọc sách", TaskStatus::TODO, 3, QDateTime::currentDateTime().addDays(-1));
    qDebug() << "Sau khi thêm 3 task, số lượng:" << manager.getAllTasks().size();

    // 2. Lọc task
    QList<Task> todoTasks = manager.filterByStatus(TaskStatus::TODO);
    qDebug() << "Số lượng task TODO:" << todoTasks.size();

    // 3. Đánh dấu hoàn thành
    manager.markTaskDone(1);
    qDebug() << "Đã đánh dấu task 1 hoàn thành. Tỷ lệ hoàn thành:" << manager.getCompletionRate() << "%";

    // 4. Cảnh báo quá hạn
    QList<Task> overdue = manager.getOverdueTasks();
    qDebug() << "Số task quá hạn:" << overdue.size() << "(Kỳ vọng 1)";

    // 5. Task sắp đến hạn (trong 24h)
    QList<Task> dueSoon = manager.getTasksDueSoon(24);
    qDebug() << "Số task đến hạn trong 24h:" << dueSoon.size();

    // 6. Xoá task và undo (dùng stack)
    manager.deleteTask(2);
    qDebug() << "Sau khi xoá task 2, số lượng:" << manager.getAllTasks().size();
    manager.undoDelete();
    qDebug() << "Sau khi undo, số lượng:" << manager.getAllTasks().size();

    // 7. Lưu file JSON
    QString file = "tasks.json";
    if (manager.saveToFile(file)) {
        qDebug() << "Đã lưu vào" << file;
    }

    // 8. Đọc file JSON
    TaskManager manager2;
    if (manager2.loadFromFile(file)) {
        qDebug() << "Đã đọc từ file, số lượng:" << manager2.getAllTasks().size();
        for (const Task& t : manager2.getAllTasks()) {
            qDebug() << " - [" << t.getId() << "]" << t.getTitle() << "-" << Task::statusToString(t.getStatus());
        }
    }
    qDebug() << "--- KẾT THÚC TEST ---";
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Bỏ qua load Translator và giao diện tạm thời để test Core
    runTests();

    // Không gọi exec() để app dừng ngay sau khi test xong
    return 0;
}
