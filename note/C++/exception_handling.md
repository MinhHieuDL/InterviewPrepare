**Exceptions:**
  - The notion of an _exception_ is provided to help get information from the point where an error is detected to a point where it can be handled
  - A function that cannot cope with a problem _throws_ an exception, hoping that its (direct or indirect) caller can handle the problem
  - A function that wants to handle a kind of problem indicates that by _catching_ the corresponding exception:
    - A calling component indicates the kinds of failures that it is willing to handle by specifying those exceptions in a **catch**-clause of a **try**-block
    - A called component that cannot complete its assigned task reports its failure to do so by throwing an exception using a **throw**-expression
        ```cpp
        void taskmaster(){
            try {
                auto resurlt = do_task();
                //use result
            }
            catch(Some_error){
                // failure to do_task: handle problem
            }
        }

        int do_task()
        {
            // ..
            if(/* could perform the task*/)
                return result;
            else
                throw Some_error{};
        }
        ```
    - The **taskmaster()** asks **do_task()** to do a job. If **do_task()** can do that job and return a correct result, all is fine. Otherwise, **do_task()** must report a failure by throwing some exception. The **taskmaster()** is prepared to handle a **Some_error**, but some other kind of exception may be thrown. For example, **do_task()** may call other functions to do a lot of subtasks, and one of those may throw because it can’t do its assigned subtask. An exception different from **Some_error** indicates a failure of **taskmaster()** to do its job and must be handled by whatever code invoked **taskmaster()**.
  - A called function cannot just return with an indication that an error happened. If the program is to continue working (and not just print an error message and terminate), the returning function must leave the program in a good state and not leak any resource
  - The exception-handling mechanism is integrated with the constructor/destructor mechanisms and the concurrency mechanisms to help ensure that. The excepion-handling mechanism:
    - Is an alternative to the traditional techniques when they are insufficient, inelegant, or error-prone
    - Is complete; it can be used to handle all errors detected by ordinary code
    - Allows the programmer to explicitly separate error-handling code from "ordinary code," thus making the program more readable and more amenable to tools
    - Supports a more regular style of error handling, thus simplifying cooperation between separately written progam fragments
  - An exception is an object **throw**n to represent the occurence of an error. It can be any type that can be copied, but it is strongly recommended to use only user-defined types specifically defined for that purpose
  - An exception is caught by code that has expressed interest in handling a particular type of exception (a **catch-**clause). Thus, the simplest way of defining an exception is to define a class specifically for a kind of error and throw that. For example:
    ```cpp
    struct Range_for{};

    void f(int n)
    {
        if(n<0 || max<n) throw Range_error{};
        // ...
    }
    ```
**Traditional Error Handling:**
- Terminate the program. For example:
  ```cpp
  if(something_wrong) exit(1);
  ```
  - For most errors, we can and must do better
  - In most situation we should at least write out a decent error message or log the error before terminating
  - A library that doesn’t know about the purpose and general strategy of the program in which it is embedded cannot simply exit() or abort()
  - Library that unconditionally terminates cannot be used in a program that cannot afford to crash
- Return an error value:
  - This is not always feasible because there is often no acceptable ‘‘error value.’
- Return a legal value and leave the program in an "error state"
  - This has the problem that the calling function may not notice that the program has been put in an error state
  - Example of this approach is: many standard C library function set the nonlocal variable **errno** to indicate an error
- Call an error-handler function. For example:
    ```cpp
    if(something_wrong) something_handler(); // and possible continue here
    ```
**When we can't use exceptions**
- A time-critical component of an embedded system where an operation must be guranteed to complete in a specific maximum time. In the absence of tools that can accurately estimate the maximum time for an exception to propagate from a **throw** to a **catch**, alternative error-handling methods must be used
- A large old program in which resource management is an ad hoc mess (e.g., free store is unsystematically "managed" using "naked" pointers, **new**s and **delete**s), rather than relying on some systematic scheme, such as resource handles (e.g., **string** and **vector**)
- Using two popular techiniques for handling such cases above:
  - To mimic RAII, give every class with a constructor an **invalid()** operation that returns some **error_code**. A useful convention is for **error_code==0** to represent success. If the constructor fails to establish the class invariant, it ensures that no resource is leaked and invalid() returns a nonzero **error_code**. This solves the problem of how to get an error condition out of a constructor. A user can then systematically test invalid() after each construction of an object and engage in suitable error handling in case of failure. For example:
    ```cpp
    void f(int n)
    {
      my_vector<int> x(n);
      if(x.invalid()){
        // ...deal with error
      }
      // ...
    }
    ```
  - To mimic a function either returning a value or throwing an exception, a function can return a **pair<Value,Error_code>**. A user can then systematically test the error_code after each function call and engage in suitable error handling in case of failure. For example:
    ```cpp
    void g(int n)
    {
      auto v = make_vector(n); // return a pair
      if(v.second)
      {
        // ... deal with error
      }
      auto val = v.first;
      // ...
    }
    ```