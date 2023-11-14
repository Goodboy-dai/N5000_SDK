#include "mainwindow.h"
#include "waveplot.h"
#include <QWidget>


WavePlot::WavePlot(QWidget* pParent, QString windowName, int type)
    : QCustomPlot(pParent)
{
    //new window
    plot_window = new MyWindow();
    plot_window->resize(800,600);
    plot_window->setWindowTitle(windowName);
    //plot_window->show();
    connect(plot_window, &MyWindow::windowShowSig, this, &WavePlot::windowShowCb);
    connect(plot_window, &MyWindow::windowCloseSig, this, &WavePlot::windowCloseCb);

    //new layout
    plot_layout = new QHBoxLayout(plot_window);
    plot_layout->addWidget(this);


    //marker table
    markerTable = new QTableWidget();
    plot_layout->addWidget(markerTable);
    markerTable->setFixedWidth(100);
    markerTable->insertColumn(0);
    markerTable->setHorizontalHeaderItem(0,new QTableWidgetItem("Marker(s)"));
    markerTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    markerTable->setColumnWidth(0,80);
    markerTable->setContextMenuPolicy(Qt::CustomContextMenu);
    markerTable->setSelectionMode(QAbstractItemView::SingleSelection);  //设置单选


    //右键空白处可以新建marker和清空所有marker
    //右键表格内容可以删除marker数据
    //双击表格内容可以编辑marker数据
    m_markerTableBlankRightButtonMenu = new QMenu(markerTable);
    m_addMarkerAction = new QAction(tr("add"),markerTable);
    m_clearMarkerAction = new QAction(tr("clear"), markerTable);
    m_markerTableRightButtonMenu = new QMenu(markerTable);
    m_delMarkerAction = new QAction(tr("delete"), markerTable);

    //空白处右键
    m_markerTableBlankRightButtonMenu->addAction(m_addMarkerAction);
    m_markerTableBlankRightButtonMenu->addSeparator();
    m_markerTableBlankRightButtonMenu->addAction(m_clearMarkerAction);

    //表格内容右键
    m_markerTableRightButtonMenu->addAction(m_addMarkerAction);
    m_markerTableRightButtonMenu->addAction(m_delMarkerAction);
    m_markerTableRightButtonMenu->addSeparator();
    m_markerTableRightButtonMenu->addAction(m_clearMarkerAction);

    connect(m_addMarkerAction, SIGNAL(triggered()), this, SLOT(onAddMarkerActionTriggered()));
    connect(m_clearMarkerAction, SIGNAL(triggered()), this, SLOT(onClearMarkerActionTriggered()));
    connect(m_delMarkerAction, SIGNAL(triggered()), this, SLOT(onDelMarkerActionTriggered()));

    connect(markerTable, &QTableWidget::cellChanged, this, &WavePlot::onEditMarkerTableTriggered);

    if(type == DIRECT_SAMPLE){
        connect(markerTable, &QTableWidget::customContextMenuRequested, this, &WavePlot::onMarkerTableCustomContextMenuRequested);
        setMarkerTableVisible(true);
    }
    else if(type == VFT_TASK){
        //设置表格不可以编辑
        markerTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
        //默认不显示markerTable
        setMarkerTableVisible(false);
    }


    //设置坐标轴名称
    this->xAxis->setLabel("time(s)");
    this->yAxis->setLabel("adc raw");
    //隐藏网格线
    this->xAxis->grid()->setVisible(false);
    this->yAxis->grid()->setVisible(false);

//    //for test
    this->xAxis2->setVisible(false);
//    this->xAxis2->setLabel("");
//    this->xAxis2->setTicks(false);
    this->yAxis2->setVisible(false);
//    this->yAxis2->setLabel("");
//    this->yAxis2->setTicks(false);
//    this->xAxis2->setBasePen(QPen(Qt::white));
//    this->xAxis2->setTickPen(QPen(Qt::white));
//    this->xAxis2->setTickLabelColor(Qt::white);
//    this->xAxis2->setLabelColor(Qt::white);
//    this->yAxis2->setBasePen(QPen(Qt::white));
//    this->yAxis2->setTickPen(QPen(Qt::white));
//    this->yAxis2->setTickLabelColor(Qt::white);
//    this->yAxis2->setLabelColor(Qt::white);

    //滚轮缩放
    this->setInteractions(QCP::iRangeZoom);
    this->axisRect()->setRangeZoomFactor(1.0,1.0);

    //设置背景颜色，坐标轴相关颜色
    this->setBackground(QBrush(Qt::black));
    this->xAxis->setBasePen(QPen(Qt::white));
    this->xAxis->setTickPen(QPen(Qt::white));
    this->xAxis->setTickLabelColor(Qt::white);
    this->xAxis->setLabelColor(Qt::white);
    this->yAxis->setBasePen(QPen(Qt::white));
    this->yAxis->setTickPen(QPen(Qt::white));
    this->yAxis->setTickLabelColor(Qt::white);
    this->yAxis->setLabelColor(Qt::white);

    //设置边界宽度
    //this->axisRect()->setMargins(QMargins(0,0,0,0));

    QFont font;
    font.setPixelSize(10);
    this->legend->setFont(font);
    this->legend->setSelectedFont(font);
    //图例可见
    this->legend->setVisible(true);
    //设置图例颜色
    this->legend->setTextColor(QColor(255,255,255,255));//设置图例文字颜色
    //图例在右上角
    this->axisRect()->insetLayout()->setInsetAlignment(0,Qt::AlignTop|Qt::AlignRight);
    //设置图例背景
    this->legend->setBrush(QColor(255,255,255,0));
    //设置边框隐藏
    this->legend->setBorderPen(Qt::NoPen);
    //启用图例的可拖拽性
    this->legend->setSelectableParts(QCPLegend::spItems);
    //设置图例间隔
    this->legend->setRowSpacing(-5);

    //设置图例中图形与文字距离图例边框的距离
    this->legend->setMargins(QMargins(0,0,0,0));

    // 绑定legend click回调函数
    connect(this, &QCustomPlot::legendClick, this, &WavePlot::legendClickCb);

    m_rightButtonMenu = new QMenu(this);
    m_autoScaleAction = new QAction(tr("auto scale"),this);
    m_freeScaleAction = new QAction(tr("free scale"), this);
    m_fullScaleAction = new QAction(tr("full scale"), this);
    m_xScaleAction = new QAction(tr("X scale"), this);
    m_yScaleAction = new QAction(tr("Y scale"), this);
    m_clearAction = new QAction(tr("clear"), this);
    m_graphAllVisibleAction = new QAction(tr("all visible"), this);
    m_graphNoneVisibleAction = new QAction(tr("none visible"), this);
    m_markerTableVisibleAction = new QAction(tr("marker table"), this);
    m_gridVisibleAction = new QAction(tr("grid visible"), this);

    //设置可以选中
    m_autoScaleAction->setCheckable(true);
    m_freeScaleAction->setCheckable(true);
    m_fullScaleAction->setCheckable(true);
    m_xScaleAction->setCheckable(true);
    m_yScaleAction->setCheckable(true);
    m_markerTableVisibleAction->setCheckable(true);
    if(type == DIRECT_SAMPLE) m_markerTableVisibleAction->setChecked(true);
    else if(type == VFT_TASK) m_markerTableVisibleAction->setChecked(false);
    m_gridVisibleAction->setCheckable(true);
    m_gridVisibleAction->setChecked(false);

    //设置自定义属性，便于区分哪个action
    m_autoScaleAction->setProperty("TYPE",AUTO_SCALE);
    m_freeScaleAction->setProperty("TYPE",FREE_SCALE);
    m_fullScaleAction->setProperty("TYPE",FULL_SCALE);
    m_xScaleAction->setProperty("TYPE",X_SCALE);
    m_yScaleAction->setProperty("TYPE",Y_SCALE);

    //设置初始scale type
    if(type == DIRECT_SAMPLE){
        m_autoScaleAction->setChecked(true);
        m_autoScaleAction->setDisabled(true);
        scale_type = AUTO_SCALE;
        m_lastScaleAction = m_autoScaleAction;

        //将action绑定到menu
        m_rightButtonMenu->addAction(m_autoScaleAction);
        m_rightButtonMenu->addAction(m_freeScaleAction);
        m_rightButtonMenu->addAction(m_fullScaleAction);
        m_rightButtonMenu->addAction(m_xScaleAction);
        m_rightButtonMenu->addAction(m_yScaleAction);
        m_rightButtonMenu->addSeparator();
        m_rightButtonMenu->addAction(m_clearAction);
        m_rightButtonMenu->addSeparator();
        m_rightButtonMenu->addAction(m_graphAllVisibleAction);
        m_rightButtonMenu->addAction(m_graphNoneVisibleAction);
        m_rightButtonMenu->addAction(m_markerTableVisibleAction);
        m_rightButtonMenu->addAction(m_gridVisibleAction);
    } else if(type == VFT_TASK){
        m_fullScaleAction->setChecked(true);
        m_fullScaleAction->setDisabled(true);
        scale_type = FULL_SCALE;
        m_lastScaleAction = m_fullScaleAction;

        //将action绑定到menu
        m_rightButtonMenu->addAction(m_autoScaleAction);
        m_rightButtonMenu->addAction(m_freeScaleAction);
        m_rightButtonMenu->addAction(m_fullScaleAction);
        m_rightButtonMenu->addAction(m_xScaleAction);
        m_rightButtonMenu->addAction(m_yScaleAction);
        //m_rightButtonMenu->addSeparator();
        //m_rightButtonMenu->addAction(m_clearAction);
        m_rightButtonMenu->addSeparator();
        m_rightButtonMenu->addAction(m_graphAllVisibleAction);
        m_rightButtonMenu->addAction(m_graphNoneVisibleAction);
        m_rightButtonMenu->addAction(m_markerTableVisibleAction);
        m_rightButtonMenu->addAction(m_gridVisibleAction);
    }

    //绑定action到回调函数
    connect(m_autoScaleAction, SIGNAL(triggered()), this, SLOT(onScaleActionTriggered()));
    connect(m_freeScaleAction, SIGNAL(triggered()), this, SLOT(onScaleActionTriggered()));
    connect(m_fullScaleAction, SIGNAL(triggered()), this, SLOT(onScaleActionTriggered()));
    connect(m_xScaleAction, SIGNAL(triggered()), this, SLOT(onScaleActionTriggered()));
    connect(m_yScaleAction, SIGNAL(triggered()), this, SLOT(onScaleActionTriggered()));
    connect(m_clearAction, SIGNAL(triggered()), this, SLOT(onClearActionTriggered()));
    connect(m_graphAllVisibleAction, SIGNAL(triggered()), this, SLOT(graphAllVisibleActionTriggered()));
    connect(m_graphNoneVisibleAction, SIGNAL(triggered()), this, SLOT(graphNoneVisibleActionTriggered()));
    connect(m_markerTableVisibleAction, &QAction::toggled, this, &WavePlot::onMarkerTableVisibleChanngedTriggered);
    connect(m_gridVisibleAction, &QAction::toggled, this, &WavePlot::onGridVisibleChanngedTriggered);

    // 连接鼠标事件信号
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QCustomPlot::customContextMenuRequested, this, &WavePlot::showContextMenu);
}

