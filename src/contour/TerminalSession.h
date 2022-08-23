/**
 * This file is part of the "contour" project
 *   Copyright (c) 2019-2021 Christian Parpart <christian@parpart.family>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include <contour/Audio.h>
#include <contour/Config.h>

#include <terminal/Terminal.h>

#include <terminal_renderer/Renderer.h>

#include <crispy/point.h>

#include <fmt/format.h>

#include <QtCore/QAbstractItemModel>
#include <QtCore/QFileSystemWatcher>

#include <functional>
#include <thread>

namespace contour
{

namespace display
{
    class TerminalWidget;
}

class ContourGuiApp;

/**
 * A set of user-facing activities that are guarded behind a permission-check wall.
 */
enum class GuardedRole
{
    ChangeFont,
    CaptureBuffer,
};

/**
 * Trivial cache to remember the interactive choice when the user has to be asked
 * and the user decided to permenently decide for the current session.
 */
using PermissionCache = std::map<GuardedRole, bool>;

/**
 * Manages a single terminal session (Client, Terminal, Display)
 *
 * This class is designed to be working in:
 * - graphical displays (OpenGL, software rasterized)
 * - text based displays (think of TMUX client)
 * - headless-mode (think of TMUX server)
 */
class TerminalSession: public QAbstractItemModel, public terminal::Terminal::Events
{
    Q_OBJECT
    Q_PROPERTY(int id READ id)
    Q_PROPERTY(int pageLineCount READ pageLineCount NOTIFY lineCountChanged)
    Q_PROPERTY(int historyLineCount READ historyLineCount NOTIFY historyLineCountChanged)
    Q_PROPERTY(int scrollOffset READ scrollOffset WRITE setScrollOffset NOTIFY scrollOffsetChanged)
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    // Q_PROPERTY(QString profileName READ profileName NOTIFY profileNameChanged)

  public:
    // {{{ Model property helper
    QString title() const { return QString::fromStdString(terminal().windowTitle()); }
    void setTitle(QString const& value) { terminal().setWindowTitle(value.toStdString()); }

    int pageLineCount() const noexcept { return unbox<int>(terminal_.pageSize().lines); }
    int historyLineCount() const noexcept { return unbox<int>(terminal_.currentScreen().historyLineCount()); }

    int scrollOffset() const noexcept { return unbox<int>(terminal().viewport().scrollOffset()); }
    void setScrollOffset(int value)
    {
        terminal().viewport().scrollTo(terminal::ScrollOffset::cast_from(value));
    }

    void onScrollOffsetChanged(terminal::ScrollOffset value) override
    {
        emit scrollOffsetChanged(unbox<int>(value));
    }
    // }}}

    // {{{ QAbstractItemModel overrides
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    // }}}

    /**
     * Constructs a single terminal session.
     *
     * @param _pty a PTY object (can be process, networked, mockup, ...)
     * @param _display fronend display to render the terminal.
     */
    TerminalSession(std::unique_ptr<terminal::Pty> _pty, ContourGuiApp& _app);
    ~TerminalSession() override;

    int id() const noexcept { return id_; }

    /// Starts the VT background thread.
    void start();

    /// Initiates termination of this session, regardless of the underlying terminal state.
    void terminate();

    config::Config const& config() const noexcept { return config_; }
    config::TerminalProfile const& profile() const noexcept { return profile_; }

    double contentScale() const noexcept { return contentScale_; }
    void setContentScale(double value) noexcept { contentScale_ = value; }

    terminal::Pty& pty() noexcept { return terminal_.device(); }
    terminal::Terminal& terminal() noexcept { return terminal_; }
    terminal::Terminal const& terminal() const noexcept { return terminal_; }
    terminal::ScreenType currentScreenType() const noexcept { return currentScreenType_; }

    display::TerminalWidget* display() noexcept { return display_; }
    display::TerminalWidget const* display() const noexcept { return display_; }

    void attachDisplay(display::TerminalWidget& display);

    Q_INVOKABLE void applyPendingFontChange(bool answer, bool remember);
    Q_INVOKABLE void executePendingBufferCapture(bool answer, bool remember);

