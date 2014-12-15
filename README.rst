coll2tde
========

A command line utility to convert a mongodb collection to a TDE file.

* WTF man?

  I have the pleasure of working with a Bloatware called Tableau. It provides
  horrible interface for doing things with mouse and data. 
  This software has propriety format which is not documented, and my day job
  requires creating those files from data found in MongoDB collections.

  In my day job we are using the Tableau Python API for this purpose. It works, 
  but in a very slow pace. 

  This is an attempt to re-create those pesky TDE files using C. 


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

And then install the binary coll2tde in your path. 

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

The option `-h` accepts any mongodb URI, hence, this should also work::

    $ ./coll2tde -h mongodb://user:secret@server:port ....

The option `-d` accepts the database name, the option `-c` accepts the collection 
name. 

The DataExtract extract file name will always be the collection name with the
suffix `.tde`.

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