/*
 * 设置marker是否可见
*/
void WavePlot::onMarkerTableVisibleChanngedTriggered(){
    setMarkerTableVisible(m_markerTableVisibleAction->isChecked());
}

/*
 * 设置背景网格是否可见
*/
void WavePlot::onGridVisibleChanngedTriggered(){
    xAxis->grid()->setVisible(m_gridVisibleAction->isChecked());
    yAxis->grid()->setVisible(m_gridVisibleAction->isChecked());
}

/*
 * marker自定义右键菜单
*/
void WavePlot::onMarkerTableCustomContextMenuRequested(const QPoint &pos){
    QTableWidgetItem *item = markerTable->itemAt(pos);
    if(item)    //删除单个marker菜单
        m_markerTableRightButtonMenu->exec(markerTable->mapToGlobal(pos));
    else        //添加marker菜单和删除所有marker菜单
        m_markerTableBlankRightButtonMenu->exec(markerTable->mapToGlobal(pos));
}

/*
 * 添加marker回调函数
*/
void WavePlot::onAddMarkerActionTriggered(){
    //addmarker
    addMarker();
}

/*
 * 清除所有marker
*/
void WavePlot::onClearMarkerActionTriggered(){
    //std::cout << "clear" << std::endl;
    while(marker.size() > 0) delMarker(0);
}

