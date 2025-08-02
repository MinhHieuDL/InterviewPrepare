- A pointer allow to pass potentially large amounts of data around at low cost: instead of copying the data we simply pass its address as a pointer value
- The type of the pointer determines what can be done to the data through the pointer
- Using a pointer differs from using the name of an object in a few ways:
  - We use a different syntax, for example, ∗p instead of obj and p−>m rather than obj.m
  - We can make a pointer point to different objects at different times
  - We must be more careful when using pointers than when using an object directly: a pointer may be a nullptr or point to an object that wasn’t the one we expected
- These differences can be annoying; for example, some programmers find f(&x) ugly compared to f(x)
- Worse, managing pointer variables with varying values and protecting code against the possibility of nullptr can be a significant burden
- Finally, when we want to overload an operator, say +, we want to write x+y rather than &x+&y
- The language mechanism addressing these problem is called a _reference_
- Like a pointer, a reference is an alias for an object, is usually implemented to hold a machine address of an object, and does not impose performance overhead compared to pointers, but it differs from a pointer in that:
  - You access the reference with exactly the same syntax as the name of an object
  - A reference always refers to the object to which it was initialized
  - There is no "null reference", and we may assume that a reference refers to an object
- The main use of references is for specifying arguments and return values for functions in general and for overloaded operators in particular
- To reflect the lvalue/rvalue and const/non-const distinctions, there are three kinds of references:
  - _lvalue reference_: to refer to objects whose value we want to change
  - const reference: to refer to objects whose value we do not want to change (e.g., a constant)
  - rvalue reference: to refer to objects whose value we do not need to preserve after we have used it (e.g., a temporary)
- Collectively, they are called _references_. The first two are both called _lvalue references_
- lvalue references:
  - In a type name, the notation X& means ‘‘reference to X.’’ It is used for references to lvalues, so it is often called an _lvalue reference_
  ![lvalue reference](./images/LvalueReference.png)
  <p align="center"><i>Figure 1: lvalue reference </p></i> 

  - To ensure that a reference is a name for something (that is, that it is bound to an object), we must initialize the reference
  ![lvalue reference initialize](./images/LvalueReferenceInitialize.png)
  <p align="center"><i>Figure 2: lvalue reference initialize</p></i>

  - If we have a "int& rr{var};" ++rr does not increment the reference rr; rather, ++ is applied to the int to which rr refers, that is, to var. Consequently, the value of a reference cannot be changed after initialization; it always refers to the object it was initialized to denote. To get a pointer to the object denoted by a reference rr, we can write &rr
  - Thus, we cannot have a pointer to a reference. Furthermore, we cannot define an array of references
  - The obvious implementation of a reference is as a (constant) pointer that is dereferenced each time it is used. It doesn’t do much harm to think about references that way, as long as one remembers that a reference isn’t an object that can be manipulated the way a pointer is
  ![pointer and reference](./images/PointerVsReference.png)
  <p align="center"><i>Figure 3: pointer and reference</p></i>

  - In some cases, the compiler can optimize away a reference so that there is no object representing that reference at run time
  - Initialization of a reference is trivial when the initializer is an lvalue. The initializer for a ‘‘plain’’ T& must be an lvalue of type T
  - The initializer for a const T& need not be an lvalue or even of type T. In such cases:
    - First, implicit type conversion to T is applied if necessary
    - Then, the resulting value is placed in a temporary variable of type T
    - Finally, this temporary variable is used as the value of the initializer
    ![Initialize a const lvalue reference](./images/InitialzeConstLvalueReference.png)
    <p align="center"><i>Figure 4: Initialize a const lvalue reference</p></i>

  - References can also be used as return types. This is mostly used to define functions that can be used on both the left-hand and right-hand sides of an assignment
