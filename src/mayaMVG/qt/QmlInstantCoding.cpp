#include "mayaMVG/qt/QmlInstantCoding.hpp"
#include "mayaMVG/core/MVGLog.hpp"
#include <QDir>

namespace mayaMVG
{

QmlInstantCoding::QmlInstantCoding(QDeclarativeView* attachedView, bool watching, bool watchSource,
                                   bool verbose)
    : QObject(attachedView)
{
    QDeclarativeView* view = dynamic_cast<QDeclarativeView*>(attachedView);
    if(view == NULL)
        LOG_ERROR("QmlInstantCoding: attachedView must be a QDeclarativeView");

    _fileWatcher = new QFileSystemWatcher();
    _attachedView = attachedView;
    _verbose = verbose;
    _watching = false;
    _extensions.push_back("qml");
    _extensions.push_back("js");

    // Update the watching status
    setWatching(watching);

    // If view already has a source, add it to files to watch
    if(_attachedView->status() != QDeclarativeView::Null && watchSource)
        addFile(_attachedView->source());
}

QmlInstantCoding::~QmlInstantCoding()
{
    delete _fileWatcher;
}

void QmlInstantCoding::setWatching(bool watchValue)
{
    if(_watching == watchValue)
        return;
    _watching = watchValue;
    // Enable the watcher
    if(_watching)
    {
        // 1. Add internal list of files to the internal Qt File Watcher
        addFiles(_watchedFiles);
        // 2. Connect 'filechanged' signal
        QObject::connect(_fileWatcher, SIGNAL(fileChanged(QString)), this,
                         SLOT(onFileChanged(QString)));
    }
    // Disable the watcher
    else
    {
        // 1. Remove all files in the internal Qt File Watcher
        _fileWatcher->removePaths(_watchedFiles);
        // 2. Disconnect 'filechanged' signal
        QObject::disconnect(_fileWatcher, SIGNAL(fileChanged(QString)), this,
                            SLOT(onFileChanged(QString)));
    }
}

void QmlInstantCoding::setRemarkableExtensions(const QStringList& extensions)
{
    _extensions = extensions;
}

const QStringList& QmlInstantCoding::getRemarkableExtensions() const
{
    return _extensions;
}

void QmlInstantCoding::setVerbose(bool verboseValue)
{
    _verbose = verboseValue;
}

void QmlInstantCoding::addFile(const QString& fileName)
{
    QFile file(fileName);
    if(!file.exists())
    {
        LOG_ERROR("addFile: file " << fileName.toStdString() << " doesn't exist.");
        return;
    }

    // Return if the file is already in our internal list
    if(_watchedFiles.contains(fileName))
        return;

    // Add this file to the internal files list
    _watchedFiles.append(fileName);

    //  And, if watching is active, add it to the internal watcher as well
    if(_watching)
        _fileWatcher->addPath(fileName);
}

void QmlInstantCoding::addFile(const QUrl& fileUrl)
{
    QString fileName = fileUrl.path();
    addFile(fileName);
}

void QmlInstantCoding::addFiles(const QStringList& filenames)
{
    for(int i = 0; i < filenames.size(); ++i)
        addFile(filenames.at(i));
}

void QmlInstantCoding::addFilesFromDirectory(const QString& dirname, bool recursive)
{
    QUrl dirUrl(dirname);
    if(!dirUrl.isValid())
    {
        LOG_ERROR("addFilesFromDirectory :  " << dirname.toStdString()
                                              << " is not a valid directory.");
        return;
    }

    QDir dir = QDir(dirname);
    if(recursive)
    {
        QDirIterator it(dir.path(), QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot,
                        QDirIterator::Subdirectories);
        while(it.hasNext())
        {
            it.next();
            // Directories
            if(it.fileInfo().isDir())
                addFilesFromDirectory(it.filePath(), true);
            // Files
            if(it.fileInfo().isFile())
            {
                // Check if extenstion is right
                if(_extensions.contains(it.fileInfo().completeSuffix()))
                    addFile(QString(it.fileInfo().path() + "/" + it.fileInfo().fileName()));
            }
        }
    }
    else
    {
        QDirIterator itFiles(dir.path(), QDir::Files);
        QStringList fileList;
        while(itFiles.hasNext())
        {
            if(_extensions.contains(itFiles.fileInfo().completeSuffix()))
                fileList.append(dirname + itFiles.fileName());
        }
        addFiles(fileList);
    }
}

void QmlInstantCoding::removeFile(const QString& filename)
{
    if(_watchedFiles.contains(filename))
        _watchedFiles.removeAll(filename);
    if(_watching)
        _fileWatcher->removePath(filename);
}

const QStringList& QmlInstantCoding::getRegisteredFile() const
{
    return _watchedFiles;
}

void QmlInstantCoding::onFileChanged(const QString& sourceFile)
{
    if(_verbose)
        LOG_INFO("Source file changed :" << sourceFile.toStdString());

    // Retrieve source file from attached view
    QUrl source = _attachedView->source();

    // Clear the QDeclarativeEngine cache
    _attachedView->engine()->clearComponentCache();

    // Remove the modified file from the watched list
    removeFile(sourceFile);
    int cpTry = 0;

    // Make sure file is available before doing anything
    // NOTE: useful to handle editors (Qt Creator) that deletes the source file
    // and
    //       creates a new one when saving
    QFile file(sourceFile);
    while(!file.exists() && cpTry < 10)
        usleep(100); // 0.1 ms

    LOG_INFO("Reloading " << sourceFile.toStdString());

    // To reload the view, re-set the source
    _attachedView->setSource(source);

    // Finally, readd the modified file to the watch system
    addFile(sourceFile);
}

} // namespace