/*
 * 单独marker删除回调
*/
void WavePlot::onDelMarkerActionTriggered(){
    //std::cout << "del" << std::endl;
    QList<QTableWidgetItem *> markerTableSelectedItems = markerTable->selectedItems();
    for(int index = 0;index < markerTableSelectedItems.size();++index){
        int currentRow = markerTable->row(markerTableSelectedItems[index]);

        delMarker(currentRow);
    }
}

/*
 * 编辑marker数值
*/
void WavePlot::onEditMarkerTableTriggered(int row, int column){
    QTableWidgetItem *item = markerTable->item(row, column);
    if (item) {
        QString newValue = item->text();

        // 尝试将值转换为数值型
        bool conversionOK;
        double numericValue = newValue.toDouble(&conversionOK);

        if (conversionOK &&
            numericValue >= 0 &&
            (
                (this->graph(0)->dataCount() == 0 && numericValue == 0) ||
                (this->graph(0)->dataCount() > 0 && numericValue <= this->graph(0)->data().data()->at(this->graph(0)->dataCount()-1)->key)
            )
           ){
            // 值是有效的数值型
            marker.at(row)->point1->setCoords(numericValue, 0);
            marker.at(row)->point2->setCoords(numericValue, 1);
            //std::cout << "value valid" << std::endl;
            markerTableUpdate();
        } else {
            // 值无效，显示log警告用户
            //std::cout << "value invalid" << std::endl;
            markerTableUpdate();
        }
    }
}

