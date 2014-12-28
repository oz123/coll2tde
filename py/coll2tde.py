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

import json
import sys
import datetime
import argparse
import pymongo
import dataextract as tde

u = ("Usage: coll2tde -s SERVER -d DATABASE -c COLLECTION [-q QUERY][--fields "
     "FIELDS]"
     "[-a AGGREGATION] -f TDEFILE")


def parser():
    parser = argparse.ArgumentParser(usage=u)

    parser.add_argument("-s", "--server", help="server name", required=True)
    parser.add_argument("-d", "--database", help="database",
                        required=True)
    parser.add_argument("-c", "--collection", help="collection name",
                        required=True)
    parser.add_argument("-q", "--query", help="query", required=False)
    parser.add_argument("-a", "--aggregation", help="aggregation",
                        required=False)
    parser.add_argument("-f", "--fields", help="field names", required=False)

    return parser


class CollectionToTDE(object):

    def __init__(self, args):
        self.host = args.server
        self.db = args.database
        self.collection = args.collection
        self.args = args

    def open_tableau_file(self, fname):
        pass

    def get_cursor(self, host, database, collection, query, fields,
                   aggregation):

        mongurl = host
        try:
            client = pymongo.MongoClient(mongurl)
            print client
        except:
            print "Could not parse mongo url"
            sys.exit(2)

        db = client[database]
        collection = db[collection]

        if not aggregation:

            if not query:
                query = {}
            else:
                try:
                    query = json.loads(query)
                except ValueError:
                    print "could not understand your query ..."
                    sys.exit()

            if not fields:
                fields = {}
            else:
                try:
                    fields = json.loads(fields)
                except ValueError:
                    print "could not understand your fields ..."
                    sys.exit()

            cursor = collection.find(query, fields)

        else:

            cursor = collection.aggregate(aggregation)

        return cursor

    def make_table_definition(record, column_types, column_names, fileHandle):

        for k, v in record.iteritems():
            column_names.append(k)
            if isinstance(v, int):
                column_types.append(tde.Type.INTEGER)
            elif isinstance(v, float):
                column_types.append(tde.Type.DOUBLE)
            elif isinstance(v, unicode):
                column_types.append(tde.Type.UNICODE_STRING)
            elif isinstance(v, bool):
                column_types.append(tde.Type.BOOLEAN)
            elif isinstance(v, datetime.datetime):
                column_types.append(tde.Type.DATETIME)
            else:
                print "Found unknown type! values is of %s" % v.__class__
                sys.exit()

    def get_or_create_table(self, filename, record, column_types):

        fileHandle = tde.Extract(filename)
        column_names = []
        column_types = []

        if not fileHandle.hasTable('Extract'):
            # Table does not exist; create it
            tableDef = self.make_table_definition(record, column_types,
                                                  column_names, fileHandle)
            table = fileHandle.addTable('Extract', tableDef)
        else:
            # Open an existing table to add more rows
            table = fileHandle.openTable('Extract')

        tableDef = table.getTableDefinition()

        return table, tableDef, fileHandle, column_types

    def insert_rows(self, table_def, filename, cursor):
        pass

    def run(self, args):
        self.get_cursor(self.host, self.db, self.collection, args.query,
                        args.fields, args.aggregation)


if __name__ == '__main__':

    args = parser().parse_args()

    print args.aggregation
    if args.aggregation and (args.query or args.fields):
        print "Aggregation and Qurey are mutually exclusive ..."
        sys.exit(2)

    coll2tde = CollectionToTDE(args)
    coll2tde.run(args)
