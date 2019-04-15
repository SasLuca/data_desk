# Data Desk

## Description

Data Desk is a project utility that parses a simple C-like data description format. Input files in this data description format are parsed to create corresponding abstract syntax trees which represent the information extracted from the files. These abstract syntax trees are then sent to project-specific custom code that is written by the user. This custom code is simply a dynamic library with a few exported functions that are used as callbacks for the parser. Below is a list of the callbacks.

* `DataDeskCustomInitCallback(void)` is called when the parser starts.
* `DataDeskCustomFileCallback(char *filename)` is called when the parser starts parsing a new file.
* `DataDeskCustomStructCallback(DataDeskASTNode *root, char *filename)` is called for every structure that is parsed.
* `DataDeskCustomDeclarationCallback(DataDeskASTNode *root, char *filename)` is called for every declaration that is parsed.
* `DataDeskCustomCleanUpCallback(DataDeskASTNode *root, char *filename)` is called before the parser shuts down.

The abstract syntax tree is formed completely by `DataDeskASTNode` structures. This structure can be found in the `data_desk.h` file.

Data Desk also offers a number of utility functions for introspecting on abstract syntax trees it passes to your custom code. A list of these is in the `data_desk.h` file, which can be included into your custom layer.

## Usage

Coming soon, but you can probably get the idea from the examples, located in the example folders.