/*
 * 重新设置通道数量
 * 清除原有波形
 * 按照使能通道名字resize
*/
void WavePlot::resizeChannelLength(std::vector<uint32_t> enable_channels){

    this->clearGraphs();
    for(int i = 0; i < enable_channels.size(); ++i){
        this->addGraph();
        this->graph(i)->setPen(QPen(QColor(wave_color[i])));
        this->graph(i)->setName(QString::number(enable_channels[i]));
    }

    enable_channels_bak = enable_channels;

    //TODO: wait for test
    //std::cout << "marker.size(): " << marker.size() << std::endl;
    while(marker.size() > 0) delMarker(0);

    if(this->isVisible()) graphAllVisibleActionTriggered();
    else graphNoneVisibleActionTriggered();
}

/*
 * 按照通道数量resize
*/
void WavePlot::resizeChannelLength(uint32_t channelLength){
    this->clearGraphs();
    for(int i = 0; i < channelLength; ++i){
        this->addGraph();
        this->graph(i)->setPen(QPen(QColor(wave_color[i])));
    }
    //while(marker.size() > 0) delMarker(0);

    if(this->isVisible()) graphAllVisibleActionTriggered();
    else graphNoneVisibleActionTriggered();
}

/*
 *菜单缩放动作回调
 *设置缩放类型，鼠标左键可/不可拖动，鼠标滚轮缩放因子
*/
void WavePlot::onScaleActionTriggered(){
    QAction *senderAction = qobject_cast<QAction*>(sender());
    if(senderAction){
        m_lastScaleAction->setChecked(false);
        m_lastScaleAction->setDisabled(false);

        senderAction->setChecked(true);
        senderAction->setDisabled(true);

        m_lastScaleAction = senderAction;

        SCALE_TYPE st = SCALE_TYPE(senderAction->property("TYPE").toInt());
        switch (st){
        case AUTO_SCALE:
            scale_type = AUTO_SCALE;
            this->setInteraction(QCP::iRangeDrag, false);
            this->axisRect()->setRangeZoomFactor(1.0,1.0);

            break;
        case FREE_SCALE:
            scale_type = FREE_SCALE;
            this->setInteraction(QCP::iRangeDrag, true);
            this->axisRect()->setRangeZoomFactor(0.9,0.9);

            break;
        case FULL_SCALE:
            scale_type = FULL_SCALE;
            this->setInteraction(QCP::iRangeDrag,false);
            this->axisRect()->setRangeZoomFactor(1.0,1.0);

            break;
        case X_SCALE:
            scale_type = X_SCALE;
            this->setInteraction(QCP::iRangeDrag, true);
            this->axisRect()->setRangeZoomFactor(0.9,1.0);

            break;
        case Y_SCALE:
            scale_type = Y_SCALE;
            this->setInteraction(QCP::iRangeDrag, true);
            this->axisRect()->setRangeZoomFactor(1.0,0.9);

            break;
        default: break;
        }

    }
}

