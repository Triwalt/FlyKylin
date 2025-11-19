/**
 * @file ChatWindow.h
 * @brief Chat window for 1v1 messaging
 * @author FlyKylin Development Team
 * @date 2024-11-19
 */

#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include "ui/viewmodels/ChatViewModel.h"

namespace flykylin {
namespace ui {

/**
 * @class ChatWindow
 * @brief Chat window widget for 1v1 text messaging
 * 
 * Features:
 * - Message history display with scrolling
 * - Text input area
 * - Send button
 * - Auto-scroll to bottom on new messages
 * - Message bubbles (sent/received)
 */
class ChatWindow : public QWidget {
    Q_OBJECT
    
public:
    /**
     * @brief Constructor
     * @param parent Parent widget
     */
    explicit ChatWindow(QWidget* parent = nullptr);
    
    /**
     * @brief Destructor
     */
    ~ChatWindow() override;
    
    /**
     * @brief Set chat peer
     * @param peerId Peer user ID
     * @param peerName Peer display name
     */
    void setPeer(const QString& peerId, const QString& peerName);
    
    /**
     * @brief Get current peer ID
     * @return QString Peer ID
     */
    QString currentPeerId() const;
    
    /**
     * @brief Clear chat history (UI and data)
     */
    void clearHistory();
    
signals:
    /**
     * @brief Window closed
     */
    void closed();
    
protected:
    /**
     * @brief Handle close event
     * @param event Close event
     */
    void closeEvent(QCloseEvent* event) override;
    
private slots:
    void onSendButtonClicked();
    void onMessagesUpdated();
    void onMessageReceived(const flykylin::core::Message& message);
    void onMessageSent(const flykylin::core::Message& message);
    void onMessageFailed(const flykylin::core::Message& message, QString error);
    
private:
    void setupUI();
    void connectSignals();
    void refreshMessageList();
    void scrollToBottom();
    void addMessageBubble(const flykylin::core::Message& message);
    
    // UI Components
    QVBoxLayout* m_mainLayout;
    QLabel* m_peerNameLabel;
    QScrollArea* m_scrollArea;
    QWidget* m_messageContainer;
    QVBoxLayout* m_messageLayout;
    QTextEdit* m_inputEdit;
    QPushButton* m_sendButton;
    
    // View Model
    ui::ChatViewModel* m_viewModel;
    
    // Current peer info
    QString m_currentPeerId;
    QString m_currentPeerName;
};

} // namespace ui
} // namespace flykylin
