#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    int parse(const QString& expr, QString& errorMessage);
    int parseTerm(const QString& term, QString& errorMessage);
    double evaluate(double x) const;

private:
    Ui::MainWindow *ui;

     struct Term {
         double coefficient;
         int exponent;
     };

     std::vector<Term> terms;

public slots:
     void calculateY(void);
};
#endif // MAINWINDOW_H
