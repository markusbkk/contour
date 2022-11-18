/**
 * This file is part of the "libterminal" project
 *   Copyright (c) 2019-2022 Christian Parpart <christian@parpart.family>
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
#include <terminal/Parser.h>
#include <terminal/ParserEvents.h>
#include <terminal/Sequence.h>
#include <terminal/pty/UnixUtils.h>

#include <csignal>
#include <iostream>

#if defined(__APPLE__)
    #include <util.h>
#else
    #include <termios.h>
#endif

#include <fcntl.h>
#include <unistd.h>

using namespace std;

namespace
{

using namespace terminal;

struct BasicParserEvents: public NullParserEvents // {{{
{
    Sequence _sequence {};
    SequenceParameterBuilder _parameterBuilder;

    BasicParserEvents(): _parameterBuilder(_sequence.parameters()) {}

    void collect(char _char) override { _sequence.intermediateCharacters().push_back(_char); }

    void collectLeader(char _leader) noexcept override { _sequence.setLeader(_leader); }

    void clear() noexcept override
    {
        _sequence.clearExceptParameters();
        _parameterBuilder.reset();
    }

    void paramDigit(char _char) noexcept override
    {
        _parameterBuilder.multiplyBy10AndAdd(static_cast<uint8_t>(_char - '0'));
    }

    void paramSeparator() noexcept override { _parameterBuilder.nextParameter(); }

    void paramSubSeparator() noexcept override { _parameterBuilder.nextSubParameter(); }

    void param(char _char) noexcept override
    {
        switch (_char)
        {
            case ';': paramSeparator(); break;
            case ':': paramSubSeparator(); break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': paramDigit(_char); break;
        }
    }

    virtual void handleSequence() = 0;

    void executeSequenceHandler()
    {
        _parameterBuilder.fixiate();
        handleSequence();
    }

    void dispatchESC(char _finalChar) override
    {
        _sequence.setCategory(FunctionCategory::ESC);
        _sequence.setFinalChar(_finalChar);
        executeSequenceHandler();
    }

    void dispatchCSI(char _finalChar) override
    {
        _sequence.setCategory(FunctionCategory::CSI);
        _sequence.setFinalChar(_finalChar);
        executeSequenceHandler();
    }

    void startOSC() override { _sequence.setCategory(FunctionCategory::OSC); }

    void putOSC(char _char) override
    {
        if (_sequence.intermediateCharacters().size() + 1 < Sequence::MaxOscLength)
            _sequence.intermediateCharacters().push_back(_char);
    }

    void dispatchOSC() override
    {
        auto const [code, skipCount] = parser::extractCodePrefix(_sequence.intermediateCharacters());
        _parameterBuilder.set(static_cast<Sequence::Parameter>(code));
        _sequence.intermediateCharacters().erase(0, skipCount);
        executeSequenceHandler();
        clear();
    }

    void hook(char _finalChar) override
    {
        _sequence.setCategory(FunctionCategory::DCS);
        _sequence.setFinalChar(_finalChar);
        executeSequenceHandler();
    }
};
// }}}

struct MouseTracker final: public BasicParserEvents
{
    static bool running;

    int line = -1;
    int column = -1;
    termios savedTermios;

    parser::Parser<ParserEvents> vtInputParser;

    MouseTracker(): savedTermios { detail::getTerminalSettings(STDIN_FILENO) }, vtInputParser { *this }
    {
        auto tio = savedTermios;
        tio.c_lflag &= static_cast<tcflag_t>(~(ECHO | ICANON));
        detail::applyTerminalSettings(STDIN_FILENO, tio);

        writeToTTY("\033[?1003;1006h"); // enable mouse reporting protocols
        writeToTTY("\033[?2022h");      // enable passive mouse reporting
        writeToTTY("\033[?25l");        // hide text cursor

        signal(SIGINT, signalHandler);
        signal(SIGTERM, signalHandler);
        signal(SIGSTOP, signalHandler);
    }

    ~MouseTracker() override
    {
        detail::applyTerminalSettings(STDIN_FILENO, savedTermios);
        writeToTTY("\033[?1003;1006l"); // disable mouse reporting protocols
        writeToTTY("\033[?2022l");      // disable passive mouse reporting
        writeToTTY("\033[?25h");        // show text cursor
        writeToTTY("\nTerminating\n");
    }

    static void signalHandler(int signo)
    {
        running = false;
        signal(signo, SIG_DFL);
    }

    int run()
    {
        checkPassiveMouseTrackingSupport();
        while (running)
        {
            cout << "\r\033[KMouse position: " << line << ":" << column;
            cout.flush();
            processInput();
        }
        cout << '\n';
        return EXIT_SUCCESS;
    }

    void checkPassiveMouseTrackingSupport()
    {
        writeToTTY("\033[?2022$p");
        while (!_decrpm)
            processInput();

        auto const state = _decrpm.value().second;
        auto const supported = state == 1 || state == 2;
        fmt::print("Passive mouse tracking: {}\n", supported ? "supported" : "not supported");
    }

    void writeToTTY(string_view s) { ::write(STDOUT_FILENO, s.data(), s.size()); }

    void processInput()
    {
        char buf[128];
        auto n = ::read(STDIN_FILENO, buf, sizeof(buf));
        if (n > 0)
            vtInputParser.parseFragment(string_view(buf, static_cast<size_t>(n)));
    }

    void handleSequence() override
    {
        // \033[<{ButtonStates}{COLUMN};{LINE}M
        if (_sequence.leaderSymbol() == '<' && _sequence.finalChar() == 'M')
        {
            // NB: button states on param index 0
            column = _sequence.param_or(1, -2);
            line = _sequence.param_or(2, -2);
        }
        else if (_sequence.leaderSymbol() == '?' && _sequence.intermediateCharacters() == "$"
                 && _sequence.finalChar() == 'y' && _sequence.parameterCount() == 2)
        {
            // Receive now something like this: CSI ? 2022 ; 0 $ y
            _decrpm = pair { _sequence.param(0), _sequence.param(1) };
        }
        _sequence.clear();
    }

    std::optional<std::pair<int, int>> _decrpm;
};

bool MouseTracker::running = true;

} // namespace

int main()
{
    auto mouseTracker = MouseTracker {};
    return mouseTracker.run();
}
