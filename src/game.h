#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_game.h"

class Game : public QMainWindow
{
    Q_OBJECT

public:
    Game(QWidget *parent = nullptr);
    ~Game();

private:
    Ui::GameClass ui;
};
