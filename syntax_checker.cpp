// ==========================================================
//  Project : Syntax and Bracket Checker
//  Language: C++17 (single file)
//
//  What it does:
//    1. Checks C++ files for balanced brackets  ()  {}  []
//    2. Checks HTML / XML files for balanced tags
//    3. Uses two stacks: array stack and linked-list stack
//    4. Prints every error with line number, the code line
//       and an arrow (^) showing where the problem is
//
//  How to run:
//    g++ -std=c++17 syntax_checker.cpp -o checker
//    ./checker                  (menu)
//    ./checker myfile.cpp       (direct check)
// ==========================================================

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
using namespace std;

// Maximum size of the array stack
const int MAXSIZE = 5000;


// ==========================================================
//  PART 1 : STACKS
// ==========================================================

// One thing stored in the stack.
// For brackets: name = "(" or "{" or "["
// For tags    : name = "div", "body", etc.
struct Item {
    string name;   // bracket or tag name
    int line;      // line number where it was found
    int col;       // position in that line (starts from 0)
};

// Base class (common interface) for both stacks.
// Both stacks follow this same interface, so the
// checker functions work with any of them.
class Stack {
public:
    virtual bool push(Item x) = 0;   // returns false if stack is full
    virtual Item pop() = 0;
    virtual Item top() = 0;
    virtual bool empty() = 0;
    virtual ~Stack() {}
};


// ---------- Array based stack ----------
class ArrayStack : public Stack {
private:
    Item* data;       // array that holds the items
    int topIndex;     // index of the top item (-1 means empty)

public:
    ArrayStack() {
        data = new Item[MAXSIZE];
        topIndex = -1;
    }

    ~ArrayStack() {
        delete[] data;
    }

    bool push(Item x) {
        // overflow check: array is already full
        if (topIndex == MAXSIZE - 1) {
            cout << "Stack overflow! Array stack is full." << endl;
            return false;
        }
        topIndex++;
        data[topIndex] = x;
        return true;
    }

    Item pop() {
        // underflow check: nothing to remove
        if (topIndex == -1) {
            cout << "Stack underflow! Stack is empty." << endl;
            Item blank = {"", 0, 0};
            return blank;
        }
        Item x = data[topIndex];
        topIndex--;
        return x;
    }

    Item top() {
        if (topIndex == -1) {
            cout << "Stack is empty, no top item." << endl;
            Item blank = {"", 0, 0};
            return blank;
        }
        return data[topIndex];
    }

    bool empty() {
        return topIndex == -1;
    }
};


// ---------- Linked list based stack ----------
// Each node stores one item and a pointer to the node below it
struct Node {
    Item data;
    Node* next;
};

class LinkedStack : public Stack {
private:
    Node* head;       // head is always the top of the stack

public:
    LinkedStack() {
        head = NULL;
    }

    ~LinkedStack() {
        // free all nodes
        while (head != NULL) {
            Node* temp = head;
            head = head->next;
            delete temp;
        }
    }

    bool push(Item x) {
        // make a new node and put it at the front of the list
        Node* n = new Node;
        n->data = x;
        n->next = head;
        head = n;
        return true;   // no size limit, so it never fails
    }

    Item pop() {
        if (head == NULL) {
            cout << "Stack underflow! Stack is empty." << endl;
            Item blank = {"", 0, 0};
            return blank;
        }
        Node* temp = head;
        Item x = temp->data;
        head = head->next;
        delete temp;
        return x;
    }

    Item top() {
        if (head == NULL) {
            cout << "Stack is empty, no top item." << endl;
            Item blank = {"", 0, 0};
            return blank;
        }
        return head->data;
    }

    bool empty() {
        return head == NULL;
    }
};


// Removes everything left in a stack
void clearStack(Stack& s) {
    while (!s.empty()) {
        s.pop();
    }
}


// ==========================================================
//  PART 2 : SMALL HELPER FUNCTIONS
// ==========================================================

bool isOpening(char c) {
    return c == '(' || c == '{' || c == '[';
}

bool isClosing(char c) {
    return c == ')' || c == '}' || c == ']';
}

// true if open and close are a matching pair
bool isPair(char open, char close) {
    if (open == '(' && close == ')') return true;
    if (open == '{' && close == '}') return true;
    if (open == '[' && close == ']') return true;
    return false;
}

bool isLetter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

// true if c is a letter, digit or underscore
bool isWordChar(char c) {
    return isLetter(c) || isDigit(c) || c == '_';
}

