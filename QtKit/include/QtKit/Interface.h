#pragma once
#include <string>
#include <exception>
#include <functional>
#include <windows.h>
#include <condition_variable>
#include <atomic>
#include <vector>

struct IQtKit;
struct IQmlContext;
struct IUIWindow;
struct IUIItem;
struct IUIRectangle;
struct IUIButton;
struct IUILoader;
struct IUIImage;
struct IUIListView;
struct IUINestedListView;
struct IUIGridView;
struct IUIComboBox;
struct IUIEditComboBox;
struct IUITableView;
struct IUITreeView;
struct IUISlider;
struct IUIProgressBar;
struct IUIRepeater;
struct IUILabel;
struct IUITextField;
struct IUITextArea;
struct IUIProperty;
struct IUITextureView;
struct IImageProviderDelegate;
struct IUIImageProvider;
struct IUIBufferImage;
struct IBufferProvider;
struct IWindowHost;
struct ITransform;
struct IUIOnScreenEditor;
struct IUIMOSource;
struct IUISimbaSource;

extern "C" __declspec(dllexport) IQtKit * createQtKit();
extern "C" __declspec(dllexport) void freeQtKit(IQtKit * instance);

struct RunningTask {
    std::atomic_bool done{ false };
    std::atomic_bool cancelled{ false };
    std::atomic_bool signalled{ false };

    std::condition_variable cond;
    std::mutex mutex;
    void wait(const unsigned interval_ms) {
        std::unique_lock<std::mutex> lock(mutex);
        if (done || signalled) return;
        cond.wait_for(lock, std::chrono::milliseconds(interval_ms));
    }
    void wait() {
        std::unique_lock<std::mutex> lock(mutex);
        if (done || signalled) return;
        cond.wait(lock);
    }
    void signal() {
        std::lock_guard<std::mutex> lock(mutex);
        signalled = true;
        cond.notify_all();
    }

    void cancel() {
        std::lock_guard<std::mutex> lock(mutex);
        cancelled = true;
    }

    void markDone() {
        std::lock_guard<std::mutex> lock(mutex);
        done = true;
    }
    bool isCancelled() {
        std::lock_guard<std::mutex> lock(mutex);
        return cancelled;
    }
};

enum class PreviewAction {
    Load, Play, Pause, Stop, Seek,
};

struct IMOSourceProtocol {
    virtual void load(const char* path) = 0;
    virtual bool hasVideo() = 0;
    virtual bool hasAudio() = 0;
    virtual bool hasImage() = 0;
    virtual void setTextureUpdateAction(std::function<void(void* texture, bool realtimeUpdate, bool droppable)> action) = 0;
    virtual void play(float speed = 1.0f) = 0;
    virtual bool isPlaying() = 0;
    virtual void setPlayCompleteAction(std::function<void()> action) = 0;
    virtual void pause() = 0;
    virtual void stop() = 0;
    virtual void seek(float pos) = 0;
    virtual double getPosition() = 0;
    virtual double getDuration() = 0;
    virtual float getFramerate() = 0;
    virtual void setPositionChangedAction(std::function<void(double pos)> action) = 0;
    virtual void setCompletion(std::function<void(PreviewAction)> completion) = 0;
    virtual void setVolume(float value) = 0;
    virtual float getVolume() const = 0;
    virtual void setMute(bool) = 0;
    virtual bool getMute() const = 0;

    // For 360 navigation
    virtual bool is360Source() const = 0;
    virtual void enable360Navigation(bool enable) = 0;
    virtual void setYaw(float radian) = 0;
    virtual void setRoll(float radian) = 0;
    virtual void setPitch(float radian) = 0;

    virtual void setWindowHandle(void* hWnd) = 0;
    virtual void resetPreview() = 0;

    virtual void updateFrame() = 0;
};

struct ISimbaSourceProtocol {
    virtual void setTextureUpdateAction(std::function<void(void* texture, bool realtimeUpdate, bool droppable)> action) = 0;
    virtual void setPlayingChangedAction(std::function<void()> action) = 0;
    virtual void setPositionChangedAction(std::function<void(double pos)> action) = 0;
    virtual void play(float speed = 1.0f) = 0;
    virtual bool isPlaying() = 0;
    virtual void pause() = 0;
    virtual void stop() = 0;
    virtual void seek(float pos) = 0;
    virtual double getPosition() = 0;
    virtual double getDuration() = 0;
    virtual float getFramerate() = 0;
    virtual void updateFrame() =0 ;
    virtual void setVolume(float value) = 0;
    virtual float getVolume() const = 0;
    virtual void setMute(bool) = 0;
    virtual bool getMute() const = 0;

    // For 360 navigation
    virtual void enable360Navigation(bool enable) = 0;
    virtual void setYaw(float radian) = 0;
    virtual void setRoll(float radian) = 0;
    virtual void setPitch(float radian) = 0;

    virtual void setWindowHandle(void* hWnd) = 0;
    virtual void resetPreview() = 0;
};

// OnScreenEditor
struct IOnScreenEditorDelegateObserver {
    virtual void onBoundingAttributeChanged() = 0;
    virtual void onSelectionChange(int idxGroup, int idxLayer, int idxTitle) = 0;
};

struct BoundingTransform {
    float tx{ 0 };
    float ty{ 0 };
    float sx{ 1 };
    float sy{ 1 };
    float radian{ 0 };
};

