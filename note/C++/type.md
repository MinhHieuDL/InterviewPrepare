- A type is a concrete representation of a concept (an idea, a notion, etc.). <br> 
For example, the C++ built-in type float with its operations +, −, ∗, etc., provides a concrete approximation of the mathematical concept of a real number

- A class is a user-defined type. A new type was designed to provide a definition of a concept that has no direct counterpart among the built-in types.

- A program that provides types that closely match the concepts of the application tends to be easier to understand, easier to reason about, and easier to modify than a program does not

- The fundamental idea in defining a new type is to separate the incidental details of the implementation (the layout of the data used to store an object of the type) from the properties essential to the correct use of it (the complete list of functions that can access the data). Such a separation is best expressed by channeling all uses of the data structure and its internal housekeeping routines through a specific interface<br>
Explain: A good class separates what it does (interface) from how it does (implementation), and enforces that all usage flows through that interface — not through direct access to its internal data