// makes a copy of the string in lowercase
string toLower(string s) {
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] >= 'A' && s[i] <= 'Z') {
            s[i] = s[i] + 32;
        }
    }
    return s;
}

// true if the line ends with a backslash (line continuation)
bool endsWithBackslash(const string& line) {
    return !line.empty() && line[line.size() - 1] == '\\';
}

// true if the apostrophe at position i is a digit separator like 1'000'000
// (and not the start of a character literal like 'a')
bool isDigitSeparator(const string& line, size_t i) {
    size_t k = i;
    // go back to the start of the word before the apostrophe
    while (k > 0 && isWordChar(line[k - 1])) {
        k--;
    }
    // a number starts with a digit, e.g. 1'000
    return k < i && isDigit(line[k]);
}

// true if text has the word 'part' at position pos
bool startsWith(const string& text, size_t pos, const string& part) {
    if (pos + part.size() > text.size()) return false;
    return text.compare(pos, part.size(), part) == 0;
}

// counts new lines between two positions of the text
int countNewlines(const string& text, size_t from, size_t to) {
    int count = 0;
    for (size_t i = from; i < to && i < text.size(); i++) {
        if (text[i] == '\n') count++;
    }
    return count;
}

// finds the column of position pos (distance from start of its line)
int findColumn(const string& text, size_t pos) {
    size_t nl = text.rfind('\n', pos == 0 ? 0 : pos - 1);
    if (pos == 0 || nl == string::npos) return (int)pos;
    return (int)(pos - nl - 1);
}

// HTML void tags (these have no closing tag)
bool isVoidTag(const string& name) {
    string list[] = {"br", "img", "hr", "input", "meta", "link", "area",
                     "base", "col", "embed", "param", "source", "track", "wbr"};
    for (int i = 0; i < 14; i++) {
        if (name == list[i]) return true;
    }
    return false;
}

// true if the file name ends with the given extension
bool hasExtension(string filename, string ext) {
    filename = toLower(filename);
    if (filename.size() < ext.size()) return false;
    return filename.compare(filename.size() - ext.size(), ext.size(), ext) == 0;
}

// reads the file into a vector of lines. Returns false if it cannot open.
bool readFile(const string& filename, vector<string>& lines) {
    ifstream file(filename.c_str());
    if (!file) {
        return false;
    }
    string line;
    while (getline(file, line)) {
        // remove \r if the file has Windows line endings
        if (!line.empty() && line[line.size() - 1] == '\r') {
            line.erase(line.size() - 1);
        }
        lines.push_back(line);
    }
    file.close();
    return true;
}


// ==========================================================
//  PART 3 : ERROR PRINTING
// ==========================================================

// Prints one line of the file like this:
//     4 | int a[3] = {1, 2, 3);
//       |                    ^
// col = position of the arrow (use -1 for no arrow)
void printSnippet(const vector<string>& lines, int lineNo, int col) {
    if (lineNo < 1 || lineNo > (int)lines.size()) return;

    string text = lines[lineNo - 1];

    // replace tabs by spaces so the arrow stays in the right place
    for (size_t i = 0; i < text.size(); i++) {
        if (text[i] == '\t') text[i] = ' ';
    }

    // cut very long lines
    bool cut = false;
    if (text.size() > 70) {
        text = text.substr(0, 70);
        cut = true;
    }

    // print the line number and the code
    string num = to_string(lineNo);
    while (num.size() < 5) num = " " + num;
    cout << "    " << num << " | " << text;
    if (cut) cout << "...";
    cout << endl;

    // print the arrow under the problem
    if (col >= 0 && col < 70) {
        cout << "          | " << string(col, ' ') << "^" << endl;
    }
}

// Prints one complete error message.
// line2 > 0 means "also show a second line" (for example where the bracket was opened)
void reportError(int& errors, string type, string message,
                 const vector<string>& lines, int lineNo, int col,
                 int line2 = 0, int col2 = -1, string label2 = "") {
    errors++;
    cout << "[Error " << errors << "] " << type << "  (line " << lineNo << ")" << endl;
    cout << "    " << message << endl;
    printSnippet(lines, lineNo, col);
    if (line2 > 0) {
        cout << "    " << label2 << " (line " << line2 << "):" << endl;
        printSnippet(lines, line2, col2);
    }
    cout << endl;
}

