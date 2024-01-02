#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_game.h"

class GameUi : public QMainWindow
{
    Q_OBJECT

public:
    GameUi(QWidget *parent = nullptr);
    ~GameUi();

private:
    Ui::GameClass ui;
};
