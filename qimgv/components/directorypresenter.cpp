#include "directorypresenter.h"

DirectoryPresenter::DirectoryPresenter(QObject *parent) : QObject(parent) {
}

void DirectoryPresenter::unsetModel() {
    disconnect(model.get(), &DirectoryModel::fileRemoved,    this, &DirectoryPresenter::onFileRemoved);
    disconnect(model.get(), &DirectoryModel::fileAdded,      this, &DirectoryPresenter::onFileAdded);
    disconnect(model.get(), &DirectoryModel::fileModified,   this, &DirectoryPresenter::onFileModified);
    disconnect(model.get(), &DirectoryModel::fileModifiedInternal,   this, &DirectoryPresenter::onFileModified);
    disconnect(model.get(), &DirectoryModel::fileRenamed,    this, &DirectoryPresenter::onFileRenamed);
    disconnect(model.get(), &DirectoryModel::thumbnailReady, this, &DirectoryPresenter::onThumbnailReady);
    disconnect(this, &DirectoryPresenter::generateThumbnails, model.get(), &DirectoryModel::generateThumbnails);
    model = nullptr;
    // also empty views?
}

void DirectoryPresenter::setFolderView(std::shared_ptr<FolderViewProxy> view) {
    if(folderView)
        return;
    folderView = view;
    if(model)
        folderView->populate(model->itemCount());
    connect(folderView.get(), &FolderViewProxy::itemSelected,
            this, &DirectoryPresenter::itemSelected);
    connect(folderView.get(), &FolderViewProxy::thumbnailsRequested,
            this, &DirectoryPresenter::generateThumbnails);
}

void DirectoryPresenter::setThumbPanel(std::shared_ptr<ThumbnailStrip> view) {
    if(thumbPanel)
        return;
    thumbPanel = view;
    if(model)
        view->populate(model->itemCount());
    connect(thumbPanel.get(), &ThumbnailStrip::itemSelected,
            this, &DirectoryPresenter::itemSelected);
    connect(thumbPanel.get(), &ThumbnailStrip::thumbnailsRequested,
            this, &DirectoryPresenter::generateThumbnails);
}

void DirectoryPresenter::setModel(std::shared_ptr<DirectoryModel> newModel) {
    if(model)
        unsetModel();
    if(!newModel)
        return;
    model = newModel;
    populateViews();

    // filesystem changes
    connect(model.get(), &DirectoryModel::fileRemoved,    this, &DirectoryPresenter::onFileRemoved);
    connect(model.get(), &DirectoryModel::fileAdded,      this, &DirectoryPresenter::onFileAdded);
    connect(model.get(), &DirectoryModel::fileModified,   this, &DirectoryPresenter::onFileModified);
    connect(model.get(), &DirectoryModel::fileModifiedInternal,   this, &DirectoryPresenter::onFileModified);
    connect(model.get(), &DirectoryModel::fileRenamed,    this, &DirectoryPresenter::onFileRenamed);
    connect(model.get(), &DirectoryModel::thumbnailReady, this, &DirectoryPresenter::onThumbnailReady);
    connect(this, &DirectoryPresenter::generateThumbnails, model.get(), &DirectoryModel::generateThumbnails);
}

void DirectoryPresenter::reloadModel() {
    populateViews();
}

void DirectoryPresenter::populateViews() {
    if(!model)
        return;
    if(folderView)
        folderView->populate(model->itemCount());
    if(thumbPanel)
        thumbPanel->populate(model->itemCount());
}

void DirectoryPresenter::disconnectAllViews() {
   // todo
}

//------------------------------------------------------------------------------

void DirectoryPresenter::onFileRemoved(QString fileName, int index) {
    Q_UNUSED(fileName)
    if(folderView)
        folderView->removeItem(index);
    if(thumbPanel)
        thumbPanel->removeItem(index);
}

void DirectoryPresenter::onFileRenamed(QString from, int indexFrom, QString to, int indexTo) {
    Q_UNUSED(from)
    Q_UNUSED(to)

    if(folderView) {
        int selectedIndex = folderView->selectedIndex();
        folderView->removeItem(indexFrom);
        folderView->insertItem(indexTo);
        if(selectedIndex == indexFrom ||
           selectedIndex == -1)
        {
            folderView->selectIndex(indexTo);
            folderView->focusOn(indexTo);
        }
    }
    if(thumbPanel) {
        int selectedIndex = thumbPanel->selectedIndex();
        thumbPanel->removeItem(indexFrom);
        thumbPanel->insertItem(indexTo);
        if(selectedIndex == indexFrom ||
           selectedIndex == -1)
        {
            thumbPanel->selectIndex(indexTo);
            thumbPanel->focusOn(indexTo);
        }
    }
}

void DirectoryPresenter::onFileAdded(QString fileName) {
    int index = model->indexOf(fileName);
    if(folderView)
        folderView->insertItem(index);
    if(thumbPanel)
        thumbPanel->insertItem(index);
}

void DirectoryPresenter::onFileModified(QString fileName) {
    int index = model->indexOf(fileName);
    if(folderView)
        folderView->reloadItem(index);
    if(thumbPanel)
        thumbPanel->reloadItem(index);
}

void DirectoryPresenter::onThumbnailReady(std::shared_ptr<Thumbnail> thumb) {
    int index = model->indexOf(thumb->name());
    if(folderView)
        folderView->setThumbnail(index, thumb);
    if(thumbPanel)
        thumbPanel->setThumbnail(index, thumb);
}

// tmp -- ?
void DirectoryPresenter::setCurrentIndex(int index) {
    if(folderView)
        folderView->selectIndex(index);
    if(thumbPanel)
        thumbPanel->selectIndex(index);
}

void DirectoryPresenter::focusOn(int index) {
    if(folderView)
        folderView->focusOn(index);
    if(thumbPanel)
        thumbPanel->focusOn(index);
}

void DirectoryPresenter::selectAndFocus(int index) {
    if(folderView)  {
        folderView->selectIndex(index);
        folderView->focusOn(index);
    }
    if(thumbPanel) {
        thumbPanel->selectIndex(index);
        thumbPanel->focusOn(index);
    }
}

// TODO: in future this will behave differently when the view has multi-selection (not implemented yet)
void DirectoryPresenter::onIndexChanged(int index) {
    this->selectAndFocus(index);
}
