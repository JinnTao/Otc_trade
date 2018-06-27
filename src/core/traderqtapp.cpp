#include "traderqtapp.h"
#include <qlistwidget.h>
#include <qevent.h>
#include <iostream>
#include <qrect.h>
#include <qpoint.h>
#include <qscrollbar.h>
#include <qstatusbar.h>
#include <qradiobutton.h>
#include <qmessagebox.h>
#define CHECK_BOX_COLUMN 0
extern  QMap<QString,QAtomicInt> gOrderUnfinished;
TraderQtApp::TraderQtApp(QWidget *parent)
 : QMainWindow(parent)
{

 setWindowIcon(QIcon(QStringLiteral(":/Resources/Resources/icon")));
 ui.setupUi(this);
 connect(ui.instrumentLineEdit,SIGNAL(textChanged (const QString & )),this,SLOT(OnRadarChange(const QString & ))); 
 //quoteModel = new QSortFilterProxyModel; //使用代理来对数据filter
  //  quoteModel->setDynamicSortFilter(true);

 //设置
 //ui.quoteTableView->setUpdatesEnabled(false);
 ui.quoteTableView->setAlternatingRowColors(true);
 //   ui.quoteTableView->setModel(quoteModel);
 ui.quoteTableView->setSortingEnabled(true);
 this->ui.quoteTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
 this->ui.quoteTableView->setSelectionMode(QAbstractItemView::SingleSelection);
 this->ui.quoteTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
 this->ui.quoteTableView->resizeColumnsToContents();
// this->ui.quoteTableView->horizontalHeader()->setresize(true);
 this->ui.quoteTableView->horizontalHeader()->setStretchLastSection(true);


 m_btnGroupStrategy=  new QButtonGroup(this);
 m_btnGroupStrategy->addButton(this->ui.radioButtonFix,0);
 m_btnGroupStrategy->addButton(this->ui.radioButtonWW,1);
 m_btnGroupStrategy->addButton(this->ui.radioButtonZak,2);
 this->ui.radioButtonFix->setChecked(true);
 //QScrollBar *vecBar = ui.quoteTableView->verticalScrollBar();
 //

 ui.quoteTableView->verticalScrollBar()->setStyleSheet("QScrollBar:vertical{"  //垂直滑块整体  
                "background:#FFFFFF;"  //背景色  
                "padding-top:20px;"       //上预留位置（放置向上箭头）  
                "padding-bottom:20px;"       //下预留位置（放置向下箭头）  
                "padding-left:3px;"       //左预留位置（美观）  
                "padding-right:3px;"             //右预留位置（美观）  
                "border-left:1px solid #d7d7d7;}"         //左分割线  
                "QScrollBar::handle:vertical{"          //滑块样式  
                "background:#dbdbdb;"           //滑块颜色  
                "border-radius:6px;"           //边角圆润  
                "min-height:80px;}"             //滑块最小高度  
                "QScrollBar::handle:vertical:hover{"            //鼠标触及滑块样式  
                "background:#d0d0d0;}"           //滑块颜色  
                "QScrollBar::add-line:vertical{"              //向下箭头样式  
                "background:url(:/images/resource/images/checkout/down.png) center no-repeat;}"  
                "QScrollBar::sub-line:vertical{"              //向上箭头样式  
                "background:url(:/images/resource/images/checkout/up.png) center no-repeat;}");  
  
 ui.quoteTableView->horizontalScrollBar()->setStyleSheet("QScrollBar:horizontal{"  
                "background:#FFFFFF;"  
                "padding-top:3px;"  
                "padding-bottom:3px;"  
                "padding-left:20px;"  
                "padding-right:20px;}"  
                "QScrollBar::handle:horizontal{"  
                "background:#dbdbdb;"  
                "border-radius:6px;"  
                "min-width:80px;}"  
                "QScrollBar::handle:horizontal:hover{"  
                "background:#d0d0d0;}"  
                "QScrollBar::add-line:horizontal{"  
                "background:url(:/images/resource/images/checkout/right.png) center no-repeat;}"  
                "QScrollBar::sub-line:horizontal{"  
                "background:url(:/images/resource/images/checkout/left.png) center no-repeat;}");  
 // 状态栏label
 this->m_label  = new QLabel(this);
 m_label->setFrameStyle(QFrame::Box | QFrame::Sunken);
 m_label->setText("connecting....");
 m_label->setTextFormat(Qt::RichText);
 m_label->setOpenExternalLinks(true);
 this->ui.statusBar->addPermanentWidget(m_label);




}

