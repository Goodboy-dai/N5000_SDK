#ifndef WAVEPLOT_H
#define WAVEPLOT_H

#include "qcustomplot.h"
#include <iostream>
#include "mywindow.h"


class WavePlot : public QCustomPlot
{
    Q_OBJECT
public:

    WavePlot(QWidget* pParent = nullptr, QString windowName="sub", int type = 0);
    void resizeChannelLength(std::vector<uint32_t> enable_channels);
    void resizeChannelLength(uint32_t channelLength);
    void append(uint32_t channel, double x, double y){this->graph(channel)->addData(x, y);}
    void setWindowVisible(bool visible){if (visible) plot_window->show(); else plot_window->close();}
    void setWindowName(QString Name){this->plot_window->setWindowTitle(Name);}
    void setChannelName(uint32_t channel, QString Name){this->graph(channel)->setName(Name);}
    std::vector<QCPItemStraightLine*> getMarker(){return marker;}
    void addMarker();
    void addMarker(double key);
    void delMarker(uint32_t markerIndex);
    void clearMarker(){while(marker.size() > 0) delMarker(0);}
    void update();
    void setMarkerTableVisible(bool visible){markerTable->setVisible(visible);}
    ~WavePlot(){
        delete plot_layout;
        delete plot_window;
    }


private slots:
    void onScaleActionTriggered();
    void onClearActionTriggered();
    void graphAllVisibleActionTriggered();
    void graphNoneVisibleActionTriggered();
    void showContextMenu(const QPoint &pos){m_rightButtonMenu->exec(mapToGlobal(pos));}
    void legendClickCb(QCPLegend *legend, QCPAbstractLegendItem *item, QMouseEvent *event);
    void onMarkerTableCustomContextMenuRequested(const QPoint &pos);
    void onAddMarkerActionTriggered();
    void onClearMarkerActionTriggered();
    void onDelMarkerActionTriggered();
    void onEditMarkerTableTriggered(int row, int column);
    void onMarkerTableVisibleChanngedTriggered();
    void onGridVisibleChanngedTriggered();
    void markerTableUpdate();
    void resizeTableRowsCount(int row_count);

    void windowShowCb();
    void windowCloseCb();
private:

    enum SCALE_TYPE{
        AUTO_SCALE,
        FREE_SCALE,
        FULL_SCALE,
        X_SCALE,
        Y_SCALE
    };

    MyWindow *plot_window;
    QHBoxLayout *plot_layout;
    //右键菜单
    QMenu *m_rightButtonMenu;
    SCALE_TYPE scale_type;
    QAction *m_autoScaleAction;
    QAction *m_freeScaleAction;
    QAction *m_fullScaleAction;
    QAction *m_xScaleAction;
    QAction *m_yScaleAction;
    QAction *m_lastScaleAction;
    QAction *m_clearAction;
    QAction *m_graphAllVisibleAction;
    QAction *m_graphNoneVisibleAction;
    QAction *m_markerTableVisibleAction;
    QAction *m_gridVisibleAction;
    std::vector<uint32_t> enable_channels_bak;
    std::vector<QCPItemStraightLine*> marker;
    QTableWidget *markerTable;
    std::vector<QTableWidgetItem> markerTableVector;
    QMenu *m_markerTableRightButtonMenu;
    QMenu *m_markerTableBlankRightButtonMenu;
    QAction *m_addMarkerAction;
    QAction *m_clearMarkerAction;
    QAction *m_delMarkerAction;

    const QRgb wave_color[140]={ 0xF0F8FF,
                                  0xFAEBD7,
                                  0x00FFFF,
                                  0x7FFFD4,
                                  0xF0FFFF,
                                  0xF5F5DC,
                                  0xFFE4C4,
                                  0x00aa00,
                                  0xFFEBCD,
                                  0x0000FF,
                                  0x8A2BE2,
                                  0xA52A2A,
                                  0xDEB887,
                                  0x5F9EA0,
                                  0x7FFF00,
                                  0xD2691E,
                                  0xFF7F50,
                                  0x6495ED,
                                  0xFFF8DC,
                                  0xDC143C,
                                  0x00FFFF,
                                  0x00008B,
                                  0x008B8B,
                                  0xB8860B,
                                  0xA9A9A9,
                                  0x006400,
                                  0xBDB76B,
                                  0x8B008B,
                                  0x556B2F,
                                  0xFF8C00,
                                  0x9932CC,
                                  0x8B0000,
                                  0xE9967A,
                                  0x8FBC8F,
                                  0x483D8B,
                                  0x2F4F4F,
                                  0x00CED1,
                                  0x9400D3,
                                  0xFF1493,
                                  0x00BFFF,
                                  0x696969,
                                  0x1E90FF,
                                  0xB22222,
                                  0xFFFAF0,
                                  0x228B22,
                                  0xFF00FF,
                                  0xDCDCDC,
                                  0xF8F8FF,
                                  0xFFD700,
                                  0xDAA520,
                                  0x808080,
                                  0x008000,
                                  0xADFF2F,
                                  0xF0FFF0,
                                  0xFF69B4,
                                  0xCD5C5C,
                                  0x4B0082,
                                  0xFFFFF0,
                                  0xF0E68C,
                                  0xE6E6FA,
                                  0xFFF0F5,
                                  0x7CFC00,
                                  0xFFFACD,
                                  0xADD8E6,
                                  0xF08080,
                                  0xE0FFFF,
                                  0xFAFAD2,
                                  0x90EE90,
                                  0xD3D3D3,
                                  0xFFB6C1,
                                  0xFFA07A,
                                  0x20B2AA,
                                  0x87CEFA,
                                  0x778899,
                                  0xB0C4DE,
                                  0xFFFFE0,
                                  0x00FF00,
                                  0x32CD32,
                                  0xFAF0E6,
                                  0xFF00FF,
                                  0x800000,
                                  0x66CDAA,
                                  0x0000CD,
                                  0xBA55D3,
                                  0x9370DB,
                                  0x3CB371,
                                  0x7B68EE,
                                  0x00FA9A,
                                  0x48D1CC,
                                  0xC71585,
                                  0x191970,
                                  0xF5FFFA,
                                  0xFFE4E1,
                                  0xFFE4B5,
                                  0xFFDEAD,
                                  0x000080,
                                  0xFDF5E6,
                                  0x808000,
                                  0x6B8E23,
                                  0xFFA500,
                                  0xFF4500,
                                  0xDA70D6,
                                  0xEEE8AA,
                                  0x98FB98,
                                  0xAFEEEE,
                                  0xDB7093,
                                  0xFFEFD5,
                                  0xFFDAB9,
                                  0xCD853F,
                                  0xFFC0CB,
                                  0xDDA0DD,
                                  0xB0E0E6,
                                  0x800080,
                                  0xFF0000,
                                  0xBC8F8F,
                                  0x4169E1,
                                  0x8B4513,
                                  0xFA8072,
                                  0xFAA460,
                                  0x2E8B57,
                                  0xFFF5EE,
                                  0xA0522D,
                                  0xC0C0C0,
                                  0x87CEEB,
                                  0x6A5ACD,
                                  0x708090,
                                  0xFFFAFA,
                                  0x00FF7F,
                                  0x4682B4,
                                  0xD2B48C,
                                  0x008080,
                                  0xD8BFD8,
                                  0xFF6347,
                                  0x40E0D0,
                                  0xEE82EE,
                                  0xF5DEB3,
                                  0xFFFFFF,
                                  0xF5F5F5,
                                  0xFFFF00,
                                  0x9ACD32,
                                  };

};

#endif // WAVEPLOT_H
