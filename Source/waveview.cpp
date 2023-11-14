#include "waveview.h"
#include <iostream>
#include <QMenu>

/*
 * 自适应（auto scale），自由缩放（free scale），X轴缩放（X scale），y轴缩放（Y scale），清屏（clear）
 *
*/

/*
 *初始化动作
 *菜单按钮
*/
WaveView::WaveView(QWidget* pParent, uint32_t line_count)
    : QChartView(pParent)
{
    //初始化
    m_middleButtonPressed = false;
    m_leftButtonPressed = false;
    m_oPrePos = QPoint(0,0);
    m_rightButtonMenu = new QMenu(this);
    m_autoScaleAction = new QAction(tr("auto scale"),this);
    m_freeScaleAction = new QAction(tr("free scale"), this);
    m_xScaleAction = new QAction(tr("X scale"), this);
    m_yScaleAction = new QAction(tr("Y scale"), this);
    m_clearAction = new QAction(tr("clear"), this);

    //设置可以选中
    m_autoScaleAction->setCheckable(true);
    m_freeScaleAction->setCheckable(true);
    m_xScaleAction->setCheckable(true);
    m_yScaleAction->setCheckable(true);

    //设置自定义属性，便于区分哪个action
    m_autoScaleAction->setProperty("TYPE",AUTO_SCALE);
    m_freeScaleAction->setProperty("TYPE",FREE_SCALE);
    m_xScaleAction->setProperty("TYPE",X_SCALE);
    m_yScaleAction->setProperty("TYPE",Y_SCALE);

    //设置初始scale type
    m_autoScaleAction->setChecked(true);
    m_autoScaleAction->setDisabled(true);
    scale_type = AUTO_SCALE;
    m_lastScaleAction = m_autoScaleAction;

    //将action绑定到menu
    m_rightButtonMenu->addAction(m_autoScaleAction);
    m_rightButtonMenu->addAction(m_freeScaleAction);
    m_rightButtonMenu->addAction(m_xScaleAction);
    m_rightButtonMenu->addAction(m_yScaleAction);
    m_rightButtonMenu->addSeparator();
    m_rightButtonMenu->addAction(m_clearAction);

    //绑定action到回调函数
    connect(m_autoScaleAction, SIGNAL(triggered()), this, SLOT(onScaleActionTriggered()));
    connect(m_freeScaleAction, SIGNAL(triggered()), this, SLOT(onScaleActionTriggered()));
    connect(m_xScaleAction, SIGNAL(triggered()), this, SLOT(onScaleActionTriggered()));
    connect(m_yScaleAction, SIGNAL(triggered()), this, SLOT(onScaleActionTriggered()));
    connect(m_clearAction, SIGNAL(triggered()), this, SLOT(onClearActionTriggered()));

    //坐标轴
    m_axisX = new QValueAxis();
    m_axisY = new QValueAxis();
    m_axisX->setTitleText("time(s)");
    m_axisY->setTitleText("ADC RAW");
    m_axisX->setMin(-30);
    m_axisY->setMin(0);
    m_axisX->setMax(0);
    m_axisY->setMax(32768);

    //chart绑定坐标轴
    m_chart = new QChart();
    m_chart->addAxis(m_axisX, Qt::AlignBottom);                      // 将X轴添加到图表上
    m_chart->addAxis(m_axisY, Qt::AlignLeft);                    // 将Y轴添加到图表上
    m_chart->setAnimationOptions(QChart::SeriesAnimations);        // 动画：能使曲线绘制显示的更平滑，过渡效果更好看，但会使图像卡顿

    resizeChannelLength(line_count);

    this->setChart(m_chart);
    this->setRenderHint(QPainter::Antialiasing);       // 设置渲染：抗锯齿，如果不设置那么曲线就显得不平滑
    this->setRubberBand(QChartView::RectangleRubberBand);    //左键矩形框选择放大


    //设置波形窗口
    m_waveWindow = new QWidget();
    m_waveWindow->resize(800,600);

    //子窗口布局
    m_waveLayout = new QHBoxLayout(m_waveWindow);
    m_waveLayout->addWidget(this);

}

/*
 * 重新设置waveview的通道数量
 * 使用后果
 *  清除原波形数据
 *  重新设置通道数
*/
void WaveView::resizeChannelLength(uint32_t line_count){
    //delete series
    for(int i = 0;i < m_lineSeries.size();++i) delete m_lineSeries[i];
    m_maxLineSeries.clear();
    m_minLineSeries.clear();
    m_lineSeriesVisible.clear();
    m_chart->removeAllSeries();

    //初始化数据序列
    m_lineSeries.resize(line_count);
    m_maxLineSeries.resize(line_count);
    m_minLineSeries.resize(line_count);
    m_lineSeriesVisible.resize(line_count);
    for (int i = 0; i < line_count; i++){
        m_lineSeries[i] = new QLineSeries();                             // 创建曲线绘制对象
        m_lineSeries[i]->setVisible(true);                              // 设置数据点可见
        m_maxLineSeries[i] = -std::numeric_limits<double>::max();
        m_minLineSeries[i] = std::numeric_limits<double>::max();
        m_lineSeriesVisible[i] = true;
        m_chart->addSeries(m_lineSeries[i]);                              // 将曲线对象添加到图表上
        m_lineSeries[i]->attachAxis(m_axisX);                             // 曲线对象关联上X轴，此步骤必须在m_chart->addSeries之后
        m_lineSeries[i]->attachAxis(m_axisY);                             // 曲线对象关联上Y轴，此步骤必须在m_chart->addSeries之后
    }
}

