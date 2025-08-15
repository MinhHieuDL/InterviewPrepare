Exceptions:
  - The notion of an _exception_ is provided to help get information from the point where an error is detected to a point where it can be handled
  - A function that cannot cope with a problem _throws_ an exception, hoping that its (direct or indirect) caller can hadle the problem
  - A function that wants to handle a kind of problem indicates that by _catching_ the corresponding exception:
    - A calling component indicates the kinds of failures that is it willing to handle by specifying those exceptions in a **catch**-clause of a **try**-block
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
    - The **taskmaster()** **asks do_task()** to do a job. I do_task() can do that job and return a correct result, all is fine. Otherwise, **do_task()** must report a failure by throwing some exception. The **taskmaster()** is prepared to handle a **Some_error**, but some other kind of exception may be thrown. For example, **do_task()** may call other functions to do a lot of subtasks, and one of those may throw because it can’t do its assigned subtask. An exception different from **Some_error** indicates a failure of **taskmaster()** to do its job and must be handled by whatever code invoked **taskmaster()**.
  - A called function cannot just return with an indication that an error happened. If the program is to continue working (and not just print an error message and terminate), the returning function must leave the program in a food state and not leak any resource
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