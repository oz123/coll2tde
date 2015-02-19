
# GNU make
export PATH:=/srv/mongodb/bin:$(PATH)

TEST_DB=testtde


.PHONY: all import test

import:
	mongoimport -d $(TEST_DB) -c test --file records.json --jsonArray

drop_test_data:
	mongo $(TEST_DB)  --eval "db.dropDatabase()"

test: drop_test_data import
	$(RM) test.tde
	python2 coll2tde.py -s mongodb://localhost -c test -d $(TEST_DB)
	python2 coll2tde.py -s mongodb://localhost -c test -d $(TEST_DB)
