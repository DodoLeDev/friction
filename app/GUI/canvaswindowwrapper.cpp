#include "canvaswindowwrapper.h"

#include <QComboBox>
#include <QLabel>

#include "GUI/newcanvasdialog.h"
#include "canvaswindow.h"
#include "document.h"

class CanvasWrapperMenuBar : public StackWrapperMenu {
    friend struct CWWidgetStackLayoutItem;
    friend class CanvasWindowWrapper;
public:
    CanvasWrapperMenuBar(Document& document, CanvasWindow * const window) :
        mDocument(document), mWindow(window) {
        addAction("+", this, [this]() {
            const QString defName = "Scene " +
                    QString::number(mDocument.fScenes.count());

            const auto dialog = new CanvasSettingsDialog(defName, mWindow);
            connect(dialog, &QDialog::accepted, this, [this, dialog]() {
                const auto newScene = mDocument.createNewScene();
                dialog->applySettingsToCanvas(newScene);
                dialog->close();
                setCurrentScene(newScene);
            });

            dialog->show();
        });
        mSceneMenu = addMenu("none");
        addAction("x", this, [this]() {
            if(!mCurrentScene) return;
            mDocument.removeScene(GetAsSPtr(mCurrentScene, Canvas));
        });
        mSceneMenu->setDisabled(true);
        for(const auto& scene : mDocument.fScenes)
            addScene(scene.get());

        connect(&mDocument, qOverload<Canvas*>(&Document::sceneRemoved),
                this, &CanvasWrapperMenuBar::removeScene);
        connect(&mDocument, &Document::sceneCreated,
                this, &CanvasWrapperMenuBar::addScene);
    }
private:
    void addScene(Canvas * const scene) {
        if(!scene) return;
        if(mSceneToAct.empty()) mSceneMenu->setEnabled(true);
        const auto act = mSceneMenu->addAction(scene->getName());
        act->setCheckable(true);
        connect(act, &QAction::triggered, this,
                [this, scene, act]() { setCurrentScene(scene, act); });
        connect(scene, &Canvas::canvasNameChanged, act,
                [this, act](Canvas* const scene, const QString& name) {
            if(scene == mCurrentScene) mSceneMenu->setTitle(name);
            act->setText(name);
        });
        mSceneToAct.insert({scene, act});
        //if(!mCurrentScene) setCurrentScene(scene, act);
    }

    void removeScene(Canvas * const scene) {
        const auto removeIt = mSceneToAct.find(scene);
        if(removeIt == mSceneToAct.end()) return;
        if(mCurrentScene == scene) {
            const auto newIt = mSceneToAct.begin();
            if(newIt == mSceneToAct.end()) setCurrentScene(nullptr);
            else setCurrentScene(newIt->first, newIt->second);
        }
        mSceneMenu->removeAction(removeIt->second);
        delete removeIt->second;
        mSceneToAct.erase(removeIt);
        if(mSceneToAct.empty()) mSceneMenu->setDisabled(true);
    }

    void setCurrentScene(Canvas * const scene) {
        if(!scene) return setCurrentScene(nullptr, nullptr);
        const auto it = mSceneToAct.find(scene);
        if(it == mSceneToAct.end()) return;
        setCurrentScene(scene, it->second);
    }

    void setCurrentScene(Canvas * const scene, QAction * const act) {
        if(act) {
            act->setChecked(true);
            act->setDisabled(true);
        }
        if(mCurrentScene) {
            const auto currAct = mSceneToAct[mCurrentScene];
            currAct->setChecked(false);
            currAct->setEnabled(true);
        }
        mSceneMenu->setTitle(scene ? scene->getName() : "none");
        mWindow->setCurrentCanvas(scene);
        mCurrentScene = scene;
    }

    Canvas* getCurrentScene() const { return mCurrentScene; }

    Document& mDocument;
    CanvasWindow* const mWindow;
    QMenu * mSceneMenu;
    Canvas * mCurrentScene = nullptr;
    std::map<Canvas*, QAction*> mSceneToAct;
};

CanvasWindowWrapper::CanvasWindowWrapper(Document * const document,
                                         CWWidgetStackLayoutItem* const layItem,
                                         QWidget * const parent) :
    StackWidgetWrapper(
        layItem,
        []() {
            const auto rPtr = new CWWidgetStackLayoutItem;
            return std::unique_ptr<WidgetStackLayoutItem>(rPtr);
        },
        [document](WidgetStackLayoutItem* const layItem,
                   QWidget * const parent) {
            const auto cLayItem = static_cast<CWWidgetStackLayoutItem*>(layItem);
            const auto derived = new CanvasWindowWrapper(document, cLayItem, parent);
            return static_cast<StackWidgetWrapper*>(derived);
        },
        [document](StackWidgetWrapper * toSetup) {
            const auto window = new CanvasWindow(*document, toSetup);
            toSetup->setCentralWidget(window);
            toSetup->setMenuBar(new CanvasWrapperMenuBar(*document, window));
}, parent) {}

void CanvasWindowWrapper::setScene(Canvas * const scene) {
    const auto menu = static_cast<CanvasWrapperMenuBar*>(getMenuBar());
    menu->setCurrentScene(scene);
}

Canvas* CanvasWindowWrapper::getScene() const {
    const auto menu = static_cast<CanvasWrapperMenuBar*>(getMenuBar());
    return menu->getCurrentScene();
}

void CanvasWindowWrapper::saveDataToLayout() const {
    const auto lItem = static_cast<CWWidgetStackLayoutItem*>(getLayoutItem());
    if(!lItem) return;
    const auto sceneWidget = getSceneWidget();
    lItem->setTransform(sceneWidget->getViewTransform());
    lItem->setScene(sceneWidget->getCurrentCanvas());
}

CanvasWindow* CanvasWindowWrapper::getSceneWidget() const {
    return static_cast<CanvasWindow*>(getCentralWidget());
}

void CanvasWindowWrapper::changeEvent(QEvent *e) {
    if(e->type() == QEvent::ParentChange) {
        const auto sceneWidget = getSceneWidget();
        if(sceneWidget) {
            sceneWidget->unblockAutomaticSizeFit();
            sceneWidget->fitCanvasToSize();
        }
    }
    StackWidgetWrapper::changeEvent(e);
}

void CWWidgetStackLayoutItem::clear() {
    setScene(nullptr);
}

void CWWidgetStackLayoutItem::apply(StackWidgetWrapper * const stack) const {
    const auto cwWrapper = static_cast<CanvasWindowWrapper*>(stack);
    cwWrapper->setScene(mScene);
    const auto cw = cwWrapper->getSceneWidget();
    cw->blockAutomaticSizeFit();
    cw->setViewTransform(mTransform);
}

void CWWidgetStackLayoutItem::write(QIODevice * const dst) const {
    const int sceneId = mScene ? mScene->getWriteId() : -1;
    dst->write(rcConstChar(&sceneId), sizeof(int));
}

void CWWidgetStackLayoutItem::read(QIODevice * const src) {
    int sceneId;
    src->read(rcChar(&sceneId), sizeof(int));
    const auto sceneBox = BoundingBox::sGetBoxByReadId(sceneId);
    const auto scene = dynamic_cast<Canvas*>(sceneBox);
    setScene(scene);
}

void CWWidgetStackLayoutItem::setTransform(const QMatrix& transform) {
    mTransform = transform;
}

void CWWidgetStackLayoutItem::setScene(Canvas * const scene) {
    mScene = scene;
}