struct BoundingQuad {
    float xLeftTop{ 0 };
    float yLeftTop{ 0 };
    float xRightTop{ 1 };
    float yRightTop{ 0 };
    float xRightBottom{ 1 };
    float yRightBottom{ 1 };
    float xLeftBottom{ 0 };
    float yLeftBottom{ 1 };
};

struct WorkingSize {
    float width;
    float height;
};

struct BezierLineSegment {
    float xP0{ 0 };
    float yP0{ 0 };
    float xP1{ 0 };
    float yP1{ 0 };
    float xP2{ 0 };
    float yP2{ 0 };
    float xP3{ 0 };
    float yP3{ 0 };

};

enum GroupType {
    GroupTypeNone,
    GroupTypePip,
    GroupTypeTitle2D,
    GroupTypeLayerTemplate,
    GroupTypeMGT,
    GroupTypeEffect,
};

struct GroupAttribute {
    BoundingQuad boundingBox;
    int trackIndex{ -1 };
    int clipIndex{ -1 };
    GroupType type{ GroupTypeNone };
};

enum LayerType {
    LayerTypeNone,
    LayerTypePip,
    LayerTypeTitle2D,
    LayerTypeTitle3D,
    LayerTypeSubtitle,
    LayerTypeMGT,
    LayerTypeGeometry,
    LayerTypeParticle,
    LayerTypeEffect
};

struct LayerAttribute {
    BoundingQuad boundingBox;
    bool canNoneUniformScale{ true };
    LayerType type{ LayerTypeNone };
    float anchorX{ 0.5f };      // PiP clip-local normalized anchor [0,1], default center
    float anchorY{ 0.5f };
    bool showAnchor{ false };   // PiP only, honors DisplayAnchor flag; read-only (ignored on set)
};

struct TitleAttribute {
    BoundingQuad boundingBox;
    std::string text;
    std::vector<BoundingQuad> quadLines;
};

struct CharAttribute {
    BoundingQuad boundingBox;
    std::string character;
};

struct ITitleEditorProtocol;
struct IPipMaskEditorProtocol;
struct IEffectMaskEditorProtocol;
struct IGeometryEditorProtocol;
struct IKeyframeEditorProtocol;

struct IOnScreenEditorProtocol {
    virtual void setObserver(IOnScreenEditorDelegateObserver* observer) = 0;

    virtual WorkingSize getWorkingSize() = 0;

    virtual GroupAttribute getGroupAttribute() = 0;
    virtual void setGroupAttribute(GroupAttribute attribute) = 0;

    virtual std::vector<LayerAttribute> getLayerAttributes() = 0;
    virtual LayerAttribute getLayerAttribute(int idxLayer) = 0;
    virtual void setLayerAttribute(int idxLayer, LayerAttribute attribute) = 0;

    virtual std::vector<TitleAttribute> getTitleAttributes(int idxLayer) = 0;
    virtual void setTitleAttribute(int idxLayer, int idxTitle, TitleAttribute attribute) = 0;

    virtual std::vector<CharAttribute> getCharAttributes(int idxLayer, int idxTitle) = 0;

    virtual void updateFrame() = 0;

    virtual std::vector<GroupAttribute> getGroupAttributes() = 0;
    virtual void beginEdit(int idxGroup) = 0;
    virtual void endEdit() = 0;

    virtual void beginLayerEdit(int idxLayer) = 0;
    virtual void endLayerEdit() = 0;

    virtual void beginContinuousEdit(int idxLayer) = 0;
    virtual void endContinuousEdit(int idxLayer) = 0;

    virtual ITitleEditorProtocol* getTitleEditor() = 0;
    virtual IPipMaskEditorProtocol * getPipMaskEditor() = 0;
    virtual IEffectMaskEditorProtocol* getEffectMaskEditor() = 0;
    virtual IGeometryEditorProtocol* getGeometryEditor() = 0;
    virtual IKeyframeEditorProtocol* getKeyframeEditor() = 0;
};

enum TitleAlignment {
    TitleAlignmentLeft = 1,
    TitleAlignmentCenter = 4,
    TitleAlignmentRight = 2
};
struct ITitleEditorProtocol {
    virtual void beginTitleEdit(int idxLayer, int idxTitle) = 0;
    virtual void endTitleEdit() = 0;
    virtual void setSelection(int start, int length) = 0;
    virtual void insertText(const char* text) = 0;
    virtual void deleteText() = 0;
    virtual void setFontName(const char* fontName) = 0;
    virtual void setFontSize(float size) = 0;
    virtual void setFontColor(DWORD color) = 0;
    virtual void enableBorder(bool enable) = 0;
    virtual void setBorderColor(DWORD color) = 0;
    virtual void enableShadow(bool enable) = 0;
    virtual void setShadowColor(DWORD color) = 0;
    virtual void enableBold(bool enable) = 0;
    virtual void enableItalic(bool enable) = 0;
    virtual void enableVerticalText(bool enable) = 0;
    virtual void setAlignment(TitleAlignment alignment) = 0;

    virtual std::string getFontName() = 0;
    virtual float getFontSize() = 0;
    virtual DWORD getFontColor() = 0;
    virtual bool isBorderEnabled() = 0;
    virtual DWORD getBorderColor() = 0;
    virtual bool isShadowEnabled() = 0;
    virtual DWORD getShadowColor() = 0;
    virtual bool isBoldEnabled() = 0;
    virtual bool isItalicEnabled() = 0;
    virtual bool isVerticalTextEnabled() = 0;
    virtual TitleAlignment getAlignment() = 0;
};