TraderQtApp::~TraderQtApp()
{



}

void TraderQtApp::setQuoteModel(QAbstractItemModel *model)
{
  ui.quoteTableView->setModel(model);
}


void TraderQtApp::setCtpControl(ctpControl*manager){

 this->m_ctpController = manager;

}

void TraderQtApp::keyPressEvent ( QKeyEvent * keyevent )   
{  
   
   

 if(QApplication::focusWidget() != ui.instrumentLineEdit || instrumentListWidget->count() == 0)return;  
 int uKey = keyevent->key();  
 Qt::Key key = static_cast<Qt::Key>(uKey);  
 int iIndex = instrumentListWidget->currentRow ();  
 if(key == Qt::Key_Up)  
 {  
  iIndex--;  
  if(iIndex < 0) iIndex = 0;  
  QListWidgetItem *pItem = instrumentListWidget->item(iIndex);  
  pItem->setForeground(Qt::red);  
  instrumentListWidget->setCurrentItem(pItem);  
    
 }  
 else if (key == Qt::Key_Down)  
 {  
  iIndex++;  
  if(iIndex >= instrumentListWidget->count()) iIndex = instrumentListWidget->count() - 1;  
  QListWidgetItem *pItem = instrumentListWidget->item(iIndex);  
  instrumentListWidget->setCurrentItem(pItem);  
 }  

 else if (key == Qt::Key_Enter || key == Qt::Key_Return)  
 {  
  if(iIndex < 0){
   return;
  }
  QString curInstrument = instrumentListWidget->currentItem()->text();
  std::cout << curInstrument.toStdString() << std::endl;
  ui.instrumentLineEdit->setText(curInstrument);  
  instrumentListWidget->setVisible(false);  

  string inst_holding;//保存持仓的合约列表

  vector<QString> * vtInstrumentList = this->m_ctpController->getInstrumentListVt();
  for(int i = 0;i <vtInstrumentList->size();++i){  
  
   if (vtInstrumentList->at(i).indexOf(curInstrument) != -1)  
   {  
     
    inst_holding = inst_holding + vtInstrumentList->at(i).toStdString() + ","; // 以逗号分隔
   
   }  
  }  
  inst_holding = inst_holding.substr(0,inst_holding.length()-1);//去掉最后面的逗号 

  int sizeInstId = inst_holding.size();

  char *charNewIdList_holding_md = new char[sizeInstId+1];

  memset(charNewIdList_holding_md,0,sizeof(char)*(sizeInstId+1));

  strcpy(charNewIdList_holding_md, inst_holding.c_str());

  this->m_ctpController->subscribe_inst_data(charNewIdList_holding_md);
 }  
}

