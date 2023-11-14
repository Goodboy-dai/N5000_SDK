#ifndef VFTFLOW_H
#define VFTFLOW_H

#include <QWidget>
#include <vector>

namespace Ui {
class vftFlow;
}

class vftFlow : public QWidget
{
    Q_OBJECT

public:
    struct sampleSig_t{
        std::string Sn;
        std::string Name;
        QStringList words;
    };

    explicit vftFlow(QWidget *parent = nullptr, std::string Sn = "", QStringList questList={});
    ~vftFlow();
    void setPage(int pageIndex);
    void setWindowVisible(bool visible){if (visible) this->show(); else this->close();}

signals:
    void startSampleSig(sampleSig_t param);
    void stopSampleSig(sampleSig_t param);
    void setSampleMarker(sampleSig_t param);
private slots:
    void on_pushButton_clicked();

private:
    QStringList questionList;
    QStringList testWords;
    std::string deviceSn;
    Ui::vftFlow *ui;
    int time;
    uint32_t state;
    QTimer *p_timer;

    void secTimerCb();
    // 0 IDLE
    // 1 start countDown
    // 2 start rest
    // 3 first task
    // 4 second task
    // 5 third task
    // 6 end rest
    // 7 end page
    // 8 did end
    void setState(uint32_t switch2state);
    void updateCountDown();
};

#endif // VFTFLOW_H
