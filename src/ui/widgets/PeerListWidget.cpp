/**
 * @file PeerListWidget.cpp
 * @brief 在线用户列表控件实现
 */

#include "PeerListWidget.h"
#include "../viewmodels/PeerListViewModel.h"
#include <QDebug>

namespace flykylin {
namespace ui {

PeerListWidget::PeerListWidget(PeerListViewModel* viewModel, QWidget* parent)
    : QWidget(parent)
    , m_viewModel(viewModel)
{
    setupUi();
    connectSignals();
    updateOnlineCount();
    
    qDebug() << "[PeerListWidget] Created";
}

void PeerListWidget::setupUi()
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(10, 10, 10, 10);
    m_layout->setSpacing(5);
    
    // 标题
    m_titleLabel = new QLabel("在线用户", this);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(12);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_layout->addWidget(m_titleLabel);
    
    // 搜索框
    m_searchBox = new QLineEdit(this);
    m_searchBox->setPlaceholderText("搜索用户...");
    m_searchBox->setClearButtonEnabled(true);
    m_layout->addWidget(m_searchBox);
    
    // 列表视图
    m_listView = new QListView(this);
    m_listView->setModel(m_viewModel->peerListModel());
    m_listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_listView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_listView->setAlternatingRowColors(true);
    m_layout->addWidget(m_listView);
    
    // 状态标签
    m_statusLabel = new QLabel(this);
    m_statusLabel->setStyleSheet("color: gray; font-size: 9pt;");
    m_layout->addWidget(m_statusLabel);
    
    setLayout(m_layout);
}

void PeerListWidget::connectSignals()
{
    // 搜索框文本变化
    connect(m_searchBox, &QLineEdit::textChanged,
            this, &PeerListWidget::onSearchTextChanged);
    
    // 列表项点击
    connect(m_listView, &QListView::clicked,
            this, &PeerListWidget::onItemClicked);
    
    // 列表项双击
    connect(m_listView, &QListView::doubleClicked,
            this, &PeerListWidget::onItemDoubleClicked);
    
    // ViewModel在线人数变化
    connect(m_viewModel, &PeerListViewModel::onlineCountChanged,
            this, &PeerListWidget::updateOnlineCount);
}

QString PeerListWidget::selectedUserId() const
{
    QModelIndexList selected = m_listView->selectionModel()->selectedIndexes();
    if (selected.isEmpty()) {
        return QString();
    }
    
    QModelIndex index = selected.first();
    return index.data(Qt::UserRole).toString();
}

void PeerListWidget::onSearchTextChanged(const QString& text)
{
    m_viewModel->setFilterKeyword(text);
}

void PeerListWidget::onItemClicked(const QModelIndex& index)
{
    QString userId = index.data(Qt::UserRole).toString();
    if (!userId.isEmpty()) {
        emit userSelected(userId);
        qDebug() << "[PeerListWidget] User selected:" << userId;
    }
}

void PeerListWidget::onItemDoubleClicked(const QModelIndex& index)
{
    QString userId = index.data(Qt::UserRole).toString();
    if (!userId.isEmpty()) {
        emit userDoubleClicked(userId);
        qDebug() << "[PeerListWidget] User double-clicked:" << userId;
    }
}

void PeerListWidget::updateOnlineCount()
{
    int count = m_viewModel->onlineCount();
    m_statusLabel->setText(QString("在线：%1人").arg(count));
}

} // namespace ui
} // namespace flykylin
