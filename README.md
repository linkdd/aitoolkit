# AI Toolkit

<center>

![tests](https://img.shields.io/github/actions/workflow/status/linkdd/aitoolkit/tests?style=flat-square&logo=github)
![license](https://img.shields.io/github/license/linkdd/aitoolkit?style=flat-square&color=blue)
![version](https://img.shields.io/github/v/release/linkdd/aitoolkit?style=flat-square&color=red)

</center>

**AI Toolkit** is a header-only C++ library which provides tools for building
the brain of your game's NPCs.

## Installation

Add the `include` folder of this repository to your include paths.

Or add it as a submodule:

```
$ git submodule add https://github.com/linkdd/aitoolkit.git
$ g++ -std=c++23 -Iaiotoolkit/include main.cpp -o mygame
```

## Usage

TODO

## Documentation

The documentation is available online [here](https://linkdd.github.io/aitoolkit).

You can build it locally using [doxygen](https://www.doxygen.nl/):

```
$ make docs
```

## License

This library is released under the terms of the [MIT License](./LICENSE.txt).