enum PipMaskType {
    PipMaskTypeUnknown,
    PipMaskTypeNormal,
    PipMaskTypeLinear,
    PipMaskTypeParallel,
    PipMaskTypeVertex
};

struct IPipMaskEditorProtocol {
    virtual void beginMaskEdit() = 0;
    virtual void endMaskEdit() = 0;
    virtual PipMaskType getMaskType() = 0;
    virtual BoundingQuad getMaskBoundingBox() = 0;
    virtual void setMaskBoundingBox(BoundingQuad quad) = 0;
};

struct IEffectMaskEditorProtocol {
    virtual void beginEffectMaskEdit() = 0;
    virtual void endEffectMaskEdit() = 0;
    virtual BoundingQuad getEffectMaskBoundingBox() = 0;
    virtual void setEffectMaskBoundingBox(BoundingQuad quad) = 0;
    virtual bool isEffectMaskEdtiable() = 0;
};

enum GeometryType {
    GeometryTypeUnknown,
    GeometryTypeLine,
    GeometryTypeOthers
};

struct IGeometryEditorProtocol {
    virtual void beginGeometryEdit() = 0;
    virtual void endGeometryEdit() = 0;
    virtual GeometryType getGeometryType() = 0;
    virtual BoundingQuad getGeometryBoundingBox() = 0;
    virtual void setGeometryBoundingBox(BoundingQuad quad) = 0;
    virtual std::vector<std::pair<float, float>> getGeometryControlPoints() = 0;
    virtual void setGeometryControlPoints(std::vector<std::pair<float, float>>) = 0;
};

struct IKeyframeEditorProtocol {
    virtual void beginKeyframeEdit() = 0;
    virtual void endKeyframeEdit() = 0;
    virtual std::vector<BezierLineSegment> getPositionKeyframes() = 0;
};

struct IFontPickerSourceProtocol {
    virtual ~IFontPickerSourceProtocol() = default;
    // PDR -> QtKit callbacks
    // diff: when true, request an id-keyed diff update (preserves scroll
    // position on insert/remove). Use only when the change is structurally
    // small and rows carry stable "id" fields, e.g. favorite/unfavorite.
    virtual void setModelChangedAction(std::function<void(const std::string& jsonArrayRoot, bool diff)> action) = 0;
    virtual void setItemChangedAction(std::function<void(int index, const std::string& itemJson)> action) = 0;
    virtual void setFilterModelChangedAction(std::function<void(const std::string& jsonArrayRoot)> action) = 0;
    // QtKit -> PDR commands
    virtual void initModel() = 0;
    virtual void onFontSelected(int index) = 0;
    virtual void onDownloadFont(int index) = 0;
    virtual void onToggleFavorite(int index, bool bFavorite) = 0;
    virtual void onMenuOpened() = 0;
    virtual void onMenuClosed() = 0;
    virtual void onVisibleRangeChanged(const std::vector<std::string>& visibleFontIds) = 0;
    // filter
    virtual void onFilterSelected(int index) = 0;
    // font size
    virtual std::vector<int> getFontSizes() const = 0;
};

struct IColorPickerSourceProtocol {
    virtual ~IColorPickerSourceProtocol() = default;
    // PDR -> QtKit: register callbacks
    virtual void setColorChangedAction(std::function<void(int argbColor)> action) = 0;
    virtual void setColorCommittedAction(std::function<void(bool isOK, int argbColor)> action) = 0;
    // QtKit -> PDR: command
    virtual void openColorPicker(int initialArgbColor) = 0;
};

// CreateFactory
struct ICreateFactory {
    virtual std::shared_ptr<IMOSourceProtocol> createMOSourceDelegate() = 0;
    virtual std::shared_ptr<ISimbaSourceProtocol> createSimbaSourceDelegate() = 0;
    virtual std::shared_ptr<IOnScreenEditorProtocol> createOnScreenEditorDelegate() = 0;
    virtual std::shared_ptr<IFontPickerSourceProtocol> createFontPickerSourceDelegate() = 0;
    virtual std::shared_ptr<IColorPickerSourceProtocol> createColorPickerSourceDelegate() = 0;
};

enum class QmlThemeMode {
    Dark,
    Light,
    System,
    Auto,
};

struct IEventFilter {
    virtual void addGlobalEventCallback(std::function<bool(MSG*, LRESULT*)> onNativeEvent) = 0;
    virtual void removeGlobalEventCallback() = 0;
    virtual void addWindowEventCallback(HWND hWnd, std::function<bool(MSG*, LRESULT*)> onNativeEvent) = 0;
    virtual void removeWindowEventCallback(HWND hWnd) = 0;
};

