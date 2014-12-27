#!/usr/bin/env python2

#  This file is part of coll2tde.
#
#  coll2tde is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License, version 3
#  as published by the Free Software Foundation;
#
#  coll2tde is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with coll2tde; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#
#  ============================================================================
#  Copyright (C) 2014 Oz Nahum Tiram <nahumoz@gmail.com>
#  ============================================================================

"""
coll2tde.py is a re-implementation of coll2tde written in C. The purpose of
this program is to help benchmarking the C program, and provide a test bed for
quickly prototyping new features.
"""

import dataextract as tde
import json
import pymongo
import argparse


u = ("Usage: coll2tde -h HOST -d DATABASE -c COLLECTION [-q QUERY][--fields "
     "FIELDS]"
     "[-a AGGREGATION] -f TDEFILE")


def parser():
    parser = argparse.ArgumentParser()
    parser.add_argument("-h", "--host", help="server name", required=True)
    parser.add_argument("-d", "--database", help="database",
                        required=True)
    parser.add_argument("-c", "--collection", help="collection name",
                        required=True)
    parser.add_argument("-q", "--query", help="query", required=False)
    parser.add_argument("-f", "--field", help="field names", required=False)
    parser.add_argument("-a", "--aggregation", help="aggregation",
                        required=False)


class CollectionToTDE(object):

    def __init__(self, args):
        pass

    def open_tableau_file(self, fname):
        pass

    def get_cursor(self, host, database, collection, query, fields,
                   aggregation):
        pass

    def make_table_definition(record, column_types):
        pass

    def get_or_create_table(self, filename, record, column_types):

        extract = tde.Extract(filename)
        if not extract.hasTable('Extract'):
            # Table does not exist; create it
            tableDef = self.make_table_definition(record, column_types)
            table = extract.addTable('Extract', tableDef)
        else:
            # Open an existing table to add more rows
            table = extract.openTable('Extract')

        tableDef = table.getTableDefinition()

        return table, tableDef, extract, column_types

    def insert_rows(self, table_def, filename, cursor):
        pass


if '__name__' == '__main__':

    args = parser.parse_args()
    coll2tde = CollectionToTDE()
    coll2tde.run()
