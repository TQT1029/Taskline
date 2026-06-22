#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include "task.h"
#include <QList>
#include <QStack>
#include <QString>

class TaskManager
{
private:
    QList<Task> tasks;
    QStack<Task> undoStack;
    int nextId;

public:
    TaskManager();

    // Thêm, sửa, xóa
    void addTask(const QString &title, const QString &description, TaskStatus status, int priority, const QDateTime &deadline);
    bool editTask(int id, const QString &title, const QString &description, TaskStatus status, int priority, const QDateTime &deadline);
    bool deleteTask(int id);
    bool markTaskDone(int id);
    
    // Undo
    void undoDelete();

    // Lọc, tìm kiếm, sắp xếp
    QList<Task> getAllTasks() const;
    QList<Task> searchTasks(const QString &keyword) const;
    QList<Task> filterByStatus(TaskStatus status) const;
    QList<Task> filterByPriority(int priority) const;
    void sortByDeadline();

    // Thống kê, cảnh báo
    QList<Task> getOverdueTasks() const;
    QList<Task> getTasksDueSoon(int hours) const; // Các task sắp đến hạn trong vòng N giờ
    double getCompletionRate() const;

    // Lưu trữ
    bool saveToFile(const QString &filePath) const;
    bool loadFromFile(const QString &filePath);
};

#endif // TASKMANAGER_H