struct IQtKit {
    virtual IQmlContext* context() = 0;
    virtual void run(bool async, std::function<void()> contextCreatedAction, std::function<void()> contextRemovedAction) = 0;
    virtual void setAppArugments(int argc, char* argv[]) = 0;
    virtual void setAppName(const char* name) = 0;
    virtual void setImageDir(const char* name, const char* dir) = 0;
    virtual void setRccPath(const char* path) = 0;
    virtual void setDpiScale(float scalefactor) = 0;
    virtual void setQmPath(const char* path) = 0;
    virtual void setFontName(const char* fontname) = 0;
    virtual void setFontSize(int fontsize) = 0;
    virtual void quit() = 0;
    virtual void setUserConfigIniPath(const char* path) = 0;
    virtual void setDontShowAgainIniPath(const char* path) = 0;
    virtual void setDebugOn() = 0;
    virtual void setDebugOutput(std::function<void(const char* msg)> logger) = 0;
    virtual void setAppIconPath(const char* path) = 0;
    virtual void setCreateFactory(ICreateFactory* factory) = 0;
    virtual void enableTheme(QmlThemeMode mode) = 0;
    virtual IEventFilter* getEventFilter() = 0;
    // Opt-in for the accessibility factory that fixes UIA BoundingRectangle for
    // embedded QQuickWindows. Off by default to limit blast radius; the host
    // (e.g. PowerDirector) is expected to enable it from a registry switch.
    // Must be called before run() / runloop().
    virtual void enableEmbeddedWindowAccessibilityFix(bool enabled) = 0;
    // UI action trace -- docs/qtkit-ui-automation-trace.md. 0 (default) is
    // off and zero cost; sink defaults to this->logMessage() once run()
    // starts unless the host calls setUiTraceOutput() first. Level is also
    // readable from HKCU\Software\CyberLink\QtKit\UiTraceLevel at process
    // start. Appended at the end of the vtable, not inserted -- ABI-safe for
    // a new QtKit.dll against an older PDR build (see deploy-risk notes).
    virtual void setUiTraceOutput(std::function<void(const char* line)> sink) = 0;
    virtual void setUiTraceLevel(int level) = 0;
};

struct IQmlContext {
    // Returns false when the entry is already loaded (unload() it first) or when
    // no root object could be created from it.
    virtual bool load(const char* entry, QmlThemeMode mode = QmlThemeMode::Auto) = 0;
    virtual void unload(const char* entry) = 0;
    virtual void clearActions(const char* entry) = 0;
    virtual void addImageProvider(const char* entry, const char* name, IImageProviderDelegate* delegate) = 0;
    virtual void removeImageProvider(const char* entry, const char* name) = 0;

    virtual IUIItem& item(const char* name) = 0;
    virtual IUIRectangle& rectangle(const char* name) = 0;
    virtual IUIButton& button(const char* name) = 0;
    virtual IUILoader& loader(const char* name) = 0;
    virtual IUIImage& image(const char* name) = 0;
    virtual IUIListView& listView(const char* name) = 0;
    virtual IUINestedListView& nestedListView(const char* name) = 0;
    virtual IUIComboBox& comboBox(const char* name) = 0;
    virtual IUIEditComboBox& editComboBox(const char* name) = 0;
    virtual IUIGridView& gridView(const char* name) = 0;
    virtual IUITableView& tableView(const char* name) = 0;
    virtual IUITreeView& treeView(const char* name) = 0;
    virtual IUISlider& slider(const char* name) = 0;
    virtual IUIProgressBar& progressbar(const char* name) = 0;
    virtual IUIRepeater& repeater(const char* name) = 0;
    virtual IUIWindow& window(const char* name) = 0;
    virtual IUILabel& label(const char* name) = 0;
    virtual IUITextField& textfield(const char* name) = 0;
    virtual IUITextArea& textarea(const char* name) = 0;
    virtual IUIProperty& property(const char* name) = 0;
    virtual IUITextureView& textureview(const char* name) = 0;
    virtual IUIBufferImage& bufferImage(const char* name) = 0;
    virtual IBufferProvider& bufferProvider(const char* name) = 0;
    virtual IWindowHost& windowHost(const char* name) = 0;
    virtual IUIImageProvider& imageProvider(const char* name) = 0;
    virtual IUIOnScreenEditor& onScreenEditor(const char* name) = 0;
    virtual IUIMOSource& moSource(const char* name) = 0;
    virtual IUISimbaSource& simbaSource(const char* name) = 0;

    virtual void setBindAction(const char* name, std::function<void()> action) = 0;
    virtual void setUnbindAction(const char* name, std::function<void()> action) = 0;

    virtual std::shared_ptr<RunningTask> runOnQtThread(std::function<void()> call) = 0;
    virtual IQmlContext& runOnQtThreadSync(std::function<void()> call) = 0;
    virtual bool isQtThread() = 0;

    virtual void postCall(std::function<void()> call) = 0;
    virtual void delayCall(std::function<void()> call, unsigned interval_ms) = 0;
    virtual void dumpQtThreadTask() = 0;
    virtual bool isObjectBound(const char* name) = 0;
};

struct IUIWindow {
    virtual void title(const char* v) = 0;
    virtual std::string title() = 0;
    virtual void* window() = 0;
    virtual IUIWindow& present(const char* qmlName) = 0;
    virtual void dismiss() = 0;
    virtual void visible(bool v) = 0;
    virtual bool visible() = 0;

    virtual IUIWindow& setClosedAction(std::function<void()> action) = 0;
    virtual IUIWindow& setJsonAction(std::function<void(std::string, std::string)> action) = 0;
    virtual IUIWindow& setVisibleChangedAction(std::function<void()> action) = 0;
    virtual void notifyJsonEvent(const char* key, const char* json) = 0;
    virtual IUIWindow& setMouseMoveAction(std::function<bool(float x, float y)> action) = 0;
    virtual IUIWindow& setMousePressedAction(std::function<bool(float x, float y, int button)> action) = 0;
    virtual IUIWindow& setMouseReleasedAction(std::function<bool(float x, float y, int button)> action) = 0;
    virtual IUIWindow& setMouseClickedAction(std::function<bool(float x, float y, int button)> action) = 0;
    virtual IUIWindow& setMouseDoubleClickedAction(std::function<bool(float x, float y, int button)> action) = 0;
    virtual IUIWindow& setKeyPressedAction(std::function<bool(int key, int modifiers, std::string character)> action) = 0;
    virtual IUIWindow& setKeyReleasedAction(std::function<bool(int key, int modifiers, std::string character)> action) = 0;

