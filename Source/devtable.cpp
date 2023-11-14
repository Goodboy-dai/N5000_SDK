#include "devtable.h"
#include "qheaderview.h"
#include <QHeaderView>
#include <iostream>
#include <QMenu>
#include "mainwindow.h"

DevTable::DevTable(int type)
{
    //设置dev_table 列宽自动调整 By override resizeEvent,设置列初始宽度
    this->insertColumn(0);
    this->insertColumn(1);
    this->insertColumn(2);
    this->insertColumn(3);

    this->setHorizontalHeaderItem(0,new QTableWidgetItem("ID"));
    this->setHorizontalHeaderItem(1,new QTableWidgetItem("IP"));
    this->setHorizontalHeaderItem(2,new QTableWidgetItem("Battery"));
    this->setHorizontalHeaderItem(3,new QTableWidgetItem("Status"));

    this->horizontalHeader()->setStretchLastSection(true);
    this->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    this->setSelectionBehavior(QAbstractItemView::SelectRows); //行选中策略

    // 一次性禁止整个表格编辑
    this->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 添加右键菜单及右键菜单对应的回调函数
    this->setContextMenuPolicy(Qt::CustomContextMenu); //自定义右键策略
    connect(this, &QTableWidget::customContextMenuRequested, this, &DevTable::on_dev_table_customContextMenuRequested);
    connect(this, &QTableWidget::itemDoubleClicked, this, &DevTable::on_dev_table_itemDoubleClicked);


    // 创建右键菜单及菜单选项对应的回调函数
    m_devTableRightButtonMenu = new QMenu(this);
    m_devTableBlankRightButtonMenu = new QMenu(this);

    m_devOpenTaskWindowAction = new QAction(tr("Task Window"), this);
    m_devCfgParamAction = new QAction(tr("Config"), this);
    m_devStartSampleAction = new QAction(tr("Start"), this);
    m_devStopSampleAction = new QAction(tr("Stop"), this);
    m_devSaveFileAction = new QAction(tr("Save"), this);
    m_devOpenFileAction = new QAction(tr("Open Log"), this);
    m_addMarkerAction = new QAction(tr("Add Marker"), this);
    m_devCloseAction = new QAction(tr("Close Log"), this);

    if(type == DIRECT_SAMPLE){
        //m_devTableRightButtonMenu->addAction(m_devOpenTaskWindowAction);
        m_devTableRightButtonMenu->addAction(m_addMarkerAction);
        m_devTableRightButtonMenu->addAction(m_devCfgParamAction);
        m_devTableRightButtonMenu->addAction(m_devStartSampleAction);
        m_devTableRightButtonMenu->addAction(m_devStopSampleAction);
        m_devTableRightButtonMenu->addAction(m_devSaveFileAction);
        m_devTableRightButtonMenu->addAction(m_devOpenFileAction);
        m_devTableRightButtonMenu->addAction(m_devCloseAction);

        m_devTableBlankRightButtonMenu->addAction(m_devOpenFileAction);

    } else if(type == VFT_TASK){

        m_devTableRightButtonMenu->addAction(m_devOpenTaskWindowAction);
        m_devTableRightButtonMenu->addAction(m_devOpenFileAction);
        m_devTableRightButtonMenu->addAction(m_devCloseAction);
        m_devTableBlankRightButtonMenu->addAction(m_devOpenFileAction);
    }

    connect(m_addMarkerAction, &QAction::triggered, this, &DevTable::onAddMarkerActionRequested);
    connect(m_devOpenTaskWindowAction, &QAction::triggered, this, &DevTable::onOpenTaskWindowRequested);
    connect(m_devCfgParamAction, &QAction::triggered, this, &DevTable::onCfgParamActionTriggered);
    connect(m_devStartSampleAction, &QAction::triggered, this, &DevTable::onStartSampleActionTriggered);
    connect(m_devStopSampleAction, &QAction::triggered, this, &DevTable::onStopSampleActionTriggered);
    connect(m_devSaveFileAction, &QAction::triggered, this, &DevTable::onSaveFileActionTriggered);
    connect(m_devOpenFileAction, &QAction::triggered, this, &DevTable::onOpenFileActionTriggered);
    connect(m_devCloseAction, &QAction::triggered, this, &DevTable::onCloseDeviceActionRequested);
}

/*
 * 根据窗口大小重新设置表格大小
*/
void DevTable::resizeEvent(QResizeEvent *event){

    resizeColumnWidths();
    //不影响原有事件的处理
    QTableWidget::resizeEvent(event);

}

/*
 * 根据窗口大小计算并设置表格列宽度
*/
void DevTable::resizeColumnWidths(){
    // 获取当前dev_table窗口宽度
    int tableWidth = this->width()-1;

    // 计算每列的平均宽度
    int columnCount = this->columnCount();
    int columnWidth = tableWidth / columnCount;

    // 设置每列的宽度
    for (int col = 0; col < columnCount; ++col)
    {
        this->setColumnWidth(col, columnWidth);
    }
}

/*
 * resize table vector的大小
 * 绑定dev_table和vector可以没有任何数据
 * 会清除所有内容，所以调用此函数后需要重新设置表格内容
*/

void DevTable::resizeRowsCount(int row_count){

    //取消原有的item绑定
    for(int i = 0; i < this->rowCount(); ++i)
        for(int j = 0; j < DEV_TABLE_COLUMN_NUMBER; ++j)
            this->takeItem(i,j);

    // 清除 dev_table_vector 中的对象
    dev_table_vector.clear();

    //重新设置vector的大小
    dev_table_vector.resize(row_count);
    for(int i = 0; i < row_count; ++i)
        dev_table_vector[i].resize(DEV_TABLE_COLUMN_NUMBER);

    // 重新设置表格的行数和关联的 QTableWidgetItemR
    this->setRowCount(row_count);
    for(int i = 0; i < row_count; ++i)
        for(int j = 0; j < DEV_TABLE_COLUMN_NUMBER; ++j)
            this->setItem(i,j,&dev_table_vector[i][j]);
}

/*
 * 自定义右键菜单
*/
void DevTable::on_dev_table_customContextMenuRequested(const QPoint &pos)
{
    QTableWidgetItem *item = this->itemAt(pos);
    if(item)
        m_devTableRightButtonMenu->exec(this->mapToGlobal(pos));
    else
        m_devTableBlankRightButtonMenu->exec(this->mapToGlobal(pos));
}