// Reports everything that is still in the stack at the end of the file.
// These brackets / tags were opened but never closed.
void reportUnclosed(Stack& s, int& errors, const vector<string>& lines, bool isTag) {
    // pop everything into a vector first, so we can print in file order
    vector<Item> left;
    while (!s.empty()) {
        left.push_back(s.pop());
    }
    for (int k = (int)left.size() - 1; k >= 0; k--) {
        Item x = left[k];
        if (isTag) {
            reportError(errors, "Tag never closed",
                        "<" + x.name + "> was opened here but </" + x.name + "> is missing.",
                        lines, x.line, x.col);
        } else {
            reportError(errors, "Bracket never closed",
                        "'" + x.name + "' was opened here but never closed.",
                        lines, x.line, x.col);
        }
    }
}


// ==========================================================
//  PART 4 : BRACKET CHECKER (for C++ files)
// ==========================================================

// Returns the number of errors found
int checkBrackets(const vector<string>& lines, Stack& s) {
    int errors = 0;

    // These flags remember where we are, even when we go to the next line
    bool inBlockComment = false;      // inside /* ... */
    int blockLine = 0, blockCol = 0;  // where the /* started
    bool inRawString = false;         // inside R"( ... )"
    string rawEnd = "";               // text that ends the raw string
    int rawLine = 0, rawCol = 0;      // where the raw string started
    bool commentContinues = false;    // // comment ending with backslash

    clearStack(s);

    // go through the file line by line
    for (int lineNo = 1; lineNo <= (int)lines.size(); lineNo++) {
        string line = lines[lineNo - 1];

        // a // comment that ended with '\' continues on this line
        if (commentContinues) {
            commentContinues = endsWithBackslash(line);
            continue;
        }

        // go through the line character by character
        for (size_t i = 0; i < line.size(); i++) {
            char c = line[i];
            char next = (i + 1 < line.size()) ? line[i + 1] : '\0';

            // --- inside a block comment: look only for the end */ ---
            if (inBlockComment) {
                if (c == '*' && next == '/') {
                    inBlockComment = false;
                    i++;
                }
                continue;
            }

            // --- inside a raw string: look only for its end ---
            if (inRawString) {
                if (startsWith(line, i, rawEnd)) {
                    inRawString = false;
                    i = i + rawEnd.size() - 1;
                }
                continue;
            }

            // --- single line comment: ignore the rest of the line ---
            if (c == '/' && next == '/') {
                commentContinues = endsWithBackslash(line);
                break;
            }

            // --- start of block comment ---
            if (c == '/' && next == '*') {
                inBlockComment = true;
                blockLine = lineNo;
                blockCol = (int)i;
                i++;
                continue;
            }

            // --- string literal (and raw string) ---
            if (c == '"') {
                // raw string looks like R"delim( ... )delim"
                bool isRaw = false;
                if (i > 0 && line[i - 1] == 'R') {
                    if (i == 1) {
                        isRaw = true;
                    } else {
                        char before = line[i - 2];
                        if (!isWordChar(before) || before == 'L' || before == 'u' ||
                            before == 'U' || before == '8') {
                            isRaw = true;
                        }
                    }
                }

                if (isRaw) {
                    // find the delimiter between " and (
                    string delim = "";
                    size_t j = i + 1;
                    while (j < line.size() && line[j] != '(') {
                        delim += line[j];
                        j++;
                    }
                    rawEnd = ")" + delim + "\"";
                    inRawString = true;
                    rawLine = lineNo;
                    rawCol = (int)i;
                    i = j;
                    continue;
                }

                // normal string: find the closing quote, skipping \" inside
                size_t j = i + 1;
                bool closed = false;
                while (j < line.size()) {
                    if (line[j] == '\\') {
                        j += 2;
                    } else if (line[j] == '"') {
                        closed = true;
                        break;
                    } else {
                        j++;
                    }
                }
                if (!closed && !endsWithBackslash(line)) {
                    reportError(errors, "String not closed",
                                "This string starts with \" but has no closing \".",
                                lines, lineNo, (int)i);
                }
                i = j;
                continue;
            }

            // --- character literal like '(' or '\'' ---
            if (c == '\'') {
                // digit separator such as 1'000 is not a character literal
                if (isDigitSeparator(line, i)) {
                    continue;
                }
                size_t j = i + 1;
                bool closed = false;
                while (j < line.size()) {
                    if (line[j] == '\\') {
                        j += 2;
                    } else if (line[j] == '\'') {
                        closed = true;
                        break;
                    } else {
                        j++;
                    }
                }
                if (!closed) {
                    reportError(errors, "Character not closed",
                                "This character starts with ' but has no closing '.",
                                lines, lineNo, (int)i);
                }
                i = j;
                continue;
            }

            // --- opening bracket: push it on the stack ---
            if (isOpening(c)) {
                Item x;
                x.name = string(1, c);
                x.line = lineNo;
                x.col = (int)i;
                if (!s.push(x)) {
                    reportError(errors, "Too many brackets",
                                "Brackets are nested too deeply for the array stack. Stopping.",
                                lines, lineNo, (int)i);
                    clearStack(s);
                    return errors;
                }
            }
            // --- closing bracket: compare with the top of the stack ---
            else if (isClosing(c)) {
                if (s.empty()) {
                    reportError(errors, "Extra closing bracket",
                                string("'") + c + "' has no opening bracket before it.",
                                lines, lineNo, (int)i);
                } else {
                    Item opener = s.pop();
                    if (!isPair(opener.name[0], c)) {
                        reportError(errors, "Brackets do not match",
                                    "'" + opener.name + "' was closed by '" + string(1, c) + "'.",
                                    lines, lineNo, (int)i,
                                    opener.line, opener.col, "'" + opener.name + "' was opened here");
                    }
                }
            }
        }
    }

    // after the file ends, check what is still open
    if (inBlockComment) {
        reportError(errors, "Comment not closed",
                    "This /* comment never ends. A closing */ is missing.",
                    lines, blockLine, blockCol);
    }
    if (inRawString) {
        reportError(errors, "Raw string not closed",
                    "This raw string never ends.",
                    lines, rawLine, rawCol);
    }

    // every bracket still in the stack was never closed
    reportUnclosed(s, errors, lines, false);

    return errors;
}


