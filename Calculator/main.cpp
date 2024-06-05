/* 
    Simple calculator

    Revision history:

    Revised by Bjarne Stroustrup (bjarne@stroustrup.com) November 2023
    Revised by Bjarne Stroustrup November 2013
    Revised by Bjarne Stroustrup May 2007
    Revised by Bjarne Stroustrup August 2006
    Revised by Bjarne Stroustrup August 2004
    Originally written by Bjarne Stroustrup (bs@cs.tamu.edu) Spring 2004.

    This program implements a basic expression calculator.
    Input from cin; output to cout.
    The grammar for input is:
    Calculation:
        Statement
        Print
        Quit
        Calculation Statement
    Statement:
        Declaration
        Expression
        Print
        Quit
    Declaration:
        "let" Name "=" Expression

    Print:
        ";"
    Quit:
        "q"

    Expression:
        Term
        Expression "+" Term
        Expression "-" Term

    Term:
        Primary
        Term "*" Primary
        Term "/" Primary
        Term "%" Primary
    Primary:
        Number
        "(" Expression ")"
        "-" Primary
        "+" Primary

    Number:
        floating-point-literal
        Input comes from cin through the Token_stream called ts.

*/


#include <iostream>
#include <cmath>
#include <string>
#include <vector>
#include <cctype>


constexpr char number = '8';  // t.kind==number means that t is a number Token
constexpr char quit = 'q'; // t.kind==quit means that t is a quit Token
constexpr char print = '='; // t.kind==pr int means that t is a print Token
constexpr std::string prompt = "> ";
constexpr std::string result = "= "; // used to indicate that what follows is a result
const char name = 'a'; // name token
const char let = 'L'; // declaration token
const std::string declkey = "let"; // declaration keyword

//------------------------------------------------------------------------------
void error(std::string s)	// write ``error: s'' and exit program (for non-exception terminating error handling)
	{
		throw std::runtime_error{s};
    }

void error(std::string s, std::string msg)	// write ``error: s'' and exit program (for non-exception terminating error handling)
	{
		throw std::runtime_error{s + " " + msg};
    }


inline void keep_window_open()
{
    std::cin.clear();
    std::cout << "Please enter a character to exit\n";
    char ch;
    std::cin >> ch;
    return;
}


class Variable {
public:
    std::string name;
    double value;

public:
    double get_value(std::string s);
    void set_value(std::string s, double d);
};

std::vector<Variable> var_table;

class Token{
public:
    char kind;        // what kind of token
    double value;   
    std::string name;  // for numbers: a value 
    Token() :kind{0} {} // default constructor
    Token(char ch) :kind{ch} { } // initialize kind with ch
    Token(char ch, double val) :kind{ch}, value{val} { } // initialize kind and value
    Token(char ch, std::string n) :kind{ch}, name{n} { } // initialize kind and name
};

//------------------------------------------------------------------------------

class Token_stream {
public:
    Token_stream();   // make a Token_stream that reads from cin
    Token get();      // get a Token (get() is defined elsewhere)
    void putback(Token t);    // put a Token back
    void ignore(char c);  // discard characters up to and including a c
private:
    bool full;        // is there a Token in the buffer?
    Token buffer;     // here is where we keep a Token put back using putback()
};

//------------------------------------------------------------------------------

double Variable::get_value(std::string s)
    // retur n the value of the Var iable named s
{
    for (const Variable& v : var_table)
        if (v.name == s)
            return v.value;
    error("trying to read undefined variable ", s);
    return 0;
}

void Variable::set_value(std::string s, double d)
    // set the Variable named s to d
{
    for (Variable& v : var_table)
        if (v.name == s) {
            v.value = d;
            return;
        }
    error("trying to write undefined variable", s);
}

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

