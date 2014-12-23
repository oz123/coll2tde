coll2tde
========

A command line utility to convert a mongodb collection to Tableau DataExtract file.

Installation
------------
Dependencies
^^^^^^^^^^^^^
The dependencies are:

 1. TABLEAU C API
 2. MongoDB C API https://api.mongodb.org/
 3. MongoDB Libbson https://api.mongodb.org/Libbson
 4. lib JSMN http://zserge.com/jsmn.html
     
Just type::

   $ make 

And then install the binary coll2tde in your path:: 
 
   $ make install 

You can optionally specify the PREFIX:

    $ make PREFIX=/path/to/install install

USAGE:
------

The most basic use is to convert a collection as a whole to a DataExtract file. 
This however assumes that the collection is flat, and all documents are simple
key values. That means that no embeded documents are allowed, since DataExtract
are simple tables. 

If the documents are not key values only, you need to convert your collection 
to a simple one with aggregation. Future versions might support exporting this 
aggregation result directly to DataExtract. 

A most basic usage of this program whould be::

    $ ./coll2tde -h localhost -d test -c test

The option ``-h`` accepts any mongodb URI, hence, this should also work::

    $ ./coll2tde -h mongodb://user:secret@server:port ....

The option ``-d`` accepts the database name, the option ``-c`` accepts the collection 
name. 

The DataExtract extract file name will always be the collection name with the
suffix ``.tde``. You can override this with the flag ``-f``.

Some notes about the exported data
----------------------------------

 * Date formating in C with MongoDB is not fun. 
   It depends on the locale of the host where `coll2tde` runs and the time
   format in MongoDB.
   The MongoDB C API exports all dates as seconds since the epoch. To simplify 
   things at this early stage of development, ALL dates are converted to GM
   Time. 

 * MongoDB internal OID is allways dropped. Hence, it will not be exported to 
   the DataExtract. 

 * You can specify which fields you want to include::

   ./coll2tde -h localhost -d test -c test --fields '{"name":1, "last_name:1}'

 * You can specify a query to limit the data exported, for example::

    ./coll2tde -h localhost -d test -c test --query '{"credit": {"$gt" : 10}}'

   The flags ``-q`` and ``--fields`` support all the syntax that MongoDB's ``find`` 
   method support. 

 * One can also user the aggregation framework to manipulate the data before being 
   inserted to the DataExtract file::

    coll2tde -h localhost -d test -c test --aggregation '"{$project : ....}"'

 * Note that the options ``-a`` and ``-q`` are mutually exclusive. 

 * Column types of DataExtract are static, the software tries to detect the types
   of values stored in the database and make proper decision about which column
   type to create. However if you have a record which has ``null`` it is
   impossible to detect the type of the column with out skipping to a different 
   record. Hence, try to refine your query such the first record give by the 
   cursor does not contain nulls. If the record contains ``null``, e.g.::

    { "name" : "matt", "last_name" : "simon", 
    "registered" : { "$date" : 1389389420201 }, 
    "credit" : 0, "posts" : null, "active" : true }

   The software will exist with the following error::
   
    Found null in key [posts], can't understand which type to create... 

   This behaviour might change in the future. 
 
 * When using aggregation your json should always explicitly contain a
   pipeline and an arry of operations::

    coll2tde ... -a  '{ "pipeline" : [ { "$project" : { "name" : 1 } }, { "$group" : 
                       { "_id" : "$name", "credit" : { "$sum" : 1 } } }
                      ]}'

