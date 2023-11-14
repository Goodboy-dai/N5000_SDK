#ifndef DEVTABLE_H
#define DEVTABLE_H

#include <QTableWidget>


class DevTable : public QTableWidget
{
    Q_OBJECT
public:
    const static int DEV_TABLE_COLUMN_NUMBER = 4;
    DevTable(int type);
    void resizeColumnWidths();
    void resizeRowsCount(int row_count);
    void setItemText(int row, int column, QString text){dev_table_vector[row][column].setText(text);}

protected:
    virtual void resizeEvent(QResizeEvent *event) override;

signals:
    void plotRequested();
    void cfgParamRequested();
    void startSampleRequested();
    void stopSampleRequested();
    void saveFileRequested();
    void openFileRequested();
    void openTaskWindowRequested();
    void addMarkerRequested();
    void closeDeviceRequested();

private:
    std::vector<std::vector<QTableWidgetItem>> dev_table_vector;

    QMenu *m_devTableRightButtonMenu;
    QMenu *m_devTableBlankRightButtonMenu;
    QAction *m_devOpenTaskWindowAction;
    QAction *m_devCfgParamAction;
    QAction *m_devStartSampleAction;
    QAction *m_devStopSampleAction;
    QAction *m_devSaveFileAction;
    QAction *m_devOpenFileAction;
    QAction *m_addMarkerAction;
    QAction *m_devCloseAction;


    void on_dev_table_customContextMenuRequested(const QPoint &pos);
    void on_dev_table_itemDoubleClicked(QTableWidgetItem *item){emit plotRequested();}
    void onCfgParamActionTriggered(){emit cfgParamRequested();}
    void onStartSampleActionTriggered(){emit startSampleRequested();}
    void onStopSampleActionTriggered(){emit stopSampleRequested();}
    void onSaveFileActionTriggered(){emit saveFileRequested();}
    void onOpenFileActionTriggered(){emit openFileRequested();}
    void onOpenTaskWindowRequested(){emit openTaskWindowRequested();}
    void onAddMarkerActionRequested(){emit addMarkerRequested();}
    void onCloseDeviceActionRequested(){emit closeDeviceRequested();}
};

#endif // DEVTABLE_H
