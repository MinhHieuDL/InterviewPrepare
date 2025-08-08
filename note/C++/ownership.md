Reference from stackoverflow <br>
**Q**:<br>
What does it mean to express "Ownership of resources?" Is it the same as having ownership of pointers? Classes that involve dynamic memory management, do they express ownership of pointers or resources?
<br>
**A**:<br>
I don't think there is a universally accepted, 100% accurate and always-applicable definition, but the most useful definition of _ownership_ I've encountered in the context of C++ (and programming in general) is _responsibility for cleanup_. That is, an owner of a resource is the one who's responsible for correct cleanup of that resource.

What "resource" and "cleanup" mean depends on context. When the resource is memory dynamically allocated via new, cleanup is calling delete. When the resource is a file descriptor, cleanup is closing it. And so on.

That's why Plain Old Data cannot express ownership: a POD class must have a no-op destructor, and thus cannot perform any automatic cleanup.

Compare _int *_, _std::unique_ptr<int>_, and _std::shared_ptr<int>_. All these types are "a pointer to an int". But the first one does not represent ownership — you can do anything you want with it, and you can happily use it in ways which lead to memory leaks or double-frees.

_std::unique_ptr<int>_ represents the simplest form of ownership: it says "I am the sole owner of the resource (= the dynamically allocated int). When I get destroyed, I clean it up correctly." It is designed not to offer a sane way of violating the ownership semantics (e.g. it's noncopyable).

_std::shared_ptr<int>_ represents a more complex form of ownership: "I am one of a group of friends who are collectively responsible for the resource. The last of us to get destroyed will clean it up." Again, there are a few hoops to jump through before you can violate the semantics.