# ![TreeQuo](https://github.com/Immortale-dev/bptdb/blob/master/other/logo.svg?raw=true)

C++ Database engine based on [B+Tree](https://en.wikipedia.org/wiki/B%2B_tree) data structure [implementation][l_bplustree]. Engine provides a lot of basic database functionality including **inserting**, **updating**, **deleting** _key/value_ items as well as **searching** items by _key_, performing **upper/lower bound** with returning pointer for item and ability to **move** through the items back and forward as well as read the item content. 

Engine is well optimised. You can find performance tests results in the [additional information](#performance) section.

## Table of Contest
* [About](#section)
* [Build](#build)
* [Dependencies](#dependencies)
* [Documentation](#documentation)
	* [Definitions](#definitions)
	* [Configuration](#configuration)
		* [void forest::config_root_factor(int root_factor)](#void-forestconfig_root_factorint-root_factor)
		* [void forest::config_default_factor(int default_factor)](#void-forestconfig_default_factorint-default_factor)
		* [void forest::config_root_tree(string root_tree)](#void-forestconfig_root_treestring-root_tree)
		* [void forest::config_intr_cache_length(int length)](#void-forestconfig_intr_cache_lengthint-length)
		* [void forest::config_leaf_cache_length(int length)](#void-forestconfig_leaf_cache_lengthint-length)
		* [void forest::config_tree_cache_length(int length)](#void-forestconfig_tree_cache_lengthint-length)
		* [void forest::config_cache_bytes(int bytes)](#void-forestconfig_cache_bytesint-bytes)
		* [void forest::config_chunk_bytes(int bytes)](#void-forestconfig_chunk_bytesint-bytes)
		* [void forest::config_opened_files_limit(int count)](#void-forestconfig_opened_files_limitint-count)
		* [void forest::config_save_schedule_mks(int mks)](#void-forestconfig_save_schedule_mksint-mks)
		* [void forest::config_savior_queue_size(int length)](#void-forestconfig_savior_queue_sizeint-length)
	* [Types](#types)
	* [Initialisation](#initialisation)
		* [void forest::bloom(string path)](#void-forestbloomstring-path)
		* [void forest::fold()](#void-forestfold)
	* [Status methods](#status-methods)
		* [bool forest::blooms()](#bool-forestblooms)
		* [int forest::get_save_queue_size()](#int-forestget_save_queue_size)
		* [int forest::get_opened_files_count()](#int-forestget_opened_files_count)
	* [Working with Trees](#working-with-trees)
		* [void forest::plant_tree(TREE_TYPES type, string name, int factor, string annotation)](#void-forestplant_treetree_types-type-string-name-int-factor-string-annotation)
		* [void forest::cut_tree(string name)](#void-forestcut_treestring-name)
		* [Tree forest::find_tree(string name)](#tree-forestfind_treestring-name)
	* [Creating Leafs](#creating-leafs)
		* [DetachedLeaf forest::make_leaf(string data)](#detachedleaf-forestmake_leafstring-data)
		* [DetachedLeaf forest::make_leaf(char* buffer, size_t length)](#detachedleaf-forestmake_leafchar-buffer-size_t-length)
		* [DetachedLeaf forest::make_leaf(LeafFile file, size_t start, size_t length)](#detachedleaf-forestmake_leafleaffile-file-size_t-start-size_t-length)
		* [LeafFile forest::create_leaf_file()](#leaffile-forestcreate_leaf_file)
	* [Leafs Operations](#leafs-operations)
		* [void forest::insert_leaf(string tree_name, LeafKey key, DetachedLeaf val)](#void-forestinsert_leafstring-tree_name-leafkey-key-detachedleaf-val)
		* [void forest::insert_leaf(Tree tree, LeafKey key, DetachedLeaf val)](#void-forestinsert_leaftree-tree-leafkey-key-detachedleaf-val)
		* [void forest::update_leaf(string tree_name, LeafKey key, DetachedLeaf val)](#void-forestupdate_leafstring-tree_name-leafkey-key-detachedleaf-val)
		* [void forest::update_leaf(Tree tree, LeafKey key, DetachedLeaf val)](#void-forestupdate_leaftree-tree-leafkey-key-detachedleaf-val)
		* [void forest::remove_leaf(string tree_name, LeafKey key)](#void-forestremove_leafstring-tree_name-leafkey-key)
		* [void forest::remove_leaf(Tree tree, LeafKey key)](#void-forestremove_leaftree-tree-leafkey-key)
	* [Leafs Searching](#leafs-searching)
		* [Leaf forest::find_leaf(string tree_name, LeafKey key)](#leaf-forestfind_leafstring-tree_name-leafkey-key)
		* [Leaf forest::find_leaf(Tree tree, LeafKey key)](#leaf-forestfind_leaftree-tree-leafkey-key)
		* [Leaf forest::find_leaf(string tree_name, LEAF_POSITION position)](#leaf-forestfind_leafstring-tree_name-leaf_position-position)
		* [Leaf forest::find_leaf(Tree tree, LEAF_POSITION position)](#leaf-forestfind_leaftree-tree-leaf_position-position)
		* [Leaf forest::find_leaf(string tree_name, LeafKey key, LEAF_POSITION position)](#leaf-forestfind_leafstring-tree_name-leafkey-key-leaf_position-position)
		* [Leaf forest::find_leaf(Tree tree, LeafKey key, LEAF_POSITION position)](#leaf-forestfind_leaftree-tree-leafkey-key-leaf_position-position)
* [Other Classes/Methods](#other-classesmethods)
	* [forest::Tree](#foresttree)
		* [TREE_TYPES get_type()](#tree_types-get_type)
		* [string get_annotation()](#string-get_annotation)
	* [forest::Leaf](#forestleaf)
		* [bool eof()](#bool-eof)
		* [bool move_forward()](#bool-move_forward)
		* [bool move_back()](#bool-move_back)
		* [LeafKey key()](#leafkey-key)
		* [DetachedLeaf val()](#detachedleaf-val)
	* [forest::DetachedLeaf](#forestdetachedleaf)
		* [size_t size()](#size_t-size)
		* [LeafReader get_reader()](#leafreader-get_reader)
	* [forest::LeafReader](#forestleafreader)
		* [size_t read(char* buffer, size_t count)](#size_t-readchar-buffer-size_t-count)
* [Tests and Scripts](#tests-and-scripts)
	* [test.[sh|ps1]](#test.shps1)
	* [testrc.[sh|ps1]](#testrc.shps1)
	* [testperf.[sh|ps1]](#testperf.shps1)
	* [clear.[sh|ps1]](#clear.shps1)
* [Additional Information](#additional-information)
	* [Performance](#performance)
		* [Machine](#machine)
		* [Configuration](#configuration-1)
		* [Tests](#tests)
		* [Summary](#summary)
	* [DOTOs/Ideas](#dotosideas)
	* [From author](#from-author)
* [License](#license)

## Build
Library was tested using **GNU G++** compiler with flag **-std=c++17**. So it is recommended to use C++ 17 or higher version of compiler. Compiling with another compilers might need code corrections.

## Dependencies
* **[DBFS][l_dbfs]** -- Library to deal with operation system files
* **[BPlusTreeBase][l_bplustree]** -- Advanced extendable implementation of **B+Tree** data structure
* **[C-Logger][l_logger]** -- Library for logging data

## Documentation

### Definitions
* **forest** -- means "database" if comparing to the regular definitions.
* **tree** -- means structure containing **key**/**value** data. Something similar to "table" if comparing to the regular database definitions. Notice, all the **leafs** stored inside a **tree** are sorted by the **key** in _ascending_ order.
* **leaf**/**leaf pointer** -- means some structure instance containing single pair item of **key**/**value** data
* **node** -- means some structure containing either other **nodes** or **leafs**. **node** that contains other nodes called **internal node** and the one that contains **leafs** called **leaf node**
* **key** -- means data that stored as an "index" inside the **leaf**
* **value** -- means data contained in a **leaf** that linked to specific **key**
* **bloom** -- means initialise the **forest**
* **fold** -- means deinitialise the **forest**

___

### Configuration
There is a bunch of parameters could be configure depending on your memory type, RAM, memory or processor's speed.

#### void forest::config_root_factor(int root_factor)
**factor** corresponds to the minimal number of items available at each internal node. `root_factor` corresponds to the factor that belongs to the **main Tree** that contains information about other **threes** in the **forest**. Optimal value for this configurable value is between **50** and **2000**.  Default value is **100**

#### void forest::config_default_factor(int default_factor)
**factor** that will be assigned by default for newly created **trees** if not override by input parameter. Default value is **100**

#### void forest::config_root_tree(string root_tree)
string represents the name of **main tree** root file. This parameter should consist to be able to reopen the same forest by different sessions. Default value is **_root**

#### void forest::config_intr_cache_length(int length)
corresponds to the number of **internal nodes** that would be cached to provide as fast as possible access to the **node**'s data. Default value is **20**

#### void forest::config_leaf_cache_length(int length)
corresponds to the number of **leaf nodes** that would be cached to provide as fast as possible access to the **node**'s data. (Notice, this doesn't means that **leafs** data will be fully cached in memory. The things that is cached in memory would be **leaf node**'s keys, **values**'s internal scheme and the **values** which size is not exceed the configurable limit) Default value is **50**

#### void forest::config_tree_cache_length(int length)
corresponds to the number of **trees** that would be cached to provide as fast as possible access to the **tree**'s data. Default value is **10**

#### void forest::config_cache_bytes(int bytes)
represents the limit for the **leaf**'s value that would be cached in memory in case the **value** size does not exceed the **bytes** limit. Default value is **128** 

#### void forest::config_chunk_bytes(int bytes)
represents the number number of bytes the **forest** will use to read/write data to **nodes**. Default value is **512**

#### void forest::config_opened_files_limit(int count)
represents the number of file that allowed to be opened by the **forest** at the same time. But be aware that the actual value could be **+LEAF_CACHE_LENGTH** as each cached **leaf node** holds opened file. _Notice: set up this value smartly and check your OS system file handler limit_. Default value is **50**

#### void forest::config_save_schedule_mks(int mks)
represents the timeout between contiguously saving **nodes** calls (if there are any unsaved nodes). Values provided in **micro seconds**. Default value is **10000** (10 mili seconds)

#### void forest::config_savior_queue_size(int length)
represents the length of internal queue of **nodes** that is going to be saved to the hard drive. Best use is when this value is greater or equal to the **LEAF_CACHE_LENGTH + INTR_CACHE_LENGTH + TREE_CACHE_LENGTH** value.

***Example:***
```c++
forest::config_root_factor(100);
forest::config_default_factor(100);
forest::config_intr_cache_length(30);
forest::config_leaf_cache_length(100);
forest::config_tree_cache_length(10);
forest::config_cache_bytes(256);
forest::config_chunk_bytes(512);
forest::config_opened_files_limit(100);
forest::config_savior_queue_size(200);
```

___

### Types
* forest::**Tree** -- represents **tree** object that is used to modify or search for **leafs**
* forest::**Leaf** -- represents **leaf** object containing **key**/**value** data as well as methods to move back and forward. You can find detailed docs below.
* forest::**DetachedLeaf** -- represents object type used to _insert_ or _update_ the leaf as well as read the **leaf** data.
* forest::**LeafReader** -- represents object used to read the **value** from **DetachedLeaf** object.
* forest::**LeafFile** -- represents source file of the **leaf** data.
* forest::**LeafKey** -- represents type of **leaf**'s **key**
* forest::**size_t** -- represents type for retrieving size of **tree**, **value**, etc.
* forest::**string** -- just an alias of _std::string_
* forest::**TREE_TYPES** -- _enum class_ defines tree types available to create the **tree**, containing just one value for now: **KEY_STRING**
* forest::**LEAF_POSITION** -- _enum class_ defines the way to search **leafs** in a **tree**. Available values are: **BEGIN**, **END**, **LOWER**, **UPPER**
* forest::**TreeException** -- class for exceptions related to **forest**
___

### Initialisation
Methods that allows you to _bloom_ or _fold_ the **forest**

#### void forest::bloom(string path)
Initialise **forest** at the provided **path**. Notice, in the provided path there will be created a lot of folders and files. All the **forest** data will be stored under the provided path. To reinitialise the **forest** by next sessions all you need is to provide the same path to **bloom** method.

#### void forest::fold()
Deinitialise the **forest** - saves all the data to hard drive. It is ***strongly*** recommended to use **fold** method to close the **forest** correctly and not to lose or corrupt internal structures.

***Example:***
```c++
forest::bloom("path/to/my/folder");
// ...
// At some condition
forest::fold();
```

___

### Status methods

#### bool forest::blooms()
Checks whenever **forest** is initialised or not. And returns _boolean_ where `true` means that the **forest** is initialised.

#### int forest::get_save_queue_size()
Returns the number of **nodes** that waits in the queue to be saved. Depending on this value you might want to adjust the **SAVE_SCHEDULE_MKS** value. You can do it without **folding** the **forest**. The value will be adjusted immediately after providing new value.

#### int forest::get_opened_files_count()
Returns number of currently opened files (not including the files opened by cached **leaf nodes**). Depends on this value you might want to adjust the **OPENED_FILES_LIMIT** value. You can do it without **folding** the **forest**. The value will be adjusted immediately after providing new value.

___

### Working with Trees
Here described methods to create, modify and remove **trees** from **forest**.

#### void forest::plant_tree(TREE_TYPES type, string name, int factor, string annotation)
Method to create new **tree** in the **forest**. It accepts **2** required parameters - **type** and **name**, and **2** optional - **factor** and **annotation**.

The only available value for **type** parameters is `TREE_TYPES::KEY_STRING` for now. **name** corresponds to the name of the **tree** you are about to create. This name will be used as a **key** in the **main tree**, and all **trees** in the **forest** will be ordered by **tree**'s name. If no **factor** value provided, the default factor will be used. _Notice: you can change default factor value using `config_default_factor(int)` config method_. **annotation** is just some information you can provide on your own. If no value provided, empty string will be used. 

This methods throws **TreeException** in case of 
* **forest** is not initialised.
*  **tree** with exactly the same name is already exists.

***Example:***
```c++
forest::plant_tree(forest::TREE_TYPES::KEY_STRING, "my_tree", 500);
```

#### void forest::cut_tree(string name)
Removes **tree** form forest and deletes all the **leafs** of deleted **tree**. It accepts 1 required parameter - **name** which is the name of the **tree** you want to delete. If no **tree** with provided name exists in the **forest**, no **tree** deleted and errors are thrown.

Throws **TreeException** in case of **forest** is not initialised.

***Example:***
```c++
forest::cut_tree("my_tree");
```

#### Tree forest::find_tree(string name)
Finds _reserves_ and returns **tree** you can use to _insert_, _update_, _remove_ or _search_ its leafs **leafs**. It accepts one required _string_ parameter **name** - the name of the **tree** you are looking for. After destructing the **tree** instance _(which is smart pointer wrapper around another structure)_ **tree** will be _released_ and could be removed from cache.

Throws **TreeException** in case of there is no **tree** with the provided **name** in the **forest**.

***Example:***
```c++
forest::Tree t = forest::find_tree("my_tree");
```

___

### Creating Leafs

There is couple of methods to create a **leaf** for inserting into the **tree**. **Leaf** that is going to be inserted called **Detached** and has the type **DetachedLeaf**.
Its object contains only **value** without **key**. This kind of **leaf** you can then _"attach"_ to the tree using some **key**.   Lets move through the methods for creating **detached leafs**

#### DetachedLeaf forest::make_leaf(string data)
Creates and returns **DetachedLeaf** object with **value** equals to the data you sent. In such case this **value** will be cached until it is inserted and saved into some **tree**.

#### DetachedLeaf forest::make_leaf(char* buffer, size_t length)
Creates and returns **DetachedLeaf** object with **value** equals to the data it takes from the char buffer with size of **length**. The same as in previous case, the **value** will be cached until it is inserted and saved to some **tree**.

#### DetachedLeaf forest::make_leaf(LeafFile file, size_t start, size_t length)
Creates and returns **DetachedLeaf** object. The **value** is not cached and used as pointer inside the file you send. **LeafFile** is just a alias of `std::shared_ptr<DBFS::File>` _(you can find documentation of how to use the **DBFS** [here][l_dbfs])_ You must **NOT** close or delete file until the **leaf** is saved to the tree, otherwise behaviour is **undefined**. If you don't want to care about deleting files, you can use `forest::create_leaf_file()` method for creating files. This file will be automatically closed and removed when it is not needed anymore. 

***Example:***
```c++
forest::DetachedLeaf lf = forest::make_leaf("My Fancy Leaf Value");
forest::insert_leaf("my_tree", "key0", lf);

forest::LeafFile f = forest::create_leaf_file();
f->write("value1value2value3");

forest::insert_leaf("my_tree", "key1", forest::make_leaf(f, 0, 6));
forest::insert_leaf("my_tree", "key2", forest::make_leaf(f, 6, 6));
forest::insert_leaf("my_tree", "key3", forest::make_leaf(f, 12, 6));

// After saving temporary file "f" will be automaticallt deleted.
``` 

#### LeafFile forest::create_leaf_file()
Returns **LeafFile** - `std::shared_ptr<DBFS::File>`. But unlike the simple regular creation, this file will be automatically removed after it is closed which makes it pretty useful for creating **detached leafs** and to not care about deleting temporary files.

___

### Leafs Operations
This section describes all the methods for creating, updating and removing the **leafs**.

#### void forest::insert_leaf(string tree_name, LeafKey key, DetachedLeaf val)
Attaches **val** to a tree that match **tree_name** by **key**. If **leaf** with the same **key** already exists in the **tree**, nothing happens.

Throws a **TreeException** in case of: 
* **forest** is not initialised
* There is no **tree** found with provided **tree_name**

#### void forest::insert_leaf(Tree tree, LeafKey key, DetachedLeaf val)
Attaches **val** to the **tree** by **key**. You can get the tree by executing `forest::find_tree(string tree_name)` method. If **leaf** with the same **key** already exists in the **tree**, nothing happens.

Throws a **TreeException** in case of: **forest** is not initialised

***Example:***
```C++
Tree t = forest::find_tree("my_tree");
forest::insert_leaf(t, "my_key", forest::make_leaf("my_val"));
```

#### void forest::update_leaf(string tree_name, LeafKey key, DetachedLeaf val)
Updates **leaf value** for the **leaf** with provided **key** in the **tree** that match **tree_name**. If no **leaf** with provided **key** exists, it works exactly like **insert_leaf**. 

Throws a **TreeException** in case of: 
* **forest** is not initialised
* There is no **tree** found with provided **tree_name**

#### void forest::update_leaf(Tree tree, LeafKey key, DetachedLeaf val)
Updates or attaches **val** to the **tree** by **key**. You can get the tree by executing `forest::find_tree(string tree_name)` method. If **leaf** with the same **key** already exists in the **tree** it will be rewritten. 

Throws a **TreeException** in case of **forest** is not initialised

#### void forest::remove_leaf(string tree_name, LeafKey key)
Removes **leaf** by provided **key** form the **tree** with name **tree_name**. If no tree exists with provided key, nothing happens.

Throws a **TreeException** in case of: 
* **forest** is not initialised
* There is no **tree** found with provided **tree_name**

#### void forest::remove_leaf(Tree tree, LeafKey key)
Removes **leaf** by provided **key** form the **tree**. You can get the tree by executing `forest::find_tree(string tree_name)` method. If no tree exists with provided key, nothing happens. 

Throws a **TreeException** in case of **forest** is not initialised

### Leafs Searching

This section describe all methods for searching the **leafs**.

#### Leaf forest::find_leaf(string tree_name, LeafKey key)
Searches for the leaf where key equals to **key** in the tree that match **tree_name** and returns **leaf pointer** to it or **end leaf** if there is no **leaf** with provided **key**. 

Throws a **TreeException** in case of:
* **forest** is not initialised
* **tree** with provided name is not found

#### Leaf forest::find_leaf(Tree tree, LeafKey key)
Searches for the leaf where key equals to **key** in the provided **tree** and returns **leaf pointer** to it or **end leaf** if there is no **leaf** with provided **key**. You can get the tree by executing `forest::find_tree(string tree_name)` method.

Throws a **TreeException** in case of **forest** is not initialised.

#### Leaf forest::find_leaf(string tree_name, LEAF_POSITION position)
As an **position** accepts values as **BEGIN** or **END** and in case of **BEGIN** was passed, returns **leaf** pointing to the first element in the tree that match **tree_name**, or **end leaf** if there is no **leafs** in the tree. In case of **END** was passed as **position** parameter, returns **end leaf** pointer.

Throws a **TreeException** in case of:
* **forest** is not initialised
* **tree** with provided name is not found
* wrong **position** parameter provided

#### Leaf forest::find_leaf(Tree tree, LEAF_POSITION position)
As an **position** accepts values as **BEGIN** or **END** and in case of **BEGIN** was passed, returns **leaf** pointing to the first element in the **tree**, or **end leaf** if there is no **leafs** in the tree. In case of **END** was passed as **position** parameter, returns **end leaf** pointer.

Throws a **TreeException** in case of:
* **forest** is not initialised
* wrong **position** parameter provided

#### Leaf forest::find_leaf(string tree_name, LeafKey key, LEAF_POSITION position)
As a position accepts **LOWER** or **UPPER** values and in case of **LOWER** was passed, returns **leaf** to the first element in the tree _(that match **tree_name**)_ whose key is not considered to go before **key**, and in case of **UPPER** value was provided, returns the first element in the tree whose key is considered to go after **key**.

Throws a **TreeException** in case of:
* **forest** is not initialised
* **tree** with provided name is not found
* wrong **position** parameter provided

#### Leaf forest::find_leaf(Tree tree, LeafKey key, LEAF_POSITION position)
As a position accepts **LOWER** or **UPPER** values and in case of **LOWER** was passed, returns **leaf** to the first element in the **tree** whose key is not considered to go before **key**, and in case of **UPPER** value was provided, returns the first element in the **tree** whose key is considered to go after **key**.

Throws a **TreeException** in case of:
* **forest** is not initialised
* wrong **position** parameter provided

***Example:***
```c++
// Assume tree `my_tree` contains leafs with keys: `aa`, `ab`, `ac`, `ad`, `aaa`, `bbb`, `bbc`

forest::find_leaf("my_tree", "ac"); // points to the leaf with key `ac`
forest::find_leaf("my_tree", "a"); // points to `end leaf` as there is no leaf with key `a`
forest::find_leaf("my_tree", forest::LEAF_POSITION::BEGIN); // points to the leaf with key `aa`
forest::find_leaf("my_tree", "b", forest::LEAF_POSITION::LOWER); // points to the leaf with key `bbb`
```

## Other Classes/Methods

### forest::Tree
Allows you to get some basic information about the **tree**

***Note:*** _Object_ represents smart pointer, so you have to call all the methods using dereferencing call operator _("->")_, or dereferencing operator _("(*).")_.

#### TREE_TYPES get_type()
Returns type of the **tree**

#### string get_annotation()
Returns annotation of the **tree**

***Example:***
```c++
forest::Tree t = forest::find_tree("my_tree");
std::cout << t->get_annotation() << std::endl; // prints annotation of the tree
```

___

### forest::Leaf
Leaf objects contains couple of methods for retrieving data from the **leaf** as well as moving through the tree **leafs**. 

***Note:*** _Object_ represents smart pointer, so you have to call all the methods using dereferencing call operator _("->")_, or dereferencing operator _("(*).")_.

#### bool eof()
Returns true if this **leaf** points to the end of the **tree** _(Its not the last leaf in the tree, but the pointer that corresponds to "null" leaf. You can extract any data from this leaf)_

#### bool move_forward()
Moves current **leaf pointer** to the next **leaf** in the **tree**. _Notice, it mutate current **leaf** object_. 

Returns `false` if reached "end" of the tree _(becomes null leaf)_, otherwise returns `true`.

#### bool move_back()
Moves current **leaf pointer** to the previous **leaf** in the **tree**. _Notice, it mutate current **leaf** object_.

Returns `false` if reached "end" of the tree _(becomes null leaf)_, otherwise returns `true`.

#### LeafKey key()
Returns key of the current **leaf**. 

Throws a **TreeException** in case of accessing **end leaf**

#### DetachedLeaf val()
returns **detached leaf** containing value of the current **leaf**

Throws a **TreeException** in case of accessing **end leaf**

***Example:***
```c++
forest::Leaf lf = forest::find_leaf("my_tree", "my_key");
for(int i=0;i<10;i++){
	lf->key()
}
```

___

### forest::DetachedLeaf
Contains methods to retrieve the data assigned to this **leaf value**.

***Note:*** _Object_ represents smart pointer, so you have to call all the methods using dereferencing call operator _("->")_, or dereferencing operator _("(*).")_.

#### size_t size()
Returns the size of **leaf value** in bytes.

#### LeafReader get_reader()
Returns **leaf reader** that allows you to read **value**.

___

### forest::LeafReader
The only thing it allows to do is to read the **value**.

#### size_t read(char* buffer, size_t count)
Reads **counts** bytes form **leaf** data from to **buffer** and returns the number of bytes it put. After reaching the end of the data, this method will write nothing to the buffer, and returns **0**

***Reading the leaf example:***
```c++
// Find the leaf
forest::Leaf leaf = forest::find_leaf("my_tree", "some_key");

// Get the reader
forest::LeafReader reader = leaf->val()->get_reader();

// Prepare buffer
int sz = 512;
int cnt;
char* buf = new char[sz];

// Read data
while((cnt = reader.read(buf, sz))){
	// do something with received data
}

// Release memory
delete[] buf;
```

## Tests and Scripts

There is a bunch of tests located under the _"/tests/src"_ directory. All tests divided into couple of files each of which tests specific aspects of functionality:
* src/**single_thread.forest.test.cpp** -- simple tests for testing all basic functionality
* src/**multi_thread.forest.test.cpp** -- tests for testing work in multiple threads, including testing for race conditions and dead locks
* src/**performance.forest.test.cpp** -- tests for testing performance

Feel free to add your tests there or create a new files.

There is a couple of "main" test files that runs the files above, or provide its own tests:

* **test.cpp** -- runs single and multiple thread tests
* **rc_test.cpp** -- runs its own tests for finding race conditions
* **perf_test.cpp** -- runs performance tests
* **mtest.cpp** -- is just a sandbox file to test whatever you want

There is also **qtest.hpp** framework included to this folder that provides all testing logic, you can find documentation for this framework [here](https://github.com/immortale-dev/QTest).

There is also couple of scripts available under the _"/scripts/"_ folder to help you run the tests:

### test.[sh|ps1]
Builds all source files and _test.cpp_ in debug mode with `make all` command, and runs created binary.

### testrc.[sh|ps1]
Accepts one **integer** parameter as the number of times test should be run to find race conditions. Builds all the source files and _rc_test.cpp_, runs the binary some number of times, if at least one run fails, it prints **ERROR** to the console and return code **1**

### testperf.[sh|ps1]
Build all source files and _perf_test.cpp_ with **-O3** flag, and runs the binary. 
**Note:** _you have to clear the cache (using **clear** script)_ if there is an object files built in debug mode.

### clear.[sh|ps1]
Deletes all objects and binaries.

## Additional Information

### Performance
There was made couple of performance tests on a different machines, and here is the result of the testing:

#### Machine
* Processor: **Intel Xeon E5-2470 (2.30GHz)**
* Cores Count: **8**
* RAM: **16GB**
* File System: **HDD**

#### Configuration

* forest::**config_intr_cache_length(30);**
* forest::**config_leaf_cache_length(100);**
* forest::**config_tree_cache_length(10);**
* forest::**config_cache_bytes(256);**
* forest::**config_chunk_bytes(512);**
* forest::**config_opened_files_limit(100);**
* forest::**config_savior_queue_size(200);**
* forest::**config_save_schedule_mks(20000);** _(20ms)_

#### Tests

**Note:** _All tests was performed for the trees with factor = **500**_

* Single Thread Small Data Pool
	* Inserting **100 000** elements to **1** tree by **1** thread
		* _962ms_
	* Accessing **100 000** elements in **1** thread.
		* _362ms_
* Single Thread Big Data Pool
	* Inserting **1 000 000** elements to **1** tree by **1** thread
		* _11296ms_
	* Accessing **1 000 000** elements in **1** thread.
		* _4470ms_
* Multithread work with single tree
	* Inserting **1 000 000** elements to one three in **8** threads
		* _18522ms_ - **this case shows the overhead in processor/file system task switch processes**
	* Accessing **1 000 000** elements in **8** threads (not overlapping)
		* _5132ms_
	* Accessing **1 000 000** elements in **8** threads with overlapping all the values
		* _20559ms_ - **4M records moved in total, so ~5150ms per 1M items**
* Multithread work with multiple trees
	* Inserting **1 000 000** elements in **4** threads to **4** trees (each tree gets **250 000** items. Each thread writes to its own tree)
		* _13616ms_ - **Shows file system processes switches overhead**
	* Accessing **1 000 000** elements in **4** threads (each thread access its own tree)
		* _4711ms_

#### Summary
For single thread operations there is no any task switching/threading lock overheads, and with pretty much standard configuration you could get **~100 000 item/second** for inserting, and **~250 000 items/second** for reading items.

Using engine in multiple threads you could find some overhead when dealing with data (because of *processor task switch/thread lock/file operation switch*), and the numbers shows a bit less performance for **inserting** operations, but **reading** the data shows pretty much the same time.

Potentially those numbers could be a bit better by setting better value for **SAVE_SCHEDULE_MKS** or even setting it dynamically during the operations.

### DOTOs/Ideas
* Ability to write when moving through the **leafs**
* ZIP data before writing
* Create custom data structures for **hashtable**, **vector**, **pair** etc.
* Custom Memory allocators
* Request analisys (?)
* Better filesystem (?)

### From author
Hello fellow reader! :) 

I hope you enjoy using this product as much as I was when writing it!
If you find some bugs/issues or if you know the way to make this product better, or if you just want to ask some questions, just let me know in the comments, or create an issue here on github, thank you in advance!

Best wishes and have fun!

-- _Immortale_

## License
MIT

Have fun :)


[l_dbfs]: https://github.com/immortale-dev/dbfs
[l_logger]: https://github.com/immortale-dev/C-Logger
[l_bplustree]: https://github.com/immortale-dev/BPlusTreeBase
