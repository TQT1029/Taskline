#include "tasksearch.h"

CustomSearchEdit::CustomSearchEdit(QWidget *parent, QToolButton *btn, QCompleter *completer)
    : QLineEdit(parent), m_btn(btn), m_completer(completer) {}

void CustomSearchEdit::focusOutEvent(QFocusEvent *event) {
    QLineEdit::focusOutEvent(event);

    QTimer::singleShot(200, this, [this]() {
        if (m_completer && m_completer->popup() && m_completer->popup()->isVisible()) {
            return;
        }
        if (this->text().trimmed().isEmpty()) {
            // ANIMAION THU NHỎ LẠI KHI MẤT FOCUS VÀ Ô TRỐNG
            QPropertyAnimation *anim = new QPropertyAnimation(this, "maximumWidth");
            anim->setDuration(250); // Chạy trong 250ms
            anim->setStartValue(160);
            anim->setEndValue(0);
            anim->setEasingCurve(QEasingCurve::OutCubic); // Mượt mà ở cuối

            connect(anim, &QPropertyAnimation::finished, this, [this]() {
                this->setVisible(false);
                if (m_btn) m_btn->setVisible(true);
            });
            anim->start(QAbstractAnimation::DeleteWhenStopped);
        }
    });
}

TaskSearchHelper::TaskSearchHelper(QObject *parent) : QObject(parent) {}

void TaskSearchHelper::setupInWidget(QWidget *parentBar) {
    // 1. Tạo nút kính lúp
    searchButton = new QToolButton(parentBar);
    searchButton->setText("🔍");
    searchButton->setCursor(Qt::PointingHandCursor);
    searchButton->setStyleSheet(
        "QToolButton {"
        "   background: transparent; border: none; font-size: 16px; padding: 5px;"
        "}"
        "QToolButton:hover { color: #3498db; }"
        );

    // 2. Cấu hình Completer
    completerModel = new QStringListModel(this);
    searchCompleter = new QCompleter(this);
    searchCompleter->setModel(completerModel);
    searchCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    searchCompleter->setCompletionMode(QCompleter::PopupCompletion);

    // 3. Khởi tạo ô tìm kiếm Custom
    searchEdit = new CustomSearchEdit(parentBar, searchButton, searchCompleter);
    searchEdit->setPlaceholderText("Tìm kiếm...");
    searchEdit->setVisible(false);
    searchEdit->setStyleSheet(
        "QLineEdit {"
        "   background-color: #ffffff; border: 1px solid #bdc3c7;"
        "   border-radius: 12px; padding: 3px 10px; font-size: 13px; color: #333333;"
        "}"
        "QLineEdit:focus { border: 1px solid #3498db; }"
        );
    searchEdit->setCompleter(searchCompleter);

    // 4. Timer Debounce chống lag
    searchTimer = new QTimer(this);
    searchTimer->setSingleShot(true);

    // --- HIỆU ỨNG ANIMATION KHI BẤM VÀO KÍNH LÚP ---
    connect(searchButton, &QToolButton::clicked, this, [this]() {
        searchButton->setVisible(false);
        searchEdit->setVisible(true);
        searchEdit->setFocus();

        // Tạo hiệu ứng phóng to kích thước từ 0px -> 160px
        QPropertyAnimation *anim = new QPropertyAnimation(searchEdit, "maximumWidth");
        anim->setDuration(300); // 300 mili-giây
        anim->setStartValue(0);
        anim->setEndValue(160);
        anim->setEasingCurve(QEasingCurve::OutBack); // Hiệu ứng lao quá đích nhẹ rồi lùi lại nhìn cực hiện đại
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    });

    connect(searchEdit, &QLineEdit::textChanged, this, [this]() {
        searchTimer->start(400);
    });

    connect(searchEdit, &QLineEdit::returnPressed, this, [this]() {
        if (searchCompleter) searchCompleter->popup()->hide();
    });

    connect(searchTimer, &QTimer::timeout, this, &TaskSearchHelper::searchRequested);
}

void TaskSearchHelper::updateSuggestions(const QList<Task> &allTasks) {
    if (!searchEdit || !completerModel) return;
    QString keyword = searchEdit->text().trimmed();
    QStringList suggestions;
    for (const Task &task : allTasks) {
        if (task.getTitle().contains(keyword, Qt::CaseInsensitive)) {
            suggestions << task.getTitle();
        }
    }
    suggestions.removeDuplicates();
    completerModel->setStringList(suggestions);
}
