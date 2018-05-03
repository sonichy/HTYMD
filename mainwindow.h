#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QDragEnterEvent>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QLabel *LS1,*LS2,*LS3;
    QString path, sHTML;
    void open(QString filename);
    QString replace(QString s);
    int codeCount;

private slots:
    void on_action_new_triggered();
    void on_action_open_triggered();
    void on_action_save_triggered();
    void on_action_saveAs_triggered();
    void on_action_saveAsHTML_triggered();
    void on_action_changelog_triggered();
    void on_action_aboutQt_triggered();
    void on_action_about_triggered();
    //void on_action_zoomin_triggered();
    //void on_action_zoomout_triggered();
    void on_action_undo_triggered();
    void on_action_redo_triggered();
    //void on_action_find_triggered();
    //void on_action_indent_triggered();
    //void on_action_font_triggered();
    void on_actionReload_triggered();
    //void find();
    //void replace();
    //void replaceAll();
    void cursorPositionChange();    
    void textChange();
    void anchorClick(const QUrl& url);
    void scrollBarTEValueChanged(int v);
    void scrollBarTBValueChanged(int v);

protected:
    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);

};

#endif // MAINWINDOW_H
