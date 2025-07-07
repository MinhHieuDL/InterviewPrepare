- The difference between _move_ and _copy_ is that after a copy two objects must have the same value, whereas after a move the source of the move is not required to have its original value
- Moves can be used when the source object will not be used again
- An object is considered an object of its type after its constructor completes, and it remains an object of its type until its destructor starts executing
  ![Class life cycle syntax](./images/CalssLifeCycleSyntax.png)
  <p align="center"><i>Figure 1: Class life cycle syntax </p></i>
  <br>
- There are five situations in which an object is copied or moved:
  - As the source of an assignment
  - As an object initializer
  - As a function argument
  - As a function return value
  - As an exception
- The style of constructor/destructor-based resource management is called _Resource Acquisition Is Initialization_ or simply RAII
- Constructors and destructors interact correctly with class hierarchies:
  - Constructor: base → member → derived
  - Destructor: derived → member → base
- **Member objects** are always contructed in **the order they are declared**, not the order in the initializer list
- A destructor is invoked implicitly upon exit from a scope or by **delete**. This implies that we can prevent destruction of a class by declaring its destructor **=delete** or **private**
  ![Prevent implicit class destruction](./images/PreventImplicitClassDestruction.png)
  <p align="center"><i>Figure 2: Prevent implicit class destruction</p></i>
  <br>
- **virtual** Destructors:
  - A destructor can be declared to be virtual, and usually should be for a class with a virtual function
   ![virtual destructor](./images/VirtualDestructor.png)
  <p align="center"><i>Figure 3: virtual destructor</p></i>
  <br>
  - The reason we need a **virtual** destructor is that an object usually manipulated through the interface provided by a base class is often also deleted through that interface
   
   ![virtual destructor using](./images/VirtualDestructorUsing.png)
  <p align="center"><i>Figure 4: virtual destructor using</p></i>
  <br>