    // Terminal::Events
    //
    void requestCaptureBuffer(terminal::LineCount lineCount, bool logical) override;
    void bell() override;
    void bufferChanged(terminal::ScreenType) override;
    void renderBufferUpdated() override;
    void screenUpdated() override;
    terminal::FontDef getFontDef() override;
    void setFontDef(terminal::FontDef const& _fontSpec) override;
    void copyToClipboard(std::string_view _data) override;
    void inspect() override;
    void notify(std::string_view _title, std::string_view _body) override;
    void onClosed() override;
    void pasteFromClipboard(unsigned count) override;
    void onSelectionCompleted() override;
    void requestWindowResize(terminal::LineCount, terminal::ColumnCount) override;
    void requestWindowResize(terminal::Width, terminal::Height) override;
    void setWindowTitle(std::string_view _title) override;
    void setTerminalProfile(std::string const& _configProfileName) override;
    void discardImage(terminal::Image const&) override;
    void inputModeChanged(terminal::ViMode mode) override;
    void updateHighlights() override;
    void playSound(terminal::Sequence::Parameters const& params_) override;
    void cursorPositionChanged() override;

    // Input Events
    using Timestamp = std::chrono::steady_clock::time_point;
    void sendKeyPressEvent(terminal::Key _key, terminal::Modifier _modifier, Timestamp _now);
    void sendCharPressEvent(char32_t _value, terminal::Modifier _modifier, Timestamp _now);

    void sendMousePressEvent(terminal::Modifier _modifier,
                             terminal::MouseButton _button,
                             terminal::PixelCoordinate _pixelPosition,
                             Timestamp _now);
    void sendMouseMoveEvent(terminal::Modifier _modifier,
                            terminal::CellLocation _pos,
                            terminal::PixelCoordinate _pixelPosition,
                            Timestamp _now);
    void sendMouseReleaseEvent(terminal::Modifier _modifier,
                               terminal::MouseButton _button,
                               terminal::PixelCoordinate _pixelPosition,
                               Timestamp _now);

    void sendFocusInEvent();
    void sendFocusOutEvent();

    // Actions
    bool operator()(actions::CancelSelection);
    bool operator()(actions::ChangeProfile const&);
    bool operator()(actions::ClearHistoryAndReset);
    bool operator()(actions::CopyPreviousMarkRange);
    bool operator()(actions::CopySelection);
    bool operator()(actions::CreateDebugDump);
    bool operator()(actions::DecreaseFontSize);
    bool operator()(actions::DecreaseOpacity);
    bool operator()(actions::FollowHyperlink);
    bool operator()(actions::FocusNextSearchMatch);
    bool operator()(actions::FocusPreviousSearchMatch);
    bool operator()(actions::IncreaseFontSize);
    bool operator()(actions::IncreaseOpacity);
    bool operator()(actions::NewTerminal const&);
    bool operator()(actions::NoSearchHighlight);
    bool operator()(actions::OpenConfiguration);
    bool operator()(actions::OpenFileManager);
    bool operator()(actions::PasteClipboard);
    bool operator()(actions::PasteSelection);
    bool operator()(actions::Quit);
    bool operator()(actions::ReloadConfig const&);
    bool operator()(actions::ResetConfig);
    bool operator()(actions::ResetFontSize);
    bool operator()(actions::ScreenshotVT);
    bool operator()(actions::ScrollDown);
    bool operator()(actions::ScrollMarkDown);
    bool operator()(actions::ScrollMarkUp);
    bool operator()(actions::ScrollOneDown);
    bool operator()(actions::ScrollOneUp);
    bool operator()(actions::ScrollPageDown);
    bool operator()(actions::ScrollPageUp);
    bool operator()(actions::ScrollToBottom);
    bool operator()(actions::ScrollToTop);
    bool operator()(actions::ScrollUp);
    bool operator()(actions::SearchReverse);
    bool operator()(actions::SendChars const& _event);
    bool operator()(actions::ToggleAllKeyMaps);
    bool operator()(actions::ToggleFullscreen);
    bool operator()(actions::ToggleInputProtection);
    bool operator()(actions::ToggleStatusLine);
    bool operator()(actions::ToggleTitleBar);
    bool operator()(actions::ViNormalMode);
    bool operator()(actions::WriteScreen const& _event);