// ==========================================================
//  PART 5 : TAG CHECKER (for HTML / XML files)
// ==========================================================

// isHtml = true  -> tag names are not case sensitive, void tags are skipped
// isHtml = false -> XML, names are case sensitive
int checkTags(const vector<string>& lines, Stack& s, bool isHtml) {
    // join all lines into one big string
    string text = "";
    for (size_t k = 0; k < lines.size(); k++) {
        text += lines[k] + "\n";
    }

    // HTML tags are not case sensitive, so make everything lowercase
    // (lines is not changed, so error messages still show the original code)
    if (isHtml) {
        text = toLower(text);
    }

    clearStack(s);

    int errors = 0;
    int line = 1;
    size_t n = text.size();
    size_t i = 0;

    while (i < n) {
        // count lines as we move forward
        if (text[i] == '\n') {
            line++;
        }

        // ignore everything which is not the start of a tag
        if (text[i] != '<') {
            i++;
            continue;
        }

        int startCol = findColumn(text, i);

        // --- comment <!-- ... --> ---
        if (startsWith(text, i, "<!--")) {
            size_t e = text.find("-->", i + 4);
            if (e == string::npos) {
                reportError(errors, "Comment not closed",
                            "This <!-- comment never ends. A closing --> is missing.",
                            lines, line, startCol);
                break;
            }
            line += countNewlines(text, i, e);
            i = e + 3;
            continue;
        }

        // --- CDATA section <![CDATA[ ... ]]> ---
        if (startsWith(text, i, "<![cdata[") || startsWith(text, i, "<![CDATA[")) {
            size_t e = text.find("]]>", i);
            if (e == string::npos) {
                reportError(errors, "CDATA not closed",
                            "This CDATA section never ends. A closing ]]> is missing.",
                            lines, line, startCol);
                break;
            }
            line += countNewlines(text, i, e);
            i = e + 3;
            continue;
        }

        // --- processing instruction <? ... ?> ---
        if (startsWith(text, i, "<?")) {
            size_t e = text.find("?>", i);
            if (e == string::npos) {
                reportError(errors, "Tag not closed",
                            "'<?' is never closed with '?>'.",
                            lines, line, startCol);
                break;
            }
            line += countNewlines(text, i, e);
            i = e + 2;
            continue;
        }

        // --- <!DOCTYPE ...> and similar ---
        if (startsWith(text, i, "<!")) {
            size_t e = text.find('>', i);
            if (e == string::npos) {
                reportError(errors, "Tag not closed",
                            "'<!' is never closed with '>'.",
                            lines, line, startCol);
                break;
            }
            line += countNewlines(text, i, e);
            i = e + 1;
            continue;
        }

        // a '<' that is not followed by a letter or '/' is just normal text
        char nx = (i + 1 < n) ? text[i + 1] : ' ';
        if (!(isLetter(nx) || nx == '/' || nx == '_' || nx == ':')) {
            i++;
            continue;
        }

        // --- find the '>' that ends this tag (ignore '>' inside quotes) ---
        size_t j = i + 1;
        char quote = 0;
        while (j < n) {
            if (quote != 0) {
                if (text[j] == quote) quote = 0;
            } else if (text[j] == '"' || text[j] == '\'') {
                quote = text[j];
            } else if (text[j] == '>') {
                break;
            }
            j++;
        }
        if (j >= n) {
            reportError(errors, "Tag not closed",
                        "This tag has no closing '>'.",
                        lines, line, startCol);
            break;
        }

        // --- read the tag name ---
        string inner = text.substr(i + 1, j - i - 1);   // text between < and >
        bool closing = (inner[0] == '/');
        size_t k = closing ? 1 : 0;
        string name = "";
        while (k < inner.size() && (isWordChar(inner[k]) || inner[k] == '-' ||
               inner[k] == ':' || inner[k] == '.')) {
            name += inner[k];
            k++;
        }
        bool selfClosing = (inner[inner.size() - 1] == '/');
        int tagLine = line;    // line where this tag starts

        if (name == "") {
            reportError(errors, "Tag has no name",
                        "There is no tag name after '<'.",
                        lines, tagLine, startCol);
        }
        else if (closing) {
            // closing tag like </div>
            if (isHtml && isVoidTag(name)) {
                // </br> and such are ignored
            } else if (s.empty()) {
                reportError(errors, "Extra closing tag",
                            "</" + name + "> has no opening tag before it.",
                            lines, tagLine, startCol);
            } else {
                Item opener = s.pop();
                if (opener.name != name) {
                    reportError(errors, "Tags do not match",
                                "Found </" + name + "> but </" + opener.name + "> was expected.",
                                lines, tagLine, startCol,
                                opener.line, opener.col, "<" + opener.name + "> was opened here");
                }
            }
        }
        else if (selfClosing || (isHtml && isVoidTag(name))) {
            // <br> or <img ... /> need no closing tag, so do nothing
        }
        else {
            // normal opening tag: push it
            Item x;
            x.name = name;
            x.line = tagLine;
            x.col = startCol;
            if (!s.push(x)) {
                reportError(errors, "Too many tags",
                            "Tags are nested too deeply for the array stack. Stopping.",
                            lines, tagLine, startCol);
                clearStack(s);
                return errors;
            }

            // inside <script> and <style> the text is not HTML, so skip it
            if (isHtml && (name == "script" || name == "style")) {
                size_t e = text.find("</" + name, j + 1);
                if (e == string::npos) {
                    line += countNewlines(text, i, n);
                    i = n;
                } else {
                    line += countNewlines(text, i, e);
                    i = e;
                }
                continue;
            }
        }

        // move after this tag
        line += countNewlines(text, i, j);
        i = j + 1;
    }

    // tags still in the stack were never closed
    reportUnclosed(s, errors, lines, true);

    return errors;
}