    virtual void setParent(void* parent) = 0; // parent: HWND; pass nullptr to remove parent-child relationship before destroying the parent window
    virtual void setPosition(int x, int y, int width, int height) = 0; // pixel-based, not point-based
};

struct IUIItem {
    virtual void visible(bool v) = 0;
    virtual bool visible() = 0;
    virtual void enabled(bool v) = 0;
    virtual bool enabled() = 0;
    virtual void clip(bool v) = 0;
    virtual bool clip() = 0;
    virtual void focus(bool v) = 0;
    virtual bool focus() = 0;
    virtual void state(const char* v) = 0;
    virtual std::string state() = 0;

    virtual void* window() = 0;

    virtual IUIItem& setSizeChangedAction(std::function<void(float width, float height)> action) = 0;
    virtual IUIItem& setVisibleChangedAction(std::function<void()> action) = 0;
    virtual IUIItem& setJsonAction(std::function<void(std::string, std::string)> action) = 0;
    virtual IUIItem& present(const char* qmlName) = 0;
    virtual void dismiss() = 0;
    virtual void notifyJsonEvent(const char* key, const char* json) = 0;
    virtual IUIItem& setMouseMoveAction(std::function<bool(float x, float y)> action) = 0;
    virtual IUIItem& setMousePressedAction(std::function<bool(float x, float y, int button)> action) = 0;
    virtual IUIItem& setMouseReleasedAction(std::function<bool(float x, float y, int button)> action) = 0;
    virtual IUIItem& setMouseClickedAction(std::function<bool(float x, float y, int button)> action) = 0;
    virtual IUIItem& setMouseDoubleClickedAction(std::function<bool(float x, float y, int button)> action) = 0;
    virtual IUIItem& setKeyPressedAction(std::function<bool(int key, int modifiers, std::string character)> action) = 0;
    virtual IUIItem& setKeyReleasedAction(std::function<bool(int key, int modifiers, std::string character)> action) = 0;
};

struct IUIRectangle {
    virtual void color(const char* v) = 0;
    virtual std::string color() = 0;
};

struct IUIButton {
    virtual void text(const char* v) = 0;
    virtual std::string text() = 0;
    virtual void checked(bool v) = 0;
    virtual bool checked() = 0;

    virtual IUIButton& setClickedAction(std::function<void()> action) = 0;
    virtual IUIButton& setPressedAction(std::function<void()> action) = 0;
    virtual IUIButton& setReleasedAction(std::function<void()> action) = 0;
    virtual IUIButton& setCheckedChangedAction(std::function<void(bool)> action) = 0;
};

struct IUILoader {
    virtual void source(const char* v) = 0;
    virtual std::string source() = 0;
    virtual IUILoader& setLoadedAction(std::function<void()> action) = 0;
};

struct IUIImage {
    virtual void source(const char* path) = 0;
    virtual void image(const char* path) = 0;
    virtual void image(void* hbitmap) = 0; // HBITMAP
};

struct IUIListView {
    virtual int count() = 0;
    virtual IUIListView& reset() = 0;
    virtual IUIListView& reset(const char* json) = 0;
    virtual IUIListView& append(const char* json) = 0;
    virtual IUIListView& insert(int index, const char* json) = 0;
    virtual IUIListView& remove(int index, int count) = 0;
    virtual IUIListView& deleteLast() = 0;
    virtual IUIListView& update(const char* json) = 0;
    virtual std::string get(int index) = 0;
    virtual IUIListView& set(int index, const char* json) = 0;
    virtual IUIListView& setSelected(int index, bool selected) = 0;

    virtual IUIListView& setItemSelectedAction(std::function<void(int)> action) = 0;
    virtual IUIListView& setItemDeselectedAction(std::function<void(int)> action) = 0;
    virtual IUIListView& setItemDataChangedAction(std::function<void(int)> action) = 0;
    virtual IUIListView& addButtonClickedAction(std::function<void(int, std::string)> action,
        std::string buttonId) = 0;
    virtual IUIListView& removeButtonClickedAction(std::string buttonId) = 0;
    virtual IUIListView& addButtonRClickedAction(std::function<void(int, std::string)> action,
        std::string buttonId) = 0;
    virtual IUIListView& removeButtonRClickedAction(std::string buttonId) = 0;
};

// Nested-list-view interface: a sibling of IUIListView in which every flat
// `int index` is replaced by an `indexPath` (the NestedJsonListModel path).
// Path = std::vector<std::string>: even positions are row indices (numeric
// strings), odd positions are field names. Even-length paths address a sub-model
// node ([] = root); odd-length paths address a specific row (last element = row).
struct IUINestedListView {
    using Path = std::vector<std::string>;