void TraderQtApp::OnRadarChange(const QString &strText)  
{  
 //disconnect(ui.comboBox,SIGNAL(editTextChanged (const QString & )),this,SLOT(OnRadarChange(const QString & )));  
  instrumentListWidget->clear();  
  
 vector<QString> * vtInstrumentList = this->m_ctpController->getInstrumentListVt();
 if(strText.size() == 0 ){
  instrumentListWidget->setVisible(false);  
  return;
 }
 for(int i = 0;i <vtInstrumentList->size();++i){  
  
  if (vtInstrumentList->at(i).indexOf(strText) != -1)  
  {  
     
   instrumentListWidget->addItem(vtInstrumentList->at(i));  
  }  
 }  
 if (instrumentListWidget->count() > 0)  
 {  
  instrumentListWidget->setVisible(true);  
 }  else{
  instrumentListWidget->setVisible(false);  
 }
 //connect(ui.comboBox,SIGNAL(editTextChanged (const QString & )),this,SLOT(OnRadarChange(const QString & )));  
}  
//初始化
void TraderQtApp::initConf(){
 //创建Order Model
 this->createOrderManageModel();
 //创建一个List挂件
 instrumentListWidget = new QListWidget(this);
 instrumentListWidget->setVisible(false);  
 // 策略产品连接
 this->strategyManager = new QStrategy(this,this->m_pOrderManagerModel);
 this->m_pOrderManager = this->strategyManager->getOrderManager();
 

 //this->m_strategyProductModel = new QSortFilterProxyModel; //使用代理来对数据filter
 //声明一个委托  
 m_boxDelegat =new  CheckBoxDelegate(this);  //checkbox
 this->ui.strategyProductTableView->resizeColumnsToContents();
 this->ui.strategyProductTableView->resizeRowsToContents();
 //this->ui.strategyProductTableView->setColumnWidth(1,800);
 this->ui.strategyProductTableView->setAlternatingRowColors(true);
 this->ui.strategyProductTableView->setModel(this->strategyManager->createModel());
 this->ui.strategyProductTableView->setSortingEnabled(true);
 this->ui.strategyProductTableView->setItemDelegate(m_boxDelegat);
 this->ui.strategyProductTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
 this->ui.strategyProductTableView->setSelectionMode(QAbstractItemView::SingleSelection);
 this->ui.strategyProductTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
 this->ui.strategyProductTableView->horizontalHeader()->setStretchLastSection(true);
 //this->ui.strategyProductTableView->horizontalHeader()->setSectionResizeMode(SEQ_TITLE,QHeaderView::Stretch);
 this->ui.strategyProductTableView->resizeColumnsToContents();
 this->ui.strategyProductTableView->resizeRowsToContents();
 this->ui.strategyProductTableView->setColumnWidth(1,200);
 this->ui.strategyProductTableView->setColumnWidth(4,180);
 this->ui.strategyProductTableView->setColumnWidth(5,180);
 this->ui.strategyProductTableView->setColumnWidth(6,180);
 this->ui.strategyProductTableView->setColumnWidth(7,180);
 this->ui.strategyProductTableView->setColumnWidth(8,180);

// this->m_strategyProductModel->setSourceModel(this->strategyManager->createModel());
 this->strategyManager->freshModel();
 connect(this->ui.strategyProductTableView,SIGNAL(doubleClicked(const QModelIndex &)),this,SLOT(onEditSrategyProductClicked(const QModelIndex &)));
 // 运行 停止 删除
 connect(this->ui.runStrategyButton,SIGNAL(clicked()),this,SLOT(onRunStrategyProduct_clicked()));
 connect(this->ui.delStrategyButton,SIGNAL(clicked()),this,SLOT(onDelStrategyProduct_clicked()));
 connect(this->ui.stopStrategyButton,SIGNAL(clicked()),this,SLOT(onStopStrategyProduct_clicked()));
 connect(this->ui.updateStrategyButton,SIGNAL(clicked()),this,SLOT(onUpdateStrategyProduct_clicked()));
 connect(this->ui.settleStrategyButton,SIGNAL(clicked()),this,SLOT(onSettleStrategyProduct_clicked()));
 connect(this->ui.freshTradeParameterButton,SIGNAL(clicked()),this,SLOT(onFreshTrade_clicked()));
 //用户策略
 this->userStrategyConf();

 //browser max buffer size
 this->ui.textBrowser->document()->setMaximumBlockCount(1000);
 //产品Greeks一览表
 this->ui.positionListTableView->setModel(this->strategyManager->createStrategyGreekModel());
 this->ui.positionListTableView->setAlternatingRowColors(true);
 this->ui.positionListTableView->setSortingEnabled(true);
 this->ui.positionListTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
 this->ui.positionListTableView->setSelectionMode(QAbstractItemView::SingleSelection);
 this->ui.positionListTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
 this->ui.positionListTableView->resizeColumnsToContents();
 this->ui.positionListTableView->horizontalHeader()->setStretchLastSection(true);
 this->ui.positionListTableView->setColumnWidth(0,120);
 this->ui.positionListTableView->setColumnWidth(1,70);
 this->ui.positionListTableView->setColumnWidth(2,70);
 this->ui.positionListTableView->setColumnWidth(3,70);
 this->ui.positionListTableView->setColumnWidth(4,70);
 

 //订单管理器
 this->ui.orderManageTableView->setModel(this->m_pOrderManagerModel);
 this->ui.orderManageTableView->setAlternatingRowColors(true);
 this->ui.orderManageTableView->setSortingEnabled(true);
 this->ui.orderManageTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
 this->ui.orderManageTableView->setSelectionMode(QAbstractItemView::SingleSelection);
 this->ui.orderManageTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
 this->ui.orderManageTableView->resizeColumnsToContents();
 this->ui.orderManageTableView->horizontalHeader()->setStretchLastSection(true);

// this->ui.orderManageTableView->setColumnWidth(0,75);
// this->ui.orderManageTableView->setColumnWidth(1,160);
// this->ui.orderManageTableView->setColumnWidth(2,75);
// this->ui.orderManageTableView->setColumnWidth(3,75);
//this->ui.orderManageTableView->setColumnWidth(4,75);
 
 // === 搜索框动态展示
 QRect frameRect = this->geometry();
 QRect viewRect = this->frameGeometry();
 int windowTitleHeight = viewRect.height() - frameRect.height();
 this->strategyManager->setCtpControl(this->m_ctpController);
 QRect instrumentEditRect = ui.instrumentLineEdit->frameGeometry();
  
 instrumentListWidget->resize(instrumentEditRect.width(),160); // 重置大小
 instrumentListWidget->move(instrumentEditRect.x(),instrumentEditRect.y() + windowTitleHeight+instrumentEditRect.height()); // 移动到对应位置
 this->ui.textBrowser->append("THE LOGGING TEXT BROWSER");

 strategyManager->checkProductList();
 strategyManager->freshModel();

 // orderManager Set strategyGreeks Model
 this->m_pOrderManager->setStrategyGreeksModel(this->strategyManager->getStrategyGreekModel());

}

