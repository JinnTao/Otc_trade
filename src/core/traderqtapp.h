#ifndef TRADERQTAPP_H
#define TRADERQTAPP_H

#include <QtWidgets/QMainWindow>
#include "ui_traderqtapp.h"
#include <qsortfilterproxymodel.h>
#include "ctpcontrol.h"
#include <qlistwidget>
#include "qstrategy.h"
#include <qcheckbox.h>
#include <QItemDelegate>  
#include <qstyleditemdelegate.h>
#include "qOrderManager.h"
#include "quserstrategy.h"

class CheckBoxDelegate;
class TraderQtApp : public QMainWindow
{
    Q_OBJECT

public:
    TraderQtApp(QWidget *parent = 0);
    ~TraderQtApp();
      void setQuoteModel(QAbstractItemModel *model);
      void setCtpControl(ctpControl *ctpManage);
      void initConf();
      QStandardItemModel * TraderQtApp::createOrderManageModel();
      void userStrategyConf();

protected:
        void keyPressEvent(QKeyEvent *e);

private slots:
    void OnRadarChange(const QString &strText);
    void on_newStrategyProduct_clicked();
    
    void onRunStrategyProduct_clicked();
    void onDelStrategyProduct_clicked();
    void onStopStrategyProduct_clicked();
    void onSettleStrategyProduct_clicked();
    void onUpdateStrategyProduct_clicked();
    void onFreshTrade_clicked();
    
    void onEditSrategyProductClicked(const QModelIndex &);
    void onStrategyMsg(QString);
    void onCtpControlMsg(CTPSTATUS);
    void dataChange(const QModelIndex &,const QModelIndex &);

private:
    Ui::TraderQtAppClass ui;
    QSortFilterProxyModel *quoteModel;
    QSortFilterProxyModel *m_strategyProductModel;
    ctpControl *m_ctpController;
    vector<QString> *m_insturment_vt;
    QListWidget *instrumentListWidget;
    QStrategy *strategyManager;
    CheckBoxDelegate *m_boxDelegat;
    QLabel *m_label;
    QOrderManager *m_pOrderManager;
    QStandardItemModel *m_pOrderManagerModel;
    QUserStrategy *m_pUserStrategy;
    vector<string> m_workDayVector;
    QButtonGroup *m_btnGroupStrategy;
};
// 设定check box 代理
class CheckBoxDelegate : public QStyledItemDelegate  
{  
    Q_OBJECT  
  
public:  
    CheckBoxDelegate(QObject *parent = 0);  
    ~CheckBoxDelegate();
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);
};  
// button 代理
class ButtonDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    explicit ButtonDelegate(QObject *parent = 0);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);

signals:

public slots:

private:
    QMap<QModelIndex, QStyleOptionButton*> m_btns;

};

#endif // TRADERQTAPP_H