/*
 * 清屏动作回调
*/
void WavePlot::onClearActionTriggered(){
    QAction *senderAction = qobject_cast<QAction*>(sender());
    if(senderAction){
        resizeChannelLength(enable_channels_bak);
    }
}

/*
 * 显示所有波形
*/
void WavePlot::graphAllVisibleActionTriggered(){
    //std::cout << "show" << std::endl;
    int graphCount = this->graphCount(); // 获取曲线的数量

    for (int i = 0; i < graphCount; ++i) {
        this->graph(i)->setVisible(true);
        this->graph(i)->setLineStyle(QCPGraph::lsLine); // 显示为线型
        this->graph(i)->setScatterStyle(QCPScatterStyle::ssNone); // 不显示叉叉
    }
}

/*
 * 隐藏所有波形
*/
void WavePlot::graphNoneVisibleActionTriggered(){
    //std::cout << "close" << std::endl;
    int graphCount = this->graphCount(); // 获取曲线的数量

    for (int i = 0; i < graphCount; ++i) {
        this->graph(i)->setVisible(false);
        this->graph(i)->setLineStyle(QCPGraph::lsNone); // 不显示为线型
        this->graph(i)->setScatterStyle(QCPScatterStyle::ssCross); // 显示叉叉
    }
}

/*
 * 替代replot
 * 能够根据scale的类型更新图像
*/
void WavePlot::update(){

    //自适应模式
    //满范围模式
    if(scale_type  == AUTO_SCALE){
        //设定y轴自适应
        this->yAxis->rescale(true);
        //更新x轴坐标
        double lastXValue =this->graphCount() > 0 ? (this->graph(0)->dataCount() > 0 ? this->graph(0)->dataMainKey(this->graph(0)->dataCount()-1) : 0) : 0;
        this->xAxis->setRange(lastXValue-30,lastXValue);
    } else if(scale_type == FULL_SCALE){
        //设定y轴自适应
        this->yAxis->rescale(true);
        //设定x轴自适应
        this->xAxis->rescale(true);
    }
    this->replot();
}

/*
 * 通过单机图例的某个通道来实现波形曲线的显示/隐藏
*/
void WavePlot::legendClickCb(QCPLegend *legend, QCPAbstractLegendItem *item, QMouseEvent *event){
    if (item) {
        // 检查图例项的类型是否为 QCPPlottableLegendItem
        QCPPlottableLegendItem *plottableLegendItem = qobject_cast<QCPPlottableLegendItem *>(item);
        if (plottableLegendItem) {
            // 获取与图例项关联的曲线图
            QCPGraph *clickedGraph = qobject_cast<QCPGraph *>(plottableLegendItem->plottable());

            if (clickedGraph) {
                // 切换曲线图的可见性
                clickedGraph->setVisible(!clickedGraph->visible());
                // 根据曲线是否可见，设置线型为圆圈或空
                if (clickedGraph->visible()) {
                    clickedGraph->setLineStyle(QCPGraph::lsLine); // 显示为线型
                    clickedGraph->setScatterStyle(QCPScatterStyle::ssNone); // 不显示叉叉
                } else {
                    clickedGraph->setLineStyle(QCPGraph::lsNone); // 不显示线型
                    clickedGraph->setScatterStyle(QCPScatterStyle::ssCross); // 显示为叉叉
                }
            }
        }
    }
}