void TraderQtApp::on_newStrategyProduct_clicked(){

 this->strategyManager->loadTemplate(-1);
 if(this->strategyManager->exec() == QDialog::Accepted){
 
 }
}

void TraderQtApp::onEditSrategyProductClicked(const QModelIndex & index){
 int row = index.row(); 
 this->strategyManager->loadTemplate(row);
 if(index.isValid()){
  
  if(this->strategyManager->exec() == QDialog::Accepted){
 
  }
 
 }
}
void TraderQtApp::onStrategyMsg(QString msg){
 this->ui.textBrowser->append(msg);
}

void TraderQtApp :: dataChange(const QModelIndex& lefttop, const QModelIndex &bottomRight){
 this->m_pOrderManagerModel->dataChanged(lefttop,bottomRight);
}

void   TraderQtApp::onCtpControlMsg(CTPSTATUS status)
{

 switch (status)
 {
 case NORMAL:
  this->m_label->setText("Normal connect");
  break;
 case RECONNECT:
  this->strategyManager->freshStrategy();
  gOrderUnfinished.clear();//重置报单清空
  
  this->m_label->setText("Reconnecting");


  break;
 case DISCONNECT:
  this->m_label->setText("Disconnect");
  break;
 default:
  break;
 }

}


QStandardItemModel * TraderQtApp::createOrderManageModel(){
 
 this->m_pOrderManagerModel = new QStandardItemModel(0, 6); //指定了 父指针不需要 自己维护指针删除

 m_pOrderManagerModel->setHeaderData(0, Qt::Horizontal, QString::fromLocal8Bit("交易合约"));
 m_pOrderManagerModel->setHeaderData(1, Qt::Horizontal, QString::fromLocal8Bit("整体应对冲手数"));
 m_pOrderManagerModel->setHeaderData(2, Qt::Horizontal, QString::fromLocal8Bit("实际持仓"));
 m_pOrderManagerModel->setHeaderData(3, Qt::Horizontal, QString::fromLocal8Bit("阈值"));
 m_pOrderManagerModel->setHeaderData(4, Qt::Horizontal, QString::fromLocal8Bit("当前敞口"));
 m_pOrderManagerModel->setHeaderData(5, Qt::Horizontal, QString::fromLocal8Bit("最后更新时间"));


 return m_pOrderManagerModel;
}

void TraderQtApp::userStrategyConf(){

 this->m_pUserStrategy = new QUserStrategy(this->m_pOrderManager,this->strategyManager);
// connect(this->ui.runUserStrategyButton,SIGNAL(clicked()),this,SLOT(OnOptionArbitrageStart()));
// connect(this->ui.stopUserStrategyButton,SIGNAL(clicked()),this,SLOT(OnOptionArbitrageStop()));
 m_pUserStrategy->loadConf();//载入基本配置
 
}

