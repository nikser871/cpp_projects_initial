#include <iostream>

//------------------------------------------------------------------------------
void error(std::string s)	// write ``error: s'' and exit program (for non-exception terminating error handling)
	{
		throw std::runtime_error{s};
    }


inline void keep_window_open()
{
    std::cin.clear();
    std::cout << "Please enter a character to exit\n";
    char ch;
    std::cin >> ch;
    return;
}


class Token{
public:
    char kind;        // what kind of token
    double value;     // for numbers: a value 
    Token(char ch)    // make a Token from a char
        :kind(ch), value(0) { }
    Token(char ch, double val)     // make a Token from a char and a double
        :kind(ch), value(val) { }
};

//------------------------------------------------------------------------------

class Token_stream {
public:
    Token_stream();   // make a Token_stream that reads from cin
    Token get();      // get a Token (get() is defined elsewhere)
    void putback(Token t);    // put a Token back
private:
    bool full;        // is there a Token in the buffer?
    Token buffer;     // here is where we keep a Token put back using putback()
};

//------------------------------------------------------------------------------

// The constructor just sets full to indicate that the buffer is empty:
Token_stream::Token_stream()
    :full(false), buffer(0)    // no Token in buffer
{
}

//------------------------------------------------------------------------------

// The putback() member function puts its argument back into the Token_stream's buffer:
void Token_stream::putback(Token t)
{
    if (full) error("putback() into a full buffer");
    buffer = t;       // copy t to buffer
    full = true;      // buffer is now full
}

//------------------------------------------------------------------------------

Token Token_stream::get()
{
    if (full) {       // do we already have a Token ready?
        // remove token from buffer
        full = false;
        return buffer;
    }

    char ch;
    if (!(std::cin >> ch)) // note that >> skips whitespace (space, newline, tab, etc.)
        error("no input");    // note that >> skips whitespace (space, newline, tab, etc.)

    switch (ch) {
        case '=':    // for "print"
        case 'x':    // for "quit"
        case '(': case ')':  case '{': case '}': case '+': case '-': case '*': case '/': case '!':
            return Token(ch);        // let each character represent itself
        case '.':
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            {
                std::cin.putback(ch);         // put digit back into the input stream
                double val;
                std::cin >> val;              // read a floating-point number
                return Token('8', val);   // let '8' represent "a number"
            }
        default:
            error("Bad token");
            exit(1);
        }
}

//------------------------------------------------------------------------------

Token_stream ts;        // provides get() and putback() 

//------------------------------------------------------------------------------

double expression();    // declaration so that primary() can call expression()
int factorial(int n);
//------------------------------------------------------------------------------

// deal with numbers and parentheses
double primary()
{
    Token t = ts.get();
    switch (t.kind) {
    case '(':    // handle '(' expression ')'
    {
        double d = expression();
        t = ts.get();
        if (t.kind != ')') error("')' expected");
        return d;
    }
    case '{':    // handle '{' expression '}'
    {
        double d = expression();
        t = ts.get();
        if (t.kind != '}') error("'}' expected");
        return d;
    } 
    case '8': 
    {
        double tmp = t.value;
        t = ts.get();
        if (t.kind == '!' && tmp < 0)
            error("Factorial can't accept negative number!!!");
        if (t.kind != '!') {
            ts.putback(t);
            return tmp;
        }

        return  factorial(tmp);  // return the number's value
    }

    default:
        error("primary expected");
        exit(1);
    }
}


//------------------------------------------------------------------------------

// deal with *, /, and %
double term()
{
    double left = primary();
    Token t = ts.get();
    // std::cout << "term :" << left << '\n';        // get the next token from token stream

    while (true) {
        switch (t.kind) {
        case '*':
            left *= primary();
            t = ts.get();
            break;
        case '/':
        {
            double d = primary();
            if (d == 0) error("divide by zero");
            left /= d;
            t = ts.get();
            break;
        }
        default:
            ts.putback(t);     // put t back into the token stream
            return left;
        }
    }
}

//------------------------------------------------------------------------------

// deal with + and -
double expression()
{
    double left = term();      // read and evaluate a Term
    Token t = ts.get();
    // std::cout << "expr: " << t.kind << '\n';        // get the next token from token stream

    while (true) {
        switch (t.kind) {
        case '+':
            left += term();    // evaluate Term and add
            t = ts.get();
            break;
        case '-':
            left -= term();    // evaluate Term and subtract
            t = ts.get();
            break;
        default:
            ts.putback(t);     // put t back into the token stream
            return left;       // finally: no more + or -: return the answer
        }
    }
}

//------------------------------------------------------------------------------

int main() {
    std::cout << "Welcome to our simple calculator. Please enter expressions using floating-point numbers.\n\n\n";

    std::cout << "Operators which are available: +, -, *, /, !\nTo exit from program type 'x', to get a result type '='\n"; 
    try
    {   
        double val = 0;
        while (std::cin) {
            
            Token t = ts.get();

            if (t.kind == 'x') break; // 'x' for quit
            if (t.kind == '=')        // '=' for "print now"
                std::cout << "=" << val << '\n';
            else
                ts.putback(t);
            val = expression();
        }
        keep_window_open();
    }
    catch (std::exception& e) {
        std::cerr << "error: " << e.what() << '\n';
        keep_window_open();
        return 1;
    }   
    catch (...) {
        std::cerr << "Oops: unknown exception!\n";
        keep_window_open();
        return 2;
    }

}


int factorial(int n) {
    if (n == 0 || n == 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

//------------------------------------------------------------------------------