// ==========================================================
//  PART 6 : TESTING THE STACKS ON THEIR OWN
// ==========================================================

// prints PASS or FAIL for one test
void showResult(string what, bool ok, int& passed, int& total) {
    total++;
    if (ok) {
        passed++;
        cout << "  [PASS] " << what << endl;
    } else {
        cout << "  [FAIL] " << what << endl;
    }
}

// runs the same tests on any stack
void testStack(Stack& s, string stackName) {
    int passed = 0;
    int total = 0;
    cout << "--- Testing " << stackName << " ---" << endl;
    clearStack(s);

    // test 1 : new stack should be empty
    showResult("stack is empty at start", s.empty(), passed, total);

    // test 2 : push three items and check the top
    Item a = {"A", 1, 0};
    Item b = {"B", 2, 0};
    Item c = {"C", 3, 0};
    s.push(a);
    s.push(b);
    s.push(c);
    showResult("stack is not empty after push", !s.empty(), passed, total);
    showResult("top is the last pushed item (C)", s.top().name == "C", passed, total);

    // test 3 : pop should give items in reverse order (LIFO)
    bool order = true;
    if (s.pop().name != "C") order = false;
    if (s.pop().name != "B") order = false;
    if (s.pop().name != "A") order = false;
    showResult("pop gives C, B, A (last in first out)", order, passed, total);
    showResult("stack is empty after all pops", s.empty(), passed, total);

    // test 4 : underflow (the message below is expected)
    cout << "  (underflow message below is expected)" << endl;
    Item blank = s.pop();
    showResult("pop on empty stack gives blank item", blank.name == "" && blank.line == 0, passed, total);

    // test 5 : push more items than the array can hold
    int accepted = 0;
    for (int i = 0; i < MAXSIZE + 1; i++) {
        Item x = {"X", i, 0};
        if (s.push(x)) accepted++;
    }
    cout << "  Pushed " << MAXSIZE + 1 << " items, stack accepted " << accepted << "." << endl;
    clearStack(s);
    showResult("stack is empty after clearing", s.empty(), passed, total);

    cout << "  Result: " << passed << " / " << total << " tests passed." << endl << endl;
}


