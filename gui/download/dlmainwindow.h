#ifndef DLMAINWINDOW_H
#define DLMAINWINDOW_H
#include "abstractmainwindow.h"
#include "dlmodel.h"
#include "dltableview.h"
#include "dlcategorywidget.h"
#include "dldetailwidget.h"
#include "dldelegate.h"

class DLMainWindow : public AbstractMainWindow
{
    Q_OBJECT
public:
    explicit DLMainWindow(QWidget *parent = 0);
    
signals:
    
public slots:

private:
    DLTableView * mView;
    DLCategoryWidget * mCategoryWidget;
    DLDetailWidget * mDetailWidget;

    
};

#endif // DLMAINWINDOW_H