    virtual int count(const Path& nodePath) = 0;                    // rows in sub-model @ node
    virtual IUINestedListView& reset() = 0;                         // clear root
    virtual IUINestedListView& reset(const char* json) = 0;         // replace whole tree (root)
    virtual IUINestedListView& append(const Path& nodePath, const char* json) = 0;
    virtual IUINestedListView& insert(const Path& rowPath, const char* json) = 0;  // before rowPath
    virtual IUINestedListView& remove(const Path& rowPath, int count) = 0;         // count rows @ rowPath
    virtual IUINestedListView& deleteLast(const Path& nodePath) = 0;
    virtual IUINestedListView& update(const Path& nodePath, const char* json) = 0;
    virtual std::string get(const Path& rowPath) = 0;
    virtual IUINestedListView& set(const Path& rowPath, const char* json) = 0;
    // Patch a single field of one row (odd-length rowPath). valueJson is a JSON
    // fragment ("false", "3", "\"Music 1\""). Emits a single-role dataChanged, so a
    // scalar patch does not re-apply the row's array sub-models (e.g. a clip lane).
    virtual IUINestedListView& setField(const Path& rowPath, const char* key, const char* valueJson) = 0;
    // Patch ONE field on MANY rows of one sub-model (even-length nodePath) with ONE
    // dataChanged spanning min..max of `rows`. A bulk patch (select-all writing
    // "selected" on every clip) pays the QML consumers per SIGNAL, so per-row
    // setField calls cost seconds where one spanning signal costs milliseconds.
    // ABI note: adding this member changes the vtable - QtKit.dll and its host
    // must be rebuilt together (a stale pair miscalls every later slot).
    virtual IUINestedListView& setFieldMany(const Path& nodePath, const std::vector<int>& rows,
        const char* key, const char* valueJson) = 0;
    virtual IUINestedListView& setSelected(const Path& rowPath, bool selected) = 0;

    virtual IUINestedListView& setItemSelectedAction(std::function<void(Path)> action) = 0;
    virtual IUINestedListView& setItemDeselectedAction(std::function<void(Path)> action) = 0;
    virtual IUINestedListView& setItemDataChangedAction(std::function<void(Path)> action) = 0;
    virtual IUINestedListView& addButtonClickedAction(std::function<void(Path, std::string)> action,
        std::string buttonId) = 0;
    virtual IUINestedListView& removeButtonClickedAction(std::string buttonId) = 0;
    virtual IUINestedListView& addButtonRClickedAction(std::function<void(Path, std::string)> action,
        std::string buttonId) = 0;
    virtual IUINestedListView& removeButtonRClickedAction(std::string buttonId) = 0;
};

struct IUIGridView {
    virtual int count() = 0;
    virtual IUIGridView& reset() = 0;
    virtual IUIGridView& reset(const char* json) = 0;
    virtual IUIGridView& append(const char* json) = 0;
    virtual IUIGridView& insert(int index, const char* json) = 0;
    virtual IUIGridView& remove(int index, int count) = 0;
    virtual IUIGridView& deleteLast() = 0;
    virtual IUIGridView& update(const char* json) = 0;
    virtual std::string get(int index) = 0;
    virtual IUIGridView& set(int index, const char* json) = 0;
    virtual IUIGridView& setSelected(int index, bool selected) = 0;

    virtual IUIGridView& setItemSelectedAction(std::function<void(int)> action) = 0;
    virtual IUIGridView& setItemDeselectedAction(std::function<void(int)> action) = 0;
    virtual IUIGridView& setItemDataChangedAction(std::function<void(int)> action) = 0;
    virtual IUIGridView& addButtonClickedAction(std::function<void(int, std::string)> action,
        std::string buttonId) = 0;
    virtual IUIGridView& removeButtonClickedAction(std::string buttonId) = 0;
    virtual IUIGridView& addButtonRClickedAction(std::function<void(int, std::string)> action,
        std::string buttonId) = 0;
    virtual IUIGridView& removeButtonRClickedAction(std::string buttonId) = 0;
};

struct IUIComboBox {
    virtual int count() = 0;
    virtual IUIComboBox& reset() = 0;
    virtual IUIComboBox& reset(const char* json) = 0;
    virtual IUIComboBox& append(const char* json) = 0;
    virtual IUIComboBox& insert(int index, const char* json) = 0;
    virtual IUIComboBox& remove(int index, int count) = 0;
    virtual IUIComboBox& deleteLast() = 0;
    virtual IUIComboBox& update(const char* json) = 0;
    virtual std::string get(int index) = 0;
    virtual IUIComboBox& set(int index, const char* json) = 0;
    virtual IUIComboBox& setItemSelectedAction(std::function<void(int)> action) = 0;
    virtual IUIComboBox& setItemDeselectedAction(std::function<void(int)> action) = 0;
    virtual IUIComboBox& setItemDataChangedAction(std::function<void(int)> action) = 0;
    // virtual IUIComboBox& setCurrentIndexChangedAction(std::function<void(int)> action) = 0;
    virtual IUIComboBox& addButtonClickedAction(std::function<void(int, std::string)> action, std::string buttonId) = 0;
    virtual IUIComboBox& removeButtonClickedAction(std::string buttonId) = 0;
    virtual IUIComboBox& addButtonRClickedAction(std::function<void(int, std::string)> action, std::string buttonId) = 0;
    virtual IUIComboBox& removeButtonRClickedAction(std::string buttonId) = 0;
    virtual IUIComboBox& setCurrentIndex(int index) = 0;
};

struct IUIEditComboBox {
    virtual IUIComboBox& getComboBox() = 0;
    virtual IUITextField& getTextField() = 0;
};

struct IUITableView {
    virtual int count() = 0;
    virtual IUITableView& reset() = 0;
    virtual IUITableView& reset(const char* json) = 0;
    virtual IUITableView& append(const char* json) = 0;
    virtual IUITableView& insert(int index, const char* json) = 0;
    virtual IUITableView& remove(int index, int count) = 0;
    virtual IUITableView& deleteLast() = 0;
    virtual IUITableView& update(const char* json) = 0;
    virtual std::string get(int index) = 0;
    virtual IUITableView& set(int index, const char* json) = 0;
    virtual IUITableView& setSelected(int index, bool selected) = 0;

