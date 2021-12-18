// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "SubComponent.h"
#include "Interpreter.h"
#include "RemoteServer.h"

#include <sstream>
#include <fstream>

class TextStorage {

    std::vector<string> storage;
    
public:
    
    // Returns the contents of the whole storage as a single C string
    const char *text();
    
    // Returns the number of stored lines
    isize size() { return (isize)storage.size(); }
    
    // Returns a reference to the last line
    string &back() { return storage.back(); }
    
    // Initializes the storage with a single empty line
    void clear();
    
    // Appends a new line
    void append(const string &line);
    
    // Operator overloads
    string operator [] (isize i) const { return storage[i]; }
    string& operator [] (isize i) { return storage[i]; }
};

class RetroShell : public SubComponent {

    friend class RemoteServer;
    
    // Interpreter for commands typed into the console window
    Interpreter interpreter;

public:

    // Server for managing remote connections
    RemoteServer remoteServer = RemoteServer(amiga);

    
    //
    // Text storage
    //

private:
    
    // The text storage
    TextStorage storage;
 
    // The input history buffer
    std::vector<string> input;

    // Input prompt
    string prompt = "vAmiga% ";
    
    // The current cursor position
    isize cpos = 0;

    // The minimum cursor position in this row
    isize cposMin = 0;
    
    // The currently active input string
    isize ipos = 0;

    // Wake up cycle for interrupted scripts
    Cycle wakeUp = INT64_MAX;

public:
    
    // Indicates if TAB was the most recently pressed key
    bool tabPressed = false;
    
    // Indicates whether the shell needs to be redrawn (DEPRECATED)
    bool isDirty = false;
    
    
    //
    // Script processing
    //
    
    // The currently processed script
    std::stringstream script;
    
    // The script line counter (first line = 1)
    isize scriptLine = 0;

    
    //
    // Initializing
    //
    
public:
    
    RetroShell(Amiga& ref);
        
    // Returns the welcome message
    std::stringstream welcome() const;
    
    // Dumps the current text storage to the remote server
    void dumpToServer();
    
    
    //
    // Methods from AmigaObject
    //
    
private:
    
    const char *getDescription() const override { return "RetroShell"; }
    void _dump(dump::Category category, std::ostream& os) const override { }
    
    
    //
    // Methods from AmigaComponent
    //
    
private:
    
    void _reset(bool hard) override { }
    
    isize _size() override { return 0; }
    u64 _checksum() override { return 0; }
    isize _load(const u8 *buffer) override {return 0; }
    isize _save(u8 *buffer) override { return 0; }


    //
    // Managing user input
    //

public:

    void pressUp();
    void pressDown();
    void pressLeft();
    void pressRight();
    void pressHome();
    void pressEnd();
    void pressTab();
    void pressBackspace();
    void pressDelete();
    void pressReturn();
    void pressKey(char c);


    //
    // Working with the text storage
    //

public:
    
    const char *text() { return storage.text(); }
    
    // Returns a reference to the text storage
    // const std::vector<string> &getStorage() { return storage; }
    
    // Returns the cursor position (relative to the line end)
    isize cposAbs() { return cpos; }
    isize cposRel();

    // Moves the cursor forward to a certain column
    void tab(isize hpos);

    // Prints a message
    RetroShell &operator<<(char value);
    RetroShell &operator<<(const string &value);
    RetroShell &operator<<(int value);
    RetroShell &operator<<(long value);
    RetroShell &operator<<(std::stringstream &stream);

    // void flush();
    void newLine();
    void printPrompt();

private:

    // Returns a reference to the last line in the text storage
    // string &lastLine() { return storage.back(); }
    
    // Returns a reference to the last line in the input history buffer
    string &lastInput() { return input.back(); }

    // Clears the console window
    void clear();
    
    // Prints a help line
    void printHelp();
    
    // Clears the current line
    void clearLine() { *this << '\r'; }
    
    
    //
    // Executing commands
    //
    
public:
    
    // Executes a user command
    void exec(const string &command) throws;
    
    // Executes a user script
    void execScript(std::ifstream &fs) throws;
    void execScript(const string &contents) throws;

    // Continues a previously interrupted script
    void continueScript() throws;

    // Prints a textual description of an error in the console
    void describe(const std::exception &exception);

    
    //
    // Command handlers
    //
    
public:
    
    // void handler(const string& command) throws;
    
    template <Token t1> void exec(Arguments& argv, long param) throws;
    template <Token t1, Token t2> void exec(Arguments& argv, long param) throws;
    template <Token t1, Token t2, Token t3> void exec(Arguments& argv, long param) throws;

private:
    
    void dump(AmigaComponent &component, dump::Category category);

    
    //
    // Performing periodic events
    //
    
public:
    
    void vsyncHandler();
};
