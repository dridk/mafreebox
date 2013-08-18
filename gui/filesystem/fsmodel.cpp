#include "fsmodel.h"
#include <QDebug>
#include <QStandardItem>
#include <QMimeDatabase>
#include <QResource>

FSModel::FSModel(MaFreeBox *fbx, QObject *parent) :
    QStandardItemModel(parent)
{
    mFbx = fbx;
    setHorizontalHeaderLabels(QStringList()<<"Nom"<<"Taille"<<"Date de modification");
    setColumnCount(3);



    connect(mFbx->fileSystem(),SIGNAL(listReceived(QList<FileInfo>)),
            this,SLOT(dataReceived(QList<FileInfo>)));

    connect(mFbx->fileSystem(),SIGNAL(mkdirFinished()),this,SLOT(refreshCurrentIndex()));
    connect(mFbx->fileSystem(),SIGNAL(removeFinished()),this,SLOT(refreshCurrentIndex()));
    connect(mFbx->fileSystem(),SIGNAL(renameFinished()),this,SLOT(refreshCurrentIndex()));
    connect(mFbx->fileSystem(),SIGNAL(uploadFinished(QString)),this,SLOT(refreshCurrentIndex()));
    connect(this,SIGNAL(itemChanged(QStandardItem*)),this,SLOT(itemToBeRenamed(QStandardItem*)));


    connect(mFbx,SIGNAL(error(QString,QString)),this,SLOT(refreshCurrentIndex()));
}

bool FSModel::canFetchMore(const QModelIndex &parent) const
{
    int t = parent.data(FolderCountRole).toInt() +
            parent.data(FileCountRole).toInt();

    if (t > 0 && itemFromIndex(parent)->rowCount() == 0)
        return true;

    return QStandardItemModel::canFetchMore(parent);
}

void FSModel::fetchMore(const QModelIndex &parent)
{
    QString path = parent.data(PathRole).toString();

    mCurrentIndex = parent;
    if (!mIsLoading) {
        mFbx->fileSystem()->requestList(path,false,true,true);
        mIsLoading = true;
    }
}

bool FSModel::hasChildren(const QModelIndex &parent) const
{
    int t = parent.data(FolderCountRole).toInt();
    if (t > 0 )
        return true;
    else
        return QStandardItemModel::hasChildren(parent);
}

bool FSModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    qDebug()<<"drop";
    return true;
}

void FSModel::init()
{
    mCurrentIndex = QModelIndex();
    mFbx->fileSystem()->requestList(QString(),false,true,true);
}

void FSModel::dataReceived(const QList<FileInfo> &list)
{
    mIsLoading = false;
    QStandardItem * rootItem ;

    if (!mCurrentIndex.isValid())
        rootItem = invisibleRootItem();
    else
        rootItem = itemFromIndex(mCurrentIndex);

    qDebug()<<"CLEAR";
    int c = rootItem->rowCount();
    rootItem->removeRows(0,c);


    foreach (FileInfo i, list)
    {
        QList<QStandardItem*> lines;


        QStandardItem * firstItem  = new QStandardItem;
        firstItem->setText(i.name);
        firstItem->setColumnCount(3);
        QString iconUrl = QString(":/mime/%1.png").arg(i.mimetype.replace("/", "_").replace("-","_"));

        if (rootItem == invisibleRootItem())
            iconUrl = QString(":drive-harddisk.png");

        if(!QFile::exists(iconUrl))
            firstItem->setIcon(QIcon(":mime/application_octet_stream.png"));
        else
            firstItem->setIcon(QIcon(iconUrl));

        QStandardItem * secondItem = new QStandardItem;
        secondItem->setColumnCount(3);
        if (i.isDir)
            secondItem->setText(QString("%1 élément").arg(i.folderCount+i.fileCount));
        else
            secondItem->setText(sizeHuman(i.size));

        secondItem->setEditable(false);

        QStandardItem * thirdItem = new QStandardItem;
        thirdItem->setText(i.modified.toString("dd/MM/yyyy hh:mm"));
        thirdItem->setEditable(false);
        thirdItem->setColumnCount(3);

        firstItem->setData(i.folderCount, FolderCountRole);
        firstItem->setData(i.fileCount, FileCountRole);
        firstItem->setData(i.path, PathRole);
        firstItem->setData(i.mimetype, MimeTypeRole);
        firstItem->setData(i.isDir, IsDirRole);
        firstItem->setData(i.size, SizeRole);
        firstItem->setData(i.modified, ModifiedRole);
        firstItem->setData(i.index, IndexRole);
        firstItem->setData(i.link, IsLinkRole);
        firstItem->setData(i.hidden, IsHiddenRole);
        //        firstItem->setEditable(false);
//        firstItem->setDropEnabled(true);

        lines.append(firstItem);
        lines.append(secondItem);
        lines.append(thirdItem);
        rootItem->appendRow(lines);

    }

    QModelIndex begin  = indexFromItem(rootItem->child(0));
    QModelIndex end = indexFromItem(rootItem->child(rootItem->rowCount()));
    emit dataChanged(begin,end);



}

void FSModel::mkdir(const QString &name, const QModelIndex &parent)
{


    QString path = parent.data(PathRole).toString();
    mCurrentIndex = parent;
    mFbx->fileSystem()->requestMkdir(path, name);

}

void FSModel::remove(const QModelIndexList &indexes)
{
    QStringList paths;
    foreach (QModelIndex index, indexes) {
        paths.append(index.data(PathRole).toString());
    }

    mCurrentIndex = indexes.first().parent();
    mFbx->fileSystem()->requestRemove(paths);
}

void FSModel::upload(const QString &filename, const QModelIndex &parent)
{

    qDebug()<<"upload to "<<parent.data().toString();

    mCurrentIndex =  parent;
    mFbx->fileSystem()->requestUpload(filename,
                                      parent.data(PathRole).toString());


}

void FSModel::upload(const QStringList &list, const QModelIndex &parent)
{
    mCurrentIndex = parent;
}

void FSModel::download(const QString &destination, const QModelIndex &index)
{
    mFbx->fileSystem()->requestDownload(index.data(PathRole).toString()
                                        ,destination);

}




void FSModel::refreshCurrentIndex()
{
    fetchMore(mCurrentIndex);
}

void FSModel::itemToBeRenamed(QStandardItem *item)
{
    QString newName = item->text();
    QString path = item->data(PathRole).toString();
    mCurrentIndex = indexFromItem(item).parent();

    qDebug()<<"newName";
    qDebug()<<"path"<<path;

    mFbx->fileSystem()->requestRename(path,newName);

}

void FSModel::refresh(const QModelIndex &parent)
{
    fetchMore(parent);
}
QString FSModel::sizeHuman(int size) const
{
    float num = size;
    QStringList list;
    list << "KB" << "MB" << "GB" << "TB";

    QStringListIterator i(list);
    QString unit("bytes");

    while(num >= 1024.0 && i.hasNext())
    {
        unit = i.next();
        num /= 1024.0;
    }
    return QString().setNum(num,'f',2)+" "+unit;

}



QByteArray FSModel::currentPath(const QModelIndex &index)
{
    return index.data(PathRole).toString().toUtf8();
}




//=====================================================================
//================FilterModel