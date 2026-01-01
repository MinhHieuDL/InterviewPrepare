- A _lambda expression_, sometimes also referred to as _lambda function_ or (strictly speaking incorrectly, but colloquially) as a _lambda_, is a simplified notation for defining and using an anonymous function object
- Instead of defining a named class with an operator**()**, later making an object of that class, and finally invoking it, we can use a shorthand
- This is particularly useful when we want pass an operation as an argument to an alogorithm
- In the context of graphical user interface (and elsewhere), such operations are often referred to as _callbacks_
- A lambda expression consists of a sequence of parts:
  - A possibly empty _capture list_, specifying what names from the definition environment can be used in the lambda expression's body, and whether those are copied or accessed by reference. The captutre list is delimited by **[]**
  - An optional _parameter list_, specifying what arguments the lambda expression requires. The parameter list is delimited by **()**
  - An optional **mutable** specifier, indicating that the lambda expression’s body may modify the state of the lambda (i.e., change the lambda’s copies of variables captured by value)
  - An optional **noexcept** specifier
  - An optional return type declaration of the form **−>** type
  -  A body, specifying the code to be executed. The body is delimited by **{}**
- Implementation Model:
  - It useful to understand the semantics of a lambda by considering it a shorthand for defining and using a function object. For example
    ```cpp
    void print_modulo(const vector<int>& v, ostream& os, int m)
        // output v[i] to os if v[i]%m==0
    {
        for_each(begin(v), end(v),
            [&os,m](int x) {if (x%m ==0) os << x << '\n';}
            );
    }
    ```
  - To see what this means, we can define the equivalent _fucntion object_:
    ```cpp
    class Modulo_print {
        ostream& os; // members to hold the capture list
        int m;
    public:
        Modulo_print(ostream& os, int mm) : os(s), m(mm){} // cature
        void operator()(int x) cosnt {
            if(x%m == 0) os << x << '\n';
        }
    };
    ```
  - An object of a class generated from a lambda is called a _closure object_ (or simply a _closure_). We can now write the original function like this:
    ```cpp
    void print_modulo(const vector<int>& v, ostream& os, int m)
        // output v[i] to os if v[i]%m==0
    {
        for_each(begin(v), end(v), Modulo_print{os,m});
    }
    ```
  - If a lambda potentially captures every local variable by reference (using the capture list **[&]**), the closure may be optimized to simply contain a pointer to the enclosing stack frame
- Alternatives to Lambdas:
  - Many lambdas are small and used only once. For such uses, the realistic equivalent involes a local class defined immediately before its (only) use. For example:
    ```cpp
    void print_modulo(const vector<int>& v, ostream& os, int m)
        // output v[i] to os if v[i]%m==0
    {
        class Modulo_print {
            ostream& os; // members to hold the capture list
            int m;
        public:
            Modulo_print (ostream& s, int mm) :os(s), m(mm) {} // capture
            void operator()(int x) const
            { if (x%m==0) os << x << '\n'; }
        };

        for_each(begin(v), end(v), Modulo_print{os,m});
    }
    ```
  - Compared to that, the version using the lambda is a clear winner. If we really want a name, we can just name the lambda:
      ```cpp
      void print_modulo(const vector<int>& v, ostream& os, int m)
          // output v[i] to os if v[i]%m==0
      {
          auto Modulo_print = [&os,m](int x){if (x%m==0) os << x << '\n';};
          for_each(begin(v), end(v), Modulo_print);
      }
      ```
  - Naming the lambda is often a good idea. Doing so foreces us to consider the design of the operation a bit more carefully. It also simplifies code layout and allows for recursion
  - Writing a for-loop is an alternative to using a lambda with a **for_each()**. Consider:
      ```cpp
      void print_modulo(const vector<int>& v, ostream& os, int m)
          // output v[i] to os if v[i]%m==0
      {
          for(auto x : v)
              if(x%m==0) os << x << '\n';
      }
      ```
- Capture:
  - _Lamdba introducer_ **[]**
  - A lambda introducer can take various forms:
    - **[]**: an empty capture list. This implies that no local names from the surrounding context can be used in the lambda body. For such lambda expressions, data is obtained from arguments or from nonlocal variables
    - **[&]**: implicitly capture by reference. All local names can be used. All local variables are accessed by reference
    - **[=]**: implicit capture by value.  All local names can be used. All names refer to copies of the local variables taken at the point of call of the lambda expression
    - **[capture-list]**: explicit capture; the capture-list is the list of names of local variables to be captured (i.e., stored in the object) by reference or by value. Variables with names preceded, by & are captured by reference. Other variables are captured by value. A capture list can also contain this and names followed by ... as elements
    - **[&, capture-list]**: mplicitly capture by reference all local variables with names not mentioned in the list. The capture list can contain this. Listed names cannot be preceded by &. Variables named in the capture list are captured by value
    - **[=, capture-list]**: mplicitly capture by value all local variables with names not mentioned in the list. The capture list cannot contain this. The listed names must be preceded by &. Variables named in the capture list are captured by reference
  - Only capture by reference allows modification of variables in the calling environment
  - Use the capture by reference if need to write to the captured object or if it is large
  - Because, for lambdas, there is the added concern that a lambda might outlive its caller. When passing a lambda to another thread capturing by value (**[=]**) is typically the best: accessing another thread's stack through a reference or a pointer can be most disruptive (to performance or correctness), and trying to access the stack of a terminated thread can lead to extremely difficult-to-find errors
- **mutable** lambdas:
  - Usually, we don’t want to modify the state of the function object (the closure), so by default we can't
  - That is, the **operator()()** for the generated function object is a const member fucntion
  - If we want to modify the state (as opposed to modifying the state of some variable captured by reference), we can declare the lambda **mutable**. For example:
    ```cpp
    void alogo(vector<int>&v)
    {
        int count = v.size();
        std::generated(v.begin(), v.end(),
            [count]()mutable{return --count};);
    }
    ```
  - The **--count** decrements the copy of v'size stored in the closure 