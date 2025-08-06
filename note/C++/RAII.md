Reference from stackoverflow:<br>
**Q:** <br>
What is meant by Resource Acquisition is Initialization (RAII)?

**A:** <br>
It's a really terrible name for an incredibly powerful concept, and perhaps one of the number 1 things that C++ developers miss when they switch to other languages. There has been a bit of a movement to try to rename this concept as **Scope-Bound Resource Management**, though it doesn't seem to have caught on just yet. 

When we say 'Resource' we don't just mean memory - it could be file handles, network sockets, database handles, GDI objects...  In short, things that we have a finite supply of and so we need to be able to control their usage. The 'Scope-bound' aspect means that the lifetime of the object is bound to the scope of a variable, so when the varable goes out of scope then the destructor will release the resource. A very useful property of this is that it makes for greater exception-safety. For instance, compare this:

```cpp
RawResourceHandle* handle = createNewResource();
handle->performInvalidOperation();  // Oop, throws exception
....
deleteResouce(handle); // oh dear, never gets called so the resource leaks
```
With the **_RAII_** one
```cpp
class ManagedResourceHandle {
    public:
        ManagedResouceHandle(RawResourceHandle* rawHandle_) : m_pRawHandle{rawHandle_} {};
        ~ManagedResourceHandle() {delete rawHandle; }
        ....
    private:
        RawResourceHandle* m_pRawHandle;
}

ManagedResourceHandle handle(createNewResource());
handle->performInvalidOperation();
```
In this latteer case, when the exception is thrown and the stack is unwound, the local variables are destroyed which ensures that our resource is cleaned up and doesn't leak

**A:** <br>
This is a programming idiom which briefly means that you:
- Encapsulate a resource into a class (whose constructor usually - but not neccessary** - acquires the resource, and its destructor always releases it)
- Use the resource via a local instance of the class*
- The resource is automatically freed when the object gets out of scope

This guarantees that whatever happens while the resource is in use, it will eventually get freed (whether due to normal return, destruction of the containing object, or an exception thrown)

It is a widely used good practice in C++, because apart from being a safe way to deal with resources, it also makes your code much cleaner as you don't need to mix error handling code with the main functionality

*Update: "local" may mean a local variable, or a nonstatic member variable of a class. In the latter case the member variable is initialized and destroyed with its owner object

**Update2: as @sbi pointed out, the resource - although often is allocated inside the constructor - may also be allocated outside and passed in as a parameter