    virtual IUITableView& setCellSelectedAction(std::function<void(int, int)> action) = 0;
    virtual IUITableView& setCellDeselectedAction(std::function<void(int, int)> action) = 0;
    virtual IUITableView& setCellDataChangedAction(std::function<void(int, int)> action) = 0;

    virtual IUITableView& setRowSelectedAction(std::function<void(int)> action) = 0;
    virtual IUITableView& setRowDeselectedAction(std::function<void(int)> action) = 0;
    virtual IUITableView& setRowDataChangedAction(std::function<void(int)> action) = 0;

    virtual IUITableView& setColumnSelectedAction(std::function<void(int)> action) = 0;
    virtual IUITableView& setColumnDeselectedAction(std::function<void(int)> action) = 0;
    virtual IUITableView& setColumnDataChangedAction(std::function<void(int)> action) = 0;

    virtual IUITableView& addButtonClickedAction(std::function<void(int, std::string)> action,
        std::string buttonId) = 0;
    virtual IUITableView& removeButtonClickedAction(std::string buttonId) = 0;
    virtual IUITableView& addButtonRClickedAction(std::function<void(int, std::string)> action,
        std::string buttonId) = 0;
    virtual IUITableView& removeButtonRClickedAction(std::string buttonId) = 0;

    virtual int countColumnHeader() = 0;
    virtual IUITableView& resetColumnHeader() = 0;
    virtual IUITableView& resetColumnHeader(const char* json) = 0;
    virtual IUITableView& appendColumnHeader(const char* json) = 0;
    virtual IUITableView& setColumnHeaderSelectedAction(std::function<void(int)> action) = 0;
    virtual IUITableView& setColumnHeaderDeselectedAction(std::function<void(int)> action) = 0;
    virtual IUITableView& setColumnHeaderDataChangedAction(std::function<void(int)> action) = 0;
};

struct IUITreeView {
    virtual void json(const char* json) = 0;

    // Model access
    virtual void* model() = 0;              // Returns QAbstractItemModel*
    virtual void setModel(void* model) = 0; // Sets external QAbstractItemModel*

    // Bulk data API (mirrors IUITableView for consistency)
    virtual int count() = 0;                                        // total node count
    virtual IUITreeView& reset() = 0;                               // clear all nodes
    virtual IUITreeView& reset(const char* json) = 0;               // clear + load from JSON tree
    virtual IUITreeView& update(const char* json) = 0;              // alias for reset(json)
    virtual std::string  get(int nodeId) = 0;                       // get node data as JSON
    virtual IUITreeView& set(int nodeId, const char* json) = 0;     // update a node's data
    virtual IUITreeView& remove(int nodeId) = 0;                    // chainable removeNode

    // Built-in tree node manipulation (operates on internal JsonTreeModel)
    //   nodeJson: UTF-8 JSON object, e.g. {"display":"Blur","icon":"image://Media/fx.svg"}
    //   parentId: -1 = root level; >= 0 = node ID returned by a previous addNode() call
    //   Returns: a unique node ID that can be used as parentId for child nodes
    virtual int  addNode(const char* nodeJson, int parentId = -1) = 0;
    virtual void removeNode(int nodeId) = 0;
    virtual void clearNodes() = 0;
    virtual void setNodeProperty(int nodeId, const char* key, const char* value) = 0;

    // Expansion control
    virtual IUITreeView& expand(int row) = 0;
    virtual IUITreeView& collapse(int row) = 0;
    virtual IUITreeView& expandAll() = 0;
    virtual IUITreeView& collapseAll() = 0;
    virtual bool isExpanded(int row) = 0;

    // Selection control
    virtual IUITreeView& setSelected(int row, bool selected) = 0;
    virtual int selectedRow() = 0;

    // Event callbacks
    virtual IUITreeView& setItemSelectedAction(std::function<void(int)> action) = 0;
    virtual IUITreeView& setItemDeselectedAction(std::function<void(int)> action) = 0;
    virtual IUITreeView& setItemExpandedAction(std::function<void(int, bool)> action) = 0;
    virtual IUITreeView& setItemDoubleClickedAction(std::function<void(int)> action) = 0;
    virtual IUITreeView& setItemRightClickedAction(std::function<void(int, int, int)> action) = 0;

    // Button actions (for custom buttons in tree nodes)
    virtual IUITreeView& addButtonClickedAction(std::function<void(int, std::string)> action, std::string buttonId) = 0;
    virtual IUITreeView& removeButtonClickedAction(std::string buttonId) = 0;
};

struct IUILabel {
    virtual void text(const char* v) = 0;
    virtual std::string text() = 0;
    virtual IUILabel& setClickedAction(std::function<void()> action) = 0;
};

struct IUISlider {
    virtual void value(float v) = 0;
    virtual float value() = 0;

    virtual IUISlider& setValueChangedAction(std::function<void(float)> action) = 0;
    virtual IUISlider& setSeekedAction(std::function<void(float)> action) = 0;
};

struct IUIProgressBar {
    virtual void value(float v) = 0;
    virtual float value() = 0;

    virtual IUIProgressBar& setValueChangedAction(std::function<void(float)> action) = 0;
};

