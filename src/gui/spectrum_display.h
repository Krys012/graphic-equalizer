//
// Created by Léo KRYS on 18/12/2024.
//

#ifndef SPECTRUM_DISPLAY_H
#define SPECTRUM_DISPLAY_H

#include <QWidget>
#include <vector>

class SpectrumDisplay : public QWidget {
    Q_OBJECT

public:
    explicit SpectrumDisplay(QWidget *parent = nullptr);
    void updateSpectrum(const std::vector<float>& magnitudes);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    std::vector<float> spectrumData;
    QColor barColor;
    QColor backgroundColor;
};

#endif