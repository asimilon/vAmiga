// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Console.h"
#include "Emulator.h"
#include "Parser.h"
#include <istream>
#include <sstream>
#include <string>

namespace vamiga {

void
Console::_initialize()
{
    // Register commands
    initCommands(root);

    // Initialize the text storage
    clear();

    // Initialize the input buffer
    history.push_back( { "", 0 } );

    // Print the startup message and the input prompt
    asyncExec("welcome");
}

Console&
Console::operator<<(char value)
{
    storage << value;
    remoteManager.rshServer << value;

    if (serialPort.getConfig().device == SPD_COMMANDER) {

        serialPort << value;
    }
    needsDisplay();
    return *this;
}

Console&
Console::operator<<(const string& value)
{
    storage << value;
    remoteManager.rshServer << value;

    if (serialPort.getConfig().device == SPD_COMMANDER) {

        serialPort << value;
    }
    needsDisplay();
    return *this;
}

Console&
Console::operator<<(int value)
{
    *this << std::to_string(value);
    return *this;
}

Console&
Console::operator<<(unsigned int value)
{
    *this << std::to_string(value);
    return *this;
}

Console &
Console::operator<<(long value)
{
    *this << std::to_string(value);
    return *this;
}

Console &
Console::operator<<(unsigned long value)
{
    *this << std::to_string(value);
    return *this;
}

Console &
Console::operator<<(long long value)
{
    *this << std::to_string(value);
    return *this;
}

Console &
Console::operator<<(unsigned long long value)
{
    *this << std::to_string(value);
    return *this;
}

Console &
Console::operator<<(std::stringstream &stream)
{
    string line;
    while(std::getline(stream, line)) {
        *this << line << '\n';
    }
    return *this;
}

const char *
Console::text()
{
    static string all;

    // Add the storage contents
    storage.text(all);

    // Add the input line
    all += input + " ";

    return all.c_str();
}

void
Console::tab(isize pos)
{
    auto count = pos - (isize)storage[storage.size() - 1].size();

    if (count > 0) {

        std::string fill(count, ' ');
        storage << fill;
        remoteManager.rshServer << fill;
        needsDisplay();
    }
}

void
Console::setStream(std::ostream &os)
{
    storage.ostream = &os;
}

void
Console::needsDisplay()
{
    retroShell.isDirty = true;
}

void
Console::clear()
{
    storage.clear();
    needsDisplay();
}

void
Console::printState()
{
    std::stringstream ss;

    ss << "\n";
    cpu.dumpLogBuffer(ss, 8);
    ss << "\n";
    amiga.dump(Category::Current, ss);
    ss << "\n";
    cpu.disassembleRange(ss, cpu.getPC0(), 8);
    ss << "\n";

    *this << ss;
}

void
Console::press(RetroShellKey key, bool shift)
{
    assert_enum(RetroShellKey, key);
    assert(ipos >= 0 && ipos < historyLength());
    assert(cursor >= 0 && cursor <= inputLength());

    switch(key) {

        case RSKEY_UP:

            if (ipos > 0) {

                // Save the input line if it is currently shown
                if (ipos == historyLength() - 1) history.back() = { input, cursor };

                auto &item = history[--ipos];
                input = item.first;
                cursor = item.second;
            }
            break;

        case RSKEY_DOWN:

            if (ipos < historyLength() - 1) {

                auto &item = history[++ipos];
                input = item.first;
                cursor = item.second;
            }
            break;

        case RSKEY_LEFT:

            if (cursor > 0) cursor--;
            break;

        case RSKEY_RIGHT:

            if (cursor < (isize)input.size()) cursor++;
            break;

        case RSKEY_DEL:

            if (cursor < inputLength()) {
                input.erase(input.begin() + cursor);
            }
            break;

        case RSKEY_CUT:

            if (cursor < inputLength()) {
                input.erase(input.begin() + cursor, input.end());
            }
            break;

        case RSKEY_BACKSPACE:

            if (cursor > 0) {
                input.erase(input.begin() + --cursor);
            }
            break;

        case RSKEY_HOME:

            cursor = 0;
            break;

        case RSKEY_END:

            cursor = (isize)input.length();
            break;

        case RSKEY_TAB:

            if (tabPressed) {

                // TAB was pressed twice
                asyncExec("help \"" + input + "\"");

            } else {

                // Auto-complete the typed in command
                input = autoComplete(input);
                cursor = (isize)input.length();
            }
            break;

        case RSKEY_RETURN:

            pressReturn(shift);
            break;

        case RSKEY_CR:

            input = "";
            cursor = 0;
            break;
    }

    tabPressed = key == RSKEY_TAB;
    needsDisplay();

    assert(ipos >= 0 && ipos < historyLength());
    assert(cursor >= 0 && cursor <= inputLength());
}

void
Console::press(char c)
{
    switch (c) {

        case '\n':

            press(RSKEY_RETURN);
            break;

        case '\r':

            press(RSKEY_CR);
            break;

        case '\t':

            press(RSKEY_TAB);
            break;

        default:

            if (isprint(c)) {

                if (cursor < inputLength()) {
                    input.insert(input.begin() + cursor, c);
                } else {
                    input += c;
                }
                cursor++;
            }
    }

    tabPressed = false;
    needsDisplay();
}

void
Console::press(const string &s)
{
    for (auto c : s) press(c);
}

isize
Console::cursorRel()
{
    assert(cursor >= 0 && cursor <= inputLength());
    return cursor - (isize)input.length();
}

void
Console::pressReturn(bool shift)
{
    if (shift) {

        // Switch the interpreter
        retroShell.switchConsole();

    } else {

        // Add the command to the text storage
        *this << input << '\n';

        // Add the command to the history buffer
        history.back() = { input, (isize)input.size() };
        history.push_back( { "", 0 } );
        ipos = (isize)history.size() - 1;

        // Feed the command into the command queue
        asyncExec(input);

        // Clear the input line
        input = "";
        cursor = 0;
    }
}

Arguments
Console::split(const string& userInput)
{
    std::stringstream ss(userInput);
    Arguments result;

    string token;
    bool str = false; // String mode
    bool esc = false; // Escape mode

    for (usize i = 0; i < userInput.size(); i++) {

        char c = userInput[i];

        // Abort if a comment begins
        if (c == '#') break;

        // Check for escape mode
        if (c == '\\') { esc = true; continue; }

        // Switch between string mode and non-string mode if '"' is detected
        if (c == '"' && !esc) { str = !str; continue; }

        // Check for special characters in escape mode
        if (esc && c == 'n') c = '\n';

        // Process character
        if (c != ' ' || str) {
            token += c;
        } else {
            if (!token.empty()) result.push_back(token);
            token = "";
        }
        esc = false;
    }
    if (!token.empty()) result.push_back(token);

    return result;
}

string
Console::autoComplete(const string& userInput)
{
    string result;

    // Split input string
    Arguments tokens = split(userInput);

    // Complete all tokens
    autoComplete(tokens);

    // Recreate the command string
    for (const auto &it : tokens) { result += (result == "" ? "" : " ") + it; }

    // Add a space if the command has been fully completed
    if (!tokens.empty() && getRoot().seek(tokens)) result += " ";

    return result;
}

void
Console::autoComplete(Arguments &argv)
{
    Command *current = &getRoot();
    string prefix, token;

    for (auto it = argv.begin(); current && it != argv.end(); it++) {

        *it = current->autoComplete(*it);
        current = current->seek(*it);
    }
}

bool
Console::isBool(const string &argv)
{
    return util::isBool(argv);
}

bool
Console::isOnOff(const string  &argv)
{
    return util::isOnOff(argv);
}

long
Console::isNum(const string &argv)
{
    return util::isNum(argv);
}

bool
Console::parseBool(const string &argv)
{
    return util::parseBool(argv);
}

bool
Console::parseBool(const string &argv, bool fallback)
{
    try { return parseBool(argv); } catch(...) { return fallback; }
}

bool
Console::parseBool(const Arguments &argv, long nr, long fallback)
{
    return nr < long(argv.size()) ? parseBool(argv[nr]) : fallback;
}

bool
Console::parseOnOff(const string &argv)
{
    return util::parseOnOff(argv);
}

bool
Console::parseOnOff(const string &argv, bool fallback)
{
    try { return parseOnOff(argv); } catch(...) { return fallback; }
}

bool
Console::parseOnOff(const Arguments &argv, long nr, long fallback)
{
    return nr < long(argv.size()) ? parseOnOff(argv[nr]) : fallback;
}

long
Console::parseNum(const string &argv)
{
    return util::parseNum(argv);
}

long
Console::parseNum(const string &argv, long fallback)
{
    try { return parseNum(argv); } catch(...) { return fallback; }
}

long
Console::parseNum(const Arguments &argv, long nr, long fallback)
{
    return nr < long(argv.size()) ? parseNum(argv[nr]) : fallback;
}

string
Console::parseSeq(const string &argv)
{
    return util::parseSeq(argv);
}

string
Console::parseSeq(const string &argv, const string &fallback)
{
    try { return parseSeq(argv); } catch(...) { return fallback; }
}

Command &
Console::getRoot()
{
    return root;
}

void
Console::asyncExec(const string &command)
{
    // Feed the command into the command queue
    commands.push_back({ 0, command});
    emulator.put(Cmd(CMD_RSH_EXECUTE));
}

void
Console::exec()
{
    SYNCHRONIZED

    // Only proceed if there is anything to process
    if (commands.empty()) return;

    std::pair<isize, string> cmd;

    try {

        while (!commands.empty()) {

            cmd = commands.front();
            commands.erase(commands.begin());
            exec(cmd);
        }
        msgQueue.put(MSG_RSH_EXEC);

    } catch (ScriptInterruption &) {

        msgQueue.put(MSG_RSH_WAIT);

    } catch (...) {

        // Remove all remaining commands
        commands = { };

        msgQueue.put(MSG_RSH_ERROR);
    }

    // Print prompt
    *this << getPrompt();
}

void
Console::exec(QueuedCmd cmd)
{
    auto line = cmd.first;
    auto command = cmd.second;

    try {

        // Print the command if it comes from a script
        if (line) *this << command << '\n';

        // Call the interpreter
        exec(command);

    } catch (ScriptInterruption &) {

        // Rethrow the exception
        throw;

    } catch (std::exception &err) {

        // Print error message
        describe(err, line, command);

        // Rethrow the exception if the command is not prefixed with 'try'
        if (command.rfind("try", 0)) throw;
    }
}

void
Console::asyncExecScript(std::stringstream &ss)
{
    SYNCHRONIZED
    
    std::string line;
    isize nr = 1;

    while (std::getline(ss, line)) {

        commands.push_back({ nr++, line });
    }

    emulator.put(Cmd(CMD_RSH_EXECUTE));
}

void
Console::asyncExecScript(const std::ifstream &fs)
{
    std::stringstream ss;
    ss << fs.rdbuf();
    asyncExecScript(ss);
}

void
Console::asyncExecScript(const string &contents)
{
    std::stringstream ss;
    ss << contents;
    asyncExecScript(ss);
}

void
Console::abortScript()
{
    {   SYNCHRONIZED

        if (!commands.empty()) {

            commands.clear();
            agnus.cancel<SLOT_RSH>();
        }
    }
}

void
Console::exec(const string& userInput, bool verbose)
{
    // Split the command string
    Arguments tokens = split(userInput);

    // Skip empty lines
    if (tokens.empty()) return;

    // Remove the 'try' keyword
    if (tokens.front() == "try") tokens.erase(tokens.begin());

    // Auto complete the token list
    autoComplete(tokens);

    // Process the command
    exec(tokens, verbose);
}

void
Console::exec(const Arguments &argv, bool verbose)
{
    // In 'verbose' mode, print the token list
    if (verbose) {
        for (const auto &it : argv) *this << it << ' ';
        *this << '\n';
    }

    // Skip empty lines
    if (argv.empty()) return;

    // Seek the command in the command tree
    Command *current = &getRoot(), *next;
    Arguments args = argv;

    while (!args.empty() && ((next = current->seek(args.front())) != nullptr)) {

        current = current->seek(args.front());
        args.erase(args.begin());
    }
    if ((next = current->seek(""))) current = next;

    // Error out if no command handler is present
    if (!current->callback && !args.empty()) {
        throw util::ParseError(args.front());
    }
    if (!current->callback && args.empty()) {
        throw TooFewArgumentsError(current->fullName);
    }

    // Check the argument count
    if ((isize)args.size() < current->minArgs()) throw TooFewArgumentsError(current->fullName);
    if ((isize)args.size() > current->maxArgs()) throw TooManyArgumentsError(current->fullName);

    // Call the command handler
    current->callback(args, current->param);
}

void
Console::usage(const Command& current)
{
    *this << '\r' << "Usage: " << current.usage() << '\n';
}

void
Console::help(const string& userInput)
{
    // Split the command string
    Arguments tokens = split(userInput);

    // Auto complete the token list
    autoComplete(tokens);

    // Process the command
    help(tokens);
}

void
Console::help(const Arguments &argv)
{
    Command *current = &getRoot();
    string prefix, token;

    for (auto &it : argv) {
        if (current->seek(it) != nullptr) current = current->seek(it);
    }

    help(*current);
}

void
Console::help(const Command& current)
{
    auto indent = string("    ");

    // Print the usage string
    usage(current);

    // Determine tabular positions to align the output
    isize tab = 0;
    for (auto &it : current.subCommands) {
        tab = std::max(tab, (isize)it.fullName.length());
    }
    tab += (isize)indent.size();

    isize newlines = 1;

    for (auto &it : current.subCommands) {

        // Only proceed if the command is visible
        if (it.hidden) continue;

        // Print the group (if present)
        if (!it.groupName.empty()) {

            *this << '\n' << it.groupName << '\n';
            newlines = 1;
        }

        // Print newlines
        for (; newlines > 0; newlines--) {
            *this << '\n';
        }

        // Print command descriptioon
        *this << indent;
        *this << it.fullName;
        (*this).tab(tab);
        *this << " : ";
        *this << it.help.second;
        *this << '\n';
    }

    *this << '\n';
}

void
Console::describe(const std::exception &e, isize line, const string &cmd)
{
    if (line) *this << "Line " << line << ": " << cmd << '\n';

    if (auto err = dynamic_cast<const TooFewArgumentsError *>(&e)) {

        *this << err->what() << ": Too few arguments";
        *this << '\n';
        return;
    }

    if (auto err = dynamic_cast<const TooManyArgumentsError *>(&e)) {

        *this << err->what() << ": Too many arguments";
        *this << '\n';
        return;
    }

    if (auto err = dynamic_cast<const util::EnumParseError *>(&e)) {

        *this << err->token << " is not a valid key" << '\n';
        *this << "Expected: " << err->expected << '\n';
        return;
    }

    if (auto err = dynamic_cast<const util::ParseNumError *>(&e)) {

        *this << err->token << " is not a number";
        *this << '\n';
        return;
    }

    if (auto err = dynamic_cast<const util::ParseBoolError *>(&e)) {

        *this << err->token << " must be true or false";
        *this << '\n';
        return;
    }

    if (auto err = dynamic_cast<const util::ParseOnOffError *>(&e)) {

        *this << "'" << err->token << "' must be on or off";
        *this << '\n';
        return;
    }

    if (auto err = dynamic_cast<const util::ParseError *>(&e)) {

        *this << err->what() << ": Syntax error";
        *this << '\n';
        return;
    }

    if (auto err = dynamic_cast<const Error *>(&e)) {

        *this << err->what();
        *this << '\n';
        return;
    }
}

void
Console::dump(CoreObject &component, Category category)
{
    *this << '\n';
    _dump(component, category);
}

void
Console::dump(CoreObject &component, std::vector <Category> categories)
{
    *this << '\n';
    for(auto &category : categories) _dump(component, category);
}

void
Console::_dump(CoreObject &component, Category category)
{
    // assert(isEmulatorThread());

    std::stringstream ss;

    switch (category) {

        case Category::Slots:       ss << "Slots:\n\n"; break;
        case Category::Config:      ss << "Configuration:\n\n"; break;
        case Category::Properties:  ss << "Properties:\n\n"; break;
        case Category::Registers:   ss << "Registers:\n\n"; break;
        case Category::State:       ss << "State:\n\n"; break;
        case Category::Stats:       ss << "Statistics:\n\n"; break;

        default:
            break;
    }

    component.dump(category, ss);

    *this << ss << '\n';
}

}
