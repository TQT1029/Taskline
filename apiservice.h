#ifndef APISERVICE_H
#define APISERVICE_H
#include <QObject>
#include <QNetworkAccessManager>
#include <QJsonArray>
#include <functional>
#include "TaskStats.h"

/**
 * @class APIService
 * @brief Singleton class quản lý các tương tác với RESTful API backend.
 * Cung cấp các phương thức bất đồng bộ để Lấy, Thêm, Sửa, Xóa và Lọc tasks.
 */
class APIService : public QObject
{
    Q_OBJECT
private:
    explicit APIService(QObject *parent = nullptr);
    QNetworkAccessManager *manager;
    
    // Hàm phụ trợ tạo request với các header cơ bản (Auth, Content-Type)
    QNetworkRequest createRequest(const QString &endpoint);

public:
    static APIService& instance();

    // Lấy danh sách toàn bộ Task
    void getTasks(std::function<void(bool, QJsonArray)> callback);
    
    // Gửi yêu cầu tạo mới một Task
    void createNewTask(TaskStats taskStats, std::function<void(bool, QJsonObject)> callback);
    
    // Gửi yêu cầu cập nhật một Task đã tồn tại
    void updateTask(TaskStats taskStats, std::function<void(bool, QJsonArray)> callback);
    
    // Gửi yêu cầu xóa một Task
    void deleteTask(int taskId, std::function<void(bool, QJsonArray)> callback);

    // Lấy danh sách Task đã được sắp xếp từ server
    void sortTasks(std::function<void(bool, QJsonArray)> callback);
    
    // Lấy danh sách Task đã quá hạn
    void getOverdue(std::function<void(bool, QJsonArray)> callback);
};

#endif // APISERVICE_H
