#pragma once

#include <QtCore>
#include <QtDeclarative>
#include "QtDeclarative/qdeclarativeview.h"
#include <iostream>
#include <ctime>
#include "mayaMVG/core/MVGLog.h"

namespace mayaMVG {

/**
*	QmlInstantCoding is an utility class helping developing QML applications.
*	It reloads its attached QDeclarativeView whenever one of the watched source file is modified.
*	As it consumes resources, make sure to disable file watching in production mode.
*/
class QmlInstantCoding : public QObject {

	Q_OBJECT

	public: 

		/**
		 *  Build a QmlInstantCoding instance.
		 * @param attachedView The QDeclarativeView on which this QmlInstantCoding is applied
		 * @param watching  if True, file watching is enable (default: True)
		 * @param watchSource watch the attached QDeclarativeView source file if it already has one (default: False)
		 * @param verbose if True, output log infos (default: False)
		 */
		QmlInstantCoding(QDeclarativeView* attachedView, bool watching = true, bool watchSource = false, bool verbose = false);
		
		~QmlInstantCoding();


		/**
		 * Enable (True) or disable (False) the file watching.
		 * Tip: file watching should be enable only when developing.
		 * @param watchValue
		 */
		void setWatching(bool watchValue);

		/**
		 * Set the list of extensions to search for when using addFilesFromDirectory.
		 * @param extensions
		 */
		void setRemarkableExtensions(const QStringList& extensions);	

		/**
		 * Returns the list of extensions used when using addFilesFromDirectory.
		 * @return 
		 */
		const QStringList& getRemarkableExtensions() const;

		/**
		 * Activate (True) or desactivate (False) the verbose.
		 * @param verboseValue
		 */
		void setVerbose(bool verboseValue);

		/**
		 *  Add the given 'fileName' to the watched files list.
		 * @param fileName
		 * @see addFile(QUrl fileUrl)
		 */
		void addFile(const QString& fileName);

		/**
		 * Add the given 'fileName' to the watched files list.
		 * @param fileUrl
		 * @see addFile(QString fileName)
		 */
		void addFile(const QUrl& fileUrl);


		/**
		 * Add the given 'filenames' to the watched files list.
		 * @param filenames a list of absolute or relative paths
		 */
		void addFiles(const QStringList& filenames);

		/**
		 * Add files from the given directory name 'dirname'.
		 * 
		 * @param dirname an absolute or a relative path
		 * @param recursive if True, will search inside each subdirectories recursively.
		 */
		void addFilesFromDirectory(const QString& dirname, bool recursive);

		/**
		 *  Remove the given 'filename' from the watched file list.
		 *  Tip: make sure to use relative or absolute path according to how you add this file.
		 * @param filename
		 */
		void removeFile(const QString& filename);

		/**
		 * Returns the list of watched files
		 * @return 
		 */
		const QStringList& getRegisteredFile() const;

		/**
		 * Handle changes in a watched file.
		 * @param sourceFile
		 */
		Q_SLOT void onFileChanged(const QString& sourceFile);


	private:
		QFileSystemWatcher*	_fileWatcher;
		QDeclarativeView* _attachedView;
		QStringList	_watchedFiles;
		bool _verbose;
		bool _watching;
		QStringList	_extensions;
};

} // mayaMVG