/*
 * 向波形中的一个通道添加一个元素
 *
*/
void WaveView::append(uint32_t channel, double x, double y){
    //TODO 空/错误判断
    m_lineSeries[channel]->append(QPointF(x, y));
    if(m_maxLineSeries[channel] < y) m_maxLineSeries[channel] = y;
    if(m_minLineSeries[channel] > y) m_minLineSeries[channel] = y;
    if(scale_type == AUTO_SCALE) rescaleAxis();
}

/*
 * 设置某个通道的波形是否可见
*/
void WaveView::setSeriesVisible(uint32_t channel, bool visible){
    //TODO 空/错误判断
    m_lineSeriesVisible[channel] = visible;
    m_lineSeries[channel]->setVisible(visible);
}

/*
 *重写的鼠标移动事件
 *自由缩放可以左键框选放大
 *其他缩放都可以通过中键移动曲线
*/
void WaveView::mouseMoveEvent(QMouseEvent *pEvent){
    if(m_middleButtonPressed){  //在鼠标按下事件过程中记录位移
        QPoint oDeltaPos = pEvent->pos() - m_oPrePos;
        if(scale_type == FREE_SCALE || scale_type == X_SCALE || scale_type == Y_SCALE)
            this->chart()->scroll(-oDeltaPos.x(), oDeltaPos.y());
//        else if(scale_type == X_SCALE)
//            this->chart()->scroll(-oDeltaPos.x(), 0);
//        else if(scale_type == Y_SCALE)
//            this->chart()->scroll(0, oDeltaPos.y());
        m_oPrePos = pEvent->pos();
    } else if(m_leftButtonPressed && scale_type == FREE_SCALE){
        QChartView::mouseMoveEvent(pEvent);
    }
}

/*
 *重写的鼠标按下事件
 *自由缩放可以左键框选放大
*/
void WaveView::mousePressEvent(QMouseEvent *pEvent){
    if(pEvent->button() == Qt::MiddleButton){
        m_middleButtonPressed = true;
        m_oPrePos = pEvent->pos();  //记录鼠标按下的初始位置
    } else if(pEvent->button() == Qt::LeftButton && scale_type == FREE_SCALE){
        m_leftButtonPressed = true;
        QChartView::mousePressEvent(pEvent);
    }
}

/*
 *重写的鼠标释放事件
 *自由缩放可以左键框选放大
*/
void WaveView::mouseReleaseEvent(QMouseEvent *pEvent){
    if(pEvent->button() == Qt::MiddleButton){
        m_middleButtonPressed = false;
        this->setCursor(Qt::ArrowCursor);
    }  else if(pEvent->button() == Qt::LeftButton && scale_type == FREE_SCALE){
        m_leftButtonPressed = false;
        QChartView::mouseReleaseEvent(pEvent);
    }
}

/*
 *重写的鼠标滚轮事件
 *自由缩放可以左键框选放大
 *其他缩放都可以通过中键移动曲线
*/
void WaveView::wheelEvent(QWheelEvent *pEvent){
    if(scale_type != AUTO_SCALE){
        qreal rVal = std::pow(0.999, pEvent->angleDelta().y());
        // 1. 读取视图基本信息
        QRectF oPlotAreaRect = this->chart()->plotArea();
        QPointF oCenterPoint = oPlotAreaRect.center();
        // 2. 水平调整
        if(scale_type == FREE_SCALE || scale_type == X_SCALE)
            oPlotAreaRect.setWidth(oPlotAreaRect.width() * rVal);
        // 3. 竖直调整
        if(scale_type == FREE_SCALE || scale_type == Y_SCALE)
            oPlotAreaRect.setHeight(oPlotAreaRect.height() * rVal);
        // 4.1 计算视点，视点不变，围绕中心缩放
        //QPointF oNewCenterPoint(oCenterPoint);
        // 4.2 计算视点，让鼠标点击的位置移动到窗口中心
        //QPointF oNewCenterPoint(pEvent->pos());
        // 4.3 计算视点，让鼠标点击的位置尽量保持不动(等比换算，存在一点误差)
        QPointF oNewCenterPoint(2 * oCenterPoint - pEvent->position() - (oCenterPoint - pEvent->position()) / rVal);
        // 5. 设置视点
        oPlotAreaRect.moveCenter(oNewCenterPoint);
        // 6. 提交缩放调整
        this->chart()->zoomIn(oPlotAreaRect);
    }
}

/*
 *重写的菜单事件
*/
void WaveView::contextMenuEvent(QContextMenuEvent *event){
    m_rightButtonMenu->exec(event->globalPos());
}

/*
 *菜单缩放动作回调
*/
void WaveView::onScaleActionTriggered(){
    QAction *senderAction = qobject_cast<QAction*>(sender());
    if(senderAction){
        m_lastScaleAction->setChecked(false);
        m_lastScaleAction->setDisabled(false);

        senderAction->setChecked(true);
        senderAction->setDisabled(true);

        m_lastScaleAction = senderAction;

        SCALE_TYPE st = SCALE_TYPE(senderAction->property("TYPE").toInt());
        switch (st){
            case AUTO_SCALE: scale_type = AUTO_SCALE, rescaleAxis(), m_chart->setAnimationOptions(QChart::SeriesAnimations); break;
            case FREE_SCALE: scale_type = FREE_SCALE, m_chart->setAnimationOptions(QChart::NoAnimation); break;
            case X_SCALE: scale_type = X_SCALE, m_chart->setAnimationOptions(QChart::NoAnimation); break;
            case Y_SCALE: scale_type = Y_SCALE, m_chart->setAnimationOptions(QChart::NoAnimation); break;
            default: break;
        }

    }
}

/*
 * 清屏动作回调
*/
void WaveView::onClearActionTriggered(){
    QAction *senderAction = qobject_cast<QAction*>(sender());
    if(senderAction){
        resizeChannelLength(m_lineSeriesVisible.size());
    }
}

