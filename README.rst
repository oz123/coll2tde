coll2tde
========

A command line utility to convert a mongodb collection to a TDE file.

* WTF man?

  I have the pleasure of working with a Bloatware called Tableau. It provides
  horrible interface for doing things with mouse and data. 
  This software has propriety format which is not documented, and my day job
  requires creating those file from data found in MongoDB collections.

  In my day job we are using the Tableau Python API for this purpose. It works, 
  but in a very slow pace. 

  This is an attempt to re-create those peski TDE files using C. 


## USAGE:

This CLI tools requires the Tableau API. 

This development version offers an export to CSV. To use it you need to install
libcsv, to compile it you need libcsv-dev (on debian based systems.)

libcsv - http://sourceforge.net/projects/libcsv/