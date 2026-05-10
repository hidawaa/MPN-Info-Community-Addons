#ifndef EXAMPLEADDON_H
#define EXAMPLEADDON_H

#include "interface.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>

#include <coreengine.h>

// Example Pertama
class ExamplePage : public Page
{
public:
    ExamplePage(QWidget *parent = nullptr) : Page(parent) {
        label = new QLabel("Ini adalah halaman contoh");
        label->setAlignment(Qt::AlignCenter);

        QVBoxLayout *layout = new QVBoxLayout;
        layout->addWidget(label);

        setLayout(layout);
    }

    ~ExamplePage() {}

    QStringList keys() { return QStringList() << "setText"; }
    QVariant exec(const QString &key, const QVariant &arg) {
        if (key == "setText")
            label->setText(arg.toString());
        return true;
    }

private:
    QLabel *label;
};

class ExamplePageAddOn : public AddOn
{
public:
    QString name() { return "example_page"; }
    QString group() { return "Example"; }
    QString title() { return "Example AddOn Page"; }
    AddOnTypes type() { return AddOnPage; }
    int loadFlags() { return AddOnCreateMenu; }
    int permission() { return 0; }
    PagePtr newPage() { return PagePtr(new ExamplePage); }
};

// Example Kedua
class ExampleProcess : public Process
{
public:
    void run() {
        QMessageBox::information(nullptr, "Example Process", "Ini adalah contoh process yang jalan otomasis setelah login");
    }
};

class ExampleProcessAddOn : public AddOn
{
public:
    QString name() { return "example_process"; }
    QString group() { return "Example"; }
    QString title() { return "Example AddOn Process"; }
    AddOnTypes type() { return AddOnProcess; }
    // int loadFlags() { return AddOnCreateMenu | AddOnExecAfterLogin; }
    int loadFlags() { return AddOnCreateMenu; }
    int permission() { return 0; }
    ProcessPtr newProcess() { return ProcessPtr(new ExampleProcess); }
};

// Example Ketiga
class ExampleLoadPage : public Page
{
public:
    ExampleLoadPage(CoreEngine * engine, QWidget *parent = nullptr) :
        Page(parent), mEngine(engine) {

        QLabel *label1 = new QLabel("Load New Page", this);
        QLabel *label2 = new QLabel("Load New Page with setArgs", this);
        QLabel *label3 = new QLabel("Load AddOn xlsx dan melihat available keys untuk exec", this);
        QLabel *label4 = new QLabel("Load Xlsx", this);
        QLabel *label5 = new QLabel("Save Xlsx", this);

        QPushButton *button1 = new QPushButton("Button 1", this);
        QPushButton *button2 = new QPushButton("Button 2", this);
        QPushButton *button3 = new QPushButton("Button 3", this);
        QPushButton *button4 = new QPushButton("Button 4", this);
        QPushButton *button5 = new QPushButton("Button 5", this);

        label1->move(20, 20);
        button1->move(20, 40);

        label2->move(20, 80);
        button2->move(20,100);

        label3->move(20, 140);
        button3->move(20, 160);

        label4->move(20, 200);
        button4->move(20, 220);

        label5->move(20, 260);
        button5->move(20, 280);

        connect(button1, &QPushButton::clicked, this, &ExampleLoadPage::onButton1Clicked);
        connect(button2, &QPushButton::clicked, this, &ExampleLoadPage::onButton2Clicked);
        connect(button3, &QPushButton::clicked, this, &ExampleLoadPage::onButton3Clicked);
        connect(button4, &QPushButton::clicked, this, &ExampleLoadPage::onButton4Clicked);
        connect(button5, &QPushButton::clicked, this, &ExampleLoadPage::onButton5Clicked);
    }

    ~ExampleLoadPage() {}

private slots:
    void onButton1Clicked() {
        AddOnPtr addOnPtr = mEngine->addOn("example_page");
        PagePtr pagePtr = addOnPtr->newPage();
        mEngine->window()->addPage(pagePtr, addOnPtr->title());
    }

    void onButton2Clicked() {
        AddOnPtr addOnPtr = mEngine->addOn("example_page");
        PagePtr page = addOnPtr->newPage();
        page->exec("setText", "Ini adalah halaman contoh yang isinya dapat dirubah dengan setArgs");

        mEngine->window()->addPage(page, addOnPtr->title());
    }

    void onButton3Clicked() {
        AddOnPtr addOnPtr = mEngine->addOn("xlsx");
        ObjectPtr xlsxPtr = addOnPtr->newObject();
        QMessageBox::information(nullptr, "Available Keys AddOn xlsx:", xlsxPtr->keys().join("\n"));
    }

    void onButton4Clicked() {
        AddOnPtr addOnPtr = mEngine->addOn("xlsx");
        ObjectPtr xlsxPtr = addOnPtr->newObject();

        QString filename = QFileDialog::getOpenFileName(nullptr, "Load XLSX", QString(), "Xlsx Files (*.xlsx)");
        if (filename.isEmpty())
            return;

        if (!xlsxPtr->exec("load", filename).toBool())
            return;

        QVariantMap args;

        args["cell"] = "A1";
        QString A1 = xlsxPtr->exec("value", args).toString();

        args["cell"] = "B1";
        QString B1 = xlsxPtr->exec("value", args).toString();

        args["cell"] = "C1";
        QString C1 = xlsxPtr->exec("value", args).toString();

        QMessageBox::information(nullptr, "Load Excel", QString("Isi dari cell:\nA1 %1\nB1 %2\nC1 %3\n").arg(A1, B1, C1));
    }

    void onButton5Clicked() {
        AddOnPtr addOnPtr = mEngine->addOn("xlsx");
        ObjectPtr xlsxPtr = addOnPtr->newObject();

        xlsxPtr->exec("createWorksheet", "Test");

        QVariantMap args;

        args["cell"] = "A1"; args["value"] = "satu";
        xlsxPtr->exec("setValue", args).toString();

        args["cell"] = "B1"; args["value"] = "dua";
        xlsxPtr->exec("setValue", args).toString();

        args["cell"] = "C1"; args["value"] = "tiga";
        xlsxPtr->exec("setValue", args).toString();

        QString filename = QFileDialog::getSaveFileName(nullptr, "Save XLSX", QString(), "Xlsx Files (*.xlsx)");
        if (filename.isEmpty())
            return;

        xlsxPtr->exec("save", filename);
        QMessageBox::information(nullptr, "Save Excel", "Done!");
    }

private:
    CoreEngine *mEngine;
};

class ExampleLoadPageAddOn : public AddOn
{
public:
    QString name() { return "example_load_page"; }
    QString group() { return "Example"; }
    QString title() { return "Example AddOn Load other AddOn"; }
    AddOnTypes type() { return AddOnPage; }
    int loadFlags() { return AddOnCreateMenu; }
    int permission() { return 0; }
    PagePtr newPage() { return PagePtr(new ExampleLoadPage(engine)); }
};

#endif // EXAMPLEADDON_H