/*
 * 图像添加marker，key为当前采样时间
*/
void WavePlot::addMarker(){
    double lastKey;
    marker.push_back(new QCPItemStraightLine(this));
    QCPItemStraightLine *vLine = marker.back();
    if(!(this->graph(0)->dataCount() > 0)) lastKey = 0;
    else lastKey = this->graph(0)->data().data()->at(this->graph(0)->dataCount()-1)->key;
    //std::cout << "add key: " << lastKey << std::endl;
    vLine->setLayer("overlay");

    vLine->setPen(QPen(Qt::red, 1, Qt::DashLine));
    // 超出坐标轴范围则不显示游标线
    vLine->setClipToAxisRect(true);
    // 画竖线，x为curtime，y只要0和1即可绘制直线了
    vLine->point1->setCoords(lastKey, 0);
    vLine->point2->setCoords(lastKey, 1);

    markerTableUpdate();
}

/*
 * 图像添加marker，key为输入参数
*/
void WavePlot::addMarker(double key){
    marker.push_back(new QCPItemStraightLine(this));
    QCPItemStraightLine *vLine = marker.back();
    vLine->setLayer("overlay");
    vLine->setPen(QPen(Qt::red, 1, Qt::DashLine));
    // 超出坐标轴范围则不显示游标线
    vLine->setClipToAxisRect(true);
    // 画竖线，x为curtime，y只要0和1即可绘制直线了
    vLine->point1->setCoords(key, 0);
    vLine->point2->setCoords(key, 1);

    markerTableUpdate();
}

/*
 * 从0开始的markerIndex
*/
void WavePlot::delMarker(uint32_t markerIndex){
    if(markerIndex < marker.size()){
        this->removeItem(marker[markerIndex]);
        //delete marker[markerIndex];
        marker.erase(marker.begin() + markerIndex);
    }

    markerTableUpdate();
}

/*
 * 将marker的key值同步到 markertable中
*/
void WavePlot::markerTableUpdate(){
    disconnect(markerTable, &QTableWidget::cellChanged, this, &WavePlot::onEditMarkerTableTriggered);

    resizeTableRowsCount(marker.size());
    for(int rowIndex=0; rowIndex <marker.size() ; ++rowIndex){
        markerTableVector[rowIndex].setText(QString::number(marker[rowIndex]->point1->key()));
    }

    connect(markerTable, &QTableWidget::cellChanged, this, &WavePlot::onEditMarkerTableTriggered);
}

/*
 * 重设marker表格的行数
*/
void WavePlot::resizeTableRowsCount(int row_count){
    //取消原有的item绑定
    for(int i = 0; i < markerTable->rowCount(); ++i)
        markerTable->takeItem(i,0);

    // 清除 dev_table_vector 中的对象
    markerTableVector.clear();

    //重新设置vector的大小
    markerTableVector.resize(row_count);

    // 重新设置表格的行数和关联的 QTableWidgetItem
    markerTable->setRowCount(row_count);
    for(int i = 0; i < row_count; ++i)
        markerTable->setItem(i,0,&markerTableVector[i]);
}

/*
 * 窗口打开默认所有波形显示
*/
void WavePlot::windowShowCb(){
    graphAllVisibleActionTriggered();

}

/*
 * 窗口关闭所有波形隐藏
*/
void WavePlot::windowCloseCb(){
    graphNoneVisibleActionTriggered();

}












