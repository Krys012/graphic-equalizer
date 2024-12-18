//
// Created by Léo KRYS on 18/12/2024.
//
#include "equalizer_widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

EqualizerWidget::EqualizerWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void EqualizerWidget::setupUi() {
    QHBoxLayout* mainLayout = new QHBoxLayout(this);

    for (size_t i = 0; i < frequencies.size(); ++i) {
        QVBoxLayout* bandLayout = new QVBoxLayout();

        // Créer le slider
        QSlider* slider = new QSlider(Qt::Vertical, this);
        slider->setRange(-12, 12);
        slider->setValue(0);
        slider->setTickPosition(QSlider::TicksBothSides);
        slider->setTickInterval(3);
        sliders.push_back(slider);

        // Créer les labels
        QLabel* freqLabel = new QLabel(frequencies[i] + "Hz", this);
        QLabel* valueLabel = new QLabel("0 dB", this);
        labels.push_back(valueLabel);

        // Connecter le slider
        connect(slider, &QSlider::valueChanged, [this, i, valueLabel](int value) {
            valueLabel->setText(QString::number(value) + " dB");
            emit bandChanged(i, value);
        });

        // Ajouter au layout
        bandLayout->addWidget(valueLabel);
        bandLayout->addWidget(slider);
        bandLayout->addWidget(freqLabel);

        mainLayout->addLayout(bandLayout);
    }
}