struct IUIRepeater {
    virtual int count() = 0;
    virtual IUIRepeater& reset() = 0;
    virtual IUIRepeater& reset(const char* json) = 0;
    virtual IUIRepeater& append(const char* json) = 0;
    virtual IUIRepeater& insert(int index, const char* json) = 0;
    virtual IUIRepeater& remove(int index, int count) = 0;
    virtual IUIRepeater& deleteLast() = 0;
    virtual IUIRepeater& update(const char* json) = 0;
    virtual std::string get(int index) = 0;
    virtual IUIRepeater& set(int index, const char* json) = 0;

    virtual IUIRepeater& setItemSelectedAction(std::function<void(int)> action) = 0;
    virtual IUIRepeater& setItemDeselectedAction(std::function<void(int)> action) = 0;
    virtual void selectItem(int index) = 0;
    virtual void deselectItem(int index) = 0;
};

struct IUITextField {
    virtual IUITextField& setEditingFinishedAction(std::function<void()> action) = 0;
    virtual IUITextField& setTextChangedAction(std::function<void()> action) = 0;
    virtual void text(const char* v) = 0;
    virtual std::string text() = 0;
};

struct IUITextArea {
    virtual IUITextArea& setEditingFinishedAction(std::function<void()> action) = 0;
    virtual IUITextArea& setTextChangedAction(std::function<void()> action) = 0;
    virtual void text(const char* v) = 0;
    virtual std::string text() = 0;
};

struct IUIProperty {
    virtual IUIProperty& setPropertyChangedAction(const char* property, std::function<void()> action) = 0;
    virtual void property(const char* key, const char* v) = 0;
    virtual void property(const char* key, const int v) = 0;
    virtual void property(const char* key, const unsigned int v) = 0;
    virtual void property(const char* key, const long long v) = 0;
    virtual void property(const char* key, const unsigned long long v) = 0;
    virtual void property(const char* key, const float v) = 0;
    virtual void property(const char* key, const bool v) = 0;

    virtual std::string propertyString(const char* key) = 0;
    virtual int propertyInt(const char* key) = 0;
    virtual int propertyUInt(const char* key) = 0;
    virtual long long propertyLonglong(const char* key) = 0;
    virtual unsigned long long propertyULonglong(const char* key) = 0;
    virtual float propertyFloat(const char* key) = 0;
    virtual bool propertyBool(const char* key) = 0;

    virtual void notifyJsonEvent(const char* key, const char* json) = 0;
    virtual IUIProperty& setJsonAction(std::function<void(std::string, std::string)> action) = 0;
};

struct IUITextureView {
    virtual void texture(void* texture) = 0;
    virtual void texture(void* texture, bool realtimeUpdate, bool droppable) = 0;
    virtual void source(const char* v) = 0;
};

struct IImageProviderDelegate {
    virtual void load(const char* name, const char* json) = 0;
    virtual void loadWebp(const char* name, const char* buffer, size_t size) = 0;
};

enum class UIImageFormat {
    BGR888,
    RGBA8888,
    ARGB8888,
};

struct IUIImageProvider {
    virtual void image(const char* name, const unsigned char* buffer, int width, int height, UIImageFormat format, bool upSideDown, bool rgbSwap) = 0;
    virtual void placeholder(const char* name, const char* placeholder) = 0;
    virtual void animatedImage(const char* name,
        const unsigned char** buffers,
        size_t numberBuffers,
        int frameDuration_ms,
        int width,
        int height,
        UIImageFormat format,
        bool upSideDown,
        bool rgbSwap) = 0;
};

struct IUIBufferImage {
    virtual void image(const unsigned char* buffer, int width, int height, UIImageFormat format, bool upSideDown, bool rgbSwap) = 0;
};

struct IBufferProvider {
    // Callable from any thread; buffer is deep-copied before return.
    virtual void setBuffer(const char* bufferKey, const unsigned char* buffer,
        int width, int height, UIImageFormat format, bool upSideDown, bool rgbSwap) = 0;
    virtual void removeBuffer(const char* bufferKey) = 0;
    virtual void clear() = 0;
};

struct IWindowHost {
    virtual void window(void* hWnd) = 0;
};

struct IUIOnScreenEditor {
    virtual std::shared_ptr<IOnScreenEditorProtocol> getDelegate() = 0;
};

struct IUIMOSource {
    virtual std::shared_ptr<IMOSourceProtocol> getPlayerDelegate() = 0;
    virtual void setSource(const char* path) = 0;
    virtual void setPosition(float value) = 0;
    virtual float position() const = 0;
    virtual void setActive(bool value) = 0;
    virtual void play() = 0;
    virtual void pause() = 0;
    virtual void stop() = 0;
    virtual bool playing() const = 0;
};

struct IUISimbaSource {
    virtual std::shared_ptr<ISimbaSourceProtocol> getPlayerDelegate() = 0;
    virtual void setPosition(float value) = 0;
    virtual float position() const = 0;
    virtual void setActive(bool value) = 0;
    virtual void play() = 0;
    virtual void pause() = 0;
    virtual void stop() = 0;
    virtual bool playing() const = 0;
};

// exceptions //
class TextureSharingError: public std::exception {
public:
    TextureSharingError() {}

    const char* what() const noexcept override {
        return msg;
    };
    const char* msg{ "[QtKit] Texture sharing error under renderless mode" };
};

class QmlObjectNoBoundError : public std::exception {
public:
    QmlObjectNoBoundError(const char* ms) : msg(ms) {}

    const char* what() const noexcept override {
        return msg.c_str();
    };
    std::string msg{ "" };
};
