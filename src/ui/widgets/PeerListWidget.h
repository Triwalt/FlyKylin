/**
 * @file PeerListWidget.h
 * @brief 在线用户列表控件
 */

#ifndef PEERLISTWIDGET_H
#define PEERLISTWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QListView>
#include <QLineEdit>
#include <QLabel>

namespace flykylin {
namespace ui {

class PeerListViewModel;

/**
 * @class PeerListWidget
 * @brief 显示在线用户列表的UI控件
 * 
 * 功能：
 * - 显示在线用户列表
 * - 搜索框过滤用户
 * - 显示在线人数
 */
class PeerListWidget : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param viewModel 视图模型
     * @param parent 父控件
     */
    explicit PeerListWidget(PeerListViewModel* viewModel, QWidget* parent = nullptr);

    /**
     * @brief 获取当前选中的用户ID
     * @return 用户ID，无选中返回空字符串
     */
    QString selectedUserId() const;

signals:
    /**
     * @brief 用户被选中信号
     * @param userId 用户ID
     */
    void userSelected(const QString& userId);

    /**
     * @brief 用户被双击信号（可用于打开聊天窗口）
     * @param userId 用户ID
     */
    void userDoubleClicked(const QString& userId);

private slots:
    /**
     * @brief 处理搜索框文本变化
     * @param text 搜索文本
     */
    void onSearchTextChanged(const QString& text);

    /**
     * @brief 处理列表项选中
     * @param index 索引
     */
    void onItemClicked(const QModelIndex& index);

    /**
     * @brief 处理列表项双击
     * @param index 索引
     */
    void onItemDoubleClicked(const QModelIndex& index);

    /**
     * @brief 更新在线人数显示
     */
    void updateOnlineCount();

private:
    /**
     * @brief 初始化UI
     */
    void setupUi();

    /**
     * @brief 连接信号槽
     */
    void connectSignals();

private:
    PeerListViewModel* m_viewModel;     ///< 视图模型
    QVBoxLayout* m_layout;              ///< 主布局
    QLabel* m_titleLabel;               ///< 标题标签
    QLineEdit* m_searchBox;             ///< 搜索框
    QListView* m_listView;              ///< 列表视图
    QLabel* m_statusLabel;              ///< 状态标签（在线人数）
};

} // namespace ui
} // namespace flykylin

#endif // PEERLISTWIDGET_H
