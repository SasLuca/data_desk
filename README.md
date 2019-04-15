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

To use Data Desk, you'll need to do a few things:

1. Get Data Desk
2. Make or get some Data Desk format files (.ds)
3. Make a project-specific custom layer

### Step 1: Get Data Desk

1. Run the command `git clone https://github.com/ryanfleury/data_desk`
2. `cd data_desk`
3. `build` on Windows (Linux builds not yet officially supported, but the build is extremely simple, so if you take a look at `build.bat` you can create a Linux equivalent very easily).

### Step 2: Make or get Data Desk format files (.ds)

Grab an (example)[https://github.com/ryanfleury/data_desk/blob/master/example_data/test.ds] or make your own.

### Step 3: Make a project-specific custom layer

1. An easy way to write the code for this is to check out the custom layer template, located here. Fill out the functions in your custom layer code however you want to. There are some helper functions available in `data_desk.h` that might be useful for you here. This can be dropped into your code and used.

2. To build a custom layer, you just need to build a DLL with the function callbacks you've written as the appropriate exported symbols. `data_desk.h` outlines what symbols are used for each callback.

### Step 4: Run Data Desk

To run Data Desk with your custom layer, you can use the following command template:

`data_desk --custom /path/to/custom/layer /file/to/parse/1 /file/to/parse/2 ...`