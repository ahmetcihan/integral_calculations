#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("MFA501 Assessment 2B - Ahmet Cihan AKINCA - Integral Calculation");
    connect(ui->pushButton_calculate_y, SIGNAL(clicked(bool)), this, SLOT(calculateY()));
    connect(ui->pushButton_draw_graph, SIGNAL(clicked(bool)), this, SLOT(drawGraph()));
    connect(ui->pushButton_calculate_integral, SIGNAL(clicked(bool)), this, SLOT(calculateIntegral()));

    customPlot = new QCustomPlot(this->ui->centralwidget);
    customPlot->setGeometry(500, 70, 370, 280); // (x, y, width, height)
}
int MainWindow::parse(const QString& expr, QString& errorMessage) { //returns 0 for success, -1 on error and sets errorMessage
    terms.clear();
    // Remove spaces
    QString expression = expr;
    expression.replace(" ", "");
    qDebug() << "Expression after removing spaces:" << expression;

    // Handle the first term separately (it might start with -)
    int pos = 0;
    QString first_term;

    // Check if the expression starts with a negative sign
    bool first_term_negative = false;
    if (!expression.isEmpty() && expression[0] == '-') {
        first_term_negative = true;
        pos = 1; // Skip the negative sign
    }

    // Find the next + or - after the first term
    int first_sign_pos = -1;
    for (int i = pos; i < expression.length(); ++i) {
        if (expression[i] == '+' || expression[i] == '-') {
            first_sign_pos = i;
            break;
        }
    }

    // Extract the first term
    if (first_sign_pos == -1) {
        // No + or - found, the entire expression is one term
        first_term = expression.mid(pos);
        if (first_term_negative) {
            first_term = "-" + first_term;
        }
        pos = expression.length();
    } else {
        first_term = expression.mid(pos, first_sign_pos - pos);
        if (first_term_negative) {
            first_term = "-" + first_term;
        }
        pos = first_sign_pos;
    }

    qDebug() << "First term:" << first_term;
    if (parseTerm(first_term, errorMessage) != 0) {
        return -1; // Error occurred in parseTerm
    }

    // Process remaining terms
    while (pos < expression.length()) {
        QChar sign = expression[pos]; // + or -
        pos++; // Move past the sign

        // Find the next + or -
        int next_sign_pos = pos;
        for (int i = pos; i < expression.length(); ++i) {
            if (expression[i] == '+' || expression[i] == '-') {
                next_sign_pos = i;
                break;
            }
        }
        if (next_sign_pos == pos) {
            next_sign_pos = expression.length();
        }

        // Extract the term
        QString term = expression.mid(pos, next_sign_pos - pos);
        qDebug() << "Next term:" << term << "with sign:" << sign;

        // Prepend the sign to the term for parseTerm to handle
        if (sign == '-') {
            term = sign + term;
        }

        if (parseTerm(term, errorMessage) != 0) {
            return -1; // Error occurred in parseTerm
        }

        pos = next_sign_pos;
    }

    return 0; // Success
}
int MainWindow::parseTerm(const QString& term, QString& errorMessage) {
    double coeff = 1.0;
    int exp = 0;

    int x_pos = term.indexOf('x'); //check the position of x

    qDebug() << "buraya geldi";

    if (x_pos == -1) {  //there is no x
        qDebug() << "x yok";
        bool ok;
        coeff = term.toDouble(&ok); //change term to a number
        if (!ok) {
            errorMessage = "invalid term = " + term;
            return -1;
        }
    }
    else {
        qDebug() << "x var";
        qDebug() << "x_pos" << x_pos;

        // Extract coefficient (e.g., "3" or default 1)
        if (x_pos > 0) {
            QString coeff_str = term.mid(0, x_pos); //get the coefficient

            if (coeff_str == "-") coeff = -1.0;
            else if (coeff_str.isEmpty() || coeff_str == "+") coeff = 1.0;
            else {
                bool ok;
                coeff = coeff_str.toDouble(&ok);
                if (!ok) {
                    errorMessage = "invalid coefficient = " + coeff_str;
                    return -1;
                }
            }
        }

        // Extract exponent (e.g., "^2")
        int pow_pos = term.indexOf('^', x_pos);

        if (pow_pos != -1 && pow_pos + 1 < term.length()) {
            QString exp_str = term.mid(pow_pos + 1);
            bool ok;
            exp = exp_str.toInt(&ok);
            if (!ok) {
                errorMessage = "invalid exponent = " + exp_str;
                return -1;
            }
            if (exp < 0) {
                errorMessage = "negative exponents are not supported = " + exp_str;
                return -1;
            }
        }
        else if (x_pos != -1) {
            exp = 1; //just "x" means exponent is 1
        }
    }
    terms.push_back({coeff, exp});
    return 0; // Success
}