    void scheduleRedraw();

    ContourGuiApp& app() noexcept { return app_; }

    std::chrono::steady_clock::time_point startTime() const noexcept { return startTime_; }

    float uptime() const noexcept
    {
        using namespace std::chrono;
        auto const now = steady_clock::now();
        auto const uptimeMsecs = duration_cast<milliseconds>(now - startTime_).count();
        auto const uptimeSecs = static_cast<float>(uptimeMsecs) / 1000.0f;
        return uptimeSecs;
    }

    void requestPermission(config::Permission _allowedByConfig, GuardedRole role);
    void executeRole(GuardedRole role, bool allow, bool remember);

  signals:
    void sessionClosed(TerminalSession&);
    void profileNameChanged(QString newValue);
    void lineCountChanged(int newValue);
    void historyLineCountChanged(int newValue);
    void scrollOffsetChanged(int newValue);
    void titleChanged(QString const& value);
    void onBell();
    void requestPermissionForFontChange();
    void requestPermissionForBufferCapture();
    void showNotification(QString const& title, QString const& content);

  public slots:
    void onConfigReload();
    void onHighlightUpdate();
    void configureDisplay();

  private:
    // helpers
    bool reloadConfig(config::Config _newConfig, std::string const& _profileName);
    int executeAllActions(std::vector<actions::Action> const& _actions);
    bool executeAction(actions::Action const& _action);
    void spawnNewTerminal(std::string const& _profileName);
    void activateProfile(std::string const& _newProfileName);
    bool reloadConfigWithProfile(std::string const& _profileName);
    bool resetConfig();
    void followHyperlink(terminal::HyperlinkInfo const& _hyperlink);
    void setFontSize(text::font_size _size);
    void setDefaultCursor();
    void configureTerminal();
    void configureCursor(config::CursorConfig const& cursorConfig);
    uint8_t matchModeFlags() const;
    void flushInput();
    void mainLoop();

    // private data
    //
    int id_;
    std::chrono::steady_clock::time_point startTime_;
    config::Config config_;
    std::string profileName_;
    config::TerminalProfile profile_;
    double contentScale_ = 1.0;
    ContourGuiApp& app_;

    terminal::Terminal terminal_;
    bool terminatedAndWaitingForKeyPress_ = false;
    display::TerminalWidget* display_ = nullptr;

    std::unique_ptr<QFileSystemWatcher> configFileChangeWatcher_;

    bool terminating_ = false;
    std::thread::id mainLoopThreadID_ {};
    std::unique_ptr<std::thread> screenUpdateThread_;

    // state vars
    //
    terminal::ScreenType currentScreenType_ = terminal::ScreenType::Primary;
    terminal::CellLocation currentMousePosition_ = terminal::CellLocation {};
    bool allowKeyMappings_ = true;
    Audio audio;
    std::vector<int> musicalNotesBuffer_;

    terminal::LineCount lastHistoryLineCount_;

    struct CaptureBufferRequest
    {
        terminal::LineCount lines;
        bool logical;
    };
    std::optional<CaptureBufferRequest> pendingBufferCapture_;
    std::optional<terminal::FontDef> pendingFontChange_;
    PermissionCache rememberedPermissions_;
};

} // namespace contour

Q_DECLARE_INTERFACE(contour::TerminalSession, "org.contour.TerminalSession")

namespace fmt
{

template <>
struct formatter<contour::GuardedRole>
{
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(contour::GuardedRole value, FormatContext& ctx)
    {
        switch (value)
        {
            case contour::GuardedRole::ChangeFont: return fmt::format_to(ctx.out(), "Change Font");
            case contour::GuardedRole::CaptureBuffer: return fmt::format_to(ctx.out(), "Capture Buffer");
        }
        crispy::unreachable();
    }
};

} // namespace fmt