// ==========================================================
//  PART 7 : RUNNING A CHECK AND MAIN
// ==========================================================

// type 1 = C++ brackets, type 2 = HTML tags, type 3 = XML tags
int detectType(string filename) {
    if (hasExtension(filename, ".html") || hasExtension(filename, ".htm")) return 2;
    if (hasExtension(filename, ".xml") || hasExtension(filename, ".xhtml") ||
        hasExtension(filename, ".svg")) return 3;
    return 1;
}

// Runs one complete check and prints header + result
void runChecker(int type, string filename, Stack& s, string stackName) {
    vector<string> lines;
    if (!readFile(filename, lines)) {
        cout << endl << "Could not open the file '" << filename << "'." << endl;
        cout << "Check the file name and try again." << endl;
        return;
    }

    string kind = (type == 1) ? "C++ brackets" : (type == 2 ? "HTML tags" : "XML tags");

    cout << endl;
    cout << "========================================================" << endl;
    cout << " File  : " << filename << " (" << lines.size() << " lines)" << endl;
    cout << " Check : " << kind << endl;
    cout << " Stack : " << stackName << endl;
    cout << "========================================================" << endl << endl;

    int errors = 0;
    if (type == 1) {
        errors = checkBrackets(lines, s);
    } else {
        errors = checkTags(lines, s, type == 2);
    }

    cout << "========================================================" << endl;
    if (errors == 0) {
        cout << " RESULT: OK - no errors found. Everything is balanced." << endl;
    } else {
        cout << " RESULT: " << errors << " error(s) found. Please fix them and run again." << endl;
    }
    cout << "========================================================" << endl;
}

// asks the user which stack to use, creates it and runs the check
void askAndRun(int type, string filename) {
    cout << "Choose stack type:" << endl;
    cout << "  1. Array stack" << endl;
    cout << "  2. Linked list stack (no size limit)" << endl;
    cout << "Enter choice: ";
    string input;
    getline(cin, input);

    if (atoi(input.c_str()) == 1) {
        ArrayStack s;
        runChecker(type, filename, s, "Array stack");
    } else {
        LinkedStack s;
        runChecker(type, filename, s, "Linked list stack");
    }
}

int main(int argc, char* argv[]) {
    // if a file name is given, check it directly (type found from extension)
    if (argc >= 2) {
        string filename = argv[1];
        LinkedStack s;
        runChecker(detectType(filename), filename, s, "Linked list stack");
        return 0;
    }

    string input;
    string filename;
    int choice = 0;

    // menu
    while (choice != 5) {
        cout << endl;
        cout << "======== SYNTAX AND BRACKET CHECKER ========" << endl;
        cout << " 1. Check brackets in a C++ file" << endl;
        cout << " 2. Check tags in an HTML file" << endl;
        cout << " 3. Check tags in an XML file" << endl;
        cout << " 4. Test both stacks" << endl;
        cout << " 5. Exit" << endl;
        cout << "============================================" << endl;
        cout << "Enter choice: ";
        getline(cin, input);
        choice = atoi(input.c_str());

        if (choice >= 1 && choice <= 3) {
            cout << "Enter file name: ";
            getline(cin, filename);
            askAndRun(choice, filename);
        }
        else if (choice == 4) {
            cout << endl;
            ArrayStack as;
            LinkedStack ls;
            testStack(as, "Array Stack");
            testStack(ls, "Linked List Stack");
        }
        else if (choice == 5) {
            cout << "Goodbye!" << endl;
        }
        else {
            cout << "Invalid choice, please enter 1 to 5." << endl;
        }
    }
    return 0;
}
