#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDesktopWidget>
#include <QFileDialog>
#include <QFile>
#include <QProcess>
#include <QTextStream>
#include <QTextCodec>
#include <QMessageBox>
#include <QPushButton>
#include <QDebug>
#include <QMimeData>
#include <QDesktopServices>
#include <QScrollBar>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->textBrowser->zoomIn(4);
    ui->textBrowser_HTML->zoomIn(4);
    ui->textBrowser_HTML->hide();
    move((QApplication::desktop()->width() - width())/2, (QApplication::desktop()->height() - height())/2);
    connect(ui->action_quit, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(ui->textEdit, SIGNAL(cursorPositionChanged()), this, SLOT(cursorPositionChange()));
    connect(ui->textEdit, SIGNAL(textChanged()), this, SLOT(textChange()));
    connect(ui->textEdit->verticalScrollBar(), SIGNAL(valueChanged(int)),this,SLOT(scrollBarTEValueChanged(int)));
    //connect(ui->textBrowser->verticalScrollBar(), SIGNAL(valueChanged(int)),this,SLOT(scrollBarTBValueChanged(int)));
    connect(ui->textBrowser, SIGNAL(anchorClicked(const QUrl&)), this, SLOT(anchorClick(const QUrl&)));

    //状态栏分割
    LS1 = new QLabel("欢迎使用海天鹰 Markdown 编辑器！");
    LS1->setMinimumSize(500,20);
    //LS1->setFixedHeight(20);
    LS1->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    LS1->setStyleSheet("padding:0px 3px;");
    //LS1->setFrameShape(QFrame::WinPanel);
    //LS1->setFrameShadow(QFrame::Sunken);
    LS2 = new QLabel("行,列:1,0");
    LS2->setMinimumSize(20,20);
    LS2->setStyleSheet("padding:0px 3px;");
    LS3 = new QLabel("编码");
    LS3->setMinimumSize(20,20);
    LS3->setStyleSheet("padding:0px 3px;");
    ui->statusBar->addWidget(LS1);
    ui->statusBar->addWidget(LS2);
    ui->statusBar->addWidget(LS3);

    QStringList Largs = QApplication::arguments();
    qDebug() << Largs;
    if (Largs.length() > 1) {
        QUrl url(Largs.at(1));
        open(url.toLocalFile());
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_action_new_triggered()
{
    //path = "";
    ui->textEdit->setText("");
    setWindowTitle("新建 - HTYMD");
    LS1->setText("新建文件");
}

void MainWindow::on_action_open_triggered()
{
    if (path == "") {
        path = QFileDialog::getOpenFileName(this, "打开文本", ".", "*.md");
    } else {
        path = QFileDialog::getOpenFileName(this, "打开文本", path, "*.md");
    }
    if (!path.isEmpty()) {
        open(path);
    }
}

void MainWindow::on_actionReload_triggered()
{
    if (path != "") {
        open(path);
    }
}

void MainWindow::open(QString filename)
{
    qDebug() << "open" << filename;
    path = filename;
    setWindowTitle(QFileInfo(filename).fileName() + "[*]");
    // 检测编码
    QProcess *process = new QProcess;
    process->start("file --mime-encoding " + filename);
    process->waitForFinished();
    QString PO = process->readAllStandardOutput();
    QString scodec = PO.mid(PO.indexOf(": ") + 2).replace("\n","").toUpper();
    //qDebug() << scodec;
    LS3->setText(scodec);

    QFile *file = new QFile;
    file->setFileName(filename);
    bool ok = file->open(QIODevice::ReadOnly);
    if(ok){
        QTextStream ts(file);
        // 还是乱码
        QTextCodec *codec = QTextCodec::codecForName(scodec.toLatin1());
        ts.setCodec(codec);
        QString s = ts.readAll();
        file->close();
        delete file;
        ui->textEdit->setPlainText(s);
        ui->textEdit->setFont(QFont("Consolas",15));
        LS1->setText("打开 " + filename);
        LS2->setText("行,列：1,0");
    }else{
        QMessageBox::warning(this,"错误", QString(" %1:\n%2").arg(filename).arg(file->errorString()));
    }
    setWindowModified(false);
}

void MainWindow::on_actionViewHTML_triggered()
{
    if(ui->actionViewHTML->isChecked()){
        ui->textBrowser_HTML->setVisible(true);
    }else{
        ui->textBrowser_HTML->setVisible(false);
    }
}

void MainWindow::on_action_save_triggered()
{
    if (path == "") {
        on_action_saveAs_triggered();
    } else {
        QFile file(path);
        if (file.open(QFile::WriteOnly)) {
            QTextStream ts(&file);
            QString s = ui->textEdit->toPlainText();
            ts << s;
            //setWindowTitle(QFileInfo(path).fileName() + "[*]");
            setWindowModified(false);
            LS1->setText("保存 " + path);
        }else{
            QMessageBox::warning(this,"错误", QString(" %1:\n%2").arg(path).arg(file.errorString()));
        }
    }
}

void MainWindow::on_action_saveAs_triggered()
{
    if (path == "") {
        path = QFileDialog::getSaveFileName(this, "保存文本", "./未命名");
    } else {
        path = QFileDialog::getSaveFileName(this, "保存文本", path);
    }
    if (!path.isEmpty()) {
        on_action_save_triggered();
    }
}

void MainWindow::on_action_saveAsHTML_triggered()
{
    QString pathHTML;
    if (path == "") {
        pathHTML = QFileDialog::getSaveFileName(this, "另保存为HTML", "./未命名.htm");
    } else {
        pathHTML = QFileInfo(path).filePath() + "/" + QFileInfo(path).baseName() + ".htm";
        pathHTML = QFileDialog::getSaveFileName(this, "另保存为HTML", pathHTML);
    }
    if (!pathHTML.isEmpty()) {
        QFile file(pathHTML);
        if (file.open(QFile::WriteOnly)) {
            QTextStream ts(&file);
            //QString s = ui->textBrowser->toHtml();
            //ts << s;
            ts << sHTML;
            LS1->setText("另存为 " + pathHTML);
        }else{
            QMessageBox::warning(this,"错误", QString(" %1:\n%2").arg(path).arg(file.errorString()));
        }
    }
}

void MainWindow::cursorPositionChange()
{
    QTextCursor cursor = ui->textEdit->textCursor();
    LS2->setText(QString("行,列: %1,%2").arg(cursor.blockNumber()+1).arg(cursor.columnNumber()));
}

void MainWindow::on_action_aboutQt_triggered()
{
    QMessageBox::aboutQt(NULL, "关于 Qt");
}

void MainWindow::on_action_about_triggered()
{
    QMessageBox aboutMB(QMessageBox::NoIcon, "关于", "海天鹰 Markdown 编辑器 1.0\n\n一款基于 Qt 的 Markdown 编辑程序。\n作者：黄颖\nE-mail: sonichy@163.com\n主页：https://github.com/sonichy");
    aboutMB.setIconPixmap(QPixmap(":/icon.svg"));
    aboutMB.setWindowIcon(QIcon(":/icon.svg"));
    aboutMB.exec();
}

void MainWindow::on_action_undo_triggered()
{
    ui->textEdit->undo();
}

void MainWindow::on_action_redo_triggered()
{
    ui->textEdit->redo();
}

void MainWindow::on_action_changelog_triggered()
{
    QString s = "1.0\n2019-04\n删除线符号~~替换为<s>\n增加HTML源码查看分栏。\n2018-04\n制作分栏布局，从文本编辑器向 Markdown 迁移。";
    QDialog *dialog = new QDialog;
    dialog->setWindowTitle("更新历史");
    dialog->setFixedSize(400,300);
    QVBoxLayout *vbox = new QVBoxLayout;
    QTextBrowser *textBrowser = new QTextBrowser;
    textBrowser->setText(s);
    textBrowser->zoomIn();
    vbox->addWidget(textBrowser);
    QHBoxLayout *hbox = new QHBoxLayout;
    QPushButton *btnConfirm = new QPushButton("确定");
    hbox->addStretch();
    hbox->addWidget(btnConfirm);
    hbox->addStretch();
    vbox->addLayout(hbox);
    dialog->setLayout(vbox);
    dialog->show();
    connect(btnConfirm, SIGNAL(clicked()), dialog, SLOT(accept()));
    if(dialog->exec() == QDialog::Accepted){
        dialog->close();
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
    qDebug() << "dragEnter:" << e->mimeData()->formats().at(0);
    if(e->mimeData()->hasFormat("text/uri-list")) //只能打开文本文件
        e->acceptProposedAction(); //可以在这个窗口部件上拖放对象
}

void MainWindow::dropEvent(QDropEvent *e) //释放对方时，执行的操作
{
    QList<QUrl> urls = e->mimeData()->urls();
    if(urls.isEmpty())
        return ;

    QString fileName = urls.first().toLocalFile();

    foreach (QUrl u, urls) {
        qDebug() << u.toString();
    }
    qDebug() << urls.size();

    if(fileName.isEmpty())
        return;

    qDebug() << "drop:" << fileName;
    open(fileName);
}

void MainWindow::textChange()
{
    setWindowModified(true);
    LS1->setText("");
    QString s = ui->textEdit->toPlainText();
    QStringList SL = s.split("\n");
    QString s1 = "<html>\r\n<head>\r\n<style>\r\n"
                 "table{border-collapse:collapse;}\r\n"
                 "td { border:1px solid black; padding:5px; }\r\n"
                 "a { text-decoration:none; }\r\n"
                 "pre { background:#eeeeee; width:fit-content;}\r\n"
                 "code { background:#eeeeee; }\r\n"
                 "</style>\r\n</head>\r\n<body>\r\n";
    codeCount = 0;
    for(int i=0; i<SL.size(); i++){
        if(SL.at(i) == ""){
            s1 += "<br>";
        }else if(SL.at(i) == "```"){
            codeCount++;
            if (codeCount % 2 == 0) {
                s1 += "</pre>\r\n";
            } else {
                s1 += "<pre>\r\n";
            }
            continue;
        }
        if (codeCount % 2 == 0) {
            s1 += replace(SL.at(i));
        }else{
            QString sCode = SL.at(i) + "\r\n";
            s1 += sCode.replace("<","&lt;").replace(">","&gt;");
        }
    }
    s1 += "</body>\r\n</html>";
    ui->textBrowser->setHtml(s1);
    ui->textBrowser_HTML->setPlainText(s1);
    sHTML = s1;
}

QString MainWindow::replace(QString s)
{
    QString s1;
    //替换<h4-1>
    if(s.contains("# ")){
        int index = s.indexOf("#### ");
        if(index != -1){
            s1 = "<h4>" + s.right(s.length()-index-5) + "</h4>\r\n";
            //qDebug() << index << s << "->" << s1;
            return s1;
        }
        index = s.indexOf("### ");
        if(index != -1){
            s1 = "<h3>" + s.right(s.length()-index-4) + "</h3>\r\n";
            //qDebug() << index << s << "->" << s1;
            return s1;
        }
        index = s.indexOf("## ");
        if(index != -1){
            s1 = "<h2>" + s.right(s.length()-index-3) + "</h2>\r\n";
            //qDebug() << index << s << "->" << s1;
            return s1;
        }
        index = s.indexOf("# ");
        if(index != -1){
            s1 = "<h1>" + s.right(s.length()-index-2) + "</h1>\r\n";
            //qDebug() << index << s << "->" << s1;
            return s1;
        }
    }


    if(s.contains("![")){
        //qDebug() << s;
        // https://forum.qt.io/topic/89445/qregexp-replace
        QRegularExpression r("!\\[([^[]*)\\]\\(([^(]*)\\)");
        for (QRegularExpressionMatch m = r.match(s); m.hasMatch(); m = r.match(s)) {
            QString url = m.captured(2);
            if (!url.contains("://")) url = QFileInfo(path).absolutePath() + "/" + url;
            QString alt = m.captured(1);
            if (alt=="alt") {
                s.replace(m.capturedStart(0), m.capturedLength(0), QString("<img src='%1'>").arg(url));
            } else {
                s.replace(m.capturedStart(0), m.capturedLength(0), QString("<img src='%1' alt='%2'>").arg(url).arg(alt));
            }
        }
        //if (s.right(2)=="  ") s = s.left(s.length()-2) + "<br>";
        qDebug() << s;
        //return s;
    } else if(s.contains("[")) {
        qDebug() << s;
        QRegularExpression r("\\[([^[]*)\\]\\(([^(]*)\\)");
        for (QRegularExpressionMatch m = r.match(s); m.hasMatch(); m = r.match(s)) {
            QString url = m.captured(2);
            if (!url.contains("://")) url = QFileInfo(path).absolutePath() + "/" + url;
            QString text = m.captured(1);
            s.replace(m.capturedStart(0), m.capturedLength(0), QString("<a href='%1'>%2</a>").arg(url).arg(text));
        }
        //if (s.right(2)=="  ") s = s.left(s.length()-2) + "<br>";
        qDebug() << s;
        //return s;
    }

    if(s.contains("* ")){
        s = "<ul><li>" + s.right(s.length()-2) + "</li></ul>";
        qDebug() << s << "->" << s1;
        //return s1;
    }

    if(s.contains("```")){
        QStringList SL = s.split("```");
        s = "";
        for(int i=0; i<SL.size(); i++){
            s += SL.at(i);
            if(i<SL.size()-1){
                if(i%2 == 0){
                    s += "<code>";
                }else{
                    s += "</code>";
                }
            }
        }
    }

    if(s.contains("://")) s = "<a href='" + s + "'>" + s + "</a>" ;

    //删除线
    if(s.contains("~~")){
        //s.replace("~~","<s>");
        s.replace(QRegularExpression("~~(.*?)~~"), "<s>\\1</s>");
    }

    if (s.right(2) == "  ") s = s.left(s.length()-2) + "<br>";

    s += "\r\n";
    return s;
}

void MainWindow::anchorClick(const QUrl& url)
{
    QDesktopServices::openUrl(url);
}

void MainWindow::scrollBarTEValueChanged(int v)
{
    //qDebug() << v;
    QScrollBar *scrollBarTB = ui->textBrowser->verticalScrollBar();
    scrollBarTB->setValue(v);
}

void MainWindow::scrollBarTBValueChanged(int v)
{
    //qDebug() << v;
    QScrollBar *scrollBarTE = ui->textEdit->verticalScrollBar();
    scrollBarTE->setValue(v);
}