// Evaluate the function at a given x value
double MainWindow::evaluate(double x) const {
    double result = 0.0;

    for (int i = 0; i < terms.size(); i++) {
        double term_value = terms[i].coefficient;

        // x^exponent hesaplaması için iç döngü
        for (int j = 0; j < terms[i].exponent; j++) {
            term_value = term_value * x; // term_value *= x yerine açık yazım
        }

        result = result + term_value; // result += term_value yerine açık yazım
    }

    return result;
}
// Slot to calculate y and display the result
void MainWindow::calculateY() {
    QString errorMessage;

    int result = parse(ui->lineEdit_function->text(), errorMessage);

    if (result != 0) { //error occurred
        ui->label_y_result->setText(QString("Error: %1").arg(errorMessage));
        return;
    }

    double x = ui->doubleSpinBox_x->value();
    double y = evaluate(x);

    ui->label_y_result->setText(QString("y = %1").arg(y));
}
void MainWindow::drawGraph() {
    QString errorMessage;
    int result = parse(ui->lineEdit_function->text(), errorMessage);
    if (result != 0) {
        ui->label_y_result->setText(QString("Error: %1").arg(errorMessage));
        return;
    }

    double x1 = ui->doubleSpinBox_x_1->value();
    double x2 = ui->doubleSpinBox_x_2->value();
    double delta_x = ui->doubleSpinBox_delta_x->value();

    if (delta_x <= 0) {
        ui->label_y_result->setText("Error: Δx must be positive");
        return;
    }
    if (x1 >= x2) {
        ui->label_y_result->setText("Error: x1 must be less than x2");
        return;
    }

    QVector<double> x, y;

    for (double val = x1; val <= x2; val += delta_x) {
        x.append(val);
        y.append(evaluate(val));
    }

    if ((x.isEmpty()) || (x.last() < x2)) {
        x.append(x2);
        y.append(evaluate(x2));
    }

    customPlot->clearGraphs();
    customPlot->addGraph();
    customPlot->graph(0)->setData(x, y);
    customPlot->graph(0)->setPen(QPen(Qt::blue));

    customPlot->xAxis->setLabel("x");
    customPlot->yAxis->setLabel("y");
    customPlot->graph(0)->rescaleAxes();
    customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

    customPlot->replot();

    ui->label_y_result->setText("Graph drawn successfully");
}
void MainWindow::calculateIntegral() {
    QString errorMessage;
    int result = parse(ui->lineEdit_function->text(), errorMessage);
    if (result != 0) {
        ui->label_integral_result->setText(QString("Error: %1").arg(errorMessage));
        return;
    }

    double a = ui->doubleSpinBox_x_1->value();
    double b = ui->doubleSpinBox_x_2->value();
    int n = ui->doubleSpinBox_n->value();

    if (a >= b) {
        ui->label_integral_result->setText("Error: a must be less than b");
        return;
    }
    if (n <= 0) {
        ui->label_integral_result->setText("Error: n must be positive");
        return;
    }

    // Yamuk kuralı ile integral hesaplama
    double h = (b - a) / n;
    double sum = 0.0;

    // İlk ve son terimler: f(a) ve f(b)
    sum += evaluate(a) + evaluate(b);

    // Ara terimler: 2 * f(x_i)
    for (int i = 1; i < n; i++) {
        double x = a + i * h;
        sum += 2 * evaluate(x);
    }

    double integral = (h / 2.0) * sum;

    // Sonucu göster
    ui->label_integral_result->setText(QString("Integral = %1").arg(integral));

    // Grafiği çiz ve alanı gölgelendir
    double x1 = ui->doubleSpinBox_x_1->value();
    double x2 = ui->doubleSpinBox_x_2->value();
    double delta_x = ui->doubleSpinBox_delta_x->value();

    if (delta_x <= 0) {
        ui->label_integral_result->setText("Error: Δx must be positive");
        return;
    }
    if (x1 >= x2) {
        ui->label_integral_result->setText("Error: x1 must be less than x2");
        return;
    }

    // Grafik için veriler
    QVector<double> x, y;
    for (double val = x1; val <= x2; val += delta_x) {
        x.append(val);
        y.append(evaluate(val));
    }
    if (x.isEmpty() || x.last() < x2) {
        x.append(x2);
        y.append(evaluate(x2));
    }

    // Grafiği temizle ve yeni bir grafik ekle
    customPlot->clearGraphs();
    customPlot->addGraph();
    customPlot->graph(0)->setData(x, y);
    customPlot->graph(0)->setPen(QPen(Qt::blue));

    // Gölgelendirme için x-eksenine kadar bir taban çizgisi oluştur
    customPlot->addGraph();
    QVector<double> x_base, y_base;
    double h_shade = (b - a) / n;
    for (double val = a; val <= b; val += h_shade) {
        x_base.append(val);
        y_base.append(0.0); // y = 0 (x-ekseni)
    }
    if (x_base.isEmpty() || x_base.last() < b) {
        x_base.append(b);
        y_base.append(0.0);
    }
    customPlot->graph(1)->setData(x_base, y_base);
    customPlot->graph(1)->setPen(Qt::NoPen); // Taban çizgisini görünmez yap

    // Gölgelendirme için a ile b arasındaki bölgeyi çiz
    customPlot->addGraph();
    QVector<double> x_shade, y_shade;
    for (double val = a; val <= b; val += h_shade) {
        x_shade.append(val);
        y_shade.append(evaluate(val));
    }
    if (x_shade.isEmpty() || x_shade.last() < b) {
        x_shade.append(b);
        y_shade.append(evaluate(b));
    }
    customPlot->graph(2)->setData(x_shade, y_shade);
    customPlot->graph(2)->setPen(Qt::NoPen); // Çizgiyi görünmez yap, sadece gölgelendirme kalsın
    customPlot->graph(2)->setBrush(QBrush(QColor(255, 0, 0, 50))); // Yarı saydam kırmızı gölgelendirme
    customPlot->graph(2)->setChannelFillGraph(customPlot->graph(1)); // x-eksenine kadar gölgelendirme

    // Eksenleri ayarla
    customPlot->xAxis->setLabel("x");
    customPlot->yAxis->setLabel("y");
    customPlot->graph(0)->rescaleAxes();
    customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

    customPlot->replot();
}
MainWindow::~MainWindow()
{
    delete ui;
}