void TraderQtApp::onRunStrategyProduct_clicked(){
 bool status = this->ui.allowTradeCheckBox->isChecked();
 int hedgeType = 0;
 switch(this->m_btnGroupStrategy->checkedId())  
 {  
 case 0:  
  this->ui.textBrowser->append("Fix Strategy.");
  hedgeType = 0;
  break;  
 case 1:  
  this->ui.textBrowser->append("WW Strategy.");  
  hedgeType = 1;
  break;  
 case 2:  
  this->ui.textBrowser->append("Zak Strategy."); 
  hedgeType = 2;
  break;  
 }
 int spanTime = this->ui.requestTimSpanSpinBox->value();
 double aversionCoefficient = this->ui.aversionCoefficientDSB->value();
 this->m_pOrderManager->setAversionCofficient(aversionCoefficient);
 this->m_pOrderManager->setHedgeType(hedgeType);
 this->strategyManager->onRunClicked(status,spanTime);
}
void TraderQtApp::onDelStrategyProduct_clicked(){
 this->strategyManager->onDelClicked();
}
// 批量结算并打印当日pnl
void TraderQtApp::onSettleStrategyProduct_clicked(){
 this->strategyManager->onSettleClicked(false);

}
// 批量更新策略参数   onUpdateStrategyProduct_clicked
void TraderQtApp::onUpdateStrategyProduct_clicked(){
 QMessageBox Msg(QMessageBox::Question, QString::fromLocal8Bit("TV update"), QString::fromLocal8Bit("是否确认刷已选BOOK 参数 -"));

 QAbstractButton *pYesBtn = (QAbstractButton *)Msg.addButton(QString::fromLocal8Bit("是"), QMessageBox::YesRole);

 QAbstractButton *pNoBtn = (QAbstractButton *)Msg.addButton(QString::fromLocal8Bit("否"), QMessageBox::NoRole);


 Msg.exec();



 if (Msg.clickedButton() != pYesBtn)
 {
  return;
 }else{
 
    this->strategyManager->onUpdateClicked();
 }


}

void TraderQtApp::onFreshTrade_clicked(){

 this->strategyManager->onFreshTrade();
}
void TraderQtApp::onStopStrategyProduct_clicked(){
 this->strategyManager->onStopClicked();
}

//=====================


CheckBoxDelegate::CheckBoxDelegate(QObject *parent)
 : QStyledItemDelegate(parent)
{

}

CheckBoxDelegate::~CheckBoxDelegate()
{

}

// 绘制复选框
void CheckBoxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
 QStyleOptionViewItem viewOption(option);
 initStyleOption(&viewOption, index);
 if (option.state.testFlag(QStyle::State_HasFocus))
  viewOption.state = viewOption.state ^ QStyle::State_HasFocus;

 QStyledItemDelegate::paint(painter, viewOption, index);

 if (index.column() == CHECK_BOX_COLUMN)
 {
  bool data = index.model()->data(index, Qt::UserRole).toBool();

  QStyleOptionButton checkBoxStyle;
  checkBoxStyle.state = data ? QStyle::State_On : QStyle::State_Off;
  checkBoxStyle.state |= QStyle::State_Enabled;
  checkBoxStyle.iconSize = QSize(20, 20);
  checkBoxStyle.rect = option.rect;

  QCheckBox checkBox;
  QApplication::style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &checkBoxStyle, painter, &checkBox);
 }
}

// 响应鼠标事件，更新数据
bool CheckBoxDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
 QRect decorationRect = option.rect;

 QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
 if (event->type() == QEvent::MouseButtonPress && decorationRect.contains(mouseEvent->pos()))
 {
  if (index.column() == CHECK_BOX_COLUMN)
  {
   bool data = model->data(index, Qt::UserRole).toBool();
   model->setData(index, !data, Qt::UserRole);
  }
 }

 return QStyledItemDelegate::editorEvent(event, model, option, index);
}







ButtonDelegate::ButtonDelegate(QObject *parent) :
 QItemDelegate(parent)
{
}

void ButtonDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
 QStyleOptionButton* button = m_btns.value(index);
 if (!button) {
  button = new QStyleOptionButton();
  button->rect = option.rect.adjusted(4, 4, -4, -4);
  button->text = "X";
  button->state |= QStyle::State_Enabled;

  (const_cast<ButtonDelegate *>(this))->m_btns.insert(index, button);
 }
  //  painter->save();

 if (option.state & QStyle::State_Selected) {
 // painter->fillRect(option.rect, option.palette.highlight());

 }
 //  painter->restore();
 QApplication::style()->drawControl(QStyle::CE_PushButton, button, painter);

}

bool ButtonDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
 if (event->type() == QEvent::MouseButtonPress) {

  QMouseEvent* e =(QMouseEvent*)event;

  if (option.rect.adjusted(4, 4, -4, -4).contains(e->x(), e->y()) && m_btns.contains(index)) {
   m_btns.value(index)->state |= QStyle::State_Sunken;
  }
 }
 if (event->type() == QEvent::MouseButtonRelease) {
  QMouseEvent* e =(QMouseEvent*)event;

  if (option.rect.adjusted(4, 4, -4, -4).contains(e->x(), e->y()) && m_btns.contains(index)) {
   m_btns.value(index)->state &= (~QStyle::State_Sunken);

   QDialog *d = new QDialog();

   d->setGeometry(0, 0, 200, 200);
    //  d->move(QApplication::desktop()->screenGeometry().center() - d->rect().center());
   d->show();
  }
 }
 return true;
}