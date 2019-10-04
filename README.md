# Shared Memory Library

1. Creates a IPC storage with atomic locks for every datachunck. So it is thread safe.
2. It has C interface and can be used in C
3. C interface allows to create bindings for lots of languages.
4. Every time of data storing is possible
5. Works on all *nix OS


# Performance:

70 frames per second in resolution 1224*1024 can be transferred through shared memory storage.


# In progress:

1. RPC via shared memory
2. Iterators
3. STL containers based on shared memory
4. Deb package

# Done:

1. Interface
2. Thread Safety
2. Conan Package
