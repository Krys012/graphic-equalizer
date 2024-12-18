#ifndef EQUALIZER_WIDGET_H
#define EQUALIZER_WIDGET_H

#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <vector>

class EqualizerWidget : public QWidget {
    Q_OBJECT

public:
    explicit EqualizerWidget(QWidget *parent = nullptr);

    signals:
        void bandChanged(int band, int value);

private:
    void setupUi();
    std::vector<QSlider*> sliders;
    std::vector<QLabel*> labels;
    const std::vector<QString> frequencies = {
        "32", "64", "125", "250", "500",
        "1k", "2k", "4k", "8k", "16k"
    };
};

#endif // EQUALIZER_WIDGET_H