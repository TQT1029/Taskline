#include "apiservice.h"
#include "task.h" // Sửa lỗi HACK: include task.h thay vì task.cpp để tránh duplicate symbol
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

// Khởi tạo Singleton pattern, đảm bảo chỉ có 1 instance của APIService trong suốt vòng đời ứng dụng
APIService& APIService::instance() {
    static APIService _instance;
    return _instance;
}

// Constructor khởi tạo QNetworkAccessManager dùng để gọi API
APIService::APIService(QObject *parent) : QObject(parent) {
    manager = new QNetworkAccessManager(this);
}

// Hàm phụ trợ tạo QNetworkRequest với các cấu hình chung (URL, Timeout, Headers)
QNetworkRequest APIService::createRequest(const QString &endpoint) {
    QNetworkRequest request(QUrl("https://localhost:3000" + endpoint));
    request.setTransferTimeout(5000); // Tránh treo UI nếu server không phản hồi sau 5s
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", "Bearer PassworD"); // Token giả lập
    return request;
}

// Hàm phụ trợ bỏ qua lỗi SSL khi làm việc với localhost (chứng chỉ tự ký)
void handleSslErrors(QNetworkReply *reply) {
    QObject::connect(reply, &QNetworkReply::sslErrors, [reply]() {
        reply->ignoreSslErrors(); 
    });
}

/**
 * @brief Lấy toàn bộ danh sách Task từ server
 * @param callback Hàm callback trả về trạng thái (thành công/thất bại) và dữ liệu JSON
 */
void APIService::getTasks(std::function<void(bool, QJsonArray)> callback) {
    QNetworkRequest request = createRequest("/tasks"); // GET endpoint
    QNetworkReply *reply = manager->get(request);
    handleSslErrors(reply); // Quan trọng để localhost HTTPS không bị chặn

    connect(reply, &QNetworkReply::finished, [reply, callback]() {
        if (reply->error() != QNetworkReply::NoError) {
            callback(false, QJsonArray());
            reply->deleteLater(); // Dọn dẹp bộ nhớ
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        callback(true, doc.array());
        reply->deleteLater();
    });
}

/**
 * @brief Tạo mới một Task và gửi lên server
 */
void APIService::createNewTask(TaskStats taskStats, std::function<void(bool, QJsonArray)> callback) {
    QNetworkRequest request = createRequest("/tasks"); // POST endpoint
    
    QJsonObject json;
    json["ID"] = taskStats.id;
    json["Title"] = taskStats.title;
    json["Description"] = taskStats.description;
    json["Status"] = Task::statusToString(taskStats.status);
    json["Priority"] = taskStats.priority;
    json["Deadline"] = taskStats.deadline.toString("yyyy-MM-dd HH:mm:ss");

    QByteArray data = QJsonDocument(json).toJson();
    QNetworkReply *reply = manager->post(request, data);
    handleSslErrors(reply);

    connect(reply, &QNetworkReply::finished, [reply, callback]() {
        if (reply->error() != QNetworkReply::NoError) {
            callback(false, QJsonArray());
            reply->deleteLater();
            return;
        }
        
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        callback(true, doc.array());
        reply->deleteLater();
    });
}

/**
 * @brief Cập nhật thông tin của một Task
 */
void APIService::updateTask(TaskStats taskStats, std::function<void(bool, QJsonArray)> callback) {
    // Thường update dùng ID trên URL, ví dụ: /tasks/123
    QNetworkRequest request = createRequest(QString("/tasks/%1").arg(taskStats.id)); 
    
    QJsonObject json;
    json["Title"] = taskStats.title;
    json["Description"] = taskStats.description;
    json["Status"] = Task::statusToString(taskStats.status);
    json["Priority"] = taskStats.priority;
    json["Deadline"] = taskStats.deadline.toString("yyyy-MM-dd HH:mm:ss");

    QByteArray data = QJsonDocument(json).toJson();
    QNetworkReply *reply = manager->put(request, data); // REST chuẩn dùng PUT/PATCH cho update
    handleSslErrors(reply);

    connect(reply, &QNetworkReply::finished, [reply, callback]() {
        if (reply->error() != QNetworkReply::NoError) {
            callback(false, QJsonArray());
            reply->deleteLater();
            return;
        }
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        callback(true, doc.array());
        reply->deleteLater();
    });
}

/**
 * @brief Xóa một Task dựa trên ID
 */
void APIService::deleteTask(QString taskId, std::function<void(bool, QJsonArray)> callback) {
    QNetworkRequest request = createRequest(QString("/tasks/%1").arg(taskId)); 
    QNetworkReply *reply = manager->deleteResource(request);
    handleSslErrors(reply);

    connect(reply, &QNetworkReply::finished, [reply, callback]() {
        if (reply->error() != QNetworkReply::NoError) {
            callback(false, QJsonArray());
            reply->deleteLater();
            return;
        }
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        callback(true, doc.array());
        reply->deleteLater();
    });
}

/**
 * @brief Lấy danh sách Task đã được server sắp xếp
 */
void APIService::sortTasks(std::function<void(bool, QJsonArray)> callback) {
    QNetworkRequest request = createRequest("/tasks/sorted");
    QNetworkReply *reply = manager->get(request);
    handleSslErrors(reply);

    connect(reply, &QNetworkReply::finished, [reply, callback]() {
        if (reply->error() != QNetworkReply::NoError) {
            callback(false, QJsonArray());
            reply->deleteLater();
            return;
        }
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        callback(true, doc.array());
        reply->deleteLater();
    });
}

/**
 * @brief Lấy danh sách Task quá hạn từ server
 */
void APIService::getOverdue(std::function<void(bool, QJsonArray)> callback) {
    QNetworkRequest request = createRequest("/tasks/overdue");
    QNetworkReply *reply = manager->get(request);
    handleSslErrors(reply);

    connect(reply, &QNetworkReply::finished, [reply, callback]() {
        if (reply->error() != QNetworkReply::NoError) {
            callback(false, QJsonArray());
            reply->deleteLater();
            return;
        }
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        callback(true, doc.array());
        reply->deleteLater();
    });
}
