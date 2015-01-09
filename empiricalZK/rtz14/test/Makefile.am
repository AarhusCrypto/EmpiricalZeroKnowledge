include $(top_srcdir)/rtz14.am
testdir=test
test_PROGRAMS = test_circuit_parser test_fun_tokenizer test_poc_visitor test_inputgatevisitor test_emittervisitor

test_emittervisitor_SOURCES = test_emitter.c
test_emittervisitor_LDADD = ../src/librtz14.a

test_inputgatevisitor_SOURCES = test_inputgatevisitor.c
test_inputgatevisitor_LDADD = ../src/librtz14.a

test_poc_visitor_SOURCES = test_pocvisitor.c
test_poc_visitor_LDADD = ../src/librtz14.a

test_fun_tokenizer_SOURCES = test_funtokenizer.c
test_fun_tokenizer_LDADD = ../src/librtz14.a

test_circuit_parser_SOURCES = test_circuitparser.c
test_circuit_parser_LDADD = ../src/librtz14.a