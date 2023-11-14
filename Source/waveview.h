#ifndef WAVEVIEW_H
#define WAVEVIEW_H

#include <QChartView>
#include <QMenu>
#include <QValueAxis>
#include <QLineSeries>
#include <QHBoxLayout>
#include <iostream>

class WaveView : public QChartView
{
    Q_OBJECT
public:

    enum SCALE_TYPE{
        AUTO_SCALE,
        FREE_SCALE,
        X_SCALE,
        Y_SCALE
    };

    WaveView(QWidget* pParent = nullptr, uint32_t line_count = 3);
    void resizeChannelLength(uint32_t line_count);
    void append(uint32_t channel, double x, double y);
    void setSeriesVisible(uint32_t channel, bool visible);
    void setLegendName(int channel, QString legendName){m_lineSeries[channel]->setName(legendName);}
    uint32_t getWaveChannelSize(){return m_lineSeries.size();}
    void setWindowVisible(bool visible){if (visible) m_waveWindow->show(); else m_waveWindow->close();}
    ~WaveView(){
        for(int i = 0;i < m_lineSeries.size();i++){
            delete m_lineSeries[i];
        }
        delete m_axisX;
        delete m_axisY;
        delete m_chart;
    }

protected:
    virtual void mouseMoveEvent(QMouseEvent *pEvent) override;
    virtual void mousePressEvent(QMouseEvent *pEvent) override;
    virtual void mouseReleaseEvent(QMouseEvent *pEvent) override;
    virtual void wheelEvent(QWheelEvent *pEvent) override;
    virtual void contextMenuEvent(QContextMenuEvent *event) override;

private:
    //和鼠标事件有关
    bool m_middleButtonPressed;
    bool m_leftButtonPressed;
    QPoint m_oPrePos;
    QMenu *m_rightButtonMenu;
    QAction *m_autoScaleAction;
    QAction *m_freeScaleAction;
    QAction *m_xScaleAction;
    QAction *m_yScaleAction;
    QAction *m_clearAction;
    QAction *m_lastScaleAction;
    SCALE_TYPE scale_type;
    //坐标轴
    QValueAxis *m_axisX, *m_axisY;
    //曲线，各个通道最大最小值，通道是否可见
    std::vector<QLineSeries*> m_lineSeries;
    std::vector<double> m_maxLineSeries;
    std::vector<double> m_minLineSeries;
    std::vector<bool> m_lineSeriesVisible;
    //qchart
    QChart *m_chart;
    //波形window和layout TODO：mpu plot
    QWidget *m_waveWindow;
    QHBoxLayout *m_waveLayout;

    //自动调整坐标轴范围
    void rescaleAxis(){
        if(m_lineSeries[0]->count() > 0){

            //找出y轴最值
            double maxY = -std::numeric_limits<double>::max(), minY = std::numeric_limits<double>::max();
            for(int i = 0; i < m_lineSeries.size(); ++i){
                if(m_lineSeriesVisible[i]){
                    if(maxY < m_maxLineSeries[i]) maxY = m_maxLineSeries[i];
                    if(minY > m_minLineSeries[i]) minY = m_minLineSeries[i];
                }
            }

            //y轴缩放到合适的尺寸
            m_axisY->setRange(minY, maxY);
            //x轴缩放到合适的尺寸
            m_axisX->setRange(m_lineSeries[0]->points().last().x()-30,m_lineSeries[0]->points().last().x()-0.2);
        }
    }


private slots:
    void onScaleActionTriggered();
    void onClearActionTriggered();

};

#endif // WAVEVIEW_H
