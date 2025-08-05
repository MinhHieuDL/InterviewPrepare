- constexpr: Evaluate at compile time
- A _constant expression_ is an expression that a compiler can evaluate
- It cannot use values that are not known at compile time and it cannot have side effects
- A constant expression must start out with an integral value, a floating-point value, or an enumerator, and we can combine those using operators and **constexpr** functions that in turn produce values
- There are a variety of reasons why someone might want a named constant rather than a literal or a value stored in a variable:
  - Named constants make the code easier to understand and maintain
  - A variable might be changed (so we have to be more careful in our reasoning than for a constant)
  - The language requires constant expressions for array sizes, case labels, and template value arguments
  - Embedded systems programmers like to put immutable data into read-only memory because read-only memory is cheaper than dynamic memory (in terms of cost and energy consumption), and often more plentiful. Also, data in read-only memory is immune to most system crashes
  - If initialization is done at compile time, there can be no data races on that object in a multi-threaded system
  - Sometimes, evaluating something once (at compile time) gives significantly better performance than doing so a million times at run time
- The use of **constexpr**:
  - Symbolic Constants: 
    - the most important single use of constants (**constexpr** or **const** values) is simply to provide symbolic names for values
    - Symbolic names should be used systematically to avoid "magic numbers" in code
  - Literal Types:
    - A sufficiently simpler user-defined type can be used in a constant expression. For example:
        ```cpp
        struct Point{
            int x,y,z;
            constexpr Point up(int d){return {x,y,z+d};}
            constexpr Point move(int dx, int dy) {return {x+dx,y+dy};}
            // ...
        }
        ```
    - A class with a **constexpr** constructor is called _literal type_
    - To be simple enough to be constexpr, a constuctor must have an empty body and all members must be initializerd by potentially constant expressions
    - We can define **constepxr** functions to take arguments of literal type:
        ```cpp
        constexpr int square(int x) {return x*x;}

        constexpr int radial_distance(Point p) {
            return isqrt(square(p.x)+square(p.y)+square(p.z));
        }

        constexpr Point p1 {10,20,30}; // the default constructor is constexpr
        constexpr p2 {p1.up(20)};      // Point::up() is constexpr
        constexpr int dist = radial_distance(p2);
        ```
  - Reference Arguments:
    - When working with **constexpr**, the key thing to remember is that **constexpr** is all about values
    - There are no objects that can change values or side effects here: **constexpr** provides a miniature compile-time functional programming language
    - Consider:
        ```cpp
        template<> class complex<double> {
            public;
                constexpr complex(double re = 0.0, double im = 0.0);
                constexpr complex(const complex<float>&);
                explicit constexpr complex(const complex<long double>&);

                constexpr double real(); // read the real part
                void real(double);       //   set the real part
                constexpr double imag(); //read the imaginary part
                void imag(double);       // set the imaginary part

                complex<double>& operator= (double);
                complex<double>& operator+=(double);
                // ...
        }
        ```
    - Operations, such as **=** and **+=**, that modify an object cannot be **constexpr**
    - Operations that simply read the an object can be **constexpr** and be evaluated at complie time given a constant expression
    - The copy constructor works because the compiler recognizes that the reference (the const complex<float>&)