void Token_stream::ignore(char c)
    // c represents the kind of Token
{
    if (full && c==buffer.kind) {
        full = false;
        return;
    }
    full = false;

    //now search input
    char ch = 0;
    while (std::cin >> ch) {
        if (ch == c)
            return;
    }
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
        // case '=':    // for "print"
        // case 'x':
        case quit:  
        case print: 
        case '(':
        case ')':
        case '{':
        case '}': 
        case '+': 
        case '-': 
        case '*': 
        case '/': 
        case '!': 
        case '%':
            return Token(ch);        // let each character represent itself
        case '.':                    // a floating-point-literal can start with a dot   
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9': // numer ic literal
            {
                std::cin.putback(ch);         // put digit back into the input stream
                double val;
                std::cin >> val;              // read a floating-point number
                return Token(number, val);   
            }
        default:
            if (isalpha(ch)) {
                std::string s;
                s += ch;
                while (std::cin.get(ch) && (isalpha(ch) || isdigit(ch)))
                    s+=ch;

                std::cin.putback(ch);
                
                if (s == declkey)   
                    return Token{let};

                return Token{name, s}; 
            }
            error("Bad token");
            exit(1);
        }
}

//------------------------------------------------------------------------------

Token_stream ts;        // provides get() and putback() 



double get_value_from_variable(std::string var) {
    for (const Variable& v : var_table)
        if (v.name == var)
            return v.value;
    return 0;
}
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
    case 'a':
    case number:   
    {   
        double tmp{};
        if (t.kind == 'a') tmp = get_value_from_variable(t.name);
        if (t.kind == number) tmp = t.value;
        t = ts.get();
        if (t.kind == '!' && tmp < 0)
            error("Factorial can't accept negative number!!!");
        if (t.kind != '!') {
            ts.putback(t);
            return tmp; // return the number’s value
        }

        return  factorial(tmp);  // return factorial of the number's value
    }

    case '-':
        return -primary();

    case '+':
        return primary();


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
        case '%':
        {   
            double d = primary();
            if (d == 0) 
                error("%: divide by zero");
            left = fmod(left, d);
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



bool is_declared(std::string var)
// is var already in var_table?
{
    for (const Variable& v : var_table)
        if (v.name == var)
            return true;
    return false;
}

double define_name(std::string var, double val)
// add {var,val} to var_table
{
    if (is_declared(var))
        error(var," declared twice");
    var_table.push_back(Variable{var,val});
    return val;
}

double declaration()
    // assume we have seen "let"
    // handle: name = expression
    // declare a variable called "name" with the initial value "expression"
{
    Token t = ts.get();
    if (t.kind != name)
        error ("name expected in declaration");

    Token t2 = ts.get();
    if (t2.kind != '=')
            error("= missing in declaration of ", t.name);

    double d = expression();
    define_name(t.name ,d);
    return d;
}

void clean_up_mess() {
    ts.ignore(print);
}
double statement()
{
    Token t = ts.get();
    switch (t.kind) {
        case let:
            return declaration();
        default:
            ts.putback(t);
            return expression();
    }
}
void calculate() {
    // expression evaluation loop
    while (std::cin) 
    try{
        std::cout << prompt;
        Token t = ts.get();
        while (t.kind == print) 
            t = ts.get();     

        if (t.kind == quit)
            return;    

        ts.putback(t);
        std::cout << result << statement() << '\n';
    } catch (std::exception& e){
        std::cerr << e.what() << std::endl;
        clean_up_mess();
    }
        
    
}

//------------------------------------------------------------------------------

int main() {
    std::cout << "Welcome to our simple calculator. Please enter expressions using floating-point numbers.\n\n\n";

    std::cout << "Operators which are available: +, -, *, /, !, %\nTo exit from program type 'x', to get a result type '='\n"; 
    try
    {   

        // predefine names:
        define_name("pi",3.1415926535);
        define_name("e",2.7182818284);

        calculate();
        return 0;
        // keep_window_open();
    }
    catch (std::exception& e) {
        std::cerr << "error: " << e.what() << '\n';
        // keep_window_open();
        return 1;
    }   
    catch (...) {
        std::cerr << "Exception!\n";
        // keep_window_open();
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