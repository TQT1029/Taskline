#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <Qwidget>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QScrollArea>
#include <QCheckBox>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void addNewTask();

private:
    Ui::MainWindow *ui;

    // 2. Khai báo con trỏ nút bấm ở mục private
    QWidget *centralWidget;
    QVBoxLayout *mainLayout;
    QWidget *topBar;
    QWidget *contentArea;
    QScrollArea *scrollArea;
    QWidget *scrollContent;
    QVBoxLayout *contentLayout;
    QLabel *titleLabel;
    QHBoxLayout *topBarLayout;
    QToolButton *menuButton;
    QPushButton *newButton;
};

#endif // MAINWINDOW_H