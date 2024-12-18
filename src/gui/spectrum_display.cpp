//
// Created by Léo KRYS on 18/12/2024.
//

#include "spectrum_display.h"
#include <QPainter>

SpectrumDisplay::SpectrumDisplay(QWidget *parent)
    : QWidget(parent)
    , barColor(Qt::blue)
    , backgroundColor(Qt::black)
{
    setMinimumHeight(200);
    setMinimumWidth(400);
}

void SpectrumDisplay::updateSpectrum(const std::vector<float>& magnitudes) {
    spectrumData = magnitudes;
    update(); // Déclenche un nouveau paintEvent
}

void SpectrumDisplay::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.fillRect(rect(), backgroundColor);

    if (spectrumData.empty())
        return;

    const int width = this->width();
    const int height = this->height();
    const int barWidth = width / spectrumData.size();

    painter.setPen(Qt::NoPen);
    painter.setBrush(barColor);

    for (size_t i = 0; i < spectrumData.size(); ++i) {
        int barHeight = int(spectrumData[i] * height);
        QRect bar(i * barWidth, height - barHeight,
                 barWidth - 1, barHeight);
        painter.drawRect(